/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#define NS_LOG_APPEND_CONTEXT \
  if (m_node) { std::clog << Simulator::Now ().GetSeconds () << " [node " << m_node->GetId () << "] "; }


#include "ns3/abort.h"
#include "ns3/node.h"
#include "ns3/dim-ordered-socket-address.h"
#include "ns3/log.h"
#include "ns3/dim-ordered.h"
#include "ns3/dim-ordered-interface-address.h"
#include "ns3/simulation-singleton.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/trace-source-accessor.h"
#include "do-tcp-socket-base.h"
#include "do-tcp-l4-protocol.h"
#include "dim-ordered-end-point.h"
#include "do-tcp-header.h"
#include "ns3/rtt-estimator.h"

#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("DoTcpSocketBase");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoTcpSocketBase);

TypeId
DoTcpSocketBase::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoTcpSocketBase")
    .SetParent<DoTcpSocket> ()
//    .AddAttribute ("TcpState", "State in TCP state machine",
//                   TypeId::ATTR_GET,
//                   EnumValue (CLOSED),
//                   MakeEnumAccessor (&DoTcpSocketBase::m_state),
//                   MakeEnumChecker (CLOSED, "Closed"))
    .AddAttribute ("MaxSegLifetime",
                   "Maximum segment lifetime in seconds, use for TIME_WAIT state transition to CLOSED state",
                   DoubleValue (120), /* RFC793 says MSL=2 minutes*/
                   MakeDoubleAccessor (&DoTcpSocketBase::m_msl),
                   MakeDoubleChecker<double> (0))
    .AddAttribute ("MaxWindowSize", "Max size of advertised window",
                   UintegerValue (65535),
                   MakeUintegerAccessor (&DoTcpSocketBase::m_maxWinSize),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("RTO",
                     "Retransmission timeout",
                     MakeTraceSourceAccessor (&DoTcpSocketBase::m_rto))
    .AddTraceSource ("RTT",
                     "Last RTT sample",
                     MakeTraceSourceAccessor (&DoTcpSocketBase::m_lastRtt))
    .AddTraceSource ("NextTxSequence",
                     "Next sequence number to send (SND.NXT)",
                     MakeTraceSourceAccessor (&DoTcpSocketBase::m_nextTxSequence))
    .AddTraceSource ("HighestSequence",
                     "Highest sequence number ever sent in socket's life time",
                     MakeTraceSourceAccessor (&DoTcpSocketBase::m_highTxMark))
    .AddTraceSource ("State",
                     "TCP state",
                     MakeTraceSourceAccessor (&DoTcpSocketBase::m_state))
    .AddTraceSource ("RWND",
                     "Remote side's flow control window",
                     MakeTraceSourceAccessor (&DoTcpSocketBase::m_rWnd))
  ;
  return tid;
}

DoTcpSocketBase::DoTcpSocketBase (void)
  : m_dupAckCount (0),
    m_delAckCount (0),
    m_endPoint (0),
    m_node (0),
    m_tcp (0),
    m_rtt (0),
    m_nextTxSequence (0),
    // Change this for non-zero initial sequence number
    m_highTxMark (0),
    m_rxBuffer (0),
    m_txBuffer (0),
    m_state (CLOSED),
    m_errno (ERROR_NOTERROR),
    m_closeNotified (false),
    m_closeOnEmpty (false),
    m_shutdownSend (false),
    m_shutdownRecv (false),
    m_connected (false),
    m_segmentSize (0),
    // For attribute initialization consistency (quiet valgrind)
    m_rWnd (0)
{
  NS_LOG_FUNCTION (this);
}

DoTcpSocketBase::DoTcpSocketBase (const DoTcpSocketBase& sock)
  : DoTcpSocket (sock),
    //copy object::m_tid and socket::callbacks
    m_dupAckCount (sock.m_dupAckCount),
    m_delAckCount (0),
    m_delAckMaxCount (sock.m_delAckMaxCount),
    m_noDelay (sock.m_noDelay),
    m_cnRetries (sock.m_cnRetries),
    m_delAckTimeout (sock.m_delAckTimeout),
    m_persistTimeout (sock.m_persistTimeout),
    m_cnTimeout (sock.m_cnTimeout),
    m_endPoint (0),
    m_node (sock.m_node),
    m_tcp (sock.m_tcp),
    m_rtt (0),
    m_nextTxSequence (sock.m_nextTxSequence),
    m_highTxMark (sock.m_highTxMark),
    m_rxBuffer (sock.m_rxBuffer),
    m_txBuffer (sock.m_txBuffer),
    m_state (sock.m_state),
    m_errno (sock.m_errno),
    m_closeNotified (sock.m_closeNotified),
    m_closeOnEmpty (sock.m_closeOnEmpty),
    m_shutdownSend (sock.m_shutdownSend),
    m_shutdownRecv (sock.m_shutdownRecv),
    m_connected (sock.m_connected),
    m_msl (sock.m_msl),
    m_segmentSize (sock.m_segmentSize),
    m_maxWinSize (sock.m_maxWinSize),
    m_rWnd (sock.m_rWnd)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
  // Copy the rtt estimator if it is set
  if (sock.m_rtt)
    {
      m_rtt = sock.m_rtt->Copy ();
    }
  // Reset all callbacks to null
  Callback<void, Ptr< Socket > > vPS = MakeNullCallback<void, Ptr<Socket> > ();
  Callback<void, Ptr<Socket>, const Address &> vPSA = MakeNullCallback<void, Ptr<Socket>, const Address &> ();
  Callback<void, Ptr<Socket>, uint32_t> vPSUI = MakeNullCallback<void, Ptr<Socket>, uint32_t> ();
  SetConnectCallback (vPS, vPS);
  SetDataSentCallback (vPSUI);
  SetSendCallback (vPSUI);
  SetRecvCallback (vPS);
}

DoTcpSocketBase::~DoTcpSocketBase (void)
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  if (m_endPoint != 0)
    {
      NS_ASSERT (m_tcp != 0);
      /*
       * Upon Bind, an DimensionOrderedEndpoint is allocated and set to m_endPoint, and
       * DestroyCallback is set to DoTcpSocketBase::Destroy. If we called
       * m_tcp->DeAllocate, it wil destroy its DimensionOrderedEndpointDemux::DeAllocate,
       * which in turn destroys my m_endPoint, and in turn invokes
       * DoTcpSocketBase::Destroy to nullify m_node, m_endPoint, and m_tcp.
       */
      NS_ASSERT (m_endPoint != 0);
      m_tcp->DeAllocate (m_endPoint);
      NS_ASSERT (m_endPoint == 0);
    }
  m_tcp = 0;
  CancelAllTimers ();
}

/** Associate a node with this TCP socket */
void
DoTcpSocketBase::SetNode (Ptr<Node> node)
{
  m_node = node;
}

/** Associate the L4 protocol (e.g. mux/demux) with this socket */
void
DoTcpSocketBase::SetTcp (Ptr<DoTcpL4Protocol> tcp)
{
  m_tcp = tcp;
}

/** Set an RTT estimator with this socket */
void
DoTcpSocketBase::SetRtt (Ptr<RttEstimator> rtt)
{
  m_rtt = rtt;
}

/** Inherit from Socket class: Returns error code */
enum Socket::SocketErrno
DoTcpSocketBase::GetErrno (void) const
{
  return m_errno;
}

/** Inherit from Socket class: Returns socket type, NS3_SOCK_STREAM */
enum Socket::SocketType
DoTcpSocketBase::GetSocketType (void) const
{
  return NS3_SOCK_STREAM;
}

/** Inherit from Socket class: Returns associated node */
Ptr<Node>
DoTcpSocketBase::GetNode (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_node;
}

/** Inherit from Socket class: Bind socket to an end-point in TcpL4Protocol */
int
DoTcpSocketBase::Bind (void)
{
  NS_LOG_FUNCTION (this);
  m_endPoint = m_tcp->Allocate ();
  if (0 == m_endPoint)
    {
      m_errno = ERROR_ADDRNOTAVAIL;
      return -1;
    }
  m_tcp->m_sockets.push_back (this);
  return SetupCallback ();
}

int
DoTcpSocketBase::Bind6 (void)
{
  NS_LOG_FUNCTION (this);
  return -1;
}

/** Inherit from Socket class: Bind socket (with specific address) to an end-point in TcpL4Protocol */
int
DoTcpSocketBase::Bind (const Address &address)
{
  NS_LOG_FUNCTION (this << address);
  if (DimensionOrderedSocketAddress::IsMatchingType (address))
    {
      DimensionOrderedSocketAddress transport = DimensionOrderedSocketAddress::ConvertFrom (address);
      DimensionOrderedAddress dimOrdered = transport.GetDimensionOrderedAddress ();
      uint16_t port = transport.GetPort ();
      if (dimOrdered == DimensionOrderedAddress::GetAny () && port == 0)
        {
          m_endPoint = m_tcp->Allocate ();
        }
      else if (dimOrdered == DimensionOrderedAddress::GetAny () && port != 0)
        {
          m_endPoint = m_tcp->Allocate (port);
        }
      else if (dimOrdered != DimensionOrderedAddress::GetAny () && port == 0)
        {
          m_endPoint = m_tcp->Allocate (dimOrdered);
        }
      else if (dimOrdered != DimensionOrderedAddress::GetAny () && port != 0)
        {
          m_endPoint = m_tcp->Allocate (dimOrdered, port);
        }
      if (0 == m_endPoint)
        {
          m_errno = port ? ERROR_ADDRINUSE : ERROR_ADDRNOTAVAIL;
          return -1;
        }
    }
  else
    {
      m_errno = ERROR_INVAL;
      return -1;
    }
  m_tcp->m_sockets.push_back (this);
  NS_LOG_LOGIC ("DoTcpSocketBase " << this << " got an endpoint: " << m_endPoint);

  return SetupCallback ();
}

