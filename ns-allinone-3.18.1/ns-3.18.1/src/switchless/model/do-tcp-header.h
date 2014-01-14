/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_TCP_HEADER_H
#define DO_TCP_HEADER_H

#include <stdint.h>
#include "ns3/header.h"
#include "ns3/buffer.h"
#include "ns3/do-tcp-socket-factory.h"
#include "ns3/dim-ordered-address.h"
#include "ns3/sequence-number.h"

namespace ns3 {

/**
 * \ingroup tcp
 * \brief Header for the Transmission Control Protocol in the DimensionOrdered stack
 *
 * This class has fields corresponding to those in a network TCP header
 * (port numbers, sequence and acknowledgement numbers, flags, etc) as well
 * as methods for serialization to and deserialization from a byte buffer.
 */

class DoTcpHeader : public Header 
{
public:
  DoTcpHeader ();
  virtual ~DoTcpHeader ();

  /**
   * \brief Enable checksum calculation for TCP
   *
   * \todo currently has no effect
   */
  void EnableChecksums (void);
//Setters
/**
 * \param port The source port for this DoTcpHeader
 */
  void SetSourcePort (uint16_t port);
  /**
   * \param port the destination port for this DoTcpHeader
   */
  void SetDestinationPort (uint16_t port);
  /**
   * \param sequenceNumber the sequence number for this DoTcpHeader
   */
  void SetSequenceNumber (SequenceNumber32 sequenceNumber);
  /**
   * \param ackNumber the ACK number for this DoTcpHeader
   */
  void SetAckNumber (SequenceNumber32 ackNumber);
  /**
   * \param length the length of this DoTcpHeader
   */
  void SetLength (uint8_t length);
  /**
   * \param flags the flags for this DoTcpHeader
   */
  void SetFlags (uint8_t flags);
  /**
   * \param windowSize the window size for this DoTcpHeader
   */
  void SetWindowSize (uint16_t windowSize);
  /**
   * \param urgentPointer the urgent pointer for this DoTcpHeader
   */
  void SetUrgentPointer (uint16_t urgentPointer);


//Getters
/**
 * \return The source port for this DoTcpHeader
 */
  uint16_t GetSourcePort () const;
  /**
   * \return the destination port for this DoTcpHeader
   */
  uint16_t GetDestinationPort () const;
  /**
   * \return the sequence number for this DoTcpHeader
   */
  SequenceNumber32 GetSequenceNumber () const;
  /**
   * \return the ACK number for this DoTcpHeader
   */
  SequenceNumber32 GetAckNumber () const;
  /**
   * \return the length of this DoTcpHeader
   */
  uint8_t  GetLength () const;
  /**
   * \return the flags for this DoTcpHeader
   */
  uint8_t  GetFlags () const;
  /**
   * \return the window size for this DoTcpHeader
   */
  uint16_t GetWindowSize () const;
  /**
   * \return the urgent pointer for this DoTcpHeader
   */
  uint16_t GetUrgentPointer () const;

  /**
   * \param source the dimensionordered source to use in the underlying
   *        dimensionordered packet.
   * \param destination the dimensionordered destination to use in the
   *        underlying dimensionordered packet.
   * \param protocol the protocol number to use in the underlying
   *        dimensionordered packet.
   *
   * If you want to use tcp checksums, you should call this
   * method prior to adding the header to a packet.
   */
  void InitializeChecksum (DimensionOrderedAddress source, 
                           DimensionOrderedAddress destination,
                           uint8_t protocol);
  void InitializeChecksum (Address source, 
                           Address destination,
                           uint8_t protocol);

  typedef enum { NONE = 0, FIN = 1, SYN = 2, RST = 4, PSH = 8, ACK = 16, 
                 URG = 32, ECE = 64, CWR = 128} Flags_t;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * \brief Is the TCP checksum correct ?
   * \returns true if the checksum is correct, false otherwise.
   */
  bool IsChecksumOk (void) const;

private:
  uint16_t CalculateHeaderChecksum (uint16_t size) const;
  uint16_t m_sourcePort;
  uint16_t m_destinationPort;
  SequenceNumber32 m_sequenceNumber;
  SequenceNumber32 m_ackNumber;
  uint8_t m_length; // really a uint4_t
  uint8_t m_flags;      // really a uint6_t
  uint16_t m_windowSize;
  uint16_t m_urgentPointer;

  Address m_source;
  Address m_destination;
  uint8_t m_protocol;

  bool m_calcChecksum;
  bool m_goodChecksum;
};

} // namespace ns3

#endif /* DO_TCP_HEADER */
