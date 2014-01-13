/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_SOCKET_ADDRESS_H
#define DIM_ORDERED_SOCKET_ADDRESS_H

// C/C++ includes

// NS3 includes

// Switchless includes
#include "dim-ordered-address.h"

namespace ns3 {

/**
 * \ingroup address
 *
 * \brief an DimensionOrderedSocket address class
 *
 * This class is similar to inet_sockaddr in the BSD socket
 * API. i.e., this class holds a DimensionOrderedAddress and a port number
 * to form a dimension ordered transport endpoint.
 */
class DimensionOrderedSocketAddress
{
public:
  /**
   * \param address the DimensionOrderedAddress
   * \param port the port number
   */
  DimensionOrderedSocketAddress (DimensionOrderedAddress address, uint16_t port);
  /**
   * \param address the DimensionOrderedAddress
   *
   * The port number is set to zero by default
   */
  DimensionOrderedSocketAddress (DimensionOrderedAddress address);
  /**
   * \param port the port number
   *
   * THe address is set to the "Any" address by default.
   */
  DimensionOrderedSocketAddress (uint16_t port);
  /**
   * \param address string which represents a DimensionOrdered address
   * \param port the port number
   */
  DimensionOrderedSocketAddress (const char *address, uint16_t port);
  /**
   * \param address string which represents a DimensionOrdered address
   *
   * The port number is set to zero by default.
   */
  DimensionOrderedSocketAddress (const char *address);
  /**
   * \returns the port number
   */
  uint16_t GetPort (void) const;
  /**
   * \returns the DimensionOrderedAddress
   */
  DimensionOrderedAddress GetDimensionOrderedAddress (void) const;
  /**
   * \param port the new port number
   */
  void SetPort (uint16_t port);
  /**
   * \param address the new address
   */
  void SetDimensionOrderedAddress (DimensionOrderedAddress address);

  /**
   * \param address address to test
   * \returns true if the address matches, false otherwise
   */
  static bool IsMatchingType (const Address &address);

  /**
   * \returns an Address instance which represents this
   * DimensionOrderedSocketAddress instance.
   */
  operator Address () const;

  /**
   * \param address the Address instance to convert from.
   *
   * \returns a DimensionOrderedSocketAddress which corresponds to the input Address
   */
  static DimensionOrderedSocketAddress ConvertFrom (const Address &address);
private:
  Address ConvertTo (void) const;
  
  static uint8_t GetType (void);

  DimensionOrderedAddress m_address;
  uint16_t m_port;
};

} // namespace ns3

#endif /* DIM_ORDERED_SOCKET_ADDRESS_H */