/** Inherit from Socket class: Initiate connection to a remote address:port */
int
DoTcpSocketBase::Connect (const Address & address)
{
  NS_LOG_FUNCTION (this << address);

  // If haven't do so, Bind() this socket first
  if (DimensionOrderedSocketAddress::IsMatchingType (address))
    {
      if (m_endPoint == 0)
        {
          if (Bind () == -1)
            {
              NS_ASSERT (m_endPoint == 0);
              return -1; // Bind() failed
            }
          NS_ASSERT (m_endPoint != 0);
        }
      DimensionOrderedSocketAddress transport = DimensionOrderedSocketAddress::ConvertFrom (address);
      m_endPoint->SetPeer (transport.GetDimensionOrderedAddress (), transport.GetPort ());

      // Get the appropriate local address and port number from the routing protocol and set up endpoint
      if (SetupEndpoint () != 0)
        { // Route to destination does not exist
          return -1;
        }
    }
  else
    {
      m_errno = ERROR_INVAL;
      return -1;
    }

  // Re-initialize parameters in case this socket is being reused after CLOSE
  m_rtt->Reset ();
  m_cnCount = m_cnRetries;

  // DoConnect() will do state-checking and send a SYN packet
  return DoConnect ();
}

/** Inherit from Socket class: Listen on the endpoint for an incoming connection */
int
DoTcpSocketBase::Listen (void)
{
  NS_LOG_FUNCTION (this);
  // Linux quits EINVAL if we're not in CLOSED state, so match what they do
  if (m_state != CLOSED)
    {
      m_errno = ERROR_INVAL;
      return -1;
    }
  // In other cases, set the state to LISTEN and done
  NS_LOG_INFO ("CLOSED -> LISTEN");
  m_state = LISTEN;
  return 0;
}

/** Inherit from Socket class: Kill this socket and signal the peer (if any) */
int
DoTcpSocketBase::Close (void)
{
  NS_LOG_FUNCTION (this);
  /// \internal
  /// First we check to see if there is any unread rx data.
  /// \bugid{426} claims we should send reset in this case.
  if (m_rxBuffer.Size () != 0)
    {
      NS_LOG_INFO ("Socket " << this << " << unread rx data during close.  Sending reset");
      SendRST ();
      return 0;
    }
 
  if (m_txBuffer.SizeFromSequence (m_nextTxSequence) > 0)
    { // App close with pending data must wait until all data transmitted
      if (m_closeOnEmpty == false)
        {
          m_closeOnEmpty = true;
          NS_LOG_INFO ("Socket " << this << " deferring close, state " << TcpStateName[m_state]);
        }
      return 0;
    }
  return DoClose ();
}

/** Inherit from Socket class: Signal a termination of send */
int
DoTcpSocketBase::ShutdownSend (void)
{
  NS_LOG_FUNCTION (this);
  
  //this prevents data from being added to the buffer
  m_shutdownSend = true;
  m_closeOnEmpty = true;
  //if buffer is already empty, send a fin now
  //otherwise fin will go when buffer empties.
  if (m_txBuffer.Size () == 0)
    {
      if (m_state == ESTABLISHED || m_state == CLOSE_WAIT)
        {
          NS_LOG_INFO("Emtpy tx buffer, send fin");
          SendEmptyPacket (DoTcpHeader::FIN);  

          if (m_state == ESTABLISHED)
            { // On active close: I am the first one to send FIN
              NS_LOG_INFO ("ESTABLISHED -> FIN_WAIT_1");
              m_state = FIN_WAIT_1;
            }
          else
            { // On passive close: Peer sent me FIN already
              NS_LOG_INFO ("CLOSE_WAIT -> LAST_ACK");
              m_state = LAST_ACK;
            }  
        }
    }
 
  return 0;
}

/** Inherit from Socket class: Signal a termination of receive */
int
DoTcpSocketBase::ShutdownRecv (void)
{
  NS_LOG_FUNCTION (this);
  m_shutdownRecv = true;
  return 0;
}

/** Inherit from Socket class: Send a packet. Parameter flags is not used.
    Packet has no TCP header. Invoked by upper-layer application */
int
DoTcpSocketBase::Send (Ptr<Packet> p, uint32_t flags)
{
  NS_LOG_FUNCTION (this << p);
  NS_ABORT_MSG_IF (flags, "use of flags is not supported in DoTcpSocketBase::Send()");
  if (m_state == ESTABLISHED || m_state == SYN_SENT || m_state == CLOSE_WAIT)
    {
      // Store the packet into Tx buffer
      if (!m_txBuffer.Add (p))
        { // TxBuffer overflow, send failed
          m_errno = ERROR_MSGSIZE;
          return -1;
        }
      if (m_shutdownSend)
        {
          m_errno = ERROR_SHUTDOWN;
          return -1;
        }
      // Submit the data to lower layers
      NS_LOG_LOGIC ("txBufSize=" << m_txBuffer.Size () << " state " << TcpStateName[m_state]);
      if (m_state == ESTABLISHED || m_state == CLOSE_WAIT)
        { // Try to send the data out
          SendPendingData (m_connected);
        }
      return p->GetSize ();
    }
  else
    { // Connection not established yet
      m_errno = ERROR_NOTCONN;
      return -1; // Send failure
    }
}

/** Inherit from Socket class: In DoTcpSocketBase, it is same as Send() call */
int
DoTcpSocketBase::SendTo (Ptr<Packet> p, uint32_t flags, const Address &address)
{
  return Send (p, flags); // SendTo() and Send() are the same
}

/** Inherit from Socket class: Return data to upper-layer application. Parameter flags
    is not used. Data is returned as a packet of size no larger than maxSize */
Ptr<Packet>
DoTcpSocketBase::Recv (uint32_t maxSize, uint32_t flags)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (flags, "use of flags is not supported in DoTcpSocketBase::Recv()");
  if (m_rxBuffer.Size () == 0 && m_state == CLOSE_WAIT)
    {
      return Create<Packet> (); // Send EOF on connection close
    }
  Ptr<Packet> outPacket = m_rxBuffer.Extract (maxSize);
  if (outPacket != 0 && outPacket->GetSize () != 0)
    {
      SocketAddressTag tag;
      if (m_endPoint != 0)
        {
          tag.SetAddress (DimensionOrderedSocketAddress (m_endPoint->GetPeerAddress (), m_endPoint->GetPeerPort ()));
        }
      outPacket->AddPacketTag (tag);
    }
  return outPacket;
}

/** Inherit from Socket class: Recv and return the remote's address */
Ptr<Packet>
DoTcpSocketBase::RecvFrom (uint32_t maxSize, uint32_t flags, Address &fromAddress)
{
  NS_LOG_FUNCTION (this << maxSize << flags);
  Ptr<Packet> packet = Recv (maxSize, flags);
  // Null packet means no data to read, and an empty packet indicates EOF
  if (packet != 0 && packet->GetSize () != 0)
    {
      if (m_endPoint != 0)
        {
          fromAddress = DimensionOrderedSocketAddress (m_endPoint->GetPeerAddress (), m_endPoint->GetPeerPort ());
        }
      else
        {
          fromAddress = DimensionOrderedSocketAddress (DimensionOrderedAddress::GetZero (), 0);
        }
    }
  return packet;
}

/** Inherit from Socket class: Get the max number of bytes an app can send */
uint32_t
DoTcpSocketBase::GetTxAvailable (void) const
{
  NS_LOG_FUNCTION (this);
  return m_txBuffer.Available ();
}

/** Inherit from Socket class: Get the max number of bytes an app can read */
uint32_t
DoTcpSocketBase::GetRxAvailable (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxBuffer.Available ();
}

/** Inherit from Socket class: Return local address:port */
int
DoTcpSocketBase::GetSockName (Address &address) const
{
  NS_LOG_FUNCTION (this);
  if (m_endPoint != 0)
    {
      address = DimensionOrderedSocketAddress (m_endPoint->GetLocalAddress (), m_endPoint->GetLocalPort ());
    }
  else
    { // It is possible to call this method on a socket without a name
      // in which case, behavior is unspecified
      // Should this return an DimensionOrderedSocketAddress
      address = DimensionOrderedSocketAddress (DimensionOrderedAddress::GetZero (), 0);
    }
  return 0;
}

/** Inherit from Socket class: Bind this socket to the specified NetDevice */
void
DoTcpSocketBase::BindToNetDevice (Ptr<NetDevice> netdevice)
{
  NS_LOG_FUNCTION (netdevice);
  Socket::BindToNetDevice (netdevice); // Includes sanity check
  if (m_endPoint == 0)
    {
      if (Bind () == -1)
        {
          NS_ASSERT ((m_endPoint == 0));
          return;
        }
      NS_ASSERT ((m_endPoint != 0));
    }

  if (m_endPoint != 0)
    {
      m_endPoint->BindToNetDevice (netdevice);
    }
  // No BindToNetDevice() for Ipv6EndPoint
  return;
}

/** Clean up after Bind. Set up callback functions in the end-point. */
int
DoTcpSocketBase::SetupCallback (void)
{
  NS_LOG_FUNCTION (this);

  if (m_endPoint == 0)
    {
      return -1;
    }
  if (m_endPoint != 0)
    {
      m_endPoint->SetRxCallback (MakeCallback (&DoTcpSocketBase::ForwardUp, Ptr<DoTcpSocketBase> (this)));
      m_endPoint->SetDestroyCallback (MakeCallback (&DoTcpSocketBase::Destroy, Ptr<DoTcpSocketBase> (this)));
    }
  return 0;
}

