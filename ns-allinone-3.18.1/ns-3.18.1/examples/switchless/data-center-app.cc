/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 *  Author: Michael McKeown
 */

#include "data-center-app.h"

NS_LOG_COMPONENT_DEFINE ("DataCenterApp");
NS_OBJECT_ENSURE_REGISTERED (DataCenterApp);

void
DataCenterApp::copySendParams (SendParams& src, SendParams& dst)
{
    dst.m_sending = src.m_sending;
    dst.m_nodes = src.m_nodes;
    dst.m_receivers = src.m_receivers;
    dst.m_nReceivers = src.m_nReceivers;
    dst.m_sendPattern = src.m_sendPattern;
    dst.m_sendInterval = src.m_sendInterval;
    dst.m_maxSendInterval = src.m_maxSendInterval;
    dst.m_minSendInterval = src.m_minSendInterval;
    dst.m_packetSize = src.m_packetSize;
    dst.m_nIterations = src.m_nIterations;
}

DataCenterApp::DataCenterApp ()
  : m_sendParams (),
    m_setup (false),
    m_running (false),
    m_iterationCount (0),
    m_totalPacketsSent (0),
    m_sendInfos (),
    m_rxSocket (),
    m_acceptSocketMap ()
{
    NS_LOG_FUNCTION (this);
    // Default sending parameters
    m_sendParams.m_sending = false;
    m_sendParams.m_receivers = RECEIVERS_INVALID;
    m_sendParams.m_nReceivers = 0;
    m_sendParams.m_sendPattern = SEND_PATTERN_INVALID;
    m_sendParams.m_sendInterval = MilliSeconds (100.);
    m_sendParams.m_maxSendInterval = MilliSeconds (500.0);
    m_sendParams.m_minSendInterval = MilliSeconds (100.0);
    m_sendParams.m_packetSize = 1024;
    m_sendParams.m_nIterations = 100;
}

DataCenterApp::~DataCenterApp ()
{
    NS_LOG_FUNCTION (this);
}

bool
DataCenterApp::Setup (SendParams& sendingParams, uint32_t nodeId, bool debug)
{
    NS_LOG_FUNCTION (this << debug);

    if (sendingParams.m_nReceivers > sendingParams.m_nodes.size())
    {
        NS_LOG_ERROR ("Number of receivers is greater than number of nodes");
        return false;
    }

    if (sendingParams.m_packetSize > MAX_PACKET_SIZE)
    {
        NS_LOG_ERROR ("Packet size is greater than max packet size");
        return false;
    }

    if (sendingParams.m_minSendInterval > sendingParams.m_maxSendInterval)
    {
        NS_LOG_ERROR ("Min send interval is greater than max send interval");
        return false;
    }

    copySendParams(sendingParams, m_sendParams);

    // When debugging make packets deterministic
    if (debug)
        srand (nodeId);
    else
        srand (time (NULL));

    m_setup = true;   

    return true;
}

void
DataCenterApp::InitSendInfo (SendInfo& sendInfo, Address address, Ptr<Socket> socket)
{
    sendInfo.m_address = address;
    sendInfo.m_socket = socket;
    sendInfo.m_packetsSent = 0;
    sendInfo.m_bytesSent = 0;
}

void
DataCenterApp::InitReceiveInfo (ReceiveInfo& receiveInfo)
{
    receiveInfo.m_packetsReceived = 0;
    receiveInfo.m_bytesReceived = 0;
}

