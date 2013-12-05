/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 *  Author: Michael McKeown
 */

#include "data-center-app.h"

NS_LOG_COMPONENT_DEFINE ("DataCenterApp");
NS_OBJECT_ENSURE_REGISTERED (DataCenterApp);

DataCenterApp::DataCenterApp ()
  : m_sendParams(),
    m_sendEvent(),
    m_setup(false),
    m_running(false),
    m_packetsSent(0)
{
    NS_LOG_FUNCTION (this);
    // Default sending parameters
    m_sendParams.m_sending = false;
    m_sendParams.m_nodes = NULL;
    m_sendParams.m_receivers = RECEIVERS_INVALID;
    m_sendParams.m_nReceivers = 0;
    m_sendParams.m_sendPattern = SEND_PATTERN_INVALID;
    m_sendParams.m_sendInterval = 10;
    m_sendParams.m_packetSize = 1024;
    m_sendParams.m_nPackets = 100;
}

DataCenterApp::~DataCenterApp ()
{
    NS_LOG_FUNCTION (this);
}

void
DataCenterApp::Setup (SendParams& sendingParams, bool debug)
{
    NS_LOG_FUNCTION (this << debug);

    m_sendParams = sendingParams;

    if (debug)
        srand(0);
    else
        srand(time(NULL));

    m_setup = true;   
}

void
DataCenterApp::StartApplication (void)
{
    NS_LOG_FUNCTION (this);

    if (!m_setup)
        NS_LOG_WARN ("Application called before calling DataCenterApp::Setup");
        
    // TODO: Setup sockets here to clients and kick off sending,
    //       probably also need to setup something for receiving 

    m_packetsSent = 0;
    m_running = true;
}

void
DataCenterApp::StopApplication (void)
{
    NS_LOG_FUNCTION (this);

    m_running = false;

    if (m_sendEvent.IsRunning ())
        Simulator::Cancel (m_sendEvent);

    // TODO: Close sockets
}

void
DataCenterApp::SendPacket (uint32_t sockIndex)
{
    NS_LOG_FUNCTION (this);

    // TODO: Send a packet and schedule the next to be sent if we have not sent all the packets
}

void
DataCenterApp::ScheduleSend (void)
{
    NS_LOG_FUNCTION (this);

    // TODO: Setup a send event based on the sending pattern
}