/** Perform the real connection tasks: Send SYN if allowed, RST if invalid */
int
DoTcpSocketBase::DoConnect (void)
{
  NS_LOG_FUNCTION (this);

  // A new connection is allowed only if this socket does not have a connection
  if (m_state == CLOSED || m_state == LISTEN || m_state == SYN_SENT || m_state == LAST_ACK || m_state == CLOSE_WAIT)
    { // send a SYN packet and change state into SYN_SENT
      SendEmptyPacket (DoTcpHeader::SYN);
      NS_LOG_INFO (TcpStateName[m_state] << " -> SYN_SENT");
      m_state = SYN_SENT;
    }
  else if (m_state != TIME_WAIT)
    { // In states SYN_RCVD, ESTABLISHED, FIN_WAIT_1, FIN_WAIT_2, and CLOSING, an connection
      // exists. We send RST, tear down everything, and close this socket.
      SendRST ();
      CloseAndNotify ();
    }
  return 0;
}

/** Do the action to close the socket. Usually send a packet with appropriate
    flags depended on the current m_state. */
int
DoTcpSocketBase::DoClose (void)
{
  NS_LOG_FUNCTION (this);
  switch (m_state)
    {
    case SYN_RCVD:
    case ESTABLISHED:
      // send FIN to close the peer
      SendEmptyPacket (DoTcpHeader::FIN);
      NS_LOG_INFO ("ESTABLISHED -> FIN_WAIT_1");
      m_state = FIN_WAIT_1;
      break;
    case CLOSE_WAIT:
      // send FIN+ACK to close the peer
      SendEmptyPacket (DoTcpHeader::FIN | DoTcpHeader::ACK);
      NS_LOG_INFO ("CLOSE_WAIT -> LAST_ACK");
      m_state = LAST_ACK;
      break;
    case SYN_SENT:
    case CLOSING:
      // Send RST if application closes in SYN_SENT and CLOSING
      SendRST ();
      CloseAndNotify ();
      break;
    case LISTEN:
    case LAST_ACK:
      // In these three states, move to CLOSED and tear down the end point
      CloseAndNotify ();
      break;
    case CLOSED:
    case FIN_WAIT_1:
    case FIN_WAIT_2:
    case TIME_WAIT:
    default: /* mute compiler */
      // Do nothing in these four states
      break;
    }
  return 0;
}

/** Peacefully close the socket by notifying the upper layer and deallocate end point */
void
DoTcpSocketBase::CloseAndNotify (void)
{
  NS_LOG_FUNCTION (this);

  if (!m_closeNotified)
    {
      NotifyNormalClose ();
    }
  if (m_state != TIME_WAIT)
    {
      DeallocateEndPoint ();
    }
  m_closeNotified = true;
  NS_LOG_INFO (TcpStateName[m_state] << " -> CLOSED");
  CancelAllTimers ();
  m_state = CLOSED;
}


/** Tell if a sequence number range is out side the range that my rx buffer can
    accpet */
bool
DoTcpSocketBase::OutOfRange (SequenceNumber32 head, SequenceNumber32 tail) const
{
  if (m_state == LISTEN || m_state == SYN_SENT || m_state == SYN_RCVD)
    { // Rx buffer in these states are not initialized.
      return false;
    }
  if (m_state == LAST_ACK || m_state == CLOSING || m_state == CLOSE_WAIT)
    { // In LAST_ACK and CLOSING states, it only wait for an ACK and the
      // sequence number must equals to m_rxBuffer.NextRxSequence ()
      return (m_rxBuffer.NextRxSequence () != head);
    }

  // In all other cases, check if the sequence number is in range
  return (tail < m_rxBuffer.NextRxSequence () || m_rxBuffer.MaxRxSequence () <= head);
}

/** Function called by the L3 protocol when it received a packet to pass on to
    the TCP. This function is registered as the "RxCallback" function in
    SetupCallback(), which invoked by Bind(), and CompleteFork() */
void
DoTcpSocketBase::ForwardUp (Ptr<Packet> packet, DimensionOrderedHeader header, uint16_t port,
                          Ptr<DimensionOrderedInterface> incomingInterface)
{
  DoForwardUp (packet, header, port, incomingInterface);
}

/** The real function to handle the incoming packet from lower layers. This is
    wrapped by ForwardUp() so that this function can be overloaded by daughter
    classes. */
void
DoTcpSocketBase::DoForwardUp (Ptr<Packet> packet, DimensionOrderedHeader header, uint16_t port,
                            Ptr<DimensionOrderedInterface> incomingInterface)
{
  NS_LOG_LOGIC ("Socket " << this << " forward up " <<
                m_endPoint->GetPeerAddress () <<
                ":" << m_endPoint->GetPeerPort () <<
                " to " << m_endPoint->GetLocalAddress () <<
                ":" << m_endPoint->GetLocalPort ());
  Address fromAddress = DimensionOrderedSocketAddress (header.GetSource (), port);
  Address toAddress = DimensionOrderedSocketAddress (header.GetDestination (), m_endPoint->GetLocalPort ());

  // Peel off TCP header and do validity checking
  DoTcpHeader tcpHeader;
  packet->RemoveHeader (tcpHeader);
  if (tcpHeader.GetFlags () & DoTcpHeader::ACK)
    {
      EstimateRtt (tcpHeader);
    }
  ReadOptions (tcpHeader);

  // Update Rx window size, i.e. the flow control window
  if (m_rWnd.Get () == 0 && tcpHeader.GetWindowSize () != 0)
    { // persist probes end
      NS_LOG_LOGIC (this << " Leaving zerowindow persist state");
      m_persistEvent.Cancel ();
    }
  m_rWnd = tcpHeader.GetWindowSize ();

  // Discard fully out of range data packets
  if (packet->GetSize ()
      && OutOfRange (tcpHeader.GetSequenceNumber (), tcpHeader.GetSequenceNumber () + packet->GetSize ()))
    {
      NS_LOG_LOGIC ("At state " << TcpStateName[m_state] <<
                    " received packet of seq [" << tcpHeader.GetSequenceNumber () <<
                    ":" << tcpHeader.GetSequenceNumber () + packet->GetSize () <<
                    ") out of range [" << m_rxBuffer.NextRxSequence () << ":" <<
                    m_rxBuffer.MaxRxSequence () << ")");
      // Acknowledgement should be sent for all unacceptable packets (RFC793, p.69)
      if (m_state == ESTABLISHED && !(tcpHeader.GetFlags () & DoTcpHeader::RST))
        {
          SendEmptyPacket (DoTcpHeader::ACK);
        }
      return;
    }

  // TCP state machine code in different process functions
  // C.f.: tcp_rcv_state_process() in tcp_input.c in Linux kernel
  switch (m_state)
    {
    case ESTABLISHED:
      ProcessEstablished (packet, tcpHeader);
      break;
    case LISTEN:
      ProcessListen (packet, tcpHeader, fromAddress, toAddress);
      break;
    case TIME_WAIT:
      // Do nothing
      break;
    case CLOSED:
      // Send RST if the incoming packet is not a RST
      if ((tcpHeader.GetFlags () & ~(DoTcpHeader::PSH | DoTcpHeader::URG)) != DoTcpHeader::RST)
        { // Since m_endPoint is not configured yet, we cannot use SendRST here
          DoTcpHeader h;
          h.SetFlags (DoTcpHeader::RST);
          h.SetSequenceNumber (m_nextTxSequence);
          h.SetAckNumber (m_rxBuffer.NextRxSequence ());
          h.SetSourcePort (tcpHeader.GetDestinationPort ());
          h.SetDestinationPort (tcpHeader.GetSourcePort ());
          h.SetWindowSize (AdvertisedWindowSize ());
          AddOptions (h);
          m_tcp->SendPacket (Create<Packet> (), h, header.GetDestination (), header.GetSource (), m_boundnetdevice);
        }
      break;
    case SYN_SENT:
      ProcessSynSent (packet, tcpHeader);
      break;
    case SYN_RCVD:
      ProcessSynRcvd (packet, tcpHeader, fromAddress, toAddress);
      break;
    case FIN_WAIT_1:
    case FIN_WAIT_2:
    case CLOSE_WAIT:
      ProcessWait (packet, tcpHeader);
      break;
    case CLOSING:
      ProcessClosing (packet, tcpHeader);
      break;
    case LAST_ACK:
      ProcessLastAck (packet, tcpHeader);
      break;
    default: // mute compiler
      break;
    }
}

/** Received a packet upon ESTABLISHED state. This function is mimicking the
    role of tcp_rcv_established() in tcp_input.c in Linux kernel. */
void
DoTcpSocketBase::ProcessEstablished (Ptr<Packet> packet, const DoTcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  // Extract the flags. PSH and URG are not honoured.
  uint8_t tcpflags = tcpHeader.GetFlags () & ~(DoTcpHeader::PSH | DoTcpHeader::URG);

  // Different flags are different events
  if (tcpflags == DoTcpHeader::ACK)
    {
      ReceivedAck (packet, tcpHeader);
    }
  else if (tcpflags == DoTcpHeader::SYN)
    { // Received SYN, old NS-3 behaviour is to set state to SYN_RCVD and
      // respond with a SYN+ACK. But it is not a legal state transition as of
      // RFC793. Thus this is ignored.
    }
  else if (tcpflags == (DoTcpHeader::SYN | DoTcpHeader::ACK))
    { // No action for received SYN+ACK, it is probably a duplicated packet
    }
  else if (tcpflags == DoTcpHeader::FIN || tcpflags == (DoTcpHeader::FIN | DoTcpHeader::ACK))
    { // Received FIN or FIN+ACK, bring down this socket nicely
      PeerClose (packet, tcpHeader);
    }
  else if (tcpflags == 0)
    { // No flags means there is only data
      ReceivedData (packet, tcpHeader);
      if (m_rxBuffer.Finished ())
        {
          PeerClose (packet, tcpHeader);
        }
    }
  else
    { // Received RST or the TCP flags is invalid, in either case, terminate this socket
      if (tcpflags != DoTcpHeader::RST)
        { // this must be an invalid flag, send reset
          NS_LOG_LOGIC ("Illegal flag " << tcpflags << " received. Reset packet is sent.");
          SendRST ();
        }
      CloseAndNotify ();
    }
}

