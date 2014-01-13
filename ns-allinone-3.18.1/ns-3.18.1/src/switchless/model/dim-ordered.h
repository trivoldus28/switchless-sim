/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_H
#define DIM_ORDERED_H

// C/C++ includes
#include <tuple>

// NS3 includes
#include "ns3/object.h"
#include "ns3/net-device.h"
#include "ns3/socket.h"

// Switchless includes
#include "ns3/dim-ordered-interface-address.h"
#include "ns3/dim-ordered-header.h"
#include "ns3/dim-ordered-l4-protocol.h"

namespace ns3 {

/**
 * \ingroup switchless
 * \defgroup dimensionordered DimensionOrdered
 */
/**
 * \ingroup dimensionordered
 * \brief Access to the DimensionOrdered interfaces and configuration
 * 
 * This class defines the API to manipulate the following aspects of the
 * DimensionOrdered implementation:
 * -# register a NetDevice for use by the DimensionOrdered layer (basically, 
 * to create DimensionOrdered-related state)
 * -# manipulate the status of the NetDevice from the DimensionOrdered perspective,
 * such as marking it as Up or Down
 * -# adding, deleting, and getting addresses associated to the DimensionOrderedInterfaces
 *
 * Each NetDevice has conceptually a single DimensionOrderedInterface associatted with it.
 * Each interface may have one or more DimensionOrderedAddresses associatted with it (maybe? I dont think so).
 * Each DimensionOrderedAddress may ahve different scope, etc., so all of this per-address
 * information is stored in a DimensionOrderedInterfaceAddress class
 *
 * DimensionOrdered attributes such as whether forwarding is enabled and disabled are also stored in this class
 */
class DimensionOrdered : public Object
{
public:
  static TypeId GetTypeId (void);
  DimensionOrdered ();
  virtual ~DimensionOrdered();

  /**
   * \enum Interface Direction
   * \brief The direction a specific interface faces
   */
  enum InterfaceDirection
  {
      X_POS = 0,
      X_NEG,
      Y_POS,
      Y_NEG,
      Z_POS,
      Z_NEG,
      LOOPBACK,
      NUM_DIRS, // THIS MUST BE THE LAST ONE BEFORE INVALID
      INVALID_DIR
  };
  static std::string InterfaceDirectionToAscii (InterfaceDirection dir);

  /**
   * \param device device to add to the list of DimensionOrdered interface
   * which can be used as output interfaces during packet forwarding.
   * \param dir the direction this interface is associatted with for routing purposes
   * \returns the index of the DimensionOrdered interface added
   *
   * Once a device has been added, it can never be removed: if you want to
   * disable it, you can invoke DimensionOrdered::SetDown which will
   * make sure that it is never used during packet forwarding.
   */
  virtual void AddInterface (Ptr<NetDevice> device, InterfaceDirection dir) = 0;

  /**
   * \returns the interface for direction dir
   */
  virtual Ptr<DimensionOrderedInterface> GetInterface (InterfaceDirection dir) const = 0;

  /**
   * \returns the number of interfaces added by the user
   */
  virtual uint32_t GetNInterfaces (void) const = 0;

  /**
   * \brief Return the interface direction of the interface that has been
   * assigned the specific DimensionOrdered address.
   * 
   * \param address The DimensionOrdered address being searched for
   * \returns The interface direction of the DimensionOrdered interface witht the given
   * address (INVALID_DIR if not found)
   *
   * Each Dimension Ordered interface has one Dimension Ordered addresses associated with it.
   * This method searches the list of interfaces for one that holds a particular address. This call
   * call takes a Dimension Ordered address as a parameter and returns the interface direction of
   * the first interface that has been assigned that address, or INVALID_DIR if not found. There must be an exact
   * match; this method will not match broadcast addresses.
   */
  virtual InterfaceDirection GetInterfaceForAddress (DimensionOrderedAddress address) const = 0;

  /**
   * \param packet packet to send
   * \param source source address of packet
   * \param destination destination address of packet
   *
   * Higher-level layers call this method to send a packet down the stack to the
   * MAC and PHY layers.
   */
  virtual void Send (Ptr<Packet> packet, DimensionOrderedAddress source,
                     DimensionOrderedAddress destination, uint8_t protocol) = 0;

  /**
   * \param packet packet to send
   * \param header DimensionOrdered header
   *
   * Higher-level layers call this method to send a packet with IPv4 Header
   * (Intend to be used with DimensionOrderedHeaderInclude attribute.)
   */
  virtual void SendWithHeader (Ptr<Packet> packet, DimensionOrderedHeader header) = 0;

  /**
   * \param protocol a pointer to the protocol to add to this L4 Demux
   *
   * Adds a protocol to an internal list of L4 protocols.
   *
   */
  virtual void Insert (Ptr<DimensionOrderedL4Protocol> protocol) = 0;

  /**
   * \brief Determine whether address and interface corresponding to
   * received packet can be accepted for local delivery
   *
   * \param address The Dimension Ordered addres being considered
   * \param ifd The incoming DimensionOrdered interface direction
   *
   * This method can be used to determine whether a received packet has
   * an acceptable address for local delivery on the host.  The address
   * may be a unicast or broadcast address. This method will return true
   * if address is an exact match of a unicast address on one of the host's
   * interfaces or if address corresponds to a a broadcast address.
   */
  virtual bool IsDestinationAddress (DimensionOrderedAddress address, InterfaceDirection ifd) const = 0;

  /**
   * \param dir The direction of a DimensionOrderedInterface
   * \returns the NetDevice associatted with the DimensionOrderedInterface number.
   */
  virtual Ptr<NetDevice> GetNetDevice (InterfaceDirection dir) = 0;

