/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 *  Author: Michael McKeown
 */

#include "data-center-app.h"

NS_LOG_COMPONENT_DEFINE ("DataCenterApp");
NS_OBJECT_ENSURE_REGISTERED (DataCenterApp);

DataCenterApp::DataCenterApp ()
  : m_sendParams (),
    m_setup (false),
    m_running (false),
    m_sendInfos (),
    m_rxSocket (),
    m_acceptSocketMap ()
{
    NS_LOG_FUNCTION (this);
    // Default sending parameters
    m_sendParams.m_sending = false;
    m_sendParams.m_nodes = NULL;
    m_sendParams.m_receivers = RECEIVERS_INVALID;
    m_sendParams.m_nReceivers = 0;
    m_sendParams.m_sendPattern = SEND_PATTERN_INVALID;
    m_sendParams.m_sendInterval = MilliSeconds(100.);
    m_sendParams.m_packetSize = 1024;
    m_sendParams.m_nPackets = 100;
}

DataCenterApp::~DataCenterApp ()
{
    NS_LOG_FUNCTION (this);
}

bool
DataCenterApp::Setup (SendParams& sendingParams, uint32_t nodeId, bool debug)
{
    NS_LOG_FUNCTION (this << debug);

    if (sendingParams.m_nReceivers > sendingParams.m_nNodes)
    {
        NS_LOG_ERROR ("Number of receivers is greater than number of nodes");
        return false;
    }

    m_sendParams = sendingParams;

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
        switch (m_sendParams.m_receivers)
        {
            case ALL_IN_LIST:
            {
                // Setup socket for each node in list
                for (int i = 0; i < m_sendParams.m_nNodes; i++)
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

                break;
            }
            case RANDOM_SUBSET:
            {
                // Check we will not infinite loop when picking a receiver
                if (m_sendParams.m_nReceivers > m_sendParams.m_nNodes)
                {
                    NS_LOG_ERROR ("Number of receivers is greater than number of nodes");
                    break;
                }

                // Setup socket for each randomly picked receiver
                std::set<int> pickedReceivers;
                for (int i = 0; i < m_sendParams.m_nReceivers; i++)
                {
                    Ptr<Socket> socket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
                    
                    // Pick receiver
                    int receiver = -1;
                    while (receiver == -1)
                    {
                        int candidate = rand() % m_sendParams.m_nNodes;
                        if (pickedReceivers.find(candidate) == pickedReceivers.end())
                        {
                            pickedReceivers.insert(candidate);
                            receiver = candidate;
                            break;
                        }
                    }

                    Address nodeAddress (InetSocketAddress (m_sendParams.m_nodes[receiver], PORT));
                    socket->Bind ();
                    socket->Connect (nodeAddress);
                    socket->SetConnectCallback (MakeCallback (&DataCenterApp::HandleConnectionSucceeded, this),
                                                MakeCallback (&DataCenterApp::HandleConnectionFailed, this));
                    SendInfo sendInfo;
                    InitSendInfo (sendInfo, m_sendParams.m_nodes[receiver], socket);
                    m_sendInfos.push_back (sendInfo);
                }
                KickOffSending();

                break;
            }
            default:
                NS_LOG_ERROR ("Invalid receivers specifier");
                break;
        }
    }
}

void
DataCenterApp::StopApplication (void)
{
    NS_LOG_FUNCTION (this);

    m_running = false;

    for (int i = 0; i < m_sendInfos.size (); i++)
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
    m_rxSocket->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket>> (),
                                   MakeNullCallback<void, Ptr<Socket>> ());
    m_rxSocket->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                                   MakeNullCallback<void, Ptr<Socket>, const Address &> ());
    m_rxSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket>> ());
}

void 
DataCenterApp::KickOffSending (void)
{
    NS_LOG_FUNCTION (this);
    switch (m_sendParams.m_sendPattern)
    {
        case FIXED_INTERVAL:
        case RANDOM_INTERVAL:
            BulkSendPackets();
            break;
        case RANDOM_SPORADIC:
            for (int i = 0; i < m_sendInfos.size(); i++)
                ScheduleSend(i);
            break;
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
            uint32_t bytesReceived = packet->GetSize ();
            SeqTsHeader seqTs;
            packet->RemoveHeader (seqTs);
            uint32_t currentSeqNum = seqTs.GetSeq ();
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
    
    // Send a packet for all send infos
    for (int i = 0; i < m_sendInfos.size(); i++)
        DoSendPacket (m_sendInfos[i]);
    
    // Schedule next bulk send (just have to check packets sent for
    // one send info since all are happening synchronized, they should
    // all be the same)
    if (m_sendInfos[0].m_packetsSent < m_sendParams.m_nPackets)
        BulkScheduleSend();
}

void
DataCenterApp::SendPacket (uint32_t index)
{
    NS_LOG_FUNCTION (this);

    // Do the actual send
    DoSendPacket (m_sendInfos[index]);

    // Schedule the next send if there is another packket to send
    if (m_sendInfos[index].m_packetsSent < m_sendParams.m_nPackets)
        ScheduleSend(index);
}

void
DataCenterApp::DoSendPacket (SendInfo& sendInfo)
{
    NS_LOG_FUNCTION (this);

    Ptr<Packet> packet = Create<Packet> (m_sendParams.m_packetSize);
    sendInfo.m_socket->Send (packet);
    sendInfo.m_packetsSent++;
    sendInfo.m_bytesSent += m_sendParams.m_packetSize;

    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " TX:\n" <<
                 "    Source: " << GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () << "\n" <<
                 "    Destination: " << Ipv4Address::ConvertFrom (sendInfo.m_address) << "\n" <<
                 "    Packet Size: " << m_sendParams.m_packetSize << "\n" <<
                 "    UID: " << packet->GetUid() << "\n" <<
                 "    TXTime: " << Simulator::Now() << "\n" <<
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
                Time interval = NanoSeconds((((double) rand()) / ((double) RAND_MAX)) * 
                                m_sendParams.m_sendInterval.GetNanoSeconds());
                // Just use send event for 0th send info since we only need to schedule one event
                m_sendInfos[0].m_event = 
                    Simulator::Schedule (interval, &DataCenterApp::BulkSendPackets, this);
                break;
            }
            default:
                NS_LOG_ERROR ("BulkScheduleSend called with invald send pattern");
                break;
        }
    }
}

void
DataCenterApp::ScheduleSend (uint32_t index)
{
    NS_LOG_FUNCTION (this);

    if (m_running)
    {
        switch (m_sendParams.m_sendPattern)
        {
            case RANDOM_SPORADIC:
            {
                // Choose a random interval
                Time interval = NanoSeconds((((double) rand()) / ((double) RAND_MAX)) * 
                                m_sendParams.m_sendInterval.GetNanoSeconds());
                m_sendInfos[index].m_event =
                    Simulator::Schedule (interval, &DataCenterApp::SendPacket, this, index);
                break;
            }
            default:
                NS_LOG_ERROR ("ScheduleSend called with invalid send pattern");
                break;
        }
    }
}
