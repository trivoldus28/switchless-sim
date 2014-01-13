/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-l4-protocol.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedL4Protocol");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DimensionOrderedL4Protocol);

TypeId
DimensionOrderedL4Protocol::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::DimensionOrderedL4Protocol")
      .SetParent<Object> ()
      .AddAttribute ("ProtocolNumber", "The DimensionOrdered protocol number.",
                     UintegerValue (0),
                     MakeUintegerAccessor (&DimensionOrderedL4Protocol::GetProtocolNumber),
                     MakeUintegerChecker<int> ())
    ;
    return tid;
}

DimensionOrderedL4Protocol::~DimensionOrderedL4Protocol ()
{
    NS_LOG_FUNCTION (this);
}

// TODO: Implement icmp?
/*void
DimensionOrderedL4Protocol::ReceiveIcmp (DimensionOrderedAddress icmpSource, uint8_t icmpTtl,
                                         uint8_t icmpType, uint8_t icmpCode, uint32_t icmpInfo,
                                         DimensionOrderedAddress payloadSource, 
                                         DimensionOrderedAddress payloadDestination,
                                         const uint8_t payload[8])
{
    NS_LOG_FUNCTION (this << icmpSource << static_cast<uint32_t> (icmpTtl) << static_cast<uint32_t> (icmpType) << static_cast<uint32_t> (icmpCode) << icmpInfo << payloadSource << payloadDestination << payload);
}*/

}  // namespace ns3
