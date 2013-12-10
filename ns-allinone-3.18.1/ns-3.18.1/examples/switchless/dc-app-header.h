/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 *  Author: Michael McKeown
 */

#ifndef DC_APP_HEADER_H
#define DC_APP_HEADER_H

// NS-3 Includes
#include "ns3/core-module.h"
#include "ns3/header.h"
#include "ns3/nstime.h"

using namespace ns3;

/* 
 * Header that contains a packet type byte,
 * a 16bit sequence number, and a 64 bit timestamp
 */
class DCAppHeader : public Header
{
public:
    // Typedef for packet type
    typedef enum PACKET_TYPE_ENUM
    {
        PACKET_TYPE_INVALID = 0,
        REQUEST,
        RESPONSE
    } PACKET_TYPE;
    static std::string PacketTypeToString (PACKET_TYPE packetType);

    // Constructor/Destructor
    DCAppHeader ();
    virtual ~DCAppHeader();
    
    // Setters
    void SetPacketType (PACKET_TYPE packetType);
    void SetSequenceNumber (uint16_t sequenceNumber);
    
    // Getters
    PACKET_TYPE GetPacketType () const;
    uint16_t GetSequenceNumber () const;
    Time GetTimeStamp () const;
    
    static TypeId GetTypeId (void);
private:
    // Virtual private functions from base class
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream& os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

    // Private member, header components
    uint8_t         m_packetType;
    uint16_t        m_sequenceNumber;
    uint64_t        m_timeStamp;
};

#endif
