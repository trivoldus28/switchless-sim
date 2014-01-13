/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-raw-socket-factory-impl.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedRawSocketFactoryImpl");

namespace ns3 {

Ptr<Socket>
DimensionOrderedRawSocketFactoryImpl::CreateSocket (void)
{
    NS_LOG_FUNCTION (this);
    Ptr<DimensionOrdered> dimOrdered = GetObject<DimensionOrdered> ();
    Ptr<Socket> socket = dimOrdered->CreateRawSocket ();
    return socket;
}

} // namespace ns3
