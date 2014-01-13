/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_INTERFACE_H
#define DIM_ORDERED_INTERFACE_H

// C/C++ includes

// NS3 includes
#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/loopback-net-device.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/object.h"

// Switchless includes
#include "ns3/dim-ordered-interface-address.h"

namespace ns3 {

/**
 * \brief The dimension ordered representation of a network interface
 *
 * The main purpose of this class is to provde address-family specific
 * information (addresses) about an interface.
 *
 * By default, DimensionOrderedInterfaces are created in the "down" state
 * no DimensionOrderedAddress. Before becoming useable, the user must
 * add an address of some type and invoke Setup on them.
 */
class DimensionOrderedInterface  : public Object 
{
public:
  static TypeId GetTypeId (void);

  // Constructors and Destructors
  DimensionOrderedInterface ();
  virtual ~DimensionOrderedInterface ();

  /**
   * \brief Sets the node the interface is installed on
   * \param node Node to install interface on
   */
  void SetNode (Ptr<Node> node);
  /**
   * \brief Sets the underlying NetDevice
   * \param device Underlying NetDevice to set
   */
  void SetDevice (Ptr<NetDevice> device);

  /**
   * \returns the underlying NetDevice. This method caonnot return zero.
   */
  Ptr<NetDevice> GetDevice (void) const;

  /**
   * These are DimensionOrderedInterface states and may be distinct from
   * NetDevice states, such as found in real implementations (where the device may
   * be down but the DimensionOrderedInterface state is still up).
   *
   * \returns true if this interface is enabled, false otherwise.
   */
   bool IsUp (void) const;

   /**
    * \returns true of this interface is disabled, false otherwise.
    */
   bool IsDown (void) const;

   /**
    * \brief Enable this interface
    */
   void SetUp (void);

   /**
    * \brief Disable this interface
    */
   void SetDown (void);

   /**
    * \param p packet to send
    * \param dest next hop address of packet.
    *
    * This method will eventually call the private
    * SendTo method which must be implemented by subclasses.
    */
   void Send (Ptr<Packet> p, DimensionOrderedAddress dest);

   /**
    * \param address set the DimensionOrderedInterfaceAddress for this interface
    * \returns true if succeeded
    */
   bool SetAddress (DimensionOrderedInterfaceAddress address);

   /**
    * \returns The DimensionOrderedInterfaceAddress whose index is index
    */
   DimensionOrderedInterfaceAddress GetAddress () const;

protected:
  virtual void DoDispose (void);
private:
  bool m_ifup;
  DimensionOrderedInterfaceAddress m_address;
  Ptr<Node> m_node;
  Ptr<NetDevice> m_device;

};

} // namespace ns3

#endif /* DIM_ORDERED_INTERFACE_H */

