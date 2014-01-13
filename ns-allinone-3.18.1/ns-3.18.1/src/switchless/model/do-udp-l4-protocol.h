/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_UDP_L4_PROTOCOL_H
#define DO_UDP_L4_PROTOCOL_H

#include <stdint.h>

#include "ns3/packet.h"
#include "ns3/dim-ordered-address.h"
#include "ns3/ptr.h"
#include "ns3/dim-ordered-l4-protocol.h"
#include "dim-ordered-interface.h"
#include "dim-ordered-header.h"
#include "dim-ordered-end-point.h"
#include "dim-ordered-end-point-demux.h"

namespace ns3 {

class DoUdpSocketImpl;

/**
 * \ingroup udp
 * \brief Implementation of the Dimension Ordered UDP protocol
 */
class DoUdpL4Protocol : public DimensionOrderedL4Protocol {
public:
  static TypeId GetTypeId (void);
  static const uint8_t PROT_NUMBER;

  DoUdpL4Protocol ();
  virtual ~DoUdpL4Protocol ();

  void SetNode (Ptr<Node> node);

  virtual int GetProtocolNumber (void) const;

  /**
   * \return A smart Socket pointer to a UdpSocket, allocated by this instance
   * of the UDP protocol
   */
  Ptr<Socket> CreateSocket (void);

  DimensionOrderedEndPoint *Allocate (void);
  DimensionOrderedEndPoint *Allocate (DimensionOrderedAddress address);
  DimensionOrderedEndPoint *Allocate (uint16_t port);
  DimensionOrderedEndPoint *Allocate (DimensionOrderedAddress address, uint16_t port);
  DimensionOrderedEndPoint *Allocate (DimensionOrderedAddress localAddress, uint16_t localPort,
                                      DimensionOrderedAddress peerAddress, uint16_t peerPort);

  void DeAllocate (DimensionOrderedEndPoint *endPoint);

  // called by UdpSocket.
  /**
   * \brief Send a packet via UDP
   * \param packet The packet to send
   * \param saddr The source DimensionOrderedAddress
   * \param daddr The destination DimensionOrderdAddress
   * \param sport The source port number
   * \param dport The destination port number
   */
  void Send (Ptr<Packet> packet,
             DimensionOrderedAddress saddr, DimensionOrderedAddress daddr, 
             uint16_t sport, uint16_t dport);
  /**
   * \brief Receive a packet up the protocol stack
   * \param p The Packet to dump the contents into
   * \param header DimensionOrdered Header information
   * \param interface the interface from which the packet is coming.
   */
  // inherited from DimensionOrderedL4Protocol
  virtual enum DimensionOrderedL4Protocol::RxStatus Receive (Ptr<Packet> p,
                                                             DimensionOrderedHeader const &header,
                                                             Ptr<DimensionOrderedInterface> interface);

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
  DoUdpL4Protocol (const DoUdpL4Protocol &o);
  DoUdpL4Protocol &operator = (const DoUdpL4Protocol &o);
  std::vector<Ptr<DoUdpSocketImpl> > m_sockets;
  DimensionOrderedL4Protocol::DownTargetCallback m_downTarget;
};

} // namespace ns3

#endif /* DO_UDP_L4_PROTOCOL_H */