  /**
   * \param device The NetDevice for a DimensionOrderedInterface
   * \returns The interface direction of a DimensionOrderedInterface (INVALID_DIR if not found)
   */
  virtual InterfaceDirection GetInterfaceForDevice (Ptr<const NetDevice> device) const = 0;

  /**
   * \param dir Interface direction of a DimensionOrderedInterface
   * \param address DimensionOrderedInterfaceAddress address to associate with the 
   * underlying DimensionOrdered interface
   * \returns true if the operation succeeded
   */
  virtual bool SetAddress (InterfaceDirection dir, DimensionOrderedInterfaceAddress address) = 0;

  /**
   * \param dir Interface direction of an DimensionOrderedInterface
   * \return the DimensionOrderedInterfaceAddress associated to the interface
   */
  virtual DimensionOrderedInterfaceAddress GetAddress (InterfaceDirection dir) const = 0;

  /**
   * \brief Return the first primary source address with scope less than or equal to the
   * requested scope, to use in sending a packet to destination dst out of a specified device.
   *
   * This method mirrors the behavior of Linux inet_select_addr() and is
   * provided because interfaces may have multiple addresses configured
   * on them with different scopes, and with a primary and secondary status.
   * Secondary addresses are never returned.
   *
   * If a non-zero device pointer is provided, the method first tries to
   * return a primary address that is configured on that device, and whose
   * subnet matches that of dst and whose scope is less than or equal to
   * the requested scope.  If a primary address does not match the
   * subnet of dst but otherwise matches the scope, it is returned.
   * If no such address on the device is found, the other devices are 
   * searched in order of their interface index, but not considering dst
   * as a factor in the search.  Because a loopback interface is typically 
   * the first one configured on a node, it will be the first alternate 
   * device to be tried.  Addresses scoped at LINK scope are not returned
   * in this phase.
   * 
   * If no device pointer is provided, the same logic as above applies, only
   * that there is no preferred device that is consulted first.  This means
   * that if the device pointer is null, input parameter dst will be ignored.
   * 
   * If there are no possible addresses to return, a warning log message 
   * is issued and the all-zeroes address is returned.
   *
   * \param device output NetDevice (optionally provided, only to constrain the search)
   * \param dst Destination address to match, if device is provided 
   * \param scope Scope of returned address must be less than or equal to this
   * \returns the first primary DimensionOrderedAddress that meets the search criteria
   */ 
  virtual DimensionOrderedAddress SelectSourceAddress (Ptr<const NetDevice> device,
                                                       DimensionOrderedAddress dst,
                                                       DimensionOrderedInterfaceAddress::InterfaceAddressScope_e scope) = 0;

  /**
   * \param interface Interface direction of a DimensionOrderedInterface
   * \returns the Maximum Transmission Unit (in bytes) associated
   * to the underlying interface
   */
  virtual uint16_t GetMtu (InterfaceDirection dir) const = 0;

  /**
   * \param dir Interface direction of a DimensionOrderedInterface
   * \returns true if the underlying interface is in the "up" state,
   * false otherwise.
   */
  virtual bool IsUp (InterfaceDirection dir) const = 0;

  /**
   * \param dir Interface direction of a DimensionOrderedInterface
   * 
   * Set the interface into the "up" state. In this state, it is
   * considered valid during forwarding.
   */
  virtual void SetUp (InterfaceDirection dir) = 0;

  /**
   * \param dir Interface direction of a DimensionOrderedInterface
   * 
   * Set the interface into the "down" state. In this state, it is
   * ignored during forwarding.
   */
  virtual void SetDown (InterfaceDirection dir) = 0;

  /**
   * 
   * \param protocolNumber number of protocol to lookup
   * in this L4 Demux
   * \returns a matching L4 Protocol
   *
   * This method is typically called by lower layers
   * to forward packets up the stack to the right protocol.
   */
  virtual Ptr<DimensionOrderedL4Protocol> GetProtocol (int protocolNumber) const = 0;

  /**
   * \brief Creates a raw socket
   * 
   * \returns a smart pointer to the instantiated raw socket
   */
  virtual Ptr<Socket> CreateRawSocket (void) = 0;
  
  /**
   * \brief Deletes a particular raw socket
   * \param socket Smart pointer to the raw socket to be deleted
   */
  virtual void DeleteRawSocket (Ptr<Socket> socket) = 0;

  /**
   * \brief Sets the origin of the DimensionOrdered topology for use in routing
   * \param origin Tuple specifying the origin in the form (x,y,z)
   */
  virtual void SetOrigin (std::tuple<uint8_t, uint8_t, uint8_t> origin) = 0;

  /**
   * \brief Get the current origin of the DimensionOrdered topology
   * \returns Tuple specifiying the origin in the for (x,y,z)
   */
  virtual std::tuple<uint8_t, uint8_t, uint8_t> GetOrigin (void) const = 0;

  /**
   * \brief Sets the max value for each dimension
   * \param dimsMax Tuple specifying the maximum value for each dimension in the form (x,y,z)
   */
  virtual void SetDimensionsMax (std::tuple<uint8_t, uint8_t, uint8_t> dimsMax) = 0;

  /**
   * \brief Gets the current max value for each dimension
   * \returns Tuple specifying the max value for each dimension in the form (x,y,z)
   */
  virtual std::tuple<uint8_t, uint8_t, uint8_t> GetDimensionsMax (void) const = 0;
private:
};

} // namespace ns3

#endif /* DIM_ORDERED_H */

