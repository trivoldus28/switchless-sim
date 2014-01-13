/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "do-udp-socket.h"

NS_LOG_COMPONENT_DEFINE ("DoUdpSocket");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoUdpSocket);

TypeId
DoUdpSocket::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoUdpSocket")
    .SetParent<Socket> ()
    .AddAttribute ("RcvBufSize",
                   "DoUdpSocket maximum receive buffer size (bytes)",
                   UintegerValue (131072),
                   MakeUintegerAccessor (&DoUdpSocket::GetRcvBufSize,
                                         &DoUdpSocket::SetRcvBufSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MtuDiscover", "If enabled, every outgoing ip packet will have the DF flag set.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&DoUdpSocket::SetMtuDiscover,
                                        &DoUdpSocket::GetMtuDiscover),
                   MakeBooleanChecker ())
  ;
  return tid;
}

DoUdpSocket::DoUdpSocket ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

DoUdpSocket::~DoUdpSocket ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

} // namespace ns3
