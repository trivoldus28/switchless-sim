/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 *  Author: Michael McKeown
 */

#include "data-center-app.h"

NS_LOG_COMPONENT_DEFINE ("DataCenterApp");
NS_OBJECT_ENSURE_REGISTERED (DataCenterApp);

DataCenterApp::DataCenterApp ()
  : m_sendParams (),
    m_sendEvent (),
    m_setup (false),
    m_running (false),
    m_packetsSent (0),
    m_bytesSent (0),
    m_packetsReceived (0),
    m_bytesReceived (0),
    m_txSockets (NULL),
    m_rxSocket (NULL),
    m_acceptSocketList ()
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
        NS_LOG_WARN ("Application started before calling DataCenterApp::Setup");

    m_running = true;

    // TODO: I think this may need to be a vector of packets sent for each socket
    m_packetsSent = 0;
    m_bytesSent = 0;
    m_packetsReceived = 0;
    m_bytesReceived = 0;

    // Setup socket for receiving
    m_rxSocket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
    Address local (InetSocketAddress (Ipv4Address::GetAny (), 8080));
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
        // Allocate space for sending socket pointers
        m_txSockets = (Ptr<Socket>*)malloc (m_sendParams.m_nReceivers * sizeof(Socket*)); 

        // TODO: Need to do different things for different ways of choosing receivers
        // Setup sockets for sending
        for (int i = 0; i < m_sendParams.m_nReceivers; i++)
        {
            m_txSockets[i] = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
            Address nodeAddress (InetSocketAddress (m_sendParams.m_nodes[i], 8080));
            m_txSockets[i]->Bind ();
            m_txSockets[i]->Connect (nodeAddress);
            m_txSockets[i]->SetConnectCallback (MakeCallback (&DataCenterApp::HandleConnectionSucceeded, this),
                                                MakeCallback (&DataCenterApp::HandleConnectionFailed, this));
            // TODO: This may need to be different based on different send patterns
            //       and whether the first packet is sent at time 0 or after an interval    
            SendPacket(i);
        }
    }
}

void
DataCenterApp::StopApplication (void)
{
    NS_LOG_FUNCTION (this);

    m_running = false;

    // Cancel send event if it is running
    if (m_sendEvent.IsRunning ())
        Simulator::Cancel (m_sendEvent);

    // Cleanup sending sockets if this app is sending
    if (m_sendParams.m_sending)
    {
        // Close sockets for sending
        for (int i = 0; i < m_sendParams.m_nReceivers; i++)
        {
            m_txSockets[i]->Close ();
            m_txSockets[i] = NULL;
        }

        // Free memory allocated for sockets
        free(m_txSockets);
    }

    // Close accepted sockets
    while (!m_acceptSocketList.empty ())
    {
        Ptr<Socket> acceptedSocket = m_acceptSocketList.front();
        m_acceptSocketList.pop_front();
        acceptedSocket->Close();
    }
    
    // Close socket for receiving
    m_rxSocket->Close();
    m_rxSocket->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket> > (),
                                   MakeNullCallback<void, Ptr<Socket> > ());
    m_rxSocket->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                                   MakeNullCallback<void, Ptr<Socket>, const Address &> ());
    m_rxSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
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
    // Keep socket around
    m_acceptSocketList.push_back (socket); 
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
            uint32_t currentSeqNum = seqTs.GetSeq ();
            m_packetsReceived++;
            m_bytesReceived += packet->GetSize ();

            packet->RemoveHeader (seqTs);
            // TODO: Total packets received and total bytes received
            NS_LOG_INFO ("Node " << GetNode ()->GetId () << " RX:\n" <<
                         "    Source: " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << "\n" <<
                         "    Destination: " << GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () 
                              << "\n" <<
                         "    Payload Size: " << packet->GetSize () << " bytes\n" <<
                         "    Sequence Number: " << currentSeqNum << "\n" << 
                         "    UID: " << packet->GetUid () << "\n" <<
                         "    TXTime: " << seqTs.GetTs () << "\n" <<
                         "    RXTime: " << Simulator::Now() << "\n" <<
                         "    Delay: " << Simulator::Now() - seqTs.GetTs () << "\n" <<
                         "    Packets Received: " << m_packetsReceived << "\n" <<
                         "    Bytes Received: " << m_bytesReceived);
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
DataCenterApp::SendPacket (uint32_t sockIndex)
{
    NS_LOG_FUNCTION (this);
    
    Ptr<Packet> packet = Create<Packet> (m_sendParams.m_packetSize);
    m_txSockets[sockIndex]->Send (packet);
    m_packetsSent++;
    m_bytesSent += m_sendParams.m_packetSize;

    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " TX:\n" <<
                 "    Source: " << GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () << "\n" <<
                 "    Destination: " << "TODO" << "\n" <<
                 "    Packet Size: " << m_sendParams.m_packetSize << "\n" <<
                 "    TXTime: " << Simulator::Now() << "\n" <<
                 "    Packets Sent: " << m_packetsSent << "\n" <<
                 "    Bytes Sent: " << m_bytesSent);

    // TODO: This may need to be different for different send patterns
    if (m_packetsSent < m_sendParams.m_nPackets)
        ScheduleSend(sockIndex);
}

void
DataCenterApp::ScheduleSend (uint32_t sockIndex)
{
    NS_LOG_FUNCTION (this);

    // TODO: Setup a send event based on the sending pattern.
    //       Also, I think this should probably be a vector of send events for each socket
    if (m_running)
        m_sendEvent = Simulator::Schedule (m_sendParams.m_sendInterval, &DataCenterApp::SendPacket, this, sockIndex);
}
