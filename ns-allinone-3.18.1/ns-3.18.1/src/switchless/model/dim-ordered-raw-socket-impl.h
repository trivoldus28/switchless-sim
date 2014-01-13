/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_RAW_SOCKET_IMPL_H
#define DIM_ORDERED_RAW_SOCKET_IMPL_H

// C/C++ incudes
#include <sys/socket.h>

// NS3 includes
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/icmpv4.h"

// Switchless includes
#include "ns3/dim-ordered-header.h"
#include "ns3/dim-ordered-interface.h"
#include "ns3/dim-ordered-socket-address.h"
#include "ns3/dim-ordered.h"

namespace ns3 {

class DimensionOrderedRawSocketImpl : public Socket
{
public:
  static TypeId GetTypeId (void);

  DimensionOrderedRawSocketImpl ();

  void SetNode (Ptr<Node> node);

  virtual enum Socket::SocketErrno GetErrno (void) const;
  virtual enum Socket::SocketType GetSocketType (void) const;
  virtual Ptr<Node> GetNode (void) const;
  virtual int Bind (const Address &address);
  virtual int Bind ();
  virtual int Bind6 ();
  virtual int GetSockName (Address &address) const;
  virtual int Close (void);
  virtual int ShutdownSend (void);
  virtual int ShutdownRecv (void);
  virtual int Connect (const Address &address);
  virtual int Listen (void);
  virtual uint32_t GetTxAvailable (void) const;
  virtual int Send (Ptr<Packet> p, uint32_t flags);
  virtual int SendTo (Ptr<Packet> p, uint32_t flags, const Address &toAddress);
  virtual uint32_t GetRxAvailable (void) const;
  virtual Ptr<Packet> Recv (uint32_t maxSize, uint32_t flags);
  virtual Ptr<Packet> RecvFrom (uint32_t maxSize, uint32_t flags, Address &fromAddress);
  void SetProtocol (uint16_t protocol);
  bool ForwardUp (Ptr<const Packet> p, DimensionOrderedHeader header, 
                  Ptr<DimensionOrderedInterface> incomingInterface);
  virtual bool SetAllowBroadcast (bool allowBroadcast);
  virtual bool GetAllowBroadcast () const;

private:
  virtual void DoDispose (void);

  struct Data {
    Ptr<Packet> packet;
    DimensionOrderedAddress fromAddress;
    uint16_t fromProtocol;
  };

  enum Socket::SocketErrno m_err;
  Ptr<Node> m_node;
  DimensionOrderedAddress m_src;
  DimensionOrderedAddress m_dst;
  uint16_t m_protocol;
  std::list<struct Data> m_recv;
  bool m_shutdownSend;
  bool m_shutdownRecv;
  //TODO: implement icmp?
  //uint32_t m_icmpFilter;
  bool m_dohdrincl;
};

} // namespace ns3

#endif /* DIM_ORDERED_RAW_SOCKET_IMPL_H */

