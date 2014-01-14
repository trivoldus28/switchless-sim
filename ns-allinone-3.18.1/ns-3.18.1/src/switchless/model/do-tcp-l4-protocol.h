/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_TCP_L4_PROTOCOL_H
#define DO_TCP_L4_PROTOCOL_H

#include <stdint.h>

#include "ns3/packet.h"
#include "ns3/dim-ordered-address.h"
#include "ns3/ptr.h"
#include "ns3/object-factory.h"
#include "dim-ordered-l4-protocol.h"
#include "ns3/net-device.h"

namespace ns3 {

class Node;
class Socket;
class DoTcpHeader;
class DimensionOrderedEndPointDemux;
class DimensionOrderedInterface;
class DoTcpSocketBase;
class DimensionOrderedEndPoint;

/**
 * \ingroup tcp
 * \brief A layer between the sockets interface and DimensionOrdered
 * 
 * This class allocates "endpoint" objects (ns3::DimensionOrderedEndPoint) for TCP,
 * and SHOULD checksum packets its receives from the socket layer going down
 * the stack , but currently checksumming is disabled.  It also receives 
 * packets from IP, and forwards them up to the endpoints.
*/

class DoTcpL4Protocol : public DimensionOrderedL4Protocol {
public:
  static TypeId GetTypeId (void);
  static const uint8_t PROT_NUMBER;
  /**
   * \brief Constructor
   */
  DoTcpL4Protocol ();
  virtual ~DoTcpL4Protocol ();

  void SetNode (Ptr<Node> node);

  virtual int GetProtocolNumber (void) const;

  /**
   * \return A smart Socket pointer to a TcpSocket allocated by this instance
   * of the TCP protocol
   */
  Ptr<Socket> CreateSocket (void);
  Ptr<Socket> CreateSocket (TypeId socketTypeId);

  DimensionOrderedEndPoint *Allocate (void);
  DimensionOrderedEndPoint *Allocate (DimensionOrderedAddress address);
  DimensionOrderedEndPoint *Allocate (uint16_t port);
  DimensionOrderedEndPoint *Allocate (DimensionOrderedAddress address, uint16_t port);
  DimensionOrderedEndPoint *Allocate (DimensionOrderedAddress localAddress, uint16_t localPort,
                                      DimensionOrderedAddress peerAddress, uint16_t peerPort);

  void DeAllocate (DimensionOrderedEndPoint *endPoint);

  /**
   * \brief Send a packet via TCP
   * \param packet The packet to send
   * \param saddr The source DimensionOrderedAddress
   * \param daddr The destination DimensionOrderedAddress
   * \param sport The source port number
   * \param dport The destination port number
   * \param oif The output interface bound. Defaults to null (unspecified).
   */
  void Send (Ptr<Packet> packet,
             DimensionOrderedAddress saddr, DimensionOrderedAddress daddr, 
             uint16_t sport, uint16_t dport, Ptr<NetDevice> oif = 0);
  /**
   * \brief Receive a packet up the protocol stack
   * \param p The Packet to dump the contents into
   * \param header DimensionOrdered Header information
   * \param incomingInterface The DimensionOrderedInterface it was received on
   */
  virtual enum DimensionOrderedL4Protocol::RxStatus Receive (Ptr<Packet> p,
                                                 DimensionOrderedHeader const &header,
                                                 Ptr<DimensionOrderedInterface> incomingInterface);

  // From DimensionOrderedL4Protocol
  virtual void SetDownTarget (DimensionOrderedL4Protocol::DownTargetCallback cb);
  // From DimensionOrderedL4Protocol
  virtual DimensionOrderedL4Protocol::DownTargetCallback GetDownTarget (void) const;

protected:
  virtual void DoDispose (void);
  /* 
   * This function will notify other components connected to the node that a new stack member is now connected
   * This will be used to notify Layer 3 protocol of layer 4 protocol stack to connect them together.
   */
  virtual void NotifyNewAggregate ();
private:
  Ptr<Node> m_node;
  DimensionOrderedEndPointDemux *m_endPoints;
  TypeId m_rttTypeId;
  TypeId m_socketTypeId;
private:
  friend class DoTcpSocketBase;
  void SendPacket (Ptr<Packet>, const DoTcpHeader &,
                   DimensionOrderedAddress, DimensionOrderedAddress, Ptr<NetDevice> oif = 0);
  DoTcpL4Protocol (const DoTcpL4Protocol &o);
  DoTcpL4Protocol &operator = (const DoTcpL4Protocol &o);

  std::vector<Ptr<DoTcpSocketBase> > m_sockets;
  DimensionOrderedL4Protocol::DownTargetCallback m_downTarget;
};

} // namespace ns3

#endif /* TCP_L4_PROTOCOL_H */
