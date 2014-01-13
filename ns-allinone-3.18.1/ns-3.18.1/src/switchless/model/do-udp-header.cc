/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "do-udp-header.h"
#include "ns3/address-utils.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoUdpHeader);

/* The magic values below are used only for debugging.
 * They can be used to easily detect memory corruption
 * problems so you can see the patterns in memory.
 */
DoUdpHeader::DoUdpHeader ()
  : m_sourcePort (0xfffd),
    m_destinationPort (0xfffd),
    m_payloadSize (0xfffd),
    m_calcChecksum (false),
    m_goodChecksum (true)
{
}
DoUdpHeader::~DoUdpHeader ()
{
  m_sourcePort = 0xfffe;
  m_destinationPort = 0xfffe;
  m_payloadSize = 0xfffe;
}

void 
DoUdpHeader::EnableChecksums (void)
{
  m_calcChecksum = true;
}

void 
DoUdpHeader::SetDestinationPort (uint16_t port)
{
  m_destinationPort = port;
}
void 
DoUdpHeader::SetSourcePort (uint16_t port)
{
  m_sourcePort = port;
}
uint16_t 
DoUdpHeader::GetSourcePort (void) const
{
  return m_sourcePort;
}
uint16_t 
DoUdpHeader::GetDestinationPort (void) const
{
  return m_destinationPort;
}
void
DoUdpHeader::InitializeChecksum (Address source,
                               Address destination,
                               uint8_t protocol)
{
  m_source = source;
  m_destination = destination;
  m_protocol = protocol;
}
void
DoUdpHeader::InitializeChecksum (DimensionOrderedAddress source,
                               DimensionOrderedAddress destination,
                               uint8_t protocol)
{
  m_source = source;
  m_destination = destination;
  m_protocol = protocol;
}
uint16_t
DoUdpHeader::CalculateHeaderChecksum (uint16_t size) const
{
  Buffer buf = Buffer ((2 * Address::MAX_SIZE) + 8);
  buf.AddAtStart ((2 * Address::MAX_SIZE) + 8);
  Buffer::Iterator it = buf.Begin ();
  uint32_t hdrSize = 0;

  WriteTo (it, m_source);
  WriteTo (it, m_destination);
  // TODO: This could be wrong for dimension ordered
  if (DimensionOrderedAddress::IsMatchingType(m_source))
    {
      it.WriteU8 (0); /* protocol */
      it.WriteU8 (m_protocol); /* protocol */
      it.WriteU8 (size >> 8); /* length */
      it.WriteU8 (size & 0xff); /* length */
      hdrSize = 12;
    }

  it = buf.Begin ();
  /* we don't CompleteChecksum ( ~ ) now */
  return ~(it.CalculateIpChecksum (hdrSize));
}

bool
DoUdpHeader::IsChecksumOk (void) const
{
  return m_goodChecksum; 
}


TypeId 
DoUdpHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoUdpHeader")
    .SetParent<Header> ()
    .AddConstructor<DoUdpHeader> ()
  ;
  return tid;
}
TypeId 
DoUdpHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void 
DoUdpHeader::Print (std::ostream &os) const
{
  os << "length: " << m_payloadSize + GetSerializedSize ()
     << " " 
     << m_sourcePort << " > " << m_destinationPort
  ;
}

uint32_t 
DoUdpHeader::GetSerializedSize (void) const
{
  return 8;
}

void
DoUdpHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_sourcePort);
  i.WriteHtonU16 (m_destinationPort);
  i.WriteHtonU16 (start.GetSize ());
  i.WriteU16 (0);

  if (m_calcChecksum)
    {
      uint16_t headerChecksum = CalculateHeaderChecksum (start.GetSize ());
      i = start;
      uint16_t checksum = i.CalculateIpChecksum (start.GetSize (), headerChecksum);

      i = start;
      i.Next (6);
      i.WriteU16 (checksum);
    }
}
uint32_t
DoUdpHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_sourcePort = i.ReadNtohU16 ();
  m_destinationPort = i.ReadNtohU16 ();
  m_payloadSize = i.ReadNtohU16 () - GetSerializedSize ();
  i.Next (2);

  if(m_calcChecksum)
    {
      uint16_t headerChecksum = CalculateHeaderChecksum (start.GetSize ());
      i = start;
      uint16_t checksum = i.CalculateIpChecksum (start.GetSize (), headerChecksum);

      m_goodChecksum = (checksum == 0);
    }

  return GetSerializedSize ();
}


} // namespace ns3
