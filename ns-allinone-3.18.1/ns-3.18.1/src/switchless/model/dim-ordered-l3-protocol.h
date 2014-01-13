/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_L3_PROTOCOL_H
#define DIM_ORDERED_L3_PROTOCOL_H

// C/C++ includes

// NS3 includes
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/traced-callback.h"
#include "ns3/object-vector.h"

// Switchless includes
#include "ns3/dim-ordered.h"
#include "ns3/dim-ordered-interface.h"
#include "ns3/dim-ordered-l4-protocol.h"
#include "ns3/dim-ordered-raw-socket-impl.h"

namespace ns3 {

/**
 * \brief Implement the DimensionOrdered layer.
 *
 * This is the actual DimensionOrdered implementation.  It contains APIs to send
 * and receive packets at the DimensionOrdered layer, as well as APIs for routing.
 */
class DimensionOrderedL3Protocol : public DimensionOrdered
{
public:
  static TypeId GetTypeId (void);
  static const uint16_t PROT_NUMBER;

  DimensionOrderedL3Protocol ();
  virtual ~DimensionOrderedL3Protocol ();

  /**
   * \enum Drop Reason
   * \brief Reason why a packet has been dropped.
   */
  enum DropReason
  {
      DROP_NO_ROUTE = 1,
      DROP_INTERFACE_DOWN,
      DROP_ROUTE_ERROR
  };

  void SetNode (Ptr<Node> node);

  // functions defined in base class DimensionOrdered

  Ptr<Socket> CreateRawSocket (void);
  void DeleteRawSocket (Ptr<Socket> socket);

  /**
   * \param protocol a template for the protocol to add to this L4 Demux
   * \returns the L4Protocol effectively added
   *
   * Invoke Copy on the input template to get a copy of the input
   * protocol which can be used on the Node on which this L4 Demux
   * is running.  The new L4Protocol is registered internally as
   * a working L4 Protocol and returned from this method.
   * The caller does not get ownership of the reurned pointer.
   */
  void Insert (Ptr<DimensionOrderedL4Protocol> protocol);

  /**
   * \param protocolNumber number of protocol to lookup
   * in this L4 Demux
   * \returns a matching L4 Protocol
   *
   * This method is typically called by lower layers 
   * to forward packets up the stack to the right protocol.
   */
  Ptr<DimensionOrderedL4Protocol> GetProtocol (int protocolNumber) const;

  /**
   * \param protocol protocol to remove from this demux.
   * 
   * The input value to this method should be the value
   * returned from the DimensionOrderedL4Protocol::Insert method.
   */
  void Remove (Ptr<DimensionOrderedL4Protocol> protocol);

  /**
   * Lower layer calls this method after calling L3Demux::Lookup
   * 
   * \param device network device
   * \param p the packet
   * \param from address of the correspondent
   * \param to address of the destination
   * \param packetType type of the packet
   */
  void Receive (Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, 
                const Address &to, NetDevice::PacketType packetType);

  /** 
   * \param packet packet to send
   * \param source source address of packet
   * \param destination address of packet
   *
   * Higher-level layers call this method to send a packet
   * down the stack to the MAC and PHY layers.
   */
  void Send (Ptr<Packet> packet, DimensionOrderedAddress source, 
             DimensionOrderedAddress destination, uint8_t protocol);

  /**
   * \param packet packet to send
   * \param header DimensionOrderedHeader
   * 
   * Higher-level layers call this method to send a pakcet with DimensionOrderedHeader
   * (Intend to be used with DimensionOrderedHeaderInclude attribute.)
   */
  void SendWithHeader (Ptr<Packet> packet, DimensionOrderedHeader header);

  void AddInterface (Ptr<NetDevice> device, InterfaceDirection dir);
  Ptr<DimensionOrderedInterface> GetInterface (InterfaceDirection dir) const;
  uint32_t GetNInterfaces (void) const;

  InterfaceDirection GetInterfaceForAddress (DimensionOrderedAddress addr) const;
  InterfaceDirection GetInterfaceForDevice (Ptr<const NetDevice> device) const;
  bool IsDestinationAddress (DimensionOrderedAddress address, InterfaceDirection ifd) const;