/** Process the newly received ACK */
void
DoTcpSocketBase::ReceivedAck (Ptr<Packet> packet, const DoTcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  // Received ACK. Compare the ACK number against highest unacked seqno
  if (0 == (tcpHeader.GetFlags () & DoTcpHeader::ACK))
    { // Ignore if no ACK flag
    }
  else if (tcpHeader.GetAckNumber () < m_txBuffer.HeadSequence ())
    { // Case 1: Old ACK, ignored.
      NS_LOG_LOGIC ("Ignored ack of " << tcpHeader.GetAckNumber ());
    }
  else if (tcpHeader.GetAckNumber () == m_txBuffer.HeadSequence ())
    { // Case 2: Potentially a duplicated ACK
      if (tcpHeader.GetAckNumber () < m_nextTxSequence && packet->GetSize() == 0)
        {
          NS_LOG_LOGIC ("Dupack of " << tcpHeader.GetAckNumber ());
          DupAck (tcpHeader, ++m_dupAckCount);
        }
      // otherwise, the ACK is precisely equal to the nextTxSequence
      NS_ASSERT (tcpHeader.GetAckNumber () <= m_nextTxSequence);
    }
  else if (tcpHeader.GetAckNumber () > m_txBuffer.HeadSequence ())
    { // Case 3: New ACK, reset m_dupAckCount and update m_txBuffer
      NS_LOG_LOGIC ("New ack of " << tcpHeader.GetAckNumber ());
      NewAck (tcpHeader.GetAckNumber ());
      m_dupAckCount = 0;
    }
  // If there is any data piggybacked, store it into m_rxBuffer
  if (packet->GetSize () > 0)
    {
      ReceivedData (packet, tcpHeader);
    }
}

/** Received a packet upon LISTEN state. */
void
DoTcpSocketBase::ProcessListen (Ptr<Packet> packet, const DoTcpHeader& tcpHeader,
                              const Address& fromAddress, const Address& toAddress)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  // Extract the flags. PSH and URG are not honoured.
  uint8_t tcpflags = tcpHeader.GetFlags () & ~(DoTcpHeader::PSH | DoTcpHeader::URG);

  // Fork a socket if received a SYN. Do nothing otherwise.
  // C.f.: the LISTEN part in tcp_v4_do_rcv() in tcp_ipv4.c in Linux kernel
  if (tcpflags != DoTcpHeader::SYN)
    {
      return;
    }

  // Call socket's notify function to let the server app know we got a SYN
  // If the server app refuses the connection, do nothing
  if (!NotifyConnectionRequest (fromAddress))
    {
      return;
    }
  // Clone the socket, simulate fork
  Ptr<DoTcpSocketBase> newSock = Fork ();
  NS_LOG_LOGIC ("Cloned a DoTcpSocketBase " << newSock);
  Simulator::ScheduleNow (&DoTcpSocketBase::CompleteFork, newSock,
                          packet, tcpHeader, fromAddress, toAddress);
}

/** Received a packet upon SYN_SENT */
void
DoTcpSocketBase::ProcessSynSent (Ptr<Packet> packet, const DoTcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  // Extract the flags. PSH and URG are not honoured.
  uint8_t tcpflags = tcpHeader.GetFlags () & ~(DoTcpHeader::PSH | DoTcpHeader::URG);

  if (tcpflags == 0)
    { // Bare data, accept it and move to ESTABLISHED state. This is not a normal behaviour. Remove this?
      NS_LOG_INFO ("SYN_SENT -> ESTABLISHED");
      m_state = ESTABLISHED;
      m_connected = true;
      m_retxEvent.Cancel ();
      m_delAckCount = m_delAckMaxCount;
      ReceivedData (packet, tcpHeader);
      Simulator::ScheduleNow (&DoTcpSocketBase::ConnectionSucceeded, this);
    }
  else if (tcpflags == DoTcpHeader::ACK)
    { // Ignore ACK in SYN_SENT
    }
  else if (tcpflags == DoTcpHeader::SYN)
    { // Received SYN, move to SYN_RCVD state and respond with SYN+ACK
      NS_LOG_INFO ("SYN_SENT -> SYN_RCVD");
      m_state = SYN_RCVD;
      m_cnCount = m_cnRetries;
      m_rxBuffer.SetNextRxSequence (tcpHeader.GetSequenceNumber () + SequenceNumber32 (1));
      SendEmptyPacket (DoTcpHeader::SYN | DoTcpHeader::ACK);
    }
  else if (tcpflags == (DoTcpHeader::SYN | DoTcpHeader::ACK)
           && m_nextTxSequence + SequenceNumber32 (1) == tcpHeader.GetAckNumber ())
    { // Handshake completed
      NS_LOG_INFO ("SYN_SENT -> ESTABLISHED");
      m_state = ESTABLISHED;
      m_connected = true;
      m_retxEvent.Cancel ();
      m_rxBuffer.SetNextRxSequence (tcpHeader.GetSequenceNumber () + SequenceNumber32 (1));
      m_highTxMark = ++m_nextTxSequence;
      m_txBuffer.SetHeadSequence (m_nextTxSequence);
      SendEmptyPacket (DoTcpHeader::ACK);
      SendPendingData (m_connected);
      Simulator::ScheduleNow (&DoTcpSocketBase::ConnectionSucceeded, this);
      // Always respond to first data packet to speed up the connection.
      // Remove to get the behaviour of old NS-3 code.
      m_delAckCount = m_delAckMaxCount;
    }
  else
    { // Other in-sequence input
      if (tcpflags != DoTcpHeader::RST)
        { // When (1) rx of FIN+ACK; (2) rx of FIN; (3) rx of bad flags
          NS_LOG_LOGIC ("Illegal flag " << std::hex << static_cast<uint32_t> (tcpflags) << std::dec << " received. Reset packet is sent.");
          SendRST ();
        }
      CloseAndNotify ();
    }
}

/** Received a packet upon SYN_RCVD */
void
DoTcpSocketBase::ProcessSynRcvd (Ptr<Packet> packet, const DoTcpHeader& tcpHeader,
                               const Address& fromAddress, const Address& toAddress)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  // Extract the flags. PSH and URG are not honoured.
  uint8_t tcpflags = tcpHeader.GetFlags () & ~(DoTcpHeader::PSH | DoTcpHeader::URG);

  if (tcpflags == 0
      || (tcpflags == DoTcpHeader::ACK
          && m_nextTxSequence + SequenceNumber32 (1) == tcpHeader.GetAckNumber ()))
    { // If it is bare data, accept it and move to ESTABLISHED state. This is
      // possibly due to ACK lost in 3WHS. If in-sequence ACK is received, the
      // handshake is completed nicely.
      NS_LOG_INFO ("SYN_RCVD -> ESTABLISHED");
      m_state = ESTABLISHED;
      m_connected = true;
      m_retxEvent.Cancel ();
      m_highTxMark = ++m_nextTxSequence;
      m_txBuffer.SetHeadSequence (m_nextTxSequence);
      if (m_endPoint)
        {
          m_endPoint->SetPeer (DimensionOrderedSocketAddress::ConvertFrom (fromAddress).GetDimensionOrderedAddress (),
                               DimensionOrderedSocketAddress::ConvertFrom (fromAddress).GetPort ());
        }
      // Always respond to first data packet to speed up the connection.
      // Remove to get the behaviour of old NS-3 code.
      m_delAckCount = m_delAckMaxCount;
      ReceivedAck (packet, tcpHeader);
      NotifyNewConnectionCreated (this, fromAddress);
      // As this connection is established, the socket is available to send data now
      if (GetTxAvailable () > 0)
        {
          NotifySend (GetTxAvailable ());
        }
    }
  else if (tcpflags == DoTcpHeader::SYN)
    { // Probably the peer lost my SYN+ACK
      m_rxBuffer.SetNextRxSequence (tcpHeader.GetSequenceNumber () + SequenceNumber32 (1));
      SendEmptyPacket (DoTcpHeader::SYN | DoTcpHeader::ACK);
    }
  else if (tcpflags == (DoTcpHeader::FIN | DoTcpHeader::ACK))
    {
      if (tcpHeader.GetSequenceNumber () == m_rxBuffer.NextRxSequence ())
        { // In-sequence FIN before connection complete. Set up connection and close.
          m_connected = true;
          m_retxEvent.Cancel ();
          m_highTxMark = ++m_nextTxSequence;
          m_txBuffer.SetHeadSequence (m_nextTxSequence);
          if (m_endPoint)
            {
              m_endPoint->SetPeer (DimensionOrderedSocketAddress::ConvertFrom (fromAddress).GetDimensionOrderedAddress (),
                                   DimensionOrderedSocketAddress::ConvertFrom (fromAddress).GetPort ());
            }
          PeerClose (packet, tcpHeader);
        }
    }
  else
    { // Other in-sequence input
      if (tcpflags != DoTcpHeader::RST)
        { // When (1) rx of SYN+ACK; (2) rx of FIN; (3) rx of bad flags
          NS_LOG_LOGIC ("Illegal flag " << tcpflags << " received. Reset packet is sent.");
          if (m_endPoint)
            {
              m_endPoint->SetPeer (DimensionOrderedSocketAddress::ConvertFrom (fromAddress).GetDimensionOrderedAddress (),
                                   DimensionOrderedSocketAddress::ConvertFrom (fromAddress).GetPort ());
            }
          SendRST ();
        }
      CloseAndNotify ();
    }
}

