/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 *  Author: Michael McKeown
 */

#ifndef DATA_CENTER_APP_H
#define DATA_CENTER_APP_H

// C/C++ Includes
#include <stdlib.h>
#include <time.h>
#include <unordered_set>

// NS-3 Includes
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

// Switchless Includes
#include "ns3/switchless-module.h"
#include "dc-app-header.h"

using namespace ns3;

class DataCenterApp : public Application
{
public:

    // Which receivers to send to in the list,
    // all of them or just a random subset
    typedef enum RECEIVERS_ENUM
    {
        RECEIVERS_INVALID = 0,
        ALL_IN_LIST,
        RANDOM_SUBSET
    } RECEIVERS;
    // The pattern in which packets will be sent,
    // at fixed intervals or at random intervals
    typedef enum SEND_PATTERN_ENUM
    {
        SEND_PATTERN_INVALID = 0,
        FIXED_INTERVAL,
        RANDOM_INTERVAL,
        FIXED_SPORADIC,
        RANDOM_SPORADIC
    } SEND_PATTERN;

    // Enumeration to specify the stack to use
    typedef enum NETWORK_STACK_ENUM
    {
        INVALID_STACK = 0,
        UDP_IP_STACK,
        TCP_IP_STACK,
        UDP_DO_STACK,
        TCP_DO_STACK
    } NETWORK_STACK;
    // Struct to hold all sending parameters
    typedef struct  SendParamsStruct
    {
        bool                                    m_sending;
        std::vector<Address>                    m_nodes;
        RECEIVERS                               m_receivers;
        uint32_t                                m_nReceivers;
        SEND_PATTERN                            m_sendPattern;
        Time                                    m_sendInterval;
        Time                                    m_maxSendInterval;
        Time                                    m_minSendInterval;
        uint32_t                                m_packetSize;
        uint32_t                                m_nIterations;
    } SendParams;
    static void copySendParams(SendParams& src, SendParams& dst);


    // Constructor/Destructor
    DataCenterApp ();
    virtual ~DataCenterApp ();

    // Function to setup app
    bool Setup (SendParams& sendingParams, uint32_t nodeId, NETWORK_STACK stack, bool debug);  
private:
    // Constants
    static const uint16_t PORT = 8080;
    static const uint32_t MAX_PACKET_SIZE = 512;

    // Struct to hold information to sending to a node
    typedef struct SendInfoStruct
    {
        Address         m_address;
        Ptr<Socket>     m_socket;
        EventId         m_event;
        uint32_t        m_packetsSent;
        uint32_t        m_bytesSent;
    } SendInfo;
    static void InitSendInfo (SendInfo& sendInfo, Address address, Ptr<Socket> socket);

    // Struct to hold receive information
    typedef struct ReceiveInfoStruct
    {
        uint32_t        m_packetsReceived;
        uint32_t        m_bytesReceived;
    } ReceiveInfo;
    static void InitReceiveInfo (ReceiveInfo& receiveInfo);

    void SetupRXSocket (void);
    void SetupTXSocket (uint32_t sendParamsNodeIndex);

    // Overridden methods called when app starts and stops
    virtual void StartApplication (void);
    virtual void StopApplication (void);

    // Called from Start Application to start the sending
    void KickOffSending (void);

    // Callback functions
    bool HandleConnectionRequest (Ptr<Socket> socket, const Address& from);
    void HandleAccept (Ptr<Socket> socket, const Address& from);
    void HandleRead (Ptr<Socket> socket);
    void HandleClose (Ptr<Socket> socket);
    void HandleError (Ptr<Socket> socket);
    void HandleConnectionSucceeded (Ptr<Socket> socket);
    void HandleConnectionFailed (Ptr<Socket> socket);

    // Send a request packet
    void BulkSendPackets ();
    void SendPacket (uint32_t index);
    void DoSendPacket (SendInfo& sendInfo);
    // Schedule the next packet to send
    void BulkScheduleSend ();
    void ScheduleSend (uint32_t index);

    // Send response packet
    void SendResponsePacket (Ptr<Socket> socket, Address& to, uint16_t sequenceNumber);
    
    // Select random receivers
    void SelectRandomReceiverSubset (std::unordered_set<uint32_t>& subset); 
    uint32_t SelectRandomReceiver ();
    // Select a random interval
    Time SelectRandomInterval ();

    SendParams                          m_sendParams;
    bool                                m_setup;
    bool                                m_running;
    NETWORK_STACK                       m_stack;
    uint32_t                            m_iterationCount;
    uint32_t                            m_totalPacketsSent;
    uint32_t                            m_responseCount;
    std::vector<SendInfo>               m_sendInfos;
    std::map<Ptr<Socket>, uint32_t>     m_socketIndexMap;
    Ptr<Socket>                         m_rxSocket;
    std::map<Ptr<Socket>, ReceiveInfo>  m_acceptSocketMap;
};

#endif
