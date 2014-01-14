/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_TCP_SOCKET_FACTORY_IMPL_H
#define DO_TCP_SOCKET_FACTORY_IMPL_H

#include "ns3/do-tcp-socket-factory.h"
#include "ns3/ptr.h"

namespace ns3 {

class DoTcpL4Protocol;

/**
 * \ingroup switchless
 * \defgroup tcp Tcp
 *
 * This class serves to create sockets of the DoTcpSocketBase type.
 */

/**
 * \ingroup tcp
 *
 * \brief socket factory implementation for native ns-3 TCP for the DimensionOrdered stack
 *
 */
class DoTcpSocketFactoryImpl : public DoTcpSocketFactory
{
public:
  DoTcpSocketFactoryImpl ();
  virtual ~DoTcpSocketFactoryImpl ();

  void SetTcp (Ptr<DoTcpL4Protocol> tcp);

  virtual Ptr<Socket> CreateSocket (void);

protected:
  virtual void DoDispose (void);
private:
  Ptr<DoTcpL4Protocol> m_tcp;
};

} // namespace ns3

#endif /* DO_TCP_SOCKET_FACTORY_IMPL_H */