/** Received a packet upon CLOSE_WAIT, FIN_WAIT_1, or FIN_WAIT_2 states */
void
DoTcpSocketBase::ProcessWait (Ptr<Packet> packet, const DoTcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  // Extract the flags. PSH and URG are not honoured.
  uint8_t tcpflags = tcpHeader.GetFlags () & ~(DoTcpHeader::PSH | DoTcpHeader::URG);

  if (packet->GetSize () > 0 && tcpflags != DoTcpHeader::ACK)
    { // Bare data, accept it
      ReceivedData (packet, tcpHeader);
    }
  else if (tcpflags == DoTcpHeader::ACK)
    { // Process the ACK, and if in FIN_WAIT_1, conditionally move to FIN_WAIT_2
      ReceivedAck (packet, tcpHeader);
      if (m_state == FIN_WAIT_1 && m_txBuffer.Size () == 0
          && tcpHeader.GetAckNumber () == m_highTxMark + SequenceNumber32 (1))
        { // This ACK corresponds to the FIN sent
          NS_LOG_INFO ("FIN_WAIT_1 -> FIN_WAIT_2");
          m_state = FIN_WAIT_2;
        }
    }
  else if (tcpflags == DoTcpHeader::FIN || tcpflags == (DoTcpHeader::FIN | DoTcpHeader::ACK))
    { // Got FIN, respond with ACK and move to next state
      if (tcpflags & DoTcpHeader::ACK)
        { // Process the ACK first
          ReceivedAck (packet, tcpHeader);
        }
      m_rxBuffer.SetFinSequence (tcpHeader.GetSequenceNumber ());
    }
  else if (tcpflags == DoTcpHeader::SYN || tcpflags == (DoTcpHeader::SYN | DoTcpHeader::ACK))
    { // Duplicated SYN or SYN+ACK, possibly due to spurious retransmission
      return;
    }
  else
    { // This is a RST or bad flags
      if (tcpflags != DoTcpHeader::RST)
        {
          NS_LOG_LOGIC ("Illegal flag " << tcpflags << " received. Reset packet is sent.");
          SendRST ();
        }
      CloseAndNotify ();
      return;
    }

  // Check if the close responder sent an in-sequence FIN, if so, respond ACK
  if ((m_state == FIN_WAIT_1 || m_state == FIN_WAIT_2) && m_rxBuffer.Finished ())
    {
      if (m_state == FIN_WAIT_1)
        {
          NS_LOG_INFO ("FIN_WAIT_1 -> CLOSING");
          m_state = CLOSING;
          if (m_txBuffer.Size () == 0
              && tcpHeader.GetAckNumber () == m_highTxMark + SequenceNumber32 (1))
            { // This ACK corresponds to the FIN sent
              TimeWait ();
            }
        }
      else if (m_state == FIN_WAIT_2)
        {
          TimeWait ();
        }
      SendEmptyPacket (DoTcpHeader::ACK);
      if (!m_shutdownRecv)
        {
          NotifyDataRecv ();
        }
    }
}

/** Received a packet upon CLOSING */
void
DoTcpSocketBase::ProcessClosing (Ptr<Packet> packet, const DoTcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  // Extract the flags. PSH and URG are not honoured.
  uint8_t tcpflags = tcpHeader.GetFlags () & ~(DoTcpHeader::PSH | DoTcpHeader::URG);

  if (tcpflags == DoTcpHeader::ACK)
    {
      if (tcpHeader.GetSequenceNumber () == m_rxBuffer.NextRxSequence ())
        { // This ACK corresponds to the FIN sent
          TimeWait ();
        }
    }
  else
    { // CLOSING state means simultaneous close, i.e. no one is sending data to
      // anyone. If anything other than ACK is received, respond with a reset.
      if (tcpflags == DoTcpHeader::FIN || tcpflags == (DoTcpHeader::FIN | DoTcpHeader::ACK))
        { // FIN from the peer as well. We can close immediately.
          SendEmptyPacket (DoTcpHeader::ACK);
        }
      else if (tcpflags != DoTcpHeader::RST)
        { // Receive of SYN or SYN+ACK or bad flags or pure data
          NS_LOG_LOGIC ("Illegal flag " << tcpflags << " received. Reset packet is sent.");
          SendRST ();
        }
      CloseAndNotify ();
    }
}

/** Received a packet upon LAST_ACK */
void
DoTcpSocketBase::ProcessLastAck (Ptr<Packet> packet, const DoTcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  // Extract the flags. PSH and URG are not honoured.
  uint8_t tcpflags = tcpHeader.GetFlags () & ~(DoTcpHeader::PSH | DoTcpHeader::URG);

  if (tcpflags == 0)
    {
      ReceivedData (packet, tcpHeader);
    }
  else if (tcpflags == DoTcpHeader::ACK)
    {
      if (tcpHeader.GetSequenceNumber () == m_rxBuffer.NextRxSequence ())
        { // This ACK corresponds to the FIN sent. This socket closed peacefully.
          CloseAndNotify ();
        }
    }
  else if (tcpflags == DoTcpHeader::FIN)
    { // Received FIN again, the peer probably lost the FIN+ACK
      SendEmptyPacket (DoTcpHeader::FIN | DoTcpHeader::ACK);
    }
  else if (tcpflags == (DoTcpHeader::FIN | DoTcpHeader::ACK) || tcpflags == DoTcpHeader::RST)
    {
      CloseAndNotify ();
    }
  else
    { // Received a SYN or SYN+ACK or bad flags
      NS_LOG_LOGIC ("Illegal flag " << tcpflags << " received. Reset packet is sent.");
      SendRST ();
      CloseAndNotify ();
    }
}

/** Peer sent me a FIN. Remember its sequence in rx buffer. */
void
DoTcpSocketBase::PeerClose (Ptr<Packet> p, const DoTcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  // Ignore all out of range packets
  if (tcpHeader.GetSequenceNumber () < m_rxBuffer.NextRxSequence ()
      || tcpHeader.GetSequenceNumber () > m_rxBuffer.MaxRxSequence ())
    {
      return;
    }
  // For any case, remember the FIN position in rx buffer first
  m_rxBuffer.SetFinSequence (tcpHeader.GetSequenceNumber () + SequenceNumber32 (p->GetSize ()));
  NS_LOG_LOGIC ("Accepted FIN at seq " << tcpHeader.GetSequenceNumber () + SequenceNumber32 (p->GetSize ()));
  // If there is any piggybacked data, process it
  if (p->GetSize ())
    {
      ReceivedData (p, tcpHeader);
    }
  // Return if FIN is out of sequence, otherwise move to CLOSE_WAIT state by DoPeerClose
  if (!m_rxBuffer.Finished ())
    {
      return;
    }

  // Simultaneous close: Application invoked Close() when we are processing this FIN packet
  if (m_state == FIN_WAIT_1)
    {
      NS_LOG_INFO ("FIN_WAIT_1 -> CLOSING");
      m_state = CLOSING;
      return;
    }

  DoPeerClose (); // Change state, respond with ACK
}

/** Received a in-sequence FIN. Close down this socket. */
void
DoTcpSocketBase::DoPeerClose (void)
{
  NS_ASSERT (m_state == ESTABLISHED || m_state == SYN_RCVD);

  // Move the state to CLOSE_WAIT
  NS_LOG_INFO (TcpStateName[m_state] << " -> CLOSE_WAIT");
  m_state = CLOSE_WAIT;

  if (!m_closeNotified)
    {
      // The normal behaviour for an application is that, when the peer sent a in-sequence
      // FIN, the app should prepare to close. The app has two choices at this point: either
      // respond with ShutdownSend() call to declare that it has nothing more to send and
      // the socket can be closed immediately; or remember the peer's close request, wait
      // until all its existing data are pushed into the TCP socket, then call Close()
      // explicitly.
      NS_LOG_LOGIC ("TCP " << this << " calling NotifyNormalClose");
      NotifyNormalClose ();
      m_closeNotified = true;
    }
  if (m_shutdownSend)
    { // The application declares that it would not sent any more, close this socket
      Close ();
    }
  else
    { // Need to ack, the application will close later
      SendEmptyPacket (DoTcpHeader::ACK);
    }
  if (m_state == LAST_ACK)
    {
      NS_LOG_LOGIC ("DoTcpSocketBase " << this << " scheduling LATO1");
      m_lastAckEvent = Simulator::Schedule (m_rtt->RetransmitTimeout (),
                                            &DoTcpSocketBase::LastAckTimeout, this);
    }
}

/** Kill this socket. This is a callback function configured to m_endpoint in
   SetupCallback(), invoked when the endpoint is destroyed. */
void
DoTcpSocketBase::Destroy (void)
{
  NS_LOG_FUNCTION (this);
  m_endPoint = 0;
  if (m_tcp != 0)
    {
      std::vector<Ptr<DoTcpSocketBase> >::iterator it
        = std::find (m_tcp->m_sockets.begin (), m_tcp->m_sockets.end (), this);
      if (it != m_tcp->m_sockets.end ())
        {
          m_tcp->m_sockets.erase (it);
        }
    }
  NS_LOG_LOGIC (this << " Cancelled ReTxTimeout event which was set to expire at " <<
                (Simulator::Now () + Simulator::GetDelayLeft (m_retxEvent)).GetSeconds ());
  CancelAllTimers ();
}

