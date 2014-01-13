/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_INTERFACE_ADDRESS_H
#define DIM_ORDERED_INTERFACE_ADDRESS_H

// C/C++ includes

// NS3 includes
#include "ns3/log.h"

// Switchless includes
#include "ns3/dim-ordered-address.h"

namespace ns3 {

/**
 * \ingroup address
 *
 * \brief A class to store DimensionOrderedAddress information on an interface
 */
class DimensionOrderedInterfaceAddress {
public:
  enum InterfaceAddressScope_e {
    HOST,
    LINK,
    GLOBAL  
  };

  // Constructors and Destructors
  DimensionOrderedInterfaceAddress ();
  DimensionOrderedInterfaceAddress (DimensionOrderedAddress local);
  DimensionOrderedInterfaceAddress (const DimensionOrderedInterfaceAddress &o);
  ~DimensionOrderedInterfaceAddress ();

  void SetLocal (DimensionOrderedAddress local);
  DimensionOrderedAddress GetLocal (void) const;

  void SetScope (DimensionOrderedInterfaceAddress::InterfaceAddressScope_e scope);
  DimensionOrderedInterfaceAddress::InterfaceAddressScope_e GetScope (void) const;

  bool IsSecondary (void) const;
  void SetSecondary (void);
  void SetPrimary (void);

private:
  DimensionOrderedAddress m_local;

  InterfaceAddressScope_e m_scope;
  bool m_secondary;

  friend bool operator == (DimensionOrderedInterfaceAddress const &a, DimensionOrderedInterfaceAddress const &b);
  friend bool operator != (DimensionOrderedInterfaceAddress const &a, DimensionOrderedInterfaceAddress const &b);
};

std::ostream& operator<< (std::ostream& os, DimensionOrderedInterfaceAddress const &addr);

inline bool operator == (DimensionOrderedInterfaceAddress const &a, DimensionOrderedInterfaceAddress const &b)
{
    return (a.m_local == b.m_local && a.m_scope == b.m_scope && a.m_secondary == b.m_secondary);
}

inline bool operator != (DimensionOrderedInterfaceAddress const &a, DimensionOrderedInterfaceAddress const &b)
{
    return (a.m_local != b.m_local || a.m_scope != b.m_scope || a.m_secondary != b.m_secondary);
}

} // namespace ns3

#endif /* DIM_ORDERED_INTERFACE_ADDRESS_H */

