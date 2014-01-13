/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_ADDRESS_HELPER_H
#define DIM_ORDERED_ADDRESS_HELPER_H

// C/C++ includes
#include <tuple>

// NS3 includes
#include "ns3/net-device.h"

// Switchless includes
#include "ns3/dim-ordered.h"
#include "ns3/dim-ordered-interface-container.h"

namespace ns3 {

/**
 * \brief A helper class to make life easier while doing assignment of DimensionOrdered addresses
 * 
 * This class provides pretty much one useful method to assign a NetDevice a DimensionOrderedAddress.
 * This class does not attempt to generate and assign addresses as this is highly dependent on
 * the topology.
 */
class DimensionOrderedAddressHelper
{
public:
    DimensionOrderedAddressHelper ();
    ~DimensionOrderedAddressHelper ();

    typedef std::tuple<Ptr<NetDevice>, DimensionOrderedAddress, 
                       DimensionOrdered::InterfaceDirection> AddressAssignment;
    typedef std::vector<AddressAssignment> AddressAssignmentList;

    static DimensionOrderedInterfaceContainer Assign (AddressAssignmentList &list);
private:
    static Ptr<DimensionOrdered> Assign (Ptr<NetDevice> device, DimensionOrderedAddress const &address,
                                                  DimensionOrdered::InterfaceDirection dir);

};

} // namespace ns3

#endif /* DIM_ORDERED_ADDRESS_HELPER_H */

