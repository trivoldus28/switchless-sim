/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 *  Author: Michael McKeown
 */

#ifndef DATA_CENTER_APP_H
#define DATA_CENTER_APP_H

// C/C++ Includes
#include <stdlib.h>
#include <time.h>

// NS-3 Includes
#include "ns3/core-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

class DataCenterApp : public Application
{
public:

    // Which receivers to send to in the list,
    // all of them or just a random subset
    typedef enum RECEIVERS_ENUM
    {
        RECEIVERS_INVALID = 0,
        RANDOM_SUBSET,
        ALL_IN_LIST
    } RECEIVERS;
    // The pattern in which packets will be sent,
    // at fixed intervals or at random intervals
    typedef enum SEND_PATTERN_ENUM
    {
        SEND_PATTERN_INVALID = 0,
        FIXED_INTERVAL,
        RANDOM_INTERVAL,
        RANDOM_SPORADIC
    } SEND_PATTERN;
    // Struct to hold all sending parameters
    typedef struct  SendParamsStruct
    {
        bool            m_sending;
        Address*        m_nodes;
        RECEIVERS       m_receivers;
        uint32_t        m_nReceivers;
        SEND_PATTERN    m_sendPattern;
        uint32_t        m_sendInterval;
        uint32_t        m_packetSize;
        uint32_t        m_nPackets;
    } SendParams;

    // Constructor/Destructor
    DataCenterApp ();
    virtual ~DataCenterApp ();

    // Function to setup app
    void Setup (SendParams& sendingParams, bool debug);  
private:
    // Overridden methods called when app starts and stops
    virtual void StartApplication (void);
    virtual void StopApplication (void);

    // Send a packet
    void SendPacket (uint32_t sockIndex);
    // Schedule the next packet to send
    void ScheduleSend (void);

    SendParams          m_sendParams;
    EventId             m_sendEvent;
    bool                m_setup;
    bool                m_running;
    uint32_t            m_packetsSent;

    // TODO: Sockets and receiving
};

#endif