/** Send an empty packet with specified TCP flags */
void
DoTcpSocketBase::SendEmptyPacket (uint8_t flags)
{
  NS_LOG_FUNCTION (this << (uint32_t)flags);
  Ptr<Packet> p = Create<Packet> ();
  DoTcpHeader header;
  SequenceNumber32 s = m_nextTxSequence;

  /*
   * Add tags for each socket option.
   * Note that currently the socket adds both IPv4 tag and IPv6 tag
   * if both options are set. Once the packet got to layer three, only
   * the corresponding tags will be read.
   */
  if (IsManualIpTos ())
    {
      SocketIpTosTag ipTosTag;
      ipTosTag.SetTos (GetIpTos ());
      p->AddPacketTag (ipTosTag);
    }

  if (IsManualIpTtl ())
    {
      SocketIpTtlTag ipTtlTag;
      ipTtlTag.SetTtl (GetIpTtl ());
      p->AddPacketTag (ipTtlTag);
    }

  if (m_endPoint == 0)
    {
      NS_LOG_WARN ("Failed to send empty packet due to null endpoint");
      return;
    }
  if (flags & DoTcpHeader::FIN)
    {
      flags |= DoTcpHeader::ACK;
    }
  else if (m_state == FIN_WAIT_1 || m_state == LAST_ACK || m_state == CLOSING)
    {
      ++s;
    }

  header.SetFlags (flags);
  header.SetSequenceNumber (s);
  header.SetAckNumber (m_rxBuffer.NextRxSequence ());
  if (m_endPoint != 0)
    {
      header.SetSourcePort (m_endPoint->GetLocalPort ());
      header.SetDestinationPort (m_endPoint->GetPeerPort ());
    }
  header.SetWindowSize (AdvertisedWindowSize ());
  AddOptions (header);
  m_rto = m_rtt->RetransmitTimeout ();
  bool hasSyn = flags & DoTcpHeader::SYN;
  bool hasFin = flags & DoTcpHeader::FIN;
  bool isAck = flags == DoTcpHeader::ACK;
  if (hasSyn)
    {
      if (m_cnCount == 0)
        { // No more connection retries, give up
          NS_LOG_LOGIC ("Connection failed.");
          CloseAndNotify ();
          return;
        }
      else
        { // Exponential backoff of connection time out
          int backoffCount = 0x1 << (m_cnRetries - m_cnCount);
          m_rto = m_cnTimeout * backoffCount;
          m_cnCount--;
        }
    }
  if (m_endPoint != 0)
    {
      m_tcp->SendPacket (p, header, m_endPoint->GetLocalAddress (),
                         m_endPoint->GetPeerAddress (), m_boundnetdevice);
    }
  if (flags & DoTcpHeader::ACK)
    { // If sending an ACK, cancel the delay ACK as well
      m_delAckEvent.Cancel ();
      m_delAckCount = 0;
    }
  if (m_retxEvent.IsExpired () && (hasSyn || hasFin) && !isAck )
    { // Retransmit SYN / SYN+ACK / FIN / FIN+ACK to guard against lost
      NS_LOG_LOGIC ("Schedule retransmission timeout at time "
                    << Simulator::Now ().GetSeconds () << " to expire at time "
                    << (Simulator::Now () + m_rto.Get ()).GetSeconds ());
      m_retxEvent = Simulator::Schedule (m_rto, &DoTcpSocketBase::SendEmptyPacket, this, flags);
    }
}

/** This function closes the endpoint completely. Called upon RST_TX action. */
void
DoTcpSocketBase::SendRST (void)
{
  NS_LOG_FUNCTION (this);
  SendEmptyPacket (DoTcpHeader::RST);
  NotifyErrorClose ();
  DeallocateEndPoint ();
}

/** Deallocate the end point and cancel all the timers */
void
DoTcpSocketBase::DeallocateEndPoint (void)
{
  if (m_endPoint != 0)
    {
      m_endPoint->SetDestroyCallback (MakeNullCallback<void> ());
      m_tcp->DeAllocate (m_endPoint);
      m_endPoint = 0;
      std::vector<Ptr<DoTcpSocketBase> >::iterator it
        = std::find (m_tcp->m_sockets.begin (), m_tcp->m_sockets.end (), this);
      if (it != m_tcp->m_sockets.end ())
        {
          m_tcp->m_sockets.erase (it);
        }
      CancelAllTimers ();
    }
}

/** Configure the endpoint to a local address. Called by Connect() if Bind() didn't specify one. */
int
DoTcpSocketBase::SetupEndpoint ()
{
  NS_LOG_FUNCTION (this);
  Ptr<DimensionOrdered> dimOrdered = m_node->GetObject<DimensionOrdered> ();
  NS_ASSERT (dimOrdered != 0);
  // Create a dummy packet, then ask the routing function for the best output
  // interface's address
  DimensionOrderedHeader header;
  header.SetDestination (m_endPoint->GetPeerAddress ());
  Socket::SocketErrno errno_;
  Ptr<NetDevice> oif = m_boundnetdevice;
  
  // Determine src
  DimensionOrderedAddress src = dimOrdered->GetAddress (DimensionOrdered::X_POS).GetLocal ();
  if (src == DimensionOrderedAddress::GetZero ())
      src = dimOrdered->GetAddress (DimensionOrdered::X_NEG).GetLocal ();
  if (src == DimensionOrderedAddress::GetZero ())
      src = dimOrdered->GetAddress (DimensionOrdered::Y_POS).GetLocal ();
  if (src == DimensionOrderedAddress::GetZero ())
      src = dimOrdered->GetAddress (DimensionOrdered::Y_NEG).GetLocal ();
  if (src == DimensionOrderedAddress::GetZero ())
      src = dimOrdered->GetAddress (DimensionOrdered::Z_POS).GetLocal ();
  if (src == DimensionOrderedAddress::GetZero ())
      src = dimOrdered->GetAddress (DimensionOrdered::Z_NEG).GetLocal ();
  NS_ASSERT (src != DimensionOrderedAddress::GetZero ());
  m_endPoint->SetLocalAddress (src);
  return 0;
}

/** This function is called only if a SYN received in LISTEN state. After
   DoTcpSocketBase cloned, allocate a new end point to handle the incoming
   connection and send a SYN+ACK to complete the handshake. */
void
DoTcpSocketBase::CompleteFork (Ptr<Packet> p, const DoTcpHeader& h,
                             const Address& fromAddress, const Address& toAddress)
{
  // Get port and address from peer (connecting host)
  if (DimensionOrderedSocketAddress::IsMatchingType (toAddress))
    {
      m_endPoint = m_tcp->Allocate (DimensionOrderedSocketAddress::ConvertFrom (toAddress).GetDimensionOrderedAddress (),
                                    DimensionOrderedSocketAddress::ConvertFrom (toAddress).GetPort (),
                                    DimensionOrderedSocketAddress::ConvertFrom (fromAddress).GetDimensionOrderedAddress (),
                                    DimensionOrderedSocketAddress::ConvertFrom (fromAddress).GetPort ());
    }
  m_tcp->m_sockets.push_back (this);

  // Change the cloned socket from LISTEN state to SYN_RCVD
  NS_LOG_INFO ("LISTEN -> SYN_RCVD");
  m_state = SYN_RCVD;
  m_cnCount = m_cnRetries;
  SetupCallback ();
  // Set the sequence number and send SYN+ACK
  m_rxBuffer.SetNextRxSequence (h.GetSequenceNumber () + SequenceNumber32 (1));
  SendEmptyPacket (DoTcpHeader::SYN | DoTcpHeader::ACK);
}

void
DoTcpSocketBase::ConnectionSucceeded ()
{ // Wrapper to protected function NotifyConnectionSucceeded() so that it can
  // be called as a scheduled event
  NotifyConnectionSucceeded ();
  // The if-block below was moved from ProcessSynSent() to here because we need
  // to invoke the NotifySend() only after NotifyConnectionSucceeded() to
  // reflect the behaviour in the real world.
  if (GetTxAvailable () > 0)
    {
      NotifySend (GetTxAvailable ());
    }
}

/** Extract at most maxSize bytes from the TxBuffer at sequence seq, add the
    TCP header, and send to TcpL4Protocol */
