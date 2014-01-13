/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_RAW_SOCKET_FACTORY_IMPL_H
#define DIM_ORDERED_RAW_SOCKET_FACTORY_IMPL_H

// C/C++ includes

// NS3 includes

// Switchless includes
#include "ns3/dim-ordered.h"
#include "ns3/dim-ordered-raw-socket-factory.h"

namespace ns3 {

class DimensionOrderedRawSocketFactoryImpl : public DimensionOrderedRawSocketFactory
{
public:
  virtual Ptr<Socket> CreateSocket (void);
};

} // namespace ns3

#endif /* DIM_ORDERED_RAW_SOCKET_FACTORY_IMPL_H */
