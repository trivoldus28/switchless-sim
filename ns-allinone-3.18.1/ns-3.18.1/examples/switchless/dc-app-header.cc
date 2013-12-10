/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 *  Author: Michael McKeown
 */

#include "dc-app-header.h"

NS_LOG_COMPONENT_DEFINE ("DCAppHeader");
NS_OBJECT_ENSURE_REGISTERED (DCAppHeader);

std::string
DCAppHeader::PacketTypeToString (PACKET_TYPE packetType)
{
    switch (packetType)
    {
        case PACKET_TYPE_INVALID:
            return "Invalid Packet Type";
        case REQUEST:
            return "Request";
        case RESPONSE:
            return "Response";
        default:
            return "Invalid Packet Type";
    }

    return "Invalid Packet Type";
}

DCAppHeader::DCAppHeader ()
  : m_packetType (PACKET_TYPE_INVALID),
    m_sequenceNumber (0),
    m_timeStamp (Simulator::Now ().GetTimeStep ())
{
    NS_LOG_FUNCTION (this);
}

DCAppHeader::~DCAppHeader()
{
    NS_LOG_FUNCTION (this);
}

void
DCAppHeader::SetPacketType (PACKET_TYPE packetType)
{
    NS_LOG_FUNCTION (this << packetType);
    m_packetType = packetType;
}

void
DCAppHeader::SetSequenceNumber (uint16_t sequenceNumber)
{
    NS_LOG_FUNCTION (this << sequenceNumber);
    m_sequenceNumber = sequenceNumber;
}

DCAppHeader::PACKET_TYPE
DCAppHeader::GetPacketType () const
{
    NS_LOG_FUNCTION (this);
    return (PACKET_TYPE) m_packetType;
}

uint16_t
DCAppHeader::GetSequenceNumber () const
{
    NS_LOG_FUNCTION (this);
    return m_sequenceNumber;
}

Time
DCAppHeader::GetTimeStamp () const
{
    NS_LOG_FUNCTION (this);
    return TimeStep (m_timeStamp);
}

TypeId
DCAppHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("DCAppHeader")
        .SetParent<Header> ()
        .AddConstructor<DCAppHeader> ()
    ;
    return tid;
}

TypeId
DCAppHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

void
DCAppHeader::Print (std::ostream &os) const
{
    NS_LOG_FUNCTION (this << &os);
    os << "(type=" << PacketTypeToString ((PACKET_TYPE) m_packetType) << 
          " seq=" << m_sequenceNumber << 
          " time=" << TimeStep (m_timeStamp).GetSeconds () << ")";
}

uint32_t
DCAppHeader::GetSerializedSize (void) const
{
    NS_LOG_FUNCTION (this);
    return (sizeof (m_packetType) + sizeof (m_sequenceNumber) + sizeof (m_timeStamp));
}

void
DCAppHeader::Serialize (Buffer::Iterator start) const
{
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    i.WriteU8 (m_packetType);
    i.WriteU16 (m_sequenceNumber);
    i.WriteU64 (m_timeStamp);
}

uint32_t
DCAppHeader::Deserialize (Buffer::Iterator start)
{
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    m_packetType = i.ReadU8 ();
    m_sequenceNumber = i.ReadU16 ();
    m_timeStamp = i.ReadU64 ();
    return GetSerializedSize ();
}