void
DataCenterApp::StartApplication (void)
{
    NS_LOG_FUNCTION (this);

    if (!m_setup)
        NS_LOG_WARN ("Application started before calling DataCenterApp::Setup. Using defaults.");

    m_running = true;
    m_iterationCount = 0;
    m_totalPacketsSent = 0;

    // Setup socket for receiving
    m_rxSocket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
    Address local (InetSocketAddress (Ipv4Address::GetAny (), PORT));
    m_rxSocket->Bind (local);
    m_rxSocket->Listen ();
    m_rxSocket->ShutdownSend ();
    m_rxSocket->SetRecvCallback (MakeCallback (&DataCenterApp::HandleRead, this));
    m_rxSocket->SetAcceptCallback (MakeCallback (&DataCenterApp::HandleConnectionRequest, this),
                                   MakeCallback (&DataCenterApp::HandleAccept, this));
    m_rxSocket->SetCloseCallbacks (MakeCallback (&DataCenterApp::HandleClose, this),
                                   MakeCallback (&DataCenterApp::HandleError, this));

    // Only open sending sockets if this app is sending
    if (m_sendParams.m_sending)
    {
        // Setup socket for each node in list
        for (uint32_t i = 0; i < m_sendParams.m_nodes.size(); i++)
        {
            Ptr<Socket> socket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
            Address nodeAddress (InetSocketAddress (m_sendParams.m_nodes[i], PORT));
            socket->Bind ();
            socket->Connect (nodeAddress);
            socket->SetConnectCallback (MakeCallback (&DataCenterApp::HandleConnectionSucceeded, this),
                                        MakeCallback (&DataCenterApp::HandleConnectionFailed, this));
            SendInfo sendInfo;
            InitSendInfo (sendInfo, m_sendParams.m_nodes[i], socket);
            m_sendInfos.push_back (sendInfo);
        }
        KickOffSending();
    }
}

void
DataCenterApp::StopApplication (void)
{
    NS_LOG_FUNCTION (this);

    m_running = false;

    for (uint32_t i = 0; i < m_sendInfos.size (); i++)
    {
        if (m_sendInfos[i].m_event.IsRunning ())
            Simulator::Cancel (m_sendInfos[i].m_event);

        m_sendInfos[i].m_socket->Close ();
        m_sendInfos[i].m_socket = NULL;
    }
    m_sendInfos.clear ();

    // Close accepted sockets
    std::map<Ptr<Socket>, ReceiveInfo>::iterator it;
    for (it = m_acceptSocketMap.begin (); it != m_acceptSocketMap.end (); it++)
        it->first->Close ();
    m_acceptSocketMap.clear();
   
    // Close socket for receiving
    m_rxSocket->Close();
    m_rxSocket->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket> > (),
                                   MakeNullCallback<void, Ptr<Socket> > ());
    m_rxSocket->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                                   MakeNullCallback<void, Ptr<Socket>, const Address &> ());
    m_rxSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
}

void 
DataCenterApp::KickOffSending (void)
{
    NS_LOG_FUNCTION (this);
    switch (m_sendParams.m_sendPattern)
    {
        case FIXED_INTERVAL:
        case RANDOM_INTERVAL:
        {
            // Start first iteration
            BulkSendPackets();
            break;
        }
        case FIXED_SPORADIC:
        {
            // Check we have packets to send
            if (m_totalPacketsSent < (m_sendParams.m_nIterations * m_sendParams.m_nReceivers))
            {
                // This starts as sending at random times and then continues
                // as a fixed interval between receivers
                switch (m_sendParams.m_receivers)
                {
                    case ALL_IN_LIST:
                    {
                        // Schedule a send for every receiver
                        for (uint32_t i = 0; i < m_sendInfos.size(); i++)
                        {
                            Time interval = SelectRandomInterval ();
                            m_sendInfos[i].m_event =
                                Simulator::Schedule (interval, &DataCenterApp::SendPacket, this, i); 
                        } 
                        break;
                    }
                    case RANDOM_SUBSET:
                    {
                        // Schedule event for random receivers
                        for (uint32_t i = 0; i < m_sendParams.m_nReceivers; i++)
                        {
                            Time interval = SelectRandomInterval ();
                            uint32_t receiver = SelectRandomReceiver ();
                            m_sendInfos[receiver].m_event = 
                                Simulator::Schedule (interval, &DataCenterApp::SendPacket, this, receiver);
                        }
                        break;
                    }
                    default:
                        NS_LOG_ERROR ("Invalid receivers specifier");
                        break;
                }
            }
            break;
        }
        case RANDOM_SPORADIC:
        {
            // Check we have packets to send
            if (m_totalPacketsSent < (m_sendParams.m_nIterations * m_sendParams.m_nReceivers))
            {
                // This just schedules random events to start off with
                switch (m_sendParams.m_receivers)
                {
                    case ALL_IN_LIST:
                        // Schedule send for every receiver
                        for (uint32_t i = 0; i < m_sendInfos.size(); i++)
                            ScheduleSend (i);
                        break;
                    case RANDOM_SUBSET:
                        // Schedule send for random receivers
                        for (uint32_t i = 0; i < m_sendParams.m_nReceivers; i++)
                            ScheduleSend (SelectRandomReceiver ());
                        break;
                    default:
                        NS_LOG_ERROR ("Invalid receivers specifier");
                        break;
                }
            }   
            break;
        }
        default:
            NS_LOG_ERROR ("Invalid send pattern specified");
            break;
    }
}

