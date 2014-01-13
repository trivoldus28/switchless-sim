/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_UDP_SOCKET_FACTORY_H
#define DO_UDP_SOCKET_FACTORY_H

#include "ns3/socket-factory.h"

namespace ns3 {

/**
 * \ingroup socket
 *
 * \brief API to create Dimension Ordered UDP socket instances 
 *
 * This abstract class defines the API for Dimension Ordered UDP socket factory.
 * All UDP implementations must provide an implementation of CreateSocket
 * below.
 * 
 * \see UdpSocketFactoryImpl
 */
class DoUdpSocketFactory : public SocketFactory
{
public:
  static TypeId GetTypeId (void);

};

} // namespace ns3

#endif /* DO_UDP_SOCKET_FACTORY_H */
