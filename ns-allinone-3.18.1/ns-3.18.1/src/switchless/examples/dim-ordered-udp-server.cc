/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-udp-server.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedUdpServer");
NS_OBJECT_ENSURE_REGISTERED (DimensionOrderedUdpServer);

DimensionOrderedUdpServer::DimensionOrderedUdpServer ()
  : m_setup (false),
    m_running (false),
    m_rxSocket (0),
    m_acceptSockets ()
{
    NS_LOG_FUNCTION (this);
}

DimensionOrderedUdpServer::~DimensionOrderedUdpServer ()
{
    NS_LOG_FUNCTION (this);
}

bool
DimensionOrderedUdpServer::Setup ()
{
    NS_LOG_FUNCTION (this);

    m_setup = true;
    return true;
}

void
DimensionOrderedUdpServer::StartApplication (void)
{
    NS_LOG_FUNCTION (this);

    if (!m_setup)
        NS_LOG_WARN ("Application started before calling DimensionOrderedRawClient::Setup. Using defaults.");

    m_running = true;
    // Create socket and connect to server
    Ptr<DimensionOrdered> dimOrdered = GetNode ()->GetObject<DimensionOrdered> ();
    NS_ASSERT_MSG (dimOrdered, "Application started without a DimensionOrdered stack installed on the node");
    m_rxSocket = Socket::CreateSocket (GetNode (), DoUdpSocketFactory::GetTypeId ());
    DimensionOrderedSocketAddress local = DimensionOrderedSocketAddress (DimensionOrderedAddress::GetAny (), PORT);
    m_rxSocket->Bind (local);
    m_rxSocket->SetRecvCallback (MakeCallback (&DimensionOrderedUdpServer::HandleRead, this));
    m_rxSocket->SetAcceptCallback (MakeCallback (&DimensionOrderedUdpServer::HandleConnectionRequest, this),
                                   MakeCallback (&DimensionOrderedUdpServer::HandleAccept, this));
    m_rxSocket->SetCloseCallbacks (MakeCallback (&DimensionOrderedUdpServer::HandleClose, this),
                                   MakeCallback (&DimensionOrderedUdpServer::HandleError, this));
}

void
DimensionOrderedUdpServer::StopApplication (void)
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
DimensionOrderedUdpServer::SendResponsePacket (Ptr<Socket> socket, Address& to)
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
DimensionOrderedUdpServer::HandleConnectionRequest (Ptr<Socket> socket, const Address &from)
{
    NS_LOG_FUNCTION (this << socket << from);
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " connection request received:\n" <<
                 "      Source: " << DimensionOrderedSocketAddress::ConvertFrom (from).GetDimensionOrderedAddress () << "\n" <<
                 "      Destination: " << GetNode ()->GetObject<DimensionOrdered> ()->GetAddress (DimensionOrdered::X_POS).GetLocal () << "\n" <<
                 "      Time: " << Simulator::Now ());

}

void
DimensionOrderedUdpServer::HandleAccept (Ptr<Socket> socket, const Address &from)
{
    NS_LOG_FUNCTION (this << socket << from);
    socket->SetRecvCallback (MakeCallback (&DimensionOrderedUdpServer::HandleRead, this));
    m_acceptSockets.push_back (socket);
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " connection accepted:\n" <<
                 "      Source: " << DimensionOrderedSocketAddress::ConvertFrom (from).GetDimensionOrderedAddress () << "\n" <<
                 "      Destination: " << GetNode ()->GetObject<DimensionOrdered> ()->GetAddress (DimensionOrdered::X_POS).GetLocal () << "\n" <<
                 "      Time: " << Simulator::Now ());
}

void
DimensionOrderedUdpServer::HandleRead (Ptr<Socket> socket)
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
DimensionOrderedUdpServer::HandleClose (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " connection closed:\n" <<
                 "     Time: " << Simulator::Now());
}

void
DimensionOrderedUdpServer::HandleError (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_ERROR ("Node " << GetNode ()->GetId () << " connection error:\n" <<
                  "     Time: " << Simulator::Now());
}
