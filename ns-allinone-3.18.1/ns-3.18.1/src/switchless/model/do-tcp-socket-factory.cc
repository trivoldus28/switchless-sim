/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "do-tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoTcpSocketFactory);

TypeId
DoTcpSocketFactory::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoTcpSocketFactory")
    .SetParent<SocketFactory> ()
  ;
  return tid;
}

} // namespace ns3