bool
DataCenterApp::HandleConnectionRequest (Ptr<Socket> socket, const Address& from)
{
    NS_LOG_FUNCTION (this << socket << from);
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " Connection Request Received:\n" <<
                 "    Source: " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << "\n" <<
                 "    Destination: " << GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () << "\n" <<
                 "    Time: " << Simulator::Now());
    // Allow the connection
    return true;
}

void
DataCenterApp::HandleAccept (Ptr<Socket> socket, const Address& from)
{
    NS_LOG_FUNCTION (this << socket << from);
    // Setup receive callback
    socket->SetRecvCallback (MakeCallback (&DataCenterApp::HandleRead, this));
    // Add socket and info to map
    ReceiveInfo recvInfo;
    InitReceiveInfo (recvInfo);
    m_acceptSocketMap[socket] = recvInfo;
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " Connection Accepted:\n" <<
                 "    Source: " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << "\n" <<
                 "    Destination: " << GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () << "\n" <<
                 "    Time: " << Simulator::Now()); 
}

void
DataCenterApp::HandleRead (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    
    // Read from socket
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom (from)))
    {
        if (packet->GetSize () == 0)
            break;
        else
        {
            // Log received packet
            SeqTsHeader seqTs;
            packet->RemoveHeader (seqTs);
            uint32_t currentSeqNum = seqTs.GetSeq ();
            uint32_t bytesReceived = packet->GetSize ();
            m_acceptSocketMap[socket].m_packetsReceived++;
            m_acceptSocketMap[socket].m_bytesReceived += bytesReceived;
            NS_LOG_INFO ("Node " << GetNode ()->GetId () << " RX:\n" <<
                         "    Source: " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << "\n" <<
                         "    Destination: " << GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () 
                              << "\n" <<
                         "    Payload Size: " << bytesReceived << " bytes\n" <<
                         "    Sequence Number: " << currentSeqNum << "\n" << 
                         "    UID: " << packet->GetUid () << "\n" <<
                         "    TXTime: " << seqTs.GetTs () << "\n" <<
                         "    RXTime: " << Simulator::Now() << "\n" <<
                         "    Delay: " << Simulator::Now() - seqTs.GetTs () << "\n" <<
                         "    Packets Received: " << m_acceptSocketMap[socket].m_packetsReceived << "\n" <<
                         "    Bytes Received: " << m_acceptSocketMap[socket].m_bytesReceived);
        }
    }
}

void
DataCenterApp::HandleClose (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " Connection Closed:\n" <<
                 "    Time: " << Simulator::Now());
}

void
DataCenterApp::HandleError (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_ERROR ("Node " << GetNode ()->GetId () << " Connection Error:\n" <<
                  "    Time: " << Simulator::Now());
}

void
DataCenterApp::HandleConnectionSucceeded (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " Connection Successful:\n" << 
                 "    Time: " << Simulator::Now());
}

