/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_UDP_SOCKET_H
#define DO_UDP_SOCKET_H

#include "ns3/socket.h"
#include "ns3/traced-callback.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/object.h"

namespace ns3 {

/**
 * \ingroup socket
 *
 * \brief (abstract) base class of all DoUdpSockets
 *
 * This class exists solely for hosting DoUdpSocket attributes that can
 * be reused across different implementations, and for declaring
 * UDP-specific multicast API.
 */
class DoUdpSocket : public Socket
{
public:
  static TypeId GetTypeId (void);
 
  DoUdpSocket (void);
  virtual ~DoUdpSocket (void);
private:
  // Indirect the attribute setting and getting through private virtual methods
  virtual void SetRcvBufSize (uint32_t size) = 0;
  virtual uint32_t GetRcvBufSize (void) const = 0;
  virtual void SetMtuDiscover (bool discover) = 0;
  virtual bool GetMtuDiscover (void) const = 0;
};

} // namespace ns3

#endif /* DO_UDP_SOCKET_H */


