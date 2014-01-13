/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "do-udp-socket-factory-impl.h"
#include "do-udp-l4-protocol.h"
#include "ns3/socket.h"
#include "ns3/assert.h"

namespace ns3 {

DoUdpSocketFactoryImpl::DoUdpSocketFactoryImpl ()
  : m_udp (0)
{
}
DoUdpSocketFactoryImpl::~DoUdpSocketFactoryImpl ()
{
  NS_ASSERT (m_udp == 0);
}

void
DoUdpSocketFactoryImpl::SetUdp (Ptr<DoUdpL4Protocol> udp)
{
  m_udp = udp;
}

Ptr<Socket>
DoUdpSocketFactoryImpl::CreateSocket (void)
{
  return m_udp->CreateSocket ();
}

void 
DoUdpSocketFactoryImpl::DoDispose (void)
{
  m_udp = 0;
  DoUdpSocketFactory::DoDispose ();
}

} // namespace ns3