void
DataCenterApp::HandleConnectionFailed (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_ERROR ("Node " << GetNode ()->GetId () << " Connection Failed:\n" <<
                  "    Time: " << Simulator::Now());
}

void 
DataCenterApp::BulkSendPackets ()
{
    NS_LOG_FUNCTION (this);
    NS_ASSERT (m_sendInfos[0].m_event.IsExpired ());
      
    // Send if we are still running and there is still an iteration to do 
    if (m_running && m_iterationCount < m_sendParams.m_nIterations)
    {
        switch (m_sendParams.m_receivers)
        {
            case ALL_IN_LIST:
            {
                // Send a packet for all send infos
                for (uint32_t i = 0; i < m_sendInfos.size(); i++)
                    DoSendPacket (m_sendInfos[i]); 

                break;
            }
            case RANDOM_SUBSET:
            {
                // Choose random subset of receivers
                std::unordered_set<uint32_t> receivers;
                SelectRandomReceiverSubset (receivers);

                // Send to subset of receivers
                std::unordered_set<uint32_t>::iterator it;
                for (it = receivers.begin (); it != receivers.end (); it++)
                    DoSendPacket (m_sendInfos[*it]);
                
                break;
            }
            default:
                NS_LOG_ERROR ("Invalid receivers specifier");
                break;
        }

        m_iterationCount++;
    }

    // If there is still an iteration to do, schedule it
    if (m_iterationCount < m_sendParams.m_nIterations)
        BulkScheduleSend();
}

void
DataCenterApp::SendPacket (uint32_t index)
{
    NS_LOG_FUNCTION (this);

    // Do the actual send if we still have a packet to send
    if (m_totalPacketsSent < (m_sendParams.m_nIterations * m_sendParams.m_nReceivers))
        DoSendPacket (m_sendInfos[index]);

    // Schedule the next send if there is another packkt to send
    if (m_totalPacketsSent < (m_sendParams.m_nIterations * m_sendParams.m_nReceivers))
        ScheduleSend(index);
}

void
DataCenterApp::DoSendPacket (SendInfo& sendInfo)
{
    NS_LOG_FUNCTION (this);

    if (m_sendParams.m_packetSize > MAX_PACKET_SIZE)
    {
        NS_LOG_ERROR ("Packet size is greater than max packet size");
        return;
    }

    // Create a header with sequence number and time
    SeqTsHeader seqTs;
    seqTs.SetSeq (sendInfo.m_packetsSent);

    // Create packet and add header
    Ptr<Packet> packet = Create<Packet> (m_sendParams.m_packetSize);
    packet->AddHeader (seqTs);
    sendInfo.m_socket->Send (packet);
    sendInfo.m_packetsSent++;
    sendInfo.m_bytesSent += m_sendParams.m_packetSize;
    m_totalPacketsSent++;

    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " TX:\n" <<
                 "    Source: " << GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () << "\n" <<
                 "    Destination: " << Ipv4Address::ConvertFrom (sendInfo.m_address) << "\n" <<
                 "    Packet Size: " << m_sendParams.m_packetSize << "\n" <<
                 "    Sequence Number: " << seqTs.GetSeq () << "\n" <<
                 "    TXTime: " << seqTs.GetTs() << "\n" <<
                 "    Packets Sent: " << sendInfo.m_packetsSent << "\n" <<
                 "    Bytes Sent: " << sendInfo.m_bytesSent);
}

void
DataCenterApp::BulkScheduleSend ()
{
    NS_LOG_FUNCTION (this);
    
    if (m_running)
    {
        switch (m_sendParams.m_sendPattern)
        {
            case FIXED_INTERVAL:
            {
                // Just use send event for 0th send info since we only need to schedule one event
                m_sendInfos[0].m_event = 
                    Simulator::Schedule (m_sendParams.m_sendInterval, &DataCenterApp::BulkSendPackets, this);
                break;
            }
            case RANDOM_INTERVAL:
            {
                // Choose a random interval
                Time interval = SelectRandomInterval ();
                // Just use send event for 0th send info since we only need to schedule one event
                m_sendInfos[0].m_event = 
                    Simulator::Schedule (interval, &DataCenterApp::BulkSendPackets, this);
                break;
            }
            default:
                NS_LOG_ERROR ("BulkScheduleSend called with invalid send pattern");
                break;
        }
    }
}

