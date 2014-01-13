/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_RAW_SOCKET_FACTORY_H
#define DIM_ORDERED_RAW_SOCKET_FACTORY_H

// C/C++ includes

// NS3 includes
#include "ns3/log.h"
#include "ns3/socket-factory.h"

// Switchless includes

namespace ns3 {

/**
 * \ingroup socket
 * 
 * \brief API to create RAW socket instances
 *
 * This abstract class defines the API for RAW socket factory.
 */
class DimensionOrderedRawSocketFactory : public SocketFactory
{
public:
  static TypeId GetTypeId (void);

};

} // namespace ns3

#endif /* DIM_ORDERED_RAW_SOCKET_FACTORY_H */
