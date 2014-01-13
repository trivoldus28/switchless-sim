/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "do-udp-socket-factory.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoUdpSocketFactory);

TypeId DoUdpSocketFactory::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoUdpSocketFactory")
    .SetParent<SocketFactory> ()
  ;
  return tid;
}

} // namespace ns3
