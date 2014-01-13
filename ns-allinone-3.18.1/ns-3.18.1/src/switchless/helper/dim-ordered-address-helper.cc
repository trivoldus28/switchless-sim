/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-address-helper.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedAddressHelper");

namespace ns3 {

DimensionOrderedAddressHelper::DimensionOrderedAddressHelper ()
{
    NS_LOG_FUNCTION (this);
}

DimensionOrderedAddressHelper::~DimensionOrderedAddressHelper ()
{
    NS_LOG_FUNCTION (this);
}

DimensionOrderedInterfaceContainer
DimensionOrderedAddressHelper::Assign (AddressAssignmentList &list)
{
    NS_LOG_FUNCTION (&list);
    DimensionOrderedInterfaceContainer retval;
    for (AddressAssignmentList::iterator it = list.begin (); it != list.end (); it++)
    {
        AddressAssignment assignment = *it;
        Ptr<NetDevice> device = std::get<0> (assignment);
        DimensionOrderedAddress address = std::get<1> (assignment);
        DimensionOrdered::InterfaceDirection dir = std::get<2> (assignment);
        Ptr<DimensionOrdered> dimOrdered = Assign (device, address, dir);
        retval.Add (dimOrdered, dir);
    }
    return retval;
}

Ptr<DimensionOrdered>
DimensionOrderedAddressHelper::Assign (Ptr<NetDevice> device, const DimensionOrderedAddress &address,
                                       DimensionOrdered::InterfaceDirection dir)
{
    NS_LOG_FUNCTION (device << address << dir);

    Ptr<Node> node = device->GetNode ();
    NS_ASSERT_MSG (node, "DimensionOrderedAddressHelper::Assign(): NetDevice is not associated "
                   "with any node -> fail");

    Ptr<DimensionOrdered> dimOrdered = node->GetObject<DimensionOrdered> ();
    NS_ASSERT_MSG (dimOrdered, "DimensionOrderedAddressHelper::Assign(): NetDevice is associated "
                   "with a node without DimensionOrdered stack installed -> fail "
                   "(maybe need to use DimensionOrderedStackHelper?)");

    // Check if an interface exists for this device already
    DimensionOrdered::InterfaceDirection ifaceDir = dimOrdered->GetInterfaceForDevice (device);
    if (ifaceDir >= DimensionOrdered::INVALID_DIR)
        dimOrdered->AddInterface (device, dir);

    // Create an interface address for passed in address
    DimensionOrderedInterfaceAddress iaddr = DimensionOrderedInterfaceAddress (address);
    dimOrdered->SetAddress (dir, iaddr);
    dimOrdered->SetUp (dir);
    return dimOrdered;
}

} // namespace ns3
