/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-interface-container.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedInterfaceContainer");

namespace ns3 {

DimensionOrderedInterfaceContainer::DimensionOrderedInterfaceContainer()
  : m_interfaces ()
{
}

void
DimensionOrderedInterfaceContainer::Add (DimensionOrderedInterfaceContainer other)
{
    for (InterfaceVector::const_iterator i = other.m_interfaces.begin (); i != other.m_interfaces.end (); i++)
        m_interfaces.push_back (*i);
}

DimensionOrderedInterfaceContainer::Iterator
DimensionOrderedInterfaceContainer::Begin (void) const
{
    return m_interfaces.begin ();
}

DimensionOrderedInterfaceContainer::Iterator
DimensionOrderedInterfaceContainer::End (void) const
{
    return m_interfaces.end ();
}

uint32_t
DimensionOrderedInterfaceContainer::GetN (void) const
{
    return m_interfaces.size ();
}

DimensionOrderedAddress
DimensionOrderedInterfaceContainer::GetAddress (uint32_t i) const
{
    Ptr<DimensionOrdered> dimOrdered = m_interfaces[i].first;
    DimensionOrdered::InterfaceDirection ifaceDir = m_interfaces[i].second;
    return dimOrdered->GetAddress (ifaceDir).GetLocal ();
}

void
DimensionOrderedInterfaceContainer::Add (Ptr<DimensionOrdered> dimOrdered, 
                                         DimensionOrdered::InterfaceDirection dir)
{
    m_interfaces.push_back (std::make_pair (dimOrdered, dir));
}

void
DimensionOrderedInterfaceContainer::Add (
    std::pair<Ptr<DimensionOrdered>, DimensionOrdered::InterfaceDirection> pair)
{
    Add (pair.first, pair.second);
}

void
DimensionOrderedInterfaceContainer::Add (std::string dimOrderedName, DimensionOrdered::InterfaceDirection dir)
{
    Ptr<DimensionOrdered> dimOrdered = Names::Find<DimensionOrdered> (dimOrderedName);
    m_interfaces.push_back(std::make_pair (dimOrdered, dir));
}

std::pair<Ptr<DimensionOrdered>, DimensionOrdered::InterfaceDirection>
DimensionOrderedInterfaceContainer::Get (uint32_t i) const
{
    return m_interfaces[i];
}

} // namespace ns3