uint32_t
DoTcpSocketBase::SendDataPacket (SequenceNumber32 seq, uint32_t maxSize, bool withAck)
{
  NS_LOG_FUNCTION (this << seq << maxSize << withAck);

  Ptr<Packet> p = m_txBuffer.CopyFromSequence (maxSize, seq);
  uint32_t sz = p->GetSize (); // Size of packet
  uint8_t flags = withAck ? DoTcpHeader::ACK : 0;
  uint32_t remainingData = m_txBuffer.SizeFromSequence (seq + SequenceNumber32 (sz));

  /*
   * Add tags for each socket option.
   * Note that currently the socket adds both IPv4 tag and IPv6 tag
   * if both options are set. Once the packet got to layer three, only
   * the corresponding tags will be read.
   */
  if (IsManualIpTos ())
    {
      SocketIpTosTag ipTosTag;
      ipTosTag.SetTos (GetIpTos ());
      p->AddPacketTag (ipTosTag);
    }

  if (IsManualIpTtl ())
    {
      SocketIpTtlTag ipTtlTag;
      ipTtlTag.SetTtl (GetIpTtl ());
      p->AddPacketTag (ipTtlTag);
    }

  if (m_closeOnEmpty && (remainingData == 0))
    {
      flags |= DoTcpHeader::FIN;
      if (m_state == ESTABLISHED)
        { // On active close: I am the first one to send FIN
          NS_LOG_INFO ("ESTABLISHED -> FIN_WAIT_1");
          m_state = FIN_WAIT_1;
        }
      else if (m_state == CLOSE_WAIT)
        { // On passive close: Peer sent me FIN already
          NS_LOG_INFO ("CLOSE_WAIT -> LAST_ACK");
          m_state = LAST_ACK;
        }
    }
  DoTcpHeader header;
  header.SetFlags (flags);
  header.SetSequenceNumber (seq);
  header.SetAckNumber (m_rxBuffer.NextRxSequence ());
  if (m_endPoint)
    {
      header.SetSourcePort (m_endPoint->GetLocalPort ());
      header.SetDestinationPort (m_endPoint->GetPeerPort ());
    }
  header.SetWindowSize (AdvertisedWindowSize ());
  AddOptions (header);
  if (m_retxEvent.IsExpired () )
    { // Schedule retransmit
      m_rto = m_rtt->RetransmitTimeout ();
      NS_LOG_LOGIC (this << " SendDataPacket Schedule ReTxTimeout at time " <<
                    Simulator::Now ().GetSeconds () << " to expire at time " <<
                    (Simulator::Now () + m_rto.Get ()).GetSeconds () );
      m_retxEvent = Simulator::Schedule (m_rto, &DoTcpSocketBase::ReTxTimeout, this);
    }
  NS_LOG_LOGIC ("Send packet via DoTcpL4Protocol with flags 0x" << std::hex << static_cast<uint32_t> (flags) << std::dec);
  if (m_endPoint)
    {
      m_tcp->SendPacket (p, header, m_endPoint->GetLocalAddress (),
                         m_endPoint->GetPeerAddress (), m_boundnetdevice);
    }
  m_rtt->SentSeq (seq, sz);       // notify the RTT
  // Notify the application of the data being sent unless this is a retransmit
  if (seq == m_nextTxSequence)
    {
      Simulator::ScheduleNow (&DoTcpSocketBase::NotifyDataSent, this, sz);
    }
  // Update highTxMark
  m_highTxMark = std::max (seq + sz, m_highTxMark.Get ());
  return sz;
}

/** Send as much pending data as possible according to the Tx window. Note that
 *  this function did not implement the PSH flag
 */
bool
DoTcpSocketBase::SendPendingData (bool withAck)
{
  NS_LOG_FUNCTION (this << withAck);
  if (m_txBuffer.Size () == 0)
    {
      return false;                           // Nothing to send

    }
  if (m_endPoint == 0)
    {
      NS_LOG_INFO ("DoTcpSocketBase::SendPendingData: No endpoint; m_shutdownSend=" << m_shutdownSend);
      return false; // Is this the right way to handle this condition?
    }
  uint32_t nPacketsSent = 0;
  while (m_txBuffer.SizeFromSequence (m_nextTxSequence))
    {
      uint32_t w = AvailableWindow (); // Get available window size
      NS_LOG_LOGIC ("DoTcpSocketBase " << this << " SendPendingData" <<
                    " w " << w <<
                    " rxwin " << m_rWnd <<
                    " segsize " << m_segmentSize <<
                    " nextTxSeq " << m_nextTxSequence <<
                    " highestRxAck " << m_txBuffer.HeadSequence () <<
                    " pd->Size " << m_txBuffer.Size () <<
                    " pd->SFS " << m_txBuffer.SizeFromSequence (m_nextTxSequence));
      // Stop sending if we need to wait for a larger Tx window (prevent silly window syndrome)
      if (w < m_segmentSize && m_txBuffer.SizeFromSequence (m_nextTxSequence) > w)
        {
          break; // No more
        }
      // Nagle's algorithm (RFC896): Hold off sending if there is unacked data
      // in the buffer and the amount of data to send is less than one segment
      if (!m_noDelay && UnAckDataCount () > 0
          && m_txBuffer.SizeFromSequence (m_nextTxSequence) < m_segmentSize)
        {
          NS_LOG_LOGIC ("Invoking Nagle's algorithm. Wait to send.");
          break;
        }
      uint32_t s = std::min (w, m_segmentSize);  // Send no more than window
      uint32_t sz = SendDataPacket (m_nextTxSequence, s, withAck);
      nPacketsSent++;                             // Count sent this loop
      m_nextTxSequence += sz;                     // Advance next tx sequence
    }
  NS_LOG_LOGIC ("SendPendingData sent " << nPacketsSent << " packets");
  return (nPacketsSent > 0);
}

uint32_t
DoTcpSocketBase::UnAckDataCount ()
{
  NS_LOG_FUNCTION (this);
  return m_nextTxSequence.Get () - m_txBuffer.HeadSequence ();
}

uint32_t
DoTcpSocketBase::BytesInFlight ()
{
  NS_LOG_FUNCTION (this);
  return m_highTxMark.Get () - m_txBuffer.HeadSequence ();
}

uint32_t
DoTcpSocketBase::Window ()
{
  NS_LOG_FUNCTION (this);
  return m_rWnd;
}

uint32_t
DoTcpSocketBase::AvailableWindow ()
{
  NS_LOG_FUNCTION_NOARGS ();
  uint32_t unack = UnAckDataCount (); // Number of outstanding bytes
  uint32_t win = Window (); // Number of bytes allowed to be outstanding
  NS_LOG_LOGIC ("UnAckCount=" << unack << ", Win=" << win);
  return (win < unack) ? 0 : (win - unack);
}

uint16_t
DoTcpSocketBase::AdvertisedWindowSize ()
{
  return std::min (m_rxBuffer.MaxBufferSize () - m_rxBuffer.Size (), (uint32_t)m_maxWinSize);
}

// Receipt of new packet, put into Rx buffer
void
DoTcpSocketBase::ReceivedData (Ptr<Packet> p, const DoTcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);
  NS_LOG_LOGIC ("seq " << tcpHeader.GetSequenceNumber () <<
                " ack " << tcpHeader.GetAckNumber () <<
                " pkt size " << p->GetSize () );

  // Put into Rx buffer
  SequenceNumber32 expectedSeq = m_rxBuffer.NextRxSequence ();
  if (!m_rxBuffer.Add (p, tcpHeader))
    { // Insert failed: No data or RX buffer full
      SendEmptyPacket (DoTcpHeader::ACK);
      return;
    }
  // Now send a new ACK packet acknowledging all received and delivered data
  if (m_rxBuffer.Size () > m_rxBuffer.Available () || m_rxBuffer.NextRxSequence () > expectedSeq + p->GetSize ())
    { // A gap exists in the buffer, or we filled a gap: Always ACK
      SendEmptyPacket (DoTcpHeader::ACK);
    }
  else
    { // In-sequence packet: ACK if delayed ack count allows
      if (++m_delAckCount >= m_delAckMaxCount)
        {
          m_delAckEvent.Cancel ();
          m_delAckCount = 0;
          SendEmptyPacket (DoTcpHeader::ACK);
        }
      else if (m_delAckEvent.IsExpired ())
        {
          m_delAckEvent = Simulator::Schedule (m_delAckTimeout,
                                               &DoTcpSocketBase::DelAckTimeout, this);
          NS_LOG_LOGIC (this << " scheduled delayed ACK at " << (Simulator::Now () + Simulator::GetDelayLeft (m_delAckEvent)).GetSeconds ());
        }
    }
  // Notify app to receive if necessary
  if (expectedSeq < m_rxBuffer.NextRxSequence ())
    { // NextRxSeq advanced, we have something to send to the app
      if (!m_shutdownRecv)
        {
          NotifyDataRecv ();
        }
      // Handle exceptions
      if (m_closeNotified)
        {
          NS_LOG_WARN ("Why TCP " << this << " got data after close notification?");
        }
      // If we received FIN before and now completed all "holes" in rx buffer,
      // invoke peer close procedure
      if (m_rxBuffer.Finished () && (tcpHeader.GetFlags () & DoTcpHeader::FIN) == 0)
        {
          DoPeerClose ();
        }
    }
}

/** Called by ForwardUp() to estimate RTT */
void
DoTcpSocketBase::EstimateRtt (const DoTcpHeader& tcpHeader)
{
  // Use m_rtt for the estimation. Note, RTT of duplicated acknowledgement
  // (which should be ignored) is handled by m_rtt. Once timestamp option
  // is implemented, this function would be more elaborated.
  Time nextRtt =  m_rtt->AckSeq (tcpHeader.GetAckNumber () );

  //nextRtt will be zero for dup acks.  Don't want to update lastRtt in that case
  //but still needed to do list clearing that is done in AckSeq. 
  if(nextRtt != 0)
  {
    m_lastRtt = nextRtt;
    NS_LOG_FUNCTION(this << m_lastRtt);
  }
  
}

