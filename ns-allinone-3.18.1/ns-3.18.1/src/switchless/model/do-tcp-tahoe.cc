/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#define NS_LOG_APPEND_CONTEXT \
  if (m_node) { std::clog << Simulator::Now ().GetSeconds () << " [node " << m_node->GetId () << "] "; }

#include "do-tcp-tahoe.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/node.h"

NS_LOG_COMPONENT_DEFINE ("DoTcpTahoe");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoTcpTahoe);

TypeId
DoTcpTahoe::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoTcpTahoe")
    .SetParent<DoTcpSocketBase> ()
    .AddConstructor<DoTcpTahoe> ()
    .AddAttribute ("ReTxThreshold", "Threshold for fast retransmit",
                    UintegerValue (3),
                    MakeUintegerAccessor (&DoTcpTahoe::m_retxThresh),
                    MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("CongestionWindow",
                     "The TCP connection's congestion window",
                     MakeTraceSourceAccessor (&DoTcpTahoe::m_cWnd))
  ;
  return tid;
}

DoTcpTahoe::DoTcpTahoe (void) : m_initialCWnd (1), m_retxThresh (3)
{
  NS_LOG_FUNCTION (this);
}

DoTcpTahoe::DoTcpTahoe (const DoTcpTahoe& sock)
  : DoTcpSocketBase (sock),
    m_cWnd (sock.m_cWnd),
    m_ssThresh (sock.m_ssThresh),
    m_initialCWnd (sock.m_initialCWnd),
    m_retxThresh (sock.m_retxThresh)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
}

DoTcpTahoe::~DoTcpTahoe (void)
{
}

/** We initialize m_cWnd from this function, after attributes initialized */
int
DoTcpTahoe::Listen (void)
{
  NS_LOG_FUNCTION (this);
  InitializeCwnd ();
  return DoTcpSocketBase::Listen ();
}

/** We initialize m_cWnd from this function, after attributes initialized */
int
DoTcpTahoe::Connect (const Address & address)
{
  NS_LOG_FUNCTION (this << address);
  InitializeCwnd ();
  return DoTcpSocketBase::Connect (address);
}

/** Limit the size of in-flight data by cwnd and receiver's rxwin */
uint32_t
DoTcpTahoe::Window (void)
{
  NS_LOG_FUNCTION (this);
  return std::min (m_rWnd.Get (), m_cWnd.Get ());
}

Ptr<DoTcpSocketBase>
DoTcpTahoe::Fork (void)
{
  return CopyObject<DoTcpTahoe> (this);
}

/** New ACK (up to seqnum seq) received. Increase cwnd and call DoTcpSocketBase::NewAck() */
void
DoTcpTahoe::NewAck (SequenceNumber32 const& seq)
{
  NS_LOG_FUNCTION (this << seq);
  NS_LOG_LOGIC ("DoTcpTahoe receieved ACK for seq " << seq <<
                " cwnd " << m_cWnd <<
                " ssthresh " << m_ssThresh);
  if (m_cWnd < m_ssThresh)
    { // Slow start mode, add one segSize to cWnd. Default m_ssThresh is 65535. (RFC2001, sec.1)
      m_cWnd += m_segmentSize;
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << m_cWnd << " ssthresh " << m_ssThresh);
    }
  else
    { // Congestion avoidance mode, increase by (segSize*segSize)/cwnd. (RFC2581, sec.3.1)
      // To increase cwnd for one segSize per RTT, it should be (ackBytes*segSize)/cwnd
      double adder = static_cast<double> (m_segmentSize * m_segmentSize) / m_cWnd.Get ();
      adder = std::max (1.0, adder);
      m_cWnd += static_cast<uint32_t> (adder);
      NS_LOG_INFO ("In CongAvoid, updated to cwnd " << m_cWnd << " ssthresh " << m_ssThresh);
    }
  DoTcpSocketBase::NewAck (seq);           // Complete newAck processing
}

/** Cut down ssthresh upon triple dupack */
void
DoTcpTahoe::DupAck (const DoTcpHeader& t, uint32_t count)
{
  NS_LOG_FUNCTION (this << "t " << count);
  if (count == m_retxThresh)
    { // triple duplicate ack triggers fast retransmit (RFC2001, sec.3)
      NS_LOG_INFO ("Triple Dup Ack: old ssthresh " << m_ssThresh << " cwnd " << m_cWnd);
      // fast retransmit in Tahoe means triggering RTO earlier. Tx is restarted
      // from the highest ack and run slow start again.
      // (Fall & Floyd 1996, sec.1)
      m_ssThresh = std::max (static_cast<unsigned> (m_cWnd / 2), m_segmentSize * 2);  // Half ssthresh
      m_cWnd = m_segmentSize; // Run slow start again
      m_nextTxSequence = m_txBuffer.HeadSequence (); // Restart from highest Ack
      NS_LOG_INFO ("Triple Dup Ack: new ssthresh " << m_ssThresh << " cwnd " << m_cWnd);
      NS_LOG_LOGIC ("Triple Dup Ack: retransmit missing segment at " << Simulator::Now ().GetSeconds ());
      DoRetransmit ();
    }
}

/** Retransmit timeout */
void DoTcpTahoe::Retransmit (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ReTxTimeout Expired at time " << Simulator::Now ().GetSeconds ());
  // If erroneous timeout in closed/timed-wait state, just return
  if (m_state == CLOSED || m_state == TIME_WAIT) return;
  // If all data are received (non-closing socket and nothing to send), just return
  if (m_state <= ESTABLISHED && m_txBuffer.HeadSequence () >= m_highTxMark) return;

  m_ssThresh = std::max (static_cast<unsigned> (m_cWnd / 2), m_segmentSize * 2);  // Half ssthresh
  m_cWnd = m_segmentSize;                   // Set cwnd to 1 segSize (RFC2001, sec.2)
  m_nextTxSequence = m_txBuffer.HeadSequence (); // Restart from highest Ack
  m_rtt->IncreaseMultiplier ();             // Double the next RTO
  DoRetransmit ();                          // Retransmit the packet
}

void
DoTcpTahoe::SetSegSize (uint32_t size)
{
  NS_ABORT_MSG_UNLESS (m_state == CLOSED, "DoTcpTahoe::SetSegSize() cannot change segment size after connection started.");
  m_segmentSize = size;
}

void
DoTcpTahoe::SetSSThresh (uint32_t threshold)
{
  m_ssThresh = threshold;
}

uint32_t
DoTcpTahoe::GetSSThresh (void) const
{
  return m_ssThresh;
}

void
DoTcpTahoe::SetInitialCwnd (uint32_t cwnd)
{
  NS_ABORT_MSG_UNLESS (m_state == CLOSED, "DoTcpTahoe::SetInitialCwnd() cannot change initial cwnd after connection started.");
  m_initialCWnd = cwnd;
}

uint32_t
DoTcpTahoe::GetInitialCwnd (void) const
{
  return m_initialCWnd;
}

void 
DoTcpTahoe::InitializeCwnd (void)
{
  /*
   * Initialize congestion window, default to 1 MSS (RFC2001, sec.1) and must
   * not be larger than 2 MSS (RFC2581, sec.3.1). Both m_initiaCWnd and
   * m_segmentSize are set by the attribute system in ns3::DoTcpSocket.
   */
  m_cWnd = m_initialCWnd * m_segmentSize;
}

} // namespace ns3
