/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_UDP_SOCKET_IMPL_H
#define DO_UDP_SOCKET_IMPL_H

#include <stdint.h>
#include <queue>
#include "ns3/callback.h"
#include "ns3/traced-callback.h"
#include "ns3/socket.h"
#include "ns3/ptr.h"
#include "ns3/dim-ordered-address.h"
#include "ns3/do-udp-socket.h"
#include "ns3/dim-ordered-interface.h"
#include "ns3/do-udp-l4-protocol.h"

namespace ns3 {

/**
 * \ingroup udp
 * \brief A sockets interface to Dimension Ordered UDP
 * 
 * This class subclasses ns3::DoUdpSocket, and provides a socket interface
 * to ns3's implementation of UDP.
 */

class DoUdpSocketImpl : public DoUdpSocket
{
public:
  static TypeId GetTypeId (void);
  /**
   * Create an unbound udp socket.
   */
  DoUdpSocketImpl ();
  virtual ~DoUdpSocketImpl ();

  void SetNode (Ptr<Node> node);
  void SetUdp (Ptr<DoUdpL4Protocol> udp);

  virtual enum SocketErrno GetErrno (void) const;
  virtual enum SocketType GetSocketType (void) const;
  virtual Ptr<Node> GetNode (void) const;
  virtual int Bind (void);
  virtual int Bind6 (void);
  virtual int Bind (const Address &address);
  virtual int Close (void);
  virtual int ShutdownSend (void);
  virtual int ShutdownRecv (void);
  virtual int Connect (const Address &address);
  virtual int Listen (void);
  virtual uint32_t GetTxAvailable (void) const;
  virtual int Send (Ptr<Packet> p, uint32_t flags);
  virtual int SendTo (Ptr<Packet> p, uint32_t flags, const Address &address);
  virtual uint32_t GetRxAvailable (void) const;
  virtual Ptr<Packet> Recv (uint32_t maxSize, uint32_t flags);
  virtual Ptr<Packet> RecvFrom (uint32_t maxSize, uint32_t flags,
                                Address &fromAddress);
  virtual int GetSockName (Address &address) const; 
  virtual void BindToNetDevice (Ptr<NetDevice> netdevice);
  virtual bool SetAllowBroadcast (bool allowBroadcast);
  virtual bool GetAllowBroadcast () const;

private:
  // Attributes set through UdpSocket base class 
  virtual void SetRcvBufSize (uint32_t size);
  virtual uint32_t GetRcvBufSize (void) const;
  virtual void SetMtuDiscover (bool discover);
  virtual bool GetMtuDiscover (void) const;


  friend class DoUdpSocketFactory;
  // invoked by Udp class
  int FinishBind (void);
  void ForwardUp (Ptr<Packet> p, DimensionOrderedHeader header, uint16_t port, 
                  Ptr<DimensionOrderedInterface> incomingInterface);
  void Destroy (void);
  int DoSend (Ptr<Packet> p);
  int DoSendTo (Ptr<Packet> p, const Address &daddr);
  int DoSendTo (Ptr<Packet> p, DimensionOrderedAddress daddr, uint16_t dport);

  DimensionOrderedEndPoint *m_endPoint;
  Ptr<Node> m_node;
  Ptr<DoUdpL4Protocol> m_udp;
  Address m_defaultAddress;
  uint16_t m_defaultPort;
  TracedCallback<Ptr<const Packet> > m_dropTrace;

  enum SocketErrno m_errno;
  bool m_shutdownSend;
  bool m_shutdownRecv;
  bool m_connected;
  bool m_allowBroadcast;

  std::queue<Ptr<Packet> > m_deliveryQueue;
  uint32_t m_rxAvailable;

  // Socket attributes
  uint32_t m_rcvBufSize;
  bool m_mtuDiscover;
};

} // namespace ns3

#endif /* DO_UDP_SOCKET_IMPL_H */
