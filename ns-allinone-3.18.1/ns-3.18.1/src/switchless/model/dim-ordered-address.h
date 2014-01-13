/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_ADDRESS_H
#define DIM_ORDERED_ADDRESS_H

// C/C++ includes
#include <stdint.h>
#include <ostream>

// NS3 includes
#include "ns3/log.h"
#include "ns3/address.h"

// Switchless includes

namespace ns3 {

/**
 * \ingroup address
 *
 * \brief Dimension ordered addresses are stored in this class
 */
class DimensionOrderedAddress {
public:
  // Constructors and Destructors
  DimensionOrderedAddress ();
  /**
   * \param x X component of the address
   * \param y Y component of the address
   * \param z Z component of the address
   */
  DimensionOrderedAddress (uint8_t x, uint8_t y, uint8_t z);
  /**
   * \brief Constructs a DimensionOrderedAddress by parsing a C-string
   * 
   * Input address is in the format:
   * (xxx, yyy, zzz)
   * \param address C-string containing the address as described above
   */
  DimensionOrderedAddress (char const *address);
  ~DimensionOrderedAddress ();

  /**
   * \brief Get the X component of the address
   * \return the X component of the address
   */
  uint8_t GetAddressX (void) const;
  /**
   * \brief Get the Y component of the address
   * \return the Y component of the address
   */  
  uint8_t GetAddressY (void) const;
  /**
   * \brief Get the Z component of the address
   * \return the Z component of the address
   */
  uint8_t GetAddressZ (void) const;
  /**
   * \brief Set the X component of the address
   * \param x X component of the address
   */
  void SetAddressX (uint8_t x);
  /**
   * \brief Set the Y component of the address
   * \param y Y component of the address
   */
  void SetAddressY (uint8_t y);
  /**
   * \brief Set the Z component of the address
   * \param z Z component of the address
   */
  void SetAddressZ (uint8_t z);
  /**
   * \brief Set the address
   * \param x X component of the address
   * \param y Y component of the address
   * \param z Z component of the address
   */
  void SetAddress (uint8_t x, uint8_t y, uint8_t z);
  /**
   * \brief Comparison operation between two DimensionOrderedAddresses
   * \param other Address to which to compare this address
   * \return True if the addresses are equal. False otherwise.
   */
  bool IsEqual (const DimensionOrderedAddress &other) const;
  /**
   * \brief Serialize this address to a 3 byte buffer
   *
   * \param buf output biffer to which this address gets overwritten with this DimensionOrdered Address
   */
  void Serialize (uint8_t buf[3]) const;
  /**
   * \brief Deserialze the 3 byte buffer to a DimensionOrderedAddress
   * \param buf buffer to read address from
   * \return a DimensionOrderedAddress
   */
  static DimensionOrderedAddress Deserialize (const uint8_t buf[3]);
  /**
   * \brief Print this address to the given output stream
   *
   * The print address is in "(x, y, z)" form
   * \param os The output stream to which this address is printed
   */
  void Print (std::ostream &os) const;
  /**
    * \return true if address is (255, 255, 255); false otherwise
    */
  bool IsBroadcast (void) const;
  /**
   * \param address an address to compare type with
   *
   * \return true if the type of the address stored internally
   * is compatible with the type of the input address, false otherwise.
   */
  static bool IsMatchingType (const Address &address);
  /**
   * Convert an instance of this class to a polymorphic Address instance.
   *
   * \return a new Address instance
   */
  operator Address () const;
  /**
   * \param address a polymorphic address
   * \return a new DimensionOrderedAddress from the polymorphic address
   *
   * This function performs a type check and asserts if the
   * type of the input address is not compatible with a
   * DimensionOrderedAddress.
   */
  static DimensionOrderedAddress ConvertFrom (const Address &address);
  /**
   * \return the (0, 0, 0) address
   */
  static DimensionOrderedAddress GetZero (void);
  /**
   * \return the (255, 255, 255) address
   */
  static DimensionOrderedAddress GetAny (void);
  /**
   * \return the (255, 255, 255) address
   */
  static DimensionOrderedAddress GetBroadcast (void);
  /**
   * \return the (0, 0, 0) address
   */
  static DimensionOrderedAddress GetLoopback (void);

private:
  Address ConvertTo (void) const;
  static uint8_t GetType (void);
  void AsciiToDimensionOrderedHost (char const *address);
  uint8_t m_addressX;
  uint8_t m_addressY;
  uint8_t m_addressZ;

  friend bool operator == (DimensionOrderedAddress const &a, DimensionOrderedAddress const &b);
  friend bool operator != (DimensionOrderedAddress const &a, DimensionOrderedAddress const &b);
  friend bool operator < (DimensionOrderedAddress const &a, DimensionOrderedAddress const &b);
};

ATTRIBUTE_HELPER_HEADER (DimensionOrderedAddress);

std::ostream& operator<< (std::ostream& os, DimensionOrderedAddress const& address);
std::istream& operator>> (std::istream& is, DimensionOrderedAddress& address);

inline bool operator == (DimensionOrderedAddress const &a, DimensionOrderedAddress const &b)
{
    return ((a.m_addressX == b.m_addressX) &&
            (a.m_addressY == b.m_addressY) &&
            (a.m_addressZ == b.m_addressZ));
}

inline bool operator != (DimensionOrderedAddress const &a, DimensionOrderedAddress const &b)
{
    return ((a.m_addressX != b.m_addressX) ||
            (a.m_addressY != b.m_addressY) ||
            (a.m_addressZ != b.m_addressZ));
}

inline bool operator < (DimensionOrderedAddress const &a, DimensionOrderedAddress const &b)
{
    return ((a.m_addressX > b.m_addressX) &&
            (a.m_addressY > b.m_addressY) &&
            (a.m_addressZ > b.m_addressZ));
}

} // namespace ns3

#endif /* DIM_ORDERED_ADDRESS_H */

