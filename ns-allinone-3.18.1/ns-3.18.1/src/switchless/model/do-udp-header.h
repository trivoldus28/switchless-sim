/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_UDP_HEADER_H
#define DO_UDP_HEADER_H

#include <stdint.h>
#include <string>
#include "ns3/header.h"
#include "ns3/dim-ordered-address.h"

namespace ns3 {
/**
 * \ingroup udp
 * \brief Packet header for DimensionOrdered UDP packets
 *
 * This class has fields corresponding to those in a network UDP header
 * (port numbers, payload size, checksum) as well as methods for serialization
 * to and deserialization from a byte buffer.
 */
class DoUdpHeader : public Header 
{
public:

  /**
   * \brief Constructor
   *
   * Creates a null header
   */
  DoUdpHeader ();
  ~DoUdpHeader ();

  /**
   * \brief Enable checksum calculation for UDP 
   */
  void EnableChecksums (void);
  /**
   * \param port the destination port for this UdpHeader
   */
  void SetDestinationPort (uint16_t port);
  /**
   * \param port The source port for this UdpHeader
   */
  void SetSourcePort (uint16_t port);
  /**
   * \return The source port for this UdpHeader
   */
  uint16_t GetSourcePort (void) const;
  /**
   * \return the destination port for this UdpHeader
   */
  uint16_t GetDestinationPort (void) const;

  /**
   * \param source the ip source to use in the underlying
   *        ip packet.
   * \param destination the ip destination to use in the
   *        underlying ip packet.
   * \param protocol the protocol number to use in the underlying
   *        ip packet.
   *
   * If you want to use udp checksums, you should call this
   * method prior to adding the header to a packet.
   */
  void InitializeChecksum (Address source, 
                           Address destination,
                           uint8_t protocol);

  /**
   * \param source the ip source to use in the underlying
   *        ip packet.
   * \param destination the ip destination to use in the
   *        underlying ip packet.
   * \param protocol the protocol number to use in the underlying
   *        ip packet.
   *
   * If you want to use udp checksums, you should call this
   * method prior to adding the header to a packet.
   */
  void InitializeChecksum (DimensionOrderedAddress source, 
                           DimensionOrderedAddress destination,
                           uint8_t protocol);

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * \brief Is the UDP checksum correct ?
   * \returns true if the checksum is correct, false otherwise.
   */
  bool IsChecksumOk (void) const;

private:
  uint16_t CalculateHeaderChecksum (uint16_t size) const;
  uint16_t m_sourcePort;
  uint16_t m_destinationPort;
  uint16_t m_payloadSize;

  Address m_source;
  Address m_destination;
  uint8_t m_protocol;
  bool m_calcChecksum;
  bool m_goodChecksum;
};

} // namespace ns3

#endif /* DO_UDP_HEADER */
