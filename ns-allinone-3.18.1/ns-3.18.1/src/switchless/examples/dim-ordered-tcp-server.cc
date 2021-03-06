/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-tcp-server.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedTcpServer");
NS_OBJECT_ENSURE_REGISTERED (DimensionOrderedTcpServer);

DimensionOrderedTcpServer::DimensionOrderedTcpServer ()
  : m_setup (false),
    m_running (false),
    m_rxSocket (0),
    m_acceptSockets ()
{
    NS_LOG_FUNCTION (this);
}

DimensionOrderedTcpServer::~DimensionOrderedTcpServer ()
{
    NS_LOG_FUNCTION (this);
}

bool
DimensionOrderedTcpServer::Setup ()
{
    NS_LOG_FUNCTION (this);

    m_setup = true;
    return true;
}

void
DimensionOrderedTcpServer::StartApplication (void)
{
    NS_LOG_FUNCTION (this);

    if (!m_setup)
        NS_LOG_WARN ("Application started before calling DimensionOrderedTcpClient::Setup. Using defaults.");

    m_running = true;
    // Create socket and connect to server
    Ptr<DimensionOrdered> dimOrdered = GetNode ()->GetObject<DimensionOrdered> ();
    NS_ASSERT_MSG (dimOrdered, "Application started without a DimensionOrdered stack installed on the node");
    m_rxSocket = Socket::CreateSocket (GetNode (), DoTcpSocketFactory::GetTypeId ());
    DimensionOrderedSocketAddress local = DimensionOrderedSocketAddress (DimensionOrderedAddress::GetAny (), PORT);
    m_rxSocket->Bind (local);
    m_rxSocket->Listen ();
    m_rxSocket->SetRecvCallback (MakeCallback (&DimensionOrderedTcpServer::HandleRead, this));
    m_rxSocket->SetAcceptCallback (MakeCallback (&DimensionOrderedTcpServer::HandleConnectionRequest, this),
                                   MakeCallback (&DimensionOrderedTcpServer::HandleAccept, this));
    m_rxSocket->SetCloseCallbacks (MakeCallback (&DimensionOrderedTcpServer::HandleClose, this),
                                   MakeCallback (&DimensionOrderedTcpServer::HandleError, this));
}

void
DimensionOrderedTcpServer::StopApplication (void)
{
    NS_LOG_FUNCTION (this);

    m_running = false;
    
    // Close accepted sockets
    std::vector<Ptr<Socket> >::iterator it;
    for (it = m_acceptSockets.begin (); it != m_acceptSockets.end (); it++)
        (*it)->Close ();
    m_acceptSockets.clear ();

    // Close RX socket
    m_rxSocket->Close ();
}

void
DimensionOrderedTcpServer::SendResponsePacket (Ptr<Socket> socket, Address& to)
{
    NS_LOG_FUNCTION (this << socket << to);

    Ptr<Packet> packet = Create<Packet> (PACKET_SIZE);
    socket->SendTo (packet, 0, to);

    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " sending response to client:\n" <<
                 "      Source: " << GetNode ()->GetObject<DimensionOrdered> ()->GetAddress (DimensionOrdered::X_POS).GetLocal () << "\n" <<
                 "      Destination: " << DimensionOrderedSocketAddress::ConvertFrom (to).GetDimensionOrderedAddress () << "\n" <<
                 "      Time: " << Simulator::Now ());
}

bool
DimensionOrderedTcpServer::HandleConnectionRequest (Ptr<Socket> socket, const Address &from)
{
    NS_LOG_FUNCTION (this << socket << from);
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " connection request received:\n" <<
                 "      Source: " << DimensionOrderedSocketAddress::ConvertFrom (from).GetDimensionOrderedAddress () << "\n" <<
                 "      Destination: " << GetNode ()->GetObject<DimensionOrdered> ()->GetAddress (DimensionOrdered::X_POS).GetLocal () << "\n" <<
                 "      Time: " << Simulator::Now ());
    return true;
}

void
DimensionOrderedTcpServer::HandleAccept (Ptr<Socket> socket, const Address &from)
{
    NS_LOG_FUNCTION (this << socket << from);
    socket->SetRecvCallback (MakeCallback (&DimensionOrderedTcpServer::HandleRead, this));
    m_acceptSockets.push_back (socket);
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " connection accepted:\n" <<
                 "      Source: " << DimensionOrderedSocketAddress::ConvertFrom (from).GetDimensionOrderedAddress () << "\n" <<
                 "      Destination: " << GetNode ()->GetObject<DimensionOrdered> ()->GetAddress (DimensionOrdered::X_POS).GetLocal () << "\n" <<
                 "      Time: " << Simulator::Now ());
}

void
DimensionOrderedTcpServer::HandleRead (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);

    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom (from)))
    {
        if (packet->GetSize () > 0)
        {
            NS_LOG_INFO ("Node " << GetNode()->GetId () << " received request from client:\n" <<
                         "      Source : " << DimensionOrderedSocketAddress::ConvertFrom (from).GetDimensionOrderedAddress () << "\n" <<
                         "      Destination: " << GetNode ()->GetObject<DimensionOrdered> ()->GetAddress (DimensionOrdered::X_POS).GetLocal () << "\n" <<
                         "      Packet Size: " << packet->GetSize () << " bytes\n" <<
                         "      Time: " << Simulator::Now ());
            NS_LOG_INFO ("Sending response...");
            SendResponsePacket (socket, from);
        }
    }
}

void
DimensionOrderedTcpServer::HandleClose (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " connection closed:\n" <<
                 "     Time: " << Simulator::Now());
}

void
DimensionOrderedTcpServer::HandleError (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_ERROR ("Node " << GetNode ()->GetId () << " connection error:\n" <<
                  "     Time: " << Simulator::Now());
}
