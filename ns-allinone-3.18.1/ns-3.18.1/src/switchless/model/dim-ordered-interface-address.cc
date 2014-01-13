/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-interface-address.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedInterfaceAddress");

namespace ns3 {

DimensionOrderedInterfaceAddress::DimensionOrderedInterfaceAddress ()
  : m_local (),
    m_scope (GLOBAL),
    m_secondary (false)
{
    NS_LOG_FUNCTION (this);
}

DimensionOrderedInterfaceAddress::DimensionOrderedInterfaceAddress (DimensionOrderedAddress local)
  : m_local (local),
    m_scope (GLOBAL),
    m_secondary (false)
{
    NS_LOG_FUNCTION (this << local);
}

DimensionOrderedInterfaceAddress::DimensionOrderedInterfaceAddress (const DimensionOrderedInterfaceAddress &o)
  : m_local (o.m_local),
    m_scope (o.m_scope),
    m_secondary (o.m_secondary)
{
    NS_LOG_FUNCTION (this << &o);
}

DimensionOrderedInterfaceAddress::~DimensionOrderedInterfaceAddress ()
{
    NS_LOG_FUNCTION (this);
}

void
DimensionOrderedInterfaceAddress::SetLocal (DimensionOrderedAddress local)
{
    NS_LOG_FUNCTION (this << local);
    m_local = local;
}

DimensionOrderedAddress
DimensionOrderedInterfaceAddress::GetLocal (void) const
{
    NS_LOG_FUNCTION (this);
    return m_local;
}

void
DimensionOrderedInterfaceAddress::SetScope (DimensionOrderedInterfaceAddress::InterfaceAddressScope_e scope)
{
    NS_LOG_FUNCTION (this << scope);
    m_scope = scope;
}

DimensionOrderedInterfaceAddress::InterfaceAddressScope_e
DimensionOrderedInterfaceAddress::GetScope (void) const
{
    NS_LOG_FUNCTION (this);
    return m_scope;
}

bool
DimensionOrderedInterfaceAddress::IsSecondary (void) const
{
    NS_LOG_FUNCTION (this);
    return m_secondary;
}

void
DimensionOrderedInterfaceAddress::SetSecondary (void)
{
    NS_LOG_FUNCTION (this);
    m_secondary = true;
}

void
DimensionOrderedInterfaceAddress::SetPrimary (void)
{
    NS_LOG_FUNCTION (this);
    m_secondary = false;
}

std::ostream& operator<< (std::ostream& os, DimensionOrderedInterfaceAddress const &addr)
{
    os << "m_local=" << addr.GetLocal () << "; m_scope=" << addr.GetScope() << 
    "; m_secondary=" << addr.IsSecondary();
    return os;
}

} // namespace ns3

