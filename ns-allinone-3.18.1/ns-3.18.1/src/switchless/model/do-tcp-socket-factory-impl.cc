/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "do-tcp-socket-factory-impl.h"
#include "do-tcp-l4-protocol.h"
#include "ns3/socket.h"
#include "ns3/assert.h"

namespace ns3 {

DoTcpSocketFactoryImpl::DoTcpSocketFactoryImpl ()
  : m_tcp (0)
{
}
DoTcpSocketFactoryImpl::~DoTcpSocketFactoryImpl ()
{
  NS_ASSERT (m_tcp == 0);
}

void
DoTcpSocketFactoryImpl::SetTcp (Ptr<DoTcpL4Protocol> tcp)
{
  m_tcp = tcp;
}

Ptr<Socket>
DoTcpSocketFactoryImpl::CreateSocket (void)
{
  return m_tcp->CreateSocket ();
}

void 
DoTcpSocketFactoryImpl::DoDispose (void)
{
  m_tcp = 0;
  DoTcpSocketFactory::DoDispose ();
}

} // namespace ns3
