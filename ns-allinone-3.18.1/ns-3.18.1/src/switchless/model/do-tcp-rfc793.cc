/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "do-tcp-rfc793.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("DoTcpRfc793");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoTcpRfc793);

TypeId
DoTcpRfc793::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoTcpRfc793")
    .SetParent<DoTcpSocketBase> ()
    .AddConstructor<DoTcpRfc793> ()
  ;
  return tid;
}

DoTcpRfc793::DoTcpRfc793 (void)
{
  NS_LOG_FUNCTION (this);
  SetDelAckMaxCount (0);  // Delayed ACK is not in RFC793
}

DoTcpRfc793::DoTcpRfc793 (const DoTcpRfc793& sock) : DoTcpSocketBase (sock)
{
}

DoTcpRfc793::~DoTcpRfc793 (void)
{
}

Ptr<DoTcpSocketBase>
DoTcpRfc793::Fork (void)
{
  return CopyObject<DoTcpRfc793> (this);
}

void
DoTcpRfc793::DupAck (const DoTcpHeader& t, uint32_t count)
{
}

void
DoTcpRfc793::SetSSThresh (uint32_t threshold)
{
  NS_LOG_WARN ("DoD TCP does not perform slow start");
}

uint32_t
DoTcpRfc793::GetSSThresh (void) const
{
  NS_LOG_WARN ("DoD TCP does not perform slow start");
  return 0;
}

void
DoTcpRfc793::SetInitialCwnd (uint32_t cwnd)
{
  NS_LOG_WARN ("DoD TCP does not have congestion window");
}

uint32_t
DoTcpRfc793::GetInitialCwnd (void) const
{
  NS_LOG_WARN ("DoD TCP does not have congestion window");
  return 0;
}

} // namespace ns3