void
DataCenterApp::ScheduleSend (uint32_t index)
{
    NS_LOG_FUNCTION (this);

    // Check we are still running
    if (m_running)
    {
        switch (m_sendParams.m_sendPattern)
        {
            case FIXED_SPORADIC:
            {
                // Send based on send interval
                switch (m_sendParams.m_receivers)
                {
                    case ALL_IN_LIST:
                    {
                        // Schedule for index passed in
                        m_sendInfos[index].m_event = 
                            Simulator::Schedule (m_sendParams.m_sendInterval, &DataCenterApp::SendPacket, 
                                                 this, index);
                        break;
                    }
                    case RANDOM_SUBSET:
                    { 
                        // Pick a new random receiver and schedule for it
                        uint32_t receiver = SelectRandomReceiver ();
                        m_sendInfos[receiver].m_event =    
                            Simulator::Schedule (m_sendParams.m_sendInterval, &DataCenterApp::SendPacket,
                                                 this, receiver);
                        break;
                    }
                    default:
                        NS_LOG_ERROR ("Invalid receivers specifier");
                        break;
                }
                break;
            }
            case RANDOM_SPORADIC:
            {
                // Send based on random interval
                switch (m_sendParams.m_receivers)
                {
                    case ALL_IN_LIST:
                    {
                        // Schedule for index passed in
                        m_sendInfos[index].m_event = 
                            Simulator::Schedule (SelectRandomInterval (), &DataCenterApp::SendPacket,
                                                 this, index);
                        break;
                    }
                    case RANDOM_SUBSET:
                    {
                        // Pick a new random receiver and schedule for it
                        uint32_t receiver = SelectRandomReceiver ();
                        m_sendInfos[receiver].m_event = 
                            Simulator::Schedule (SelectRandomInterval (), &DataCenterApp::SendPacket,
                                                 this, receiver);
                        break;
                    }
                    default:
                        NS_LOG_ERROR ("Invalid receivers specifier");
                        break;
                }
                break;
            }
            default:
                NS_LOG_ERROR ("ScheduleSend called with invalid send pattern");
                break;
        }
    }
}

void 
DataCenterApp::SelectRandomReceiverSubset (std::unordered_set<uint32_t>& subset)
{
    NS_LOG_FUNCTION (this);

    // Check we will not infinite loop when picking a receiver
    if (m_sendParams.m_nReceivers > m_sendParams.m_nodes.size())
    {
        NS_LOG_ERROR ("Number of receivers is greater than number of nodes");
        return;
    }
    
    // Pick m_nReceivers unique receivers
    for (uint32_t i = 0; i < m_sendParams.m_nReceivers; i++)
    {
        // Pick a unique receiver
        while (1)
        {
            uint32_t candidate = SelectRandomReceiver ();
            if (subset.find(candidate) == subset.end())
            {
                subset.insert(candidate);
                break;
            }
        }        
    }

    NS_ASSERT (subset.size() == m_sendParams.m_nReceivers);
}

uint32_t
DataCenterApp::SelectRandomReceiver ()
{
    NS_LOG_FUNCTION (this);

    return (rand() % m_sendParams.m_nodes.size());
}

Time
DataCenterApp::SelectRandomInterval ()
{
    NS_LOG_FUNCTION (this);

    return NanoSeconds (rand () %
                        (m_sendParams.m_maxSendInterval.GetNanoSeconds () -
                         m_sendParams.m_minSendInterval.GetNanoSeconds () + 1) +
                        m_sendParams.m_minSendInterval.GetNanoSeconds());
}
