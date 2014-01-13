/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_HEADER_H
#define DIM_ORDERED_HEADER_H

// C/C++ includes

// NS3 includes
#include "ns3/header.h"

// Switchless includes
#include "ns3/dim-ordered-address.h"

namespace ns3 {

/**
 * \brief Packet header of DimensionOrdered
 */
class DimensionOrderedHeader : public Header
{
public:
  /**
   * \brief Construct a null DimensionOrdered header
   */
  DimensionOrderedHeader ();

  /**
   * \param size the size of the payload in bytes
   */
  void SetPayloadSize (uint16_t size);

  /**
   * \param num the DimensionOrdered protocol field
   */
  void SetProtocol (uint8_t num);

  /**
   * \param source the source of this packet
   */
  void SetSource (DimensionOrderedAddress source);

  /**
   * \param destination the destination of this packet
   */
  void SetDestination (DimensionOrderedAddress destination);

  /**
   * \returns the size of the payload in bytes
   */
  uint16_t GetPayloadSize (void) const;
  
  /** 
   *  \returns the protocol field of this packet
   */
  uint8_t GetProtocol (void) const;

  /**
   * \returns the source address of this packet
   */
  DimensionOrderedAddress GetSource (void) const;

  /**
   * \returns the destination address of this packet
   */
  DimensionOrderedAddress GetDestination (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
private:
  
  uint16_t m_payloadSize;
  uint32_t m_protocol : 8;
  DimensionOrderedAddress m_source;
  DimensionOrderedAddress m_destination;
  uint16_t m_headerSize; 

};

} // namespace ns3

#endif /* DIM_ORDERED_HEADER_H */

