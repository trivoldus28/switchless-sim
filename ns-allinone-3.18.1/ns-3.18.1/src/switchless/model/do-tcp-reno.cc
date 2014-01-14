/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#define NS_LOG_APPEND_CONTEXT \
  if (m_node) { std::clog << Simulator::Now ().GetSeconds () << " [node " << m_node->GetId () << "] "; }

#include "do-tcp-reno.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/node.h"

NS_LOG_COMPONENT_DEFINE ("DoTcpReno");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoTcpReno);

TypeId
DoTcpReno::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoTcpReno")
    .SetParent<DoTcpSocketBase> ()
    .AddConstructor<DoTcpReno> ()
    .AddAttribute ("ReTxThreshold", "Threshold for fast retransmit",
                    UintegerValue (3),
                    MakeUintegerAccessor (&DoTcpReno::m_retxThresh),
                    MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("CongestionWindow",
                     "The TCP connection's congestion window",
                     MakeTraceSourceAccessor (&DoTcpReno::m_cWnd))
  ;
  return tid;
}

DoTcpReno::DoTcpReno (void) : m_retxThresh (3), m_inFastRec (false)
{
  NS_LOG_FUNCTION (this);
}

DoTcpReno::DoTcpReno (const DoTcpReno& sock)
  : DoTcpSocketBase (sock),
    m_cWnd (sock.m_cWnd),
    m_ssThresh (sock.m_ssThresh),
    m_initialCWnd (sock.m_initialCWnd),
    m_retxThresh (sock.m_retxThresh),
    m_inFastRec (false)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
}

DoTcpReno::~DoTcpReno (void)
{
}

/** We initialize m_cWnd from this function, after attributes initialized */
int
DoTcpReno::Listen (void)
{
  NS_LOG_FUNCTION (this);
  InitializeCwnd ();
  return DoTcpSocketBase::Listen ();
}

/** We initialize m_cWnd from this function, after attributes initialized */
int
DoTcpReno::Connect (const Address & address)
{
  NS_LOG_FUNCTION (this << address);
  InitializeCwnd ();
  return DoTcpSocketBase::Connect (address);
}

/** Limit the size of in-flight data by cwnd and receiver's rxwin */
uint32_t
DoTcpReno::Window (void)
{
  NS_LOG_FUNCTION (this);
  return std::min (m_rWnd.Get (), m_cWnd.Get ());
}

Ptr<DoTcpSocketBase>
DoTcpReno::Fork (void)
{
  return CopyObject<DoTcpReno> (this);
}

/** New ACK (up to seqnum seq) received. Increase cwnd and call DoTcpSocketBase::NewAck() */
void
DoTcpReno::NewAck (const SequenceNumber32& seq)
{
  NS_LOG_FUNCTION (this << seq);
  NS_LOG_LOGIC ("DoTcpReno receieved ACK for seq " << seq <<
                " cwnd " << m_cWnd <<
                " ssthresh " << m_ssThresh);

  // Check for exit condition of fast recovery
  if (m_inFastRec)
    { // RFC2001, sec.4; RFC2581, sec.3.2
      // First new ACK after fast recovery: reset cwnd
      m_cWnd = m_ssThresh;
      m_inFastRec = false;
      NS_LOG_INFO ("Reset cwnd to " << m_cWnd);
    };

  // Increase of cwnd based on current phase (slow start or congestion avoidance)
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

  // Complete newAck processing
  DoTcpSocketBase::NewAck (seq);
}

// Fast recovery and fast retransmit
void
DoTcpReno::DupAck (const DoTcpHeader& t, uint32_t count)
{
  NS_LOG_FUNCTION (this << "t " << count);
  if (count == m_retxThresh && !m_inFastRec)
    { // triple duplicate ack triggers fast retransmit (RFC2581, sec.3.2)
      m_ssThresh = std::max (2 * m_segmentSize, BytesInFlight () / 2);
      m_cWnd = m_ssThresh + 3 * m_segmentSize;
      m_inFastRec = true;
      NS_LOG_INFO ("Triple dupack. Reset cwnd to " << m_cWnd << ", ssthresh to " << m_ssThresh);
      DoRetransmit ();
    }
  else if (m_inFastRec)
    { // In fast recovery, inc cwnd for every additional dupack (RFC2581, sec.3.2)
      m_cWnd += m_segmentSize;
      NS_LOG_INFO ("Increased cwnd to " << m_cWnd);
      SendPendingData (m_connected);
    };
}

// Retransmit timeout
void DoTcpReno::Retransmit (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ReTxTimeout Expired at time " << Simulator::Now ().GetSeconds ());
  m_inFastRec = false;

  // If erroneous timeout in closed/timed-wait state, just return
  if (m_state == CLOSED || m_state == TIME_WAIT) return;
  // If all data are received (non-closing socket and nothing to send), just return
  if (m_state <= ESTABLISHED && m_txBuffer.HeadSequence () >= m_highTxMark) return;

  // According to RFC2581 sec.3.1, upon RTO, ssthresh is set to half of flight
  // size and cwnd is set to 1*MSS, then the lost packet is retransmitted and
  // TCP back to slow start
  m_ssThresh = std::max (2 * m_segmentSize, BytesInFlight () / 2);
  m_cWnd = m_segmentSize;
  m_nextTxSequence = m_txBuffer.HeadSequence (); // Restart from highest Ack
  NS_LOG_INFO ("RTO. Reset cwnd to " << m_cWnd <<
               ", ssthresh to " << m_ssThresh << ", restart from seqnum " << m_nextTxSequence);
  m_rtt->IncreaseMultiplier ();             // Double the next RTO
  DoRetransmit ();                          // Retransmit the packet
}

void
DoTcpReno::SetSegSize (uint32_t size)
{
  NS_ABORT_MSG_UNLESS (m_state == CLOSED, "DoTcpReno::SetSegSize() cannot change segment size after connection started.");
  m_segmentSize = size;
}

void
DoTcpReno::SetSSThresh (uint32_t threshold)
{
  m_ssThresh = threshold;
}

uint32_t
DoTcpReno::GetSSThresh (void) const
{
  return m_ssThresh;
}

void
DoTcpReno::SetInitialCwnd (uint32_t cwnd)
{
  NS_ABORT_MSG_UNLESS (m_state == CLOSED, "DoTcpReno::SetInitialCwnd() cannot change initial cwnd after connection started.");
  m_initialCWnd = cwnd;
}

uint32_t
DoTcpReno::GetInitialCwnd (void) const
{
  return m_initialCWnd;
}

void 
DoTcpReno::InitializeCwnd (void)
{
  /*
   * Initialize congestion window, default to 1 MSS (RFC2001, sec.1) and must
   * not be larger than 2 MSS (RFC2581, sec.3.1). Both m_initiaCWnd and
   * m_segmentSize are set by the attribute system in ns3::DoTcpSocket.
   */
  m_cWnd = m_initialCWnd * m_segmentSize;
}

} // namespace ns3