  bool SetAddress (InterfaceDirection dir, DimensionOrderedInterfaceAddress address);
  DimensionOrderedInterfaceAddress GetAddress (InterfaceDirection dir) const;
  DimensionOrderedAddress SelectSourceAddress (Ptr<const NetDevice> device, DimensionOrderedAddress dst,
                                               DimensionOrderedInterfaceAddress::InterfaceAddressScope_e scope);
  
  uint16_t GetMtu (InterfaceDirection dir) const;
  bool IsUp (InterfaceDirection dir) const;
  void SetUp (InterfaceDirection dir);
  void SetDown (InterfaceDirection dir);

  Ptr<NetDevice> GetNetDevice (InterfaceDirection dir);

  void SetOrigin (std::tuple<uint8_t, uint8_t, uint8_t> origin);
  std::tuple<uint8_t, uint8_t, uint8_t> GetOrigin (void) const;
  void SetDimensionsMax (std::tuple<uint8_t, uint8_t, uint8_t> dimsMax);
  std::tuple<uint8_t, uint8_t, uint8_t> GetDimensionsMax (void) const;

protected:

  virtual void DoDispose (void);

  /**
   * This function will notify other components conected to the node that a new stack member is now connected
   * This will be used to notify layer 3 protocol of layer 4 protocol stack to connect them together.
   */
  virtual void NotifyNewAggregate ();
private:
  DimensionOrderedL3Protocol (const DimensionOrderedL3Protocol &);
  DimensionOrderedL3Protocol &operator = (const DimensionOrderedL3Protocol &);

  DimensionOrderedHeader BuildHeader (
    DimensionOrderedAddress source,
    DimensionOrderedAddress destination,
    uint8_t protocol,
    uint16_t payloadSize);

  void SendRealOut (InterfaceDirection dir, Ptr<Packet> packet, DimensionOrderedHeader const &header);
  void Forward (Ptr<const Packet> p, const DimensionOrderedHeader &header);
  InterfaceDirection FindRoute (DimensionOrderedAddress destination);

  void LocalDeliver (Ptr<const Packet> p, DimensionOrderedHeader const &header, InterfaceDirection ifd);

  void AddDimensionOrderedInterface (Ptr<DimensionOrderedInterface> interface, InterfaceDirection dir);
  void SetupLoopback (void);

  Ptr<Icmpv4L4Protocol> GetIcmp (void) const;
  bool IsUnicast (DimensionOrderedAddress ad) const;

  typedef std::list<Ptr<DimensionOrderedRawSocketImpl> > SocketList;
  typedef std::list<Ptr<DimensionOrderedL4Protocol> > L4List_t;

  L4List_t m_protocols;
  Ptr<DimensionOrderedInterface> m_interfaces[NUM_DIRS];
  std::tuple<uint8_t, uint8_t, uint8_t> m_origin;
  std::tuple<uint8_t, uint8_t, uint8_t> m_dimsMax;
  Ptr<Node> m_node;

  TracedCallback<const DimensionOrderedHeader &, Ptr<const Packet>, InterfaceDirection> m_sendOutgoingTrace;
  TracedCallback<const DimensionOrderedHeader &, Ptr<const Packet>, uint32_t> m_unicastForwardTrace;
  TracedCallback<const DimensionOrderedHeader &, Ptr<const Packet>, InterfaceDirection> m_localDeliverTrace;

  TracedCallback<Ptr<const Packet>, Ptr<DimensionOrdered>, InterfaceDirection> m_txTrace;
  TracedCallback<Ptr<const Packet>, Ptr<DimensionOrdered>, InterfaceDirection> m_rxTrace;
  TracedCallback<const DimensionOrderedHeader &, Ptr<const Packet>, DropReason, Ptr<DimensionOrdered>, 
                 InterfaceDirection> m_dropTrace;

  SocketList m_sockets;

};

}

#endif /* DIM_ORDERED_L3_PROTOCOL_H */