// Called by the ReceivedAck() when new ACK received and by ProcessSynRcvd()
// when the three-way handshake completed. This cancels retransmission timer
// and advances Tx window
void
DoTcpSocketBase::NewAck (SequenceNumber32 const& ack)
{
  NS_LOG_FUNCTION (this << ack);

  if (m_state != SYN_RCVD)
    { // Set RTO unless the ACK is received in SYN_RCVD state
      NS_LOG_LOGIC (this << " Cancelled ReTxTimeout event which was set to expire at " <<
                    (Simulator::Now () + Simulator::GetDelayLeft (m_retxEvent)).GetSeconds ());
      m_retxEvent.Cancel ();
      // On recieving a "New" ack we restart retransmission timer .. RFC 2988
      m_rto = m_rtt->RetransmitTimeout ();
      NS_LOG_LOGIC (this << " Schedule ReTxTimeout at time " <<
                    Simulator::Now ().GetSeconds () << " to expire at time " <<
                    (Simulator::Now () + m_rto.Get ()).GetSeconds ());
      m_retxEvent = Simulator::Schedule (m_rto, &DoTcpSocketBase::ReTxTimeout, this);
    }
  if (m_rWnd.Get () == 0 && m_persistEvent.IsExpired ())
    { // Zero window: Enter persist state to send 1 byte to probe
      NS_LOG_LOGIC (this << "Enter zerowindow persist state");
      NS_LOG_LOGIC (this << "Cancelled ReTxTimeout event which was set to expire at " <<
                    (Simulator::Now () + Simulator::GetDelayLeft (m_retxEvent)).GetSeconds ());
      m_retxEvent.Cancel ();
      NS_LOG_LOGIC ("Schedule persist timeout at time " <<
                    Simulator::Now ().GetSeconds () << " to expire at time " <<
                    (Simulator::Now () + m_persistTimeout).GetSeconds ());
      m_persistEvent = Simulator::Schedule (m_persistTimeout, &DoTcpSocketBase::PersistTimeout, this);
      NS_ASSERT (m_persistTimeout == Simulator::GetDelayLeft (m_persistEvent));
    }
  // Note the highest ACK and tell app to send more
  NS_LOG_LOGIC ("TCP " << this << " NewAck " << ack <<
                " numberAck " << (ack - m_txBuffer.HeadSequence ())); // Number bytes ack'ed
  m_txBuffer.DiscardUpTo (ack);
  if (GetTxAvailable () > 0)
    {
      NotifySend (GetTxAvailable ());
    }
  if (ack > m_nextTxSequence)
    {
      m_nextTxSequence = ack; // If advanced
    }
  if (m_txBuffer.Size () == 0 && m_state != FIN_WAIT_1 && m_state != CLOSING)
    { // No retransmit timer if no data to retransmit
      NS_LOG_LOGIC (this << " Cancelled ReTxTimeout event which was set to expire at " <<
                    (Simulator::Now () + Simulator::GetDelayLeft (m_retxEvent)).GetSeconds ());
      m_retxEvent.Cancel ();
    }
  // Try to send more data
  SendPendingData (m_connected);
}

// Retransmit timeout
void
DoTcpSocketBase::ReTxTimeout ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ReTxTimeout Expired at time " << Simulator::Now ().GetSeconds ());
  // If erroneous timeout in closed/timed-wait state, just return
  if (m_state == CLOSED || m_state == TIME_WAIT)
    {
      return;
    }
  // If all data are received (non-closing socket and nothing to send), just return
  if (m_state <= ESTABLISHED && m_txBuffer.HeadSequence () >= m_highTxMark)
    {
      return;
    }

  Retransmit ();
}

void
DoTcpSocketBase::DelAckTimeout (void)
{
  m_delAckCount = 0;
  SendEmptyPacket (DoTcpHeader::ACK);
}

void
DoTcpSocketBase::LastAckTimeout (void)
{
  NS_LOG_FUNCTION (this);

  m_lastAckEvent.Cancel ();
  if (m_state == LAST_ACK)
    {
      CloseAndNotify ();
    }
  if (!m_closeNotified)
    {
      m_closeNotified = true;
    }
}

// Send 1-byte data to probe for the window size at the receiver when
// the local knowledge tells that the receiver has zero window size
// C.f.: RFC793 p.42, RFC1112 sec.4.2.2.17
void
DoTcpSocketBase::PersistTimeout ()
{
  NS_LOG_LOGIC ("PersistTimeout expired at " << Simulator::Now ().GetSeconds ());
  m_persistTimeout = std::min (Seconds (60), Time (2 * m_persistTimeout)); // max persist timeout = 60s
  Ptr<Packet> p = m_txBuffer.CopyFromSequence (1, m_nextTxSequence);
  DoTcpHeader tcpHeader;
  tcpHeader.SetSequenceNumber (m_nextTxSequence);
  tcpHeader.SetAckNumber (m_rxBuffer.NextRxSequence ());
  tcpHeader.SetWindowSize (AdvertisedWindowSize ());
  if (m_endPoint != 0)
    {
      tcpHeader.SetSourcePort (m_endPoint->GetLocalPort ());
      tcpHeader.SetDestinationPort (m_endPoint->GetPeerPort ());
    }
  AddOptions (tcpHeader);

  if (m_endPoint != 0)
    {
      m_tcp->SendPacket (p, tcpHeader, m_endPoint->GetLocalAddress (),
                         m_endPoint->GetPeerAddress (), m_boundnetdevice);
    }
  NS_LOG_LOGIC ("Schedule persist timeout at time "
                << Simulator::Now ().GetSeconds () << " to expire at time "
                << (Simulator::Now () + m_persistTimeout).GetSeconds ());
  m_persistEvent = Simulator::Schedule (m_persistTimeout, &DoTcpSocketBase::PersistTimeout, this);
}

void
DoTcpSocketBase::Retransmit ()
{
  m_nextTxSequence = m_txBuffer.HeadSequence (); // Start from highest Ack
  m_rtt->IncreaseMultiplier (); // Double the timeout value for next retx timer
  m_dupAckCount = 0;
  DoRetransmit (); // Retransmit the packet
}

void
DoTcpSocketBase::DoRetransmit ()
{
  NS_LOG_FUNCTION (this);
  // Retransmit SYN packet
  if (m_state == SYN_SENT)
    {
      if (m_cnCount > 0)
        {
          SendEmptyPacket (DoTcpHeader::SYN);
        }
      else
        {
          NotifyConnectionFailed ();
        }
      return;
    }
  // Retransmit non-data packet: Only if in FIN_WAIT_1 or CLOSING state
  if (m_txBuffer.Size () == 0)
    {
      if (m_state == FIN_WAIT_1 || m_state == CLOSING)
        { // Must have lost FIN, re-send
          SendEmptyPacket (DoTcpHeader::FIN);
        }
      return;
    }
  // Retransmit a data packet: Call SendDataPacket
  NS_LOG_LOGIC ("DoTcpSocketBase " << this << " retxing seq " << m_txBuffer.HeadSequence ());
  uint32_t sz = SendDataPacket (m_txBuffer.HeadSequence (), m_segmentSize, true);
  // In case of RTO, advance m_nextTxSequence
  m_nextTxSequence = std::max (m_nextTxSequence.Get (), m_txBuffer.HeadSequence () + sz);

}

void
DoTcpSocketBase::CancelAllTimers ()
{
  m_retxEvent.Cancel ();
  m_persistEvent.Cancel ();
  m_delAckEvent.Cancel ();
  m_lastAckEvent.Cancel ();
  m_timewaitEvent.Cancel ();
}

/** Move TCP to Time_Wait state and schedule a transition to Closed state */
void
DoTcpSocketBase::TimeWait ()
{
  NS_LOG_INFO (TcpStateName[m_state] << " -> TIME_WAIT");
  m_state = TIME_WAIT;
  CancelAllTimers ();
  // Move from TIME_WAIT to CLOSED after 2*MSL. Max segment lifetime is 2 min
  // according to RFC793, p.28
  m_timewaitEvent = Simulator::Schedule (Seconds (2 * m_msl),
                                         &DoTcpSocketBase::CloseAndNotify, this);
}

/** Below are the attribute get/set functions */

void
DoTcpSocketBase::SetSndBufSize (uint32_t size)
{
  m_txBuffer.SetMaxBufferSize (size);
}

uint32_t
DoTcpSocketBase::GetSndBufSize (void) const
{
  return m_txBuffer.MaxBufferSize ();
}

void
DoTcpSocketBase::SetRcvBufSize (uint32_t size)
{
  m_rxBuffer.SetMaxBufferSize (size);
}

uint32_t
DoTcpSocketBase::GetRcvBufSize (void) const
{
  return m_rxBuffer.MaxBufferSize ();
}

void
DoTcpSocketBase::SetSegSize (uint32_t size)
{
  m_segmentSize = size;
  NS_ABORT_MSG_UNLESS (m_state == CLOSED, "Cannot change segment size dynamically.");
}

uint32_t
DoTcpSocketBase::GetSegSize (void) const
{
  return m_segmentSize;
}

void
DoTcpSocketBase::SetConnTimeout (Time timeout)
{
  m_cnTimeout = timeout;
}

Time
DoTcpSocketBase::GetConnTimeout (void) const
{
  return m_cnTimeout;
}

void
DoTcpSocketBase::SetConnCount (uint32_t count)
{
  m_cnRetries = count;
}

uint32_t
DoTcpSocketBase::GetConnCount (void) const
{
  return m_cnRetries;
}

void
DoTcpSocketBase::SetDelAckTimeout (Time timeout)
{
  m_delAckTimeout = timeout;
}

Time
DoTcpSocketBase::GetDelAckTimeout (void) const
{
  return m_delAckTimeout;
}

void
DoTcpSocketBase::SetDelAckMaxCount (uint32_t count)
{
  m_delAckMaxCount = count;
}

uint32_t
DoTcpSocketBase::GetDelAckMaxCount (void) const
{
  return m_delAckMaxCount;
}

void
DoTcpSocketBase::SetTcpNoDelay (bool noDelay)
{
  m_noDelay = noDelay;
}

bool
DoTcpSocketBase::GetTcpNoDelay (void) const
{
  return m_noDelay;
}

void
DoTcpSocketBase::SetPersistTimeout (Time timeout)
{
  m_persistTimeout = timeout;
}

Time
DoTcpSocketBase::GetPersistTimeout (void) const
{
  return m_persistTimeout;
}

bool
DoTcpSocketBase::SetAllowBroadcast (bool allowBroadcast)
{
  // Broadcast is not implemented. Return true only if allowBroadcast==false
  return (!allowBroadcast);
}

bool
DoTcpSocketBase::GetAllowBroadcast (void) const
{
  return false;
}

/** Placeholder function for future extension that reads more from the TCP header */
void
DoTcpSocketBase::ReadOptions (const DoTcpHeader&)
{
}

/** Placeholder function for future extension that changes the TCP header */
void
DoTcpSocketBase::AddOptions (DoTcpHeader&)
{
}

} // namespace ns3
