/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-header.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DimensionOrderedHeader);

DimensionOrderedHeader::DimensionOrderedHeader ()
  : m_payloadSize (0),
    m_protocol (0),
    m_source (),
    m_destination (),
    m_headerSize (9)
{
}

void
DimensionOrderedHeader::SetPayloadSize (uint16_t size)
{
    NS_LOG_FUNCTION (this << size);
    m_payloadSize = size;
}

uint16_t
DimensionOrderedHeader::GetPayloadSize (void) const
{
    NS_LOG_FUNCTION (this);
    return m_payloadSize;
}

uint8_t
DimensionOrderedHeader::GetProtocol (void) const
{
    NS_LOG_FUNCTION (this);
    return m_protocol;
}

void
DimensionOrderedHeader::SetProtocol (uint8_t protocol)
{
    NS_LOG_FUNCTION (this << static_cast<uint32_t> (protocol));
    m_protocol = protocol;
}

void
DimensionOrderedHeader::SetSource (DimensionOrderedAddress source)
{
    NS_LOG_FUNCTION (this << source);
    m_source = source;
}

DimensionOrderedAddress
DimensionOrderedHeader::GetSource (void) const
{
    NS_LOG_FUNCTION (this);
    return m_source;
}

void
DimensionOrderedHeader::SetDestination (DimensionOrderedAddress dst)
{
    NS_LOG_FUNCTION (this <<dst);
    m_destination = dst;
}

DimensionOrderedAddress
DimensionOrderedHeader::GetDestination (void) const
{
    NS_LOG_FUNCTION (this);
    return m_destination;
}

TypeId
DimensionOrderedHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::DimensionOrderedHeader")
      .SetParent<Header> ()
      .AddConstructor<DimensionOrderedHeader> ()
    ;
    return tid;
}

TypeId
DimensionOrderedHeader::GetInstanceTypeId (void) const
{
    NS_LOG_FUNCTION (this);
    return GetTypeId ();
}

void
DimensionOrderedHeader::Print (std::ostream &os) const
{
    NS_LOG_FUNCTION (this << &os);
    os << "payload size: " << m_payloadSize << " "
       << "header size: " << m_headerSize << " "
       << "protocol " << m_protocol
       << " "
       << m_source << " > " << m_destination;
}

uint32_t
DimensionOrderedHeader::GetSerializedSize (void) const
{
    NS_LOG_FUNCTION (this);
    return m_headerSize;
}

void
DimensionOrderedHeader::Serialize (Buffer::Iterator start) const
{
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;

    i.WriteHtonU16 (m_payloadSize);
    i.WriteU8 (m_protocol);
    i.WriteU8 (m_source.GetAddressX ());
    i.WriteU8 (m_source.GetAddressY ());
    i.WriteU8 (m_source.GetAddressZ ());
    i.WriteU8 (m_destination.GetAddressX ());
    i.WriteU8 (m_destination.GetAddressY ());
    i.WriteU8 (m_destination.GetAddressZ ());
}

uint32_t
DimensionOrderedHeader::Deserialize (Buffer::Iterator start)
{
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;

    m_payloadSize = i.ReadNtohU16 ();
    m_protocol = i.ReadU8 ();
    m_source.SetAddressX (i.ReadU8 ());
    m_source.SetAddressY (i.ReadU8 ());
    m_source.SetAddressZ (i.ReadU8 ());
    m_destination.SetAddressX (i.ReadU8 ());
    m_destination.SetAddressY (i.ReadU8 ());
    m_destination.SetAddressZ (i.ReadU8 ());
    return GetSerializedSize ();
}

} // namespace ns3
