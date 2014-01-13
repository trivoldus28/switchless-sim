/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-raw-socket-factory.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedRawSocketFactory");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DimensionOrderedRawSocketFactory);

TypeId
DimensionOrderedRawSocketFactory::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::DimensionOrderedRawSocketFactory")
        .SetParent<SocketFactory> ()
    ;
    return tid;
}

} // namespace ns3
