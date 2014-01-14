/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/nstime.h"
#include "do-tcp-socket.h"

NS_LOG_COMPONENT_DEFINE ("DoTcpSocket");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoTcpSocket);

const char* const DoTcpSocket::TcpStateName[LAST_STATE] = { "CLOSED", "LISTEN", "SYN_SENT", "SYN_RCVD", "ESTABLISHED", "CLOSE_WAIT", "LAST_ACK", "FIN_WAIT_1", "FIN_WAIT_2", "CLOSING", "TIME_WAIT" };

TypeId
DoTcpSocket::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoTcpSocket")
    .SetParent<Socket> ()
    .AddAttribute ("SndBufSize",
                   "DoTcpSocket maximum transmit buffer size (bytes)",
                   UintegerValue (131072), // 128k
                   MakeUintegerAccessor (&DoTcpSocket::GetSndBufSize,
                                         &DoTcpSocket::SetSndBufSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RcvBufSize",
                   "DoTcpSocket maximum receive buffer size (bytes)",
                   UintegerValue (131072),
                   MakeUintegerAccessor (&DoTcpSocket::GetRcvBufSize,
                                         &DoTcpSocket::SetRcvBufSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SegmentSize",
                   "TCP maximum segment size in bytes (may be adjusted based on MTU discovery)",
                   UintegerValue (536),
                   MakeUintegerAccessor (&DoTcpSocket::GetSegSize,
                                         &DoTcpSocket::SetSegSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SlowStartThreshold",
                   "TCP slow start threshold (bytes)",
                   UintegerValue (0xffff),
                   MakeUintegerAccessor (&DoTcpSocket::GetSSThresh,
                                         &DoTcpSocket::SetSSThresh),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("InitialCwnd",
                   "TCP initial congestion window size (segments)",
                   UintegerValue (1),
                   MakeUintegerAccessor (&DoTcpSocket::GetInitialCwnd,
                                         &DoTcpSocket::SetInitialCwnd),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ConnTimeout",
                   "TCP retransmission timeout when opening connection (seconds)",
                   TimeValue (Seconds (3)),
                   MakeTimeAccessor (&DoTcpSocket::GetConnTimeout,
                                     &DoTcpSocket::SetConnTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("ConnCount",
                   "Number of connection attempts (SYN retransmissions) before returning failure",
                   UintegerValue (6),
                   MakeUintegerAccessor (&DoTcpSocket::GetConnCount,
                                         &DoTcpSocket::SetConnCount),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DelAckTimeout",
                   "Timeout value for TCP delayed acks, in seconds",
                   TimeValue (Seconds (0.2)),
                   MakeTimeAccessor (&DoTcpSocket::GetDelAckTimeout,
                                     &DoTcpSocket::SetDelAckTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("DelAckCount",
                   "Number of packets to wait before sending a TCP ack",
                   UintegerValue (2),
                   MakeUintegerAccessor (&DoTcpSocket::GetDelAckMaxCount,
                                         &DoTcpSocket::SetDelAckMaxCount),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("TcpNoDelay", "Set to true to disable Nagle's algorithm",
                   BooleanValue (true),
                   MakeBooleanAccessor (&DoTcpSocket::GetTcpNoDelay,
                                        &DoTcpSocket::SetTcpNoDelay),
                   MakeBooleanChecker ())
    .AddAttribute ("PersistTimeout",
                   "Persist timeout to probe for rx window",
                   TimeValue (Seconds (6)),
                   MakeTimeAccessor (&DoTcpSocket::GetPersistTimeout,
                                     &DoTcpSocket::SetPersistTimeout),
                   MakeTimeChecker ())
  ;
  return tid;
}

DoTcpSocket::DoTcpSocket ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

DoTcpSocket::~DoTcpSocket ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

} // namespace ns3
