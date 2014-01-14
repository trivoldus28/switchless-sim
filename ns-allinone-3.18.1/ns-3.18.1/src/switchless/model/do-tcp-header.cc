/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <stdint.h>
#include <iostream>
#include "do-tcp-header.h"
#include "ns3/buffer.h"
#include "ns3/address-utils.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoTcpHeader);

DoTcpHeader::DoTcpHeader ()
  : m_sourcePort (0),
    m_destinationPort (0),
    m_sequenceNumber (0),
    m_ackNumber (0),
    m_length (5),
    m_flags (0),
    m_windowSize (0xffff),
    m_urgentPointer (0),
    m_calcChecksum (false),
    m_goodChecksum (true)
{
}

DoTcpHeader::~DoTcpHeader ()
{
}

void
DoTcpHeader::EnableChecksums (void)
{
  m_calcChecksum = true;
}

void DoTcpHeader::SetSourcePort (uint16_t port)
{
  m_sourcePort = port;
}
void DoTcpHeader::SetDestinationPort (uint16_t port)
{
  m_destinationPort = port;
}
void DoTcpHeader::SetSequenceNumber (SequenceNumber32 sequenceNumber)
{
  m_sequenceNumber = sequenceNumber;
}
void DoTcpHeader::SetAckNumber (SequenceNumber32 ackNumber)
{
  m_ackNumber = ackNumber;
}
void DoTcpHeader::SetLength (uint8_t length)
{
  m_length = length;
}
void DoTcpHeader::SetFlags (uint8_t flags)
{
  m_flags = flags;
}
void DoTcpHeader::SetWindowSize (uint16_t windowSize)
{
  m_windowSize = windowSize;
}
void DoTcpHeader::SetUrgentPointer (uint16_t urgentPointer)
{
  m_urgentPointer = urgentPointer;
}

uint16_t DoTcpHeader::GetSourcePort () const
{
  return m_sourcePort;
}
uint16_t DoTcpHeader::GetDestinationPort () const
{
  return m_destinationPort;
}
SequenceNumber32 DoTcpHeader::GetSequenceNumber () const
{
  return m_sequenceNumber;
}
SequenceNumber32 DoTcpHeader::GetAckNumber () const
{
  return m_ackNumber;
}
uint8_t  DoTcpHeader::GetLength () const
{
  return m_length;
}
uint8_t  DoTcpHeader::GetFlags () const
{
  return m_flags;
}
uint16_t DoTcpHeader::GetWindowSize () const
{
  return m_windowSize;
}
uint16_t DoTcpHeader::GetUrgentPointer () const
{
  return m_urgentPointer;
}

void 
DoTcpHeader::InitializeChecksum (DimensionOrderedAddress source, 
                               DimensionOrderedAddress destination,
                               uint8_t protocol)
{
  m_source = source;
  m_destination = destination;
  m_protocol = protocol;
}

void 
DoTcpHeader::InitializeChecksum (Address source, 
                               Address destination,
                               uint8_t protocol)
{
  m_source = source;
  m_destination = destination;
  m_protocol = protocol;
}

uint16_t
DoTcpHeader::CalculateHeaderChecksum (uint16_t size) const
{
  /* Buffer size must be at least as large as the largest IP pseudo-header */
  /* [per RFC2460, but without consideration for IPv6 extension hdrs]      */
  /* Src address            16 bytes (more generally, Address::MAX_SIZE)   */
  /* Dst address            16 bytes (more generally, Address::MAX_SIZE)   */
  /* Upper layer pkt len    4 bytes                                        */
  /* Zero                   3 bytes                                        */
  /* Next header            1 byte                                         */

  uint32_t maxHdrSz = (2 * Address::MAX_SIZE) + 8;
  Buffer buf = Buffer (maxHdrSz);
  buf.AddAtStart (maxHdrSz);
  Buffer::Iterator it = buf.Begin ();
  uint32_t hdrSize = 0;

  WriteTo (it, m_source);
  WriteTo (it, m_destination);
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
DoTcpHeader::IsChecksumOk (void) const
{
  return m_goodChecksum;
}

TypeId 
DoTcpHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoTcpHeader")
    .SetParent<Header> ()
    .AddConstructor<DoTcpHeader> ()
  ;
  return tid;
}
TypeId 
DoTcpHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void DoTcpHeader::Print (std::ostream &os)  const
{
  os << m_sourcePort << " > " << m_destinationPort;
  if(m_flags!=0)
    {
      os<<" [";
      if((m_flags & FIN) != 0)
        {
          os<<" FIN ";
        }
      if((m_flags & SYN) != 0)
        {
          os<<" SYN ";
        }
      if((m_flags & RST) != 0)
        {
          os<<" RST ";
        }
      if((m_flags & PSH) != 0)
        {
          os<<" PSH ";
        }
      if((m_flags & ACK) != 0)
        {
          os<<" ACK ";
        }
      if((m_flags & URG) != 0)
        {
          os<<" URG ";
        }
      if((m_flags & ECE) != 0)
        {
          os<<" ECE ";
        }
      if((m_flags & CWR) != 0)
        {
          os<<" CWR ";
        }
      os<<"]";
    }
  os<<" Seq="<<m_sequenceNumber<<" Ack="<<m_ackNumber<<" Win="<<m_windowSize;
}
uint32_t DoTcpHeader::GetSerializedSize (void)  const
{
  return 4*m_length;
}
void DoTcpHeader::Serialize (Buffer::Iterator start)  const
{
  Buffer::Iterator i = start;
  i.WriteHtonU16 (m_sourcePort);
  i.WriteHtonU16 (m_destinationPort);
  i.WriteHtonU32 (m_sequenceNumber.GetValue ());
  i.WriteHtonU32 (m_ackNumber.GetValue ());
  i.WriteHtonU16 (m_length << 12 | m_flags); //reserved bits are all zero
  i.WriteHtonU16 (m_windowSize);
  i.WriteHtonU16 (0);
  i.WriteHtonU16 (m_urgentPointer);

  if(m_calcChecksum)
    {
      uint16_t headerChecksum = CalculateHeaderChecksum (start.GetSize ());
      i = start;
      uint16_t checksum = i.CalculateIpChecksum (start.GetSize (), headerChecksum);

      i = start;
      i.Next (16);
      i.WriteU16 (checksum);
    }
}
uint32_t DoTcpHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_sourcePort = i.ReadNtohU16 ();
  m_destinationPort = i.ReadNtohU16 ();
  m_sequenceNumber = i.ReadNtohU32 ();
  m_ackNumber = i.ReadNtohU32 ();
  uint16_t field = i.ReadNtohU16 ();
  m_flags = field & 0x3F;
  m_length = field>>12;
  m_windowSize = i.ReadNtohU16 ();
  i.Next (2);
  m_urgentPointer = i.ReadNtohU16 ();

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
