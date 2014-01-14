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
    m_stack (INVALID_STACK),
    m_iterationCount (0),
    m_totalPacketsSent (0),
    m_responseCount (0),
    m_sendInfos (),
    m_socketIndexMap (),
    m_rxSocket (),
    m_acceptSocketMap ()
{
    NS_LOG_FUNCTION (this);
    // Default sending parameters
    m_sendParams.m_sending = false;
    m_sendParams.m_receivers = RECEIVERS_INVALID;
    m_sendParams.m_nReceivers = 0;
    m_sendParams.m_sendPattern = SEND_PATTERN_INVALID;
    m_sendParams.m_sendInterval = MilliSeconds (0.);
    m_sendParams.m_maxSendInterval = MilliSeconds (0.0);
    m_sendParams.m_minSendInterval = MilliSeconds (0.0);
    m_sendParams.m_packetSize = 1024;
    m_sendParams.m_nIterations = 0;
}

DataCenterApp::~DataCenterApp ()
{
    NS_LOG_FUNCTION (this);
}

bool
DataCenterApp::Setup (SendParams& sendingParams, uint32_t nodeId, NETWORK_STACK stack, bool debug)
{
    NS_LOG_FUNCTION (this << debug);

    if (stack == INVALID_STACK)
    {
        NS_LOG_ERROR ("Invalid stack specified in DataCenterApp::Setup()");
        return false;
    } 

    if (sendingParams.m_sending && sendingParams.m_nReceivers > sendingParams.m_nodes.size())
    {
        NS_LOG_ERROR ("Number of receivers is greater than number of nodes");
        return false;
    }

    if (sendingParams.m_sending && sendingParams.m_packetSize > MAX_PACKET_SIZE)
    {
        NS_LOG_ERROR ("Packet size is greater than max packet size");
        return false;
    }

    if (sendingParams.m_sending && sendingParams.m_minSendInterval > sendingParams.m_maxSendInterval)
    {
        NS_LOG_ERROR ("Min send interval is greater than max send interval");
        return false;
    }

    m_stack = stack;
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
DataCenterApp::SetupRXSocket (void)
{
    NS_LOG_FUNCTION (this);

    switch (m_stack)
    {
        case UDP_IP_STACK:
        {
            m_rxSocket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
            //m_rxSocket->SetAttribute ("SegmentSize", UintegerValue(m_sendParams.m_packetSize+11));
            //m_rxSocket->SetAttribute ("SndBufSize", UintegerValue(16384));
            //m_rxSocket->SetAttribute ("RcvBufSize", UintegerValue(16384));
            //m_rxSocket->SetAttribute ("SegmentSize", UintegerValue(16384));
            //m_rxSocket->SetAttribute ("ConnTimeout", UintegerValue(3)); 
            Address local (InetSocketAddress (Ipv4Address::GetAny (), PORT));
            m_rxSocket->Bind (local);
            break;
        }
        case TCP_IP_STACK:
        {
            m_rxSocket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
            m_rxSocket->SetAttribute ("SegmentSize", UintegerValue(m_sendParams.m_packetSize+11));
            //m_rxSocket->SetAttribute ("SndBufSize", UintegerValue(16384));
            //m_rxSocket->SetAttribute ("RcvBufSize", UintegerValue(16384));
            //m_rxSocket->SetAttribute ("SegmentSize", UintegerValue(16384));
            //m_rxSocket->SetAttribute ("ConnTimeout", UintegerValue(3)); 
            Address local (InetSocketAddress(Ipv4Address::GetAny (), PORT));
            m_rxSocket->Bind (local);
            m_rxSocket->Listen ();
            break;
        }
        case UDP_DO_STACK:
        {
            m_rxSocket = Socket::CreateSocket (GetNode (), DoUdpSocketFactory::GetTypeId ());
            //m_rxSocket->SetAttribute ("SegmentSize", UintegerValue(m_sendParams.m_packetSize+11));
            //m_rxSocket->SetAttribute ("SndBufSize", UintegerValue(16384));
            //m_rxSocket->SetAttribute ("RcvBufSize", UintegerValue(16384));
            //m_rxSocket->SetAttribute ("SegmentSize", UintegerValue(16384));
            //m_rxSocket->SetAttribute ("ConnTimeout", UintegerValue(3));
            Address local (DimensionOrderedSocketAddress (DimensionOrderedAddress::GetAny (), PORT));
            m_rxSocket->Bind (local); 
            break;
        }
        case TCP_DO_STACK:
            NS_LOG_WARN ("DimensionOrdered TCP Stack not yet supported");
            return;
            break;
        default:
            NS_LOG_ERROR ("Invalid network stack specified, did you call DataCenterApp::Setup()??");
            return;
            break;
    }

    m_rxSocket->SetRecvCallback (MakeCallback (&DataCenterApp::HandleRead, this));
    m_rxSocket->SetAcceptCallback (MakeCallback (&DataCenterApp::HandleConnectionRequest, this),
                                   MakeCallback (&DataCenterApp::HandleAccept, this));
    m_rxSocket->SetCloseCallbacks (MakeCallback (&DataCenterApp::HandleClose, this),
                                   MakeCallback (&DataCenterApp::HandleError, this)); 
}

void
DataCenterApp::SetupTXSocket (uint32_t sendParamsNodeIndex)
{
    NS_LOG_FUNCTION (this);
    
    Ptr<Socket> socket = 0;
    switch (m_stack)
    {
        case UDP_IP_STACK:
        {
            socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
            //socket->SetAttribute ("SegmentSize", UintegerValue(m_sendParams.m_packetSize+11));
            //socket->SetAttribute ("SndBufSize", UintegerValue(16384));
            //socket->SetAttribute ("RcvBufSize", UintegerValue(16384));
            //socket->SetAttribute ("SegmentSize", UintegerValue(16384));
            //socket->SetAttribute ("ConnTimeout", UintegerValue(3));
            Ipv4Address addr = Ipv4Address::ConvertFrom (m_sendParams.m_nodes[sendParamsNodeIndex]);
            Address nodeAddress (InetSocketAddress (addr, PORT));
            socket->Bind ();
            socket->Connect (nodeAddress);
            break;
        }
        case TCP_IP_STACK:
        {
            socket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
            socket->SetAttribute ("SegmentSize", UintegerValue(m_sendParams.m_packetSize+11));
            //socket->SetAttribute ("SndBufSize", UintegerValue(16384));
            //socket->SetAttribute ("RcvBufSize", UintegerValue(16384));
            //socket->SetAttribute ("SegmentSize", UintegerValue(16384));
            //socket->SetAttribute ("ConnTimeout", UintegerValue(3));
            Ipv4Address addr = Ipv4Address::ConvertFrom (m_sendParams.m_nodes[sendParamsNodeIndex]);
            Address nodeAddress (InetSocketAddress (addr, PORT));
            socket->Bind ();
            socket->Connect (nodeAddress);
            break;
        }
        case UDP_DO_STACK:
        {
            socket = Socket::CreateSocket (GetNode (), DoUdpSocketFactory::GetTypeId ());
            //socket->SetAttribute ("SegmentSize", UintegerValue(m_sendParams.m_packetSize+11));
            //socket->SetAttribute ("SndBufSize", UintegerValue(16384));
            //socket->SetAttribute ("RcvBufSize", UintegerValue(16384));
            //socket->SetAttribute ("SegmentSize", UintegerValue(16384));
            //socket->SetAttribute ("ConnTimeout", UintegerValue(3));
            DimensionOrderedAddress addr = DimensionOrderedAddress::ConvertFrom (m_sendParams.m_nodes[sendParamsNodeIndex]);
            Address nodeAddress (DimensionOrderedSocketAddress (addr, PORT));
            socket->Bind ();
            socket->Connect (nodeAddress);
            break;
        }
        case TCP_DO_STACK:
            NS_LOG_WARN ("DimensionOrdered TCP Stack not yet supported");
            return;
            break;
        default:
            NS_LOG_ERROR ("Invalid network stack specified, did you call DataCenterApp::Setup()??");
            return;
            break;
    }

    socket->SetConnectCallback (MakeCallback (&DataCenterApp::HandleConnectionSucceeded, this),
                                MakeCallback (&DataCenterApp::HandleConnectionFailed, this));
    socket->SetRecvCallback (MakeCallback (&DataCenterApp::HandleRead, this));
    SendInfo sendInfo;
    InitSendInfo (sendInfo, m_sendParams.m_nodes[sendParamsNodeIndex], socket);
    m_sendInfos.push_back (sendInfo);
    m_socketIndexMap[socket] = m_sendInfos.size() - 1;
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
    m_responseCount = 0;

    SetupRXSocket ();

    // Only open sending sockets if this app is sending
    if (m_sendParams.m_sending)
    {
        // Setup socket for each node in list
        for (uint32_t i = 0; i < m_sendParams.m_nodes.size(); i++)
            SetupTXSocket (i);
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
    m_socketIndexMap.clear();

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
    socket->SetAttribute ("SndBufSize", UintegerValue(1048576));
    socket->SetAttribute ("RcvBufSize", UintegerValue(1048576));
    //socket->SetAttribute ("SegmentSize", UintegerValue(1048576)); Can't do this
    // Allow the connection
    return true;
}

void
DataCenterApp::HandleAccept (Ptr<Socket> socket, const Address& from)
{
    NS_LOG_FUNCTION (this << socket << from);
    // Setup receive callback
    socket->SetRecvCallback (MakeCallback (&DataCenterApp::HandleRead, this));
    socket->SetAttribute ("SndBufSize", UintegerValue(1048576));
    socket->SetAttribute ("RcvBufSize", UintegerValue(1048576));
    //socket->SetAttribute ("SegmentSize", UintegerValue(1048576));  Can't do this
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
        {
            NS_LOG_INFO ("Got Here");
            break;
        }
        else
        {
            // Log received packet
            DCAppHeader hdr;
            packet->RemoveHeader (hdr);
            uint16_t currentSeqNum = hdr.GetSequenceNumber ();
            uint32_t bytesReceived = packet->GetSize ();
            m_acceptSocketMap[socket].m_packetsReceived++;
            m_acceptSocketMap[socket].m_bytesReceived += bytesReceived;

            if (InetSocketAddress::IsMatchingType (from))
            {
                NS_LOG_INFO ("Node " << GetNode ()->GetId () << " RX:\n" <<
                             "    Source: " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << "\n" <<
                             "    Destination: " << GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()
                                    << "\n" <<
                             "    Packet Size: " << bytesReceived << " bytes\n" <<
                             "    Packet Type: " << DCAppHeader::PacketTypeToString (hdr.GetPacketType ()) 
                                << "\n"
                             "    Sequence Number: " << currentSeqNum << "\n" << 
                             "    UID: " << packet->GetUid () << "\n" <<
                             "    TXTime: " << hdr.GetTimeStamp () << "\n" <<
                             "    RXTime: " << Simulator::Now() << "\n" <<
                             "    Delay: " << Simulator::Now() - hdr.GetTimeStamp () << "\n" <<
                             "    Packets Received: " << m_acceptSocketMap[socket].m_packetsReceived << "\n" <<
                             "    Bytes Received: " << m_acceptSocketMap[socket].m_bytesReceived);
                NS_LOG_DEBUG ("   " <<  GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()  <<
                              "\t  Got " << DCAppHeader::PacketTypeToString (hdr.GetPacketType ()) << 
                              " from  \t" << InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
                              "   \t Time \t" << Simulator::Now() << " Delay : "
                              << Simulator::Now() - hdr.GetTimeStamp ());  
            }
            else if (DimensionOrderedSocketAddress::IsMatchingType (from))
            {
                // Determine destination
                Ptr<DimensionOrdered> dimOrdered = GetNode ()->GetObject<DimensionOrdered> ();
                DimensionOrderedAddress dst = dimOrdered->GetAddress (DimensionOrdered::X_POS).GetLocal ();
                if (dst == DimensionOrderedAddress::GetZero ())
                    dst = dimOrdered->GetAddress (DimensionOrdered::X_NEG).GetLocal ();
                NS_ASSERT (dst != DimensionOrderedAddress::GetZero ());

                NS_LOG_INFO ("Node " << GetNode ()->GetId () << " RX:\n" <<
                             "    Source: " << DimensionOrderedSocketAddress::ConvertFrom (from)
                                                    .GetDimensionOrderedAddress () << "\n" <<
                             "    Destination: " << dst << "\n" <<
                             "    Packet Size: " << bytesReceived << " bytes\n" <<
                             "    Packet Type: " << DCAppHeader::PacketTypeToString (hdr.GetPacketType ()) 
                                << "\n"
                             "    Sequence Number: " << currentSeqNum << "\n" <<
                             "    UID: " << packet->GetUid () << "\n" <<
                             "    TXTime: " << hdr.GetTimeStamp () << "\n" <<
                             "    RXTime: " << Simulator::Now() << "\n" <<
                             "    Delay: " << Simulator::Now() - hdr.GetTimeStamp () << "\n" <<
                             "    Packets Received: " << m_acceptSocketMap[socket].m_packetsReceived << "\n" <<
                             "    Bytes Received: " << m_acceptSocketMap[socket].m_bytesReceived);
                NS_LOG_DEBUG ("   " <<  dst  <<
                              "\t  Got " << DCAppHeader::PacketTypeToString (hdr.GetPacketType ()) <<
                              " from  \t" << DimensionOrderedSocketAddress::ConvertFrom (from)
                                                .GetDimensionOrderedAddress () <<
                              "   \t Time \t" << Simulator::Now() << " Delay : "
                              << Simulator::Now() - hdr.GetTimeStamp ()); 
            }

            // Do something with the packet depending on the type
            switch (hdr.GetPacketType ())
            {
                case DCAppHeader::REQUEST:
                    SendResponsePacket (socket, from, currentSeqNum);
                    break;
                case DCAppHeader::RESPONSE:
                    switch (m_sendParams.m_sendPattern)
                    {
                        case FIXED_INTERVAL:
                        case RANDOM_INTERVAL:
                            // Increment response count
                            m_responseCount++;
                            // If we received all responses, can schedule another iteration
                            if (m_responseCount == m_sendParams.m_nReceivers)
                            {
                                m_responseCount = 0;
                                // If there is still an iteration, schedule a new bulk send
                                if (m_iterationCount < m_sendParams.m_nIterations)
                                    BulkScheduleSend();
                            }
                            break;
                        case FIXED_SPORADIC:
                        case RANDOM_SPORADIC:
                            NS_ASSERT (m_socketIndexMap[socket] >= 0);
                            // If there are still packets to send, schedule one
                            if (m_totalPacketsSent < (m_sendParams.m_nIterations * m_sendParams.m_nReceivers))
                                ScheduleSend(m_socketIndexMap[socket]);
                            break;
                        case SEND_PATTERN_INVALID:
                            break;
                    }
                    break;
                default:
                    NS_LOG_ERROR ("Received packet with invalid type");
                    break;
            }
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
    socket->SetRecvCallback (MakeCallback (&DataCenterApp::HandleRead, this));
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
}

void
DataCenterApp::SendPacket (uint32_t index)
{
    NS_LOG_FUNCTION (this);

    // Do the actual send if we still have a packet to send
    if (m_totalPacketsSent < (m_sendParams.m_nIterations * m_sendParams.m_nReceivers))
        DoSendPacket (m_sendInfos[index]);
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
    DCAppHeader hdr;
    hdr.SetPacketType (DCAppHeader::REQUEST);
    hdr.SetSequenceNumber (sendInfo.m_packetsSent);

    // Create packet and add header
    Ptr<Packet> packet = Create<Packet> (m_sendParams.m_packetSize);
    packet->AddHeader (hdr);
    sendInfo.m_socket->Send (packet);
    sendInfo.m_packetsSent++;
    sendInfo.m_bytesSent += m_sendParams.m_packetSize;
    m_totalPacketsSent++;

    if (Ipv4Address::IsMatchingType (sendInfo.m_address))
    {
        NS_LOG_INFO ("Node " << GetNode ()->GetId () << " TX:\n" <<
                     "    Source: " << GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () << "\n" <<
                     "    Destination: " << Ipv4Address::ConvertFrom (sendInfo.m_address) << "\n" <<
                     "    Packet Size: " << m_sendParams.m_packetSize << "\n" <<
                     "    Packet Type: " <<  DCAppHeader::PacketTypeToString (hdr.GetPacketType()) << "\n" <<
                     "    Sequence Number: " << hdr.GetSequenceNumber () << "\n" <<
                     "    TXTime: " << hdr.GetTimeStamp() << "\n" <<
                     "    Packets Sent: " << sendInfo.m_packetsSent << "\n" <<
                     "    Bytes Sent: " << sendInfo.m_bytesSent);
        NS_LOG_DEBUG ("   "<< GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()  << 
                      "\t  Sent to \t\t" << Ipv4Address::ConvertFrom (sendInfo.m_address) << 
                      "   \t Time \t" << Simulator::Now()) ;
    }
    else if (DimensionOrderedAddress::IsMatchingType (sendInfo.m_address))
    {
        // Src address
        Ptr<DimensionOrdered> dimOrdered = GetNode ()->GetObject <DimensionOrdered> ();
        DimensionOrderedAddress src = dimOrdered->GetAddress (DimensionOrdered::X_POS).GetLocal ();
        if (src == DimensionOrderedAddress::GetZero ())
            src = dimOrdered->GetAddress (DimensionOrdered::X_NEG).GetLocal ();
        NS_ASSERT (src != DimensionOrderedAddress::GetZero ());

        NS_LOG_INFO ("Node " << GetNode ()->GetId () << " TX:\n" <<
                     "    Source: " << src << "\n" <<
                     "    Destination: " << DimensionOrderedAddress::ConvertFrom (sendInfo.m_address) << "\n" <<
                     "    Packet Size: " << m_sendParams.m_packetSize << "\n" <<
                     "    Packet Type: " <<  DCAppHeader::PacketTypeToString (hdr.GetPacketType()) << "\n" <<
                     "    Sequence Number: " << hdr.GetSequenceNumber () << "\n" <<
                     "    TXTime: " << hdr.GetTimeStamp() << "\n" <<
                     "    Packets Sent: " << sendInfo.m_packetsSent << "\n" <<
                     "    Bytes Sent: " << sendInfo.m_bytesSent);
        NS_LOG_DEBUG ("   "<< src  <<
                      "\t  Sent to \t\t" << DimensionOrderedAddress::ConvertFrom (sendInfo.m_address) << 
                      "   \t Time \t" << Simulator::Now()) ; 
    }
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
DataCenterApp::SendResponsePacket (Ptr<Socket> socket, Address& to, uint16_t sequenceNumber)
{
    NS_LOG_FUNCTION (this << socket);

    // Create a header
    DCAppHeader hdr;
    hdr.SetPacketType (DCAppHeader::RESPONSE);
    hdr.SetSequenceNumber (sequenceNumber);
    
    // Create a packet and add header
    Ptr<Packet> packet = Create<Packet> (0);
    packet->AddHeader (hdr);
    socket->SendTo (packet, 0, to);
   
    if (InetSocketAddress::IsMatchingType (to))
    { 
        NS_LOG_INFO ("Node " << GetNode ()->GetId () << " TX:\n" <<
                     "    Source: " << GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () << "\n" <<
                     "    Destination: " << InetSocketAddress::ConvertFrom (to).GetIpv4 () << "\n" <<
                     "    Packet Size: " << 0 << "\n" <<
                     "    Packet Type: " <<  DCAppHeader::PacketTypeToString (hdr.GetPacketType()) << "\n" <<
                     "    Sequence Number: " << hdr.GetSequenceNumber () << "\n" <<
                     "    TXTime: " << hdr.GetTimeStamp());
        NS_LOG_DEBUG ("   "<< GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()  <<
                      "\t  Sent ACK to   \t" << InetSocketAddress::ConvertFrom (to).GetIpv4 () <<
                      "   \t Time \t" << Simulator::Now()) ; 
    }
    else if (DimensionOrderedSocketAddress::IsMatchingType (to))
    {
        // Determine src
        Ptr<DimensionOrdered> dimOrdered = GetNode ()->GetObject<DimensionOrdered> ();
        DimensionOrderedAddress src = dimOrdered->GetAddress (DimensionOrdered::X_POS).GetLocal ();
        if (src == DimensionOrderedAddress::GetZero ())
            src = dimOrdered->GetAddress (DimensionOrdered::X_NEG).GetLocal ();
        NS_ASSERT (src != DimensionOrderedAddress::GetZero ());

        NS_LOG_INFO ("Node " << GetNode ()->GetId () << " TX:\n" <<
                     "    Source: " << src << "\n" <<
                     "    Destination: " << DimensionOrderedSocketAddress::ConvertFrom (to)
                                                .GetDimensionOrderedAddress () << "\n" <<
                     "    Packet Size: " << 0 << "\n" <<
                     "    Packet Type: " <<  DCAppHeader::PacketTypeToString (hdr.GetPacketType()) << "\n" <<
                     "    Sequence Number: " << hdr.GetSequenceNumber () << "\n" <<
                     "    TXTime: " << hdr.GetTimeStamp());
        NS_LOG_DEBUG ("   "<< src  <<
                      "\t  Sent ACK to   \t" << DimensionOrderedSocketAddress::ConvertFrom (to)
                                                    .GetDimensionOrderedAddress () <<
                      "   \t Time \t" << Simulator::Now()) ;
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
