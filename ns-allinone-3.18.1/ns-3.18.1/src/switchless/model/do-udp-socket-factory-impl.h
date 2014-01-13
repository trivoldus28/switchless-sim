/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_UDP_SOCKET_FACTORY_IMPL_H
#define DO_UDP_SOCKET_FACTORY_IMPL_H

#include "ns3/do-udp-socket-factory.h"
#include "ns3/ptr.h"

namespace ns3 {

class DoUdpL4Protocol;

/**
 * \ingroup switchless
 * \defgroup udp Udp
 *
 * This  is  an  implementation of the User Datagram Protocol described in
 * \RFC{768}.  It implements a connectionless,  unreliable  datagram  packet
 * service.   Packets  may  be reordered or duplicated before they arrive.
 * UDP generates and checks checksums to catch transmission errors.
 *
 * The following options are not presently part of this implementation:
 * UDP_CORK, MSG_DONTROUTE, path MTU discovery control (e.g. 
 * IP_MTU_DISCOVER).  MTU handling is also weak in ns-3 for the moment;
 * it is best to send datagrams that do not exceed 1500 byte MTU (e.g.
 * 1472 byte UDP datagrams)
 */

/**
 * \ingroup udp
 * \brief Object to create DimensionOrdered UDP socket instances 
 * \internal
 *
 * This class implements the API for creating DimensionOrdered UDP sockets.
 * It is a socket factory (deriving from class SocketFactory).
 */
class DoUdpSocketFactoryImpl : public DoUdpSocketFactory
{
public:
  DoUdpSocketFactoryImpl ();
  virtual ~DoUdpSocketFactoryImpl ();

  void SetUdp (Ptr<DoUdpL4Protocol> udp);

  /**
   * \brief Implements a method to create a Udp-based socket and return
   * a base class smart pointer to the socket.
   * \internal
   *
   * \return smart pointer to Socket
   */
  virtual Ptr<Socket> CreateSocket (void);

protected:
  virtual void DoDispose (void);
private:
  Ptr<DoUdpL4Protocol> m_udp;
};

} // namespace ns3

#endif /* DO_UDP_SOCKET_FACTORY_IMPL_H */
