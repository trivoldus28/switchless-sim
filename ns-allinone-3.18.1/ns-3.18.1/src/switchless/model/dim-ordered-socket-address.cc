/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-socket-address.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedSocketAddress");

namespace ns3 {

DimensionOrderedSocketAddress::DimensionOrderedSocketAddress (DimensionOrderedAddress address, uint16_t port)
  : m_address (address),
    m_port (port)
{
    NS_LOG_FUNCTION (this << address << port);
}
DimensionOrderedSocketAddress::DimensionOrderedSocketAddress (DimensionOrderedAddress address)
  : m_address (address),
    m_port (0)
{   
    NS_LOG_FUNCTION (this << address);
}
DimensionOrderedSocketAddress::DimensionOrderedSocketAddress (const char* address, uint16_t port)
  : m_address (address),
    m_port (port)
{   
    NS_LOG_FUNCTION (this << address << port);
}
DimensionOrderedSocketAddress::DimensionOrderedSocketAddress (const char* address)
  : m_address (address),
    m_port (0)
{   
    NS_LOG_FUNCTION (this << address);
}
DimensionOrderedSocketAddress::DimensionOrderedSocketAddress (uint16_t port)
  : m_address (DimensionOrderedAddress::GetAny()),
    m_port (port)
{   
    NS_LOG_FUNCTION (this << port);
}
uint16_t
DimensionOrderedSocketAddress::GetPort (void) const
{
    NS_LOG_FUNCTION (this);
    return m_port;
}
DimensionOrderedAddress
DimensionOrderedSocketAddress::GetDimensionOrderedAddress (void) const
{
    NS_LOG_FUNCTION (this);
    return m_address;
}
void
DimensionOrderedSocketAddress::SetPort (uint16_t port)
{
    NS_LOG_FUNCTION (this << port);
    m_port = port;
}
void
DimensionOrderedSocketAddress::SetDimensionOrderedAddress (DimensionOrderedAddress address)
{
    NS_LOG_FUNCTION (this << address);
    m_address = address;
}

bool
DimensionOrderedSocketAddress::IsMatchingType (const Address &address)
{
    NS_LOG_FUNCTION (&address);
    return address.CheckCompatible (GetType (), 5);
}

DimensionOrderedSocketAddress::operator Address () const
{
    return ConvertTo ();
}

Address
DimensionOrderedSocketAddress::ConvertTo (void) const
{
    NS_LOG_FUNCTION (this);
    uint8_t buf[5];
    m_address.Serialize(buf);
    buf[3] = m_port & 0xff;
    buf[4] = (m_port >> 8) & 0xff;
    return Address (GetType (), buf, 5);
}

DimensionOrderedSocketAddress
DimensionOrderedSocketAddress::ConvertFrom (const Address &address)
{
    NS_LOG_FUNCTION (&address);
    NS_ASSERT (address.CheckCompatible (GetType (), 5));
    uint8_t buf[5];
    address.CopyTo (buf);
    DimensionOrderedAddress dimOrderedAddress = DimensionOrderedAddress::Deserialize (buf);
    uint16_t port = buf[3] | (buf[4] << 8);
    return DimensionOrderedSocketAddress (dimOrderedAddress, port);
}

uint8_t
DimensionOrderedSocketAddress::GetType (void)
{
    NS_LOG_FUNCTION_NOARGS ();
    static uint8_t type = Address::Register ();
    return type;
}

} // namespace ns3

