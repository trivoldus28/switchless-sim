/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_TCP_SOCKET_FACTORY_H
#define DO_TCP_SOCKET_FACTORY_H

#include "ns3/socket-factory.h"

namespace ns3 {

class Socket;

/**
 * \ingroup socket
 *
 * \brief API to create TCP socket instances 
 *
 * This abstract class defines the API for TCP sockets.
 * This class also holds the global default variables used to
 * initialize newly created sockets, such as values that are
 * set through the sysctl or proc interfaces in Linux.

 * All TCP socket factory implementations must provide an implementation 
 * of CreateSocket
 * below, and should make use of the default values configured below.
 * 
 * \see DoTcpSocketFactoryImpl
 *
 */
class DoTcpSocketFactory : public SocketFactory
{
public:
  static TypeId GetTypeId (void);

};

} // namespace ns3

#endif /* DO_TCP_SOCKET_FACTORY_H */
