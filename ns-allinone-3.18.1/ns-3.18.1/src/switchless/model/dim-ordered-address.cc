/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-address.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedAddress");

namespace ns3 {

DimensionOrderedAddress::DimensionOrderedAddress ()
  : m_addressX (0),
    m_addressY (0),
    m_addressZ (0)
{
    NS_LOG_FUNCTION (this);
}

DimensionOrderedAddress::DimensionOrderedAddress (uint8_t x, uint8_t y, uint8_t z)
  : m_addressX (x),
    m_addressY (y),
    m_addressZ (z)
{
    NS_LOG_FUNCTION (this << x << y << z);
}

DimensionOrderedAddress::DimensionOrderedAddress (char const *address)
  : m_addressX (0),
    m_addressY (0),
    m_addressZ (0)
{
    NS_LOG_FUNCTION (this << address);
    AsciiToDimensionOrderedHost (address);
}

DimensionOrderedAddress::~DimensionOrderedAddress ()
{
    NS_LOG_FUNCTION (this);
}

uint8_t
DimensionOrderedAddress::GetAddressX (void) const
{
    NS_LOG_FUNCTION (this);
    return m_addressX;
}

uint8_t
DimensionOrderedAddress::GetAddressY (void) const
{
    NS_LOG_FUNCTION (this);
    return m_addressY;
}

uint8_t
DimensionOrderedAddress::GetAddressZ (void) const
{
    NS_LOG_FUNCTION (this);
    return m_addressZ;
}

void
DimensionOrderedAddress::SetAddressX (uint8_t x)
{
    NS_LOG_FUNCTION (this << x);
    m_addressX = x;
}

void
DimensionOrderedAddress::SetAddressY (uint8_t y)
{
    NS_LOG_FUNCTION (this << y);
    m_addressY = y;
}

void
DimensionOrderedAddress::SetAddressZ (uint8_t z)
{
    NS_LOG_FUNCTION (this << z);
    m_addressZ = z;
}

void
DimensionOrderedAddress::SetAddress (uint8_t x, uint8_t y, uint8_t z)
{
    NS_LOG_FUNCTION (this << x << y << z);
    m_addressX = x;
    m_addressY = y;
    m_addressZ = z;
}


bool
DimensionOrderedAddress::IsEqual (const DimensionOrderedAddress &other) const
{
    NS_LOG_FUNCTION (this << other);
    return ((m_addressX == other.m_addressX) &&
            (m_addressY == other.m_addressY) &&
            (m_addressZ == other.m_addressZ));
}

void
DimensionOrderedAddress::Serialize (uint8_t buf[3]) const
{
    NS_LOG_FUNCTION (this << &buf);
    buf[0] = m_addressX;
    buf[1] = m_addressY;
    buf[2] = m_addressZ;
}

DimensionOrderedAddress
DimensionOrderedAddress::Deserialize (const uint8_t buf[3])
{
    NS_LOG_FUNCTION (&buf);
    return DimensionOrderedAddress (buf[0], buf[1], buf[2]);
}

void
DimensionOrderedAddress::Print (std::ostream &os) const
{
    NS_LOG_FUNCTION (this);
    os << "(" << (int)(m_addressX) << 
          ", " << (int)(m_addressY) << 
          ", " << (int)(m_addressZ) << ")";
}

bool
DimensionOrderedAddress::IsBroadcast (void) const
{
    NS_LOG_FUNCTION (this);
    return ((m_addressX == 255) &&
            (m_addressY == 255) &&
            (m_addressZ == 255));
}

bool
DimensionOrderedAddress::IsMatchingType (const Address &address)
{
    NS_LOG_FUNCTION (&address);
    return address.CheckCompatible (GetType (), 3);    
}

DimensionOrderedAddress::operator Address () const
{
    NS_LOG_FUNCTION (this);
    return ConvertTo ();
}

Address
DimensionOrderedAddress::ConvertTo (void) const
{
    NS_LOG_FUNCTION (this);
    uint8_t buf[3];
    Serialize (buf);
    return Address (GetType (), buf, 3);
}

DimensionOrderedAddress
DimensionOrderedAddress::ConvertFrom (const Address &address)
{
    NS_LOG_FUNCTION (&address);
    NS_ASSERT (address.CheckCompatible (GetType(), 3));
    uint8_t buf[3];
    address.CopyTo (buf);
    return Deserialize (buf);
}

uint8_t
DimensionOrderedAddress::GetType (void)
{
    NS_LOG_FUNCTION_NOARGS ();
    static uint8_t type = Address::Register ();
    return type;
}

DimensionOrderedAddress
DimensionOrderedAddress::GetZero (void)
{
    NS_LOG_FUNCTION_NOARGS ();
    return DimensionOrderedAddress (0, 0, 0);
}

DimensionOrderedAddress
DimensionOrderedAddress::GetAny (void)
{
    NS_LOG_FUNCTION_NOARGS ();
    return DimensionOrderedAddress (255, 255, 255);
}

DimensionOrderedAddress
DimensionOrderedAddress::GetBroadcast (void)
{
    NS_LOG_FUNCTION_NOARGS ();
    return DimensionOrderedAddress (255, 255, 255);
}

DimensionOrderedAddress
DimensionOrderedAddress::GetLoopback (void)
{
    NS_LOG_FUNCTION_NOARGS ();
    return DimensionOrderedAddress (0, 0, 0);
}

void 
DimensionOrderedAddress::AsciiToDimensionOrderedHost (char const *address)
{
    NS_LOG_FUNCTION (this << address);
    uint8_t dimensionPos = 0;
    bool firstParenFound = false;
    char buf[3];
    for (int i = 0; i < 3; i++)
        buf[i] = '\0';
    uint8_t bufPos = 0;
    while (true)
    {
        switch (dimensionPos)
        {
            case 0:
                if (!firstParenFound && *address == '(')
                    firstParenFound = true;
                else if (*address == ',')
                {
                    m_addressX = strtol(buf, NULL, 10);
                    for (int i = 0; i < 3; i++)
                        buf[i] = '\0';
                    bufPos = 0;
                    dimensionPos++;
                }
                else if (*address == ' ')
                {
                    // Skip over it
                }
                else
                {
                    buf[bufPos] = *address;
                    bufPos++;
                }
                break;
            case 1:
                if (*address == ',')
                {
                    m_addressY = strtol(buf, NULL, 10);
                    for (int i = 0; i < 3; i++)
                        buf[i] = '\0';
                    bufPos = 0;
                    dimensionPos++;
                }
                else if (*address == ' ')
                {
                    // Skip over it
                }
                else
                {
                    buf[bufPos] = *address;
                    bufPos++;
                }
                break;
            case 2:
                if (*address == ')')
                {
                    m_addressZ = strtol(buf, NULL, 10);
                    return;
                }
                else if (*address == ' ')
                {
                    // Skip over it
                }
                else
                {
                    buf[bufPos] = *address;
                    bufPos++;
                }
                break;
            default:
                return;
                break;
        }
        address++;
    }
}

std::ostream& operator<< (std::ostream& os, DimensionOrderedAddress const& address)
{
    address.Print (os);
    return os;
}

std::istream& operator>> (std::istream& is, DimensionOrderedAddress& address)
{
    std::string str;
    is >> str;
    address = DimensionOrderedAddress(str.c_str ());
    return is;
}

ATTRIBUTE_HELPER_CPP (DimensionOrderedAddress);

} // namespace ns3

