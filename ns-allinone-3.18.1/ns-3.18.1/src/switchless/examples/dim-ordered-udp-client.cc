/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-udp-client.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedUdpClient");
NS_OBJECT_ENSURE_REGISTERED (DimensionOrderedUdpClient);

DimensionOrderedUdpClient::DimensionOrderedUdpClient ()
  : m_setup (false),
    m_running (false),
    m_txAddress (),
    m_txSocket (0)
{
    NS_LOG_FUNCTION (this);
}

DimensionOrderedUdpClient::~DimensionOrderedUdpClient ()
{
    NS_LOG_FUNCTION (this);
}

bool
DimensionOrderedUdpClient::Setup (const Address &serverAddress)
{
    NS_LOG_FUNCTION (this << serverAddress);

    m_txAddress = serverAddress;
    m_setup = true;

    return true;
}

void
DimensionOrderedUdpClient::StartApplication (void)
{
    NS_LOG_FUNCTION (this);

    if (!m_setup)
        NS_LOG_WARN ("Application started before calling DimensionOrderedUdpClient::Setup. Using defaults.");

    m_running = true;
    // Create socket and connect to server
    Ptr<DimensionOrdered> dimOrdered = GetNode ()->GetObject<DimensionOrdered> ();
    NS_ASSERT_MSG (dimOrdered, "Application started without a DimensionOrdered stack installed on the node");
    m_txSocket = Socket::CreateSocket (GetNode (), DoUdpSocketFactory::GetTypeId ());
    Address txAddress (DimensionOrderedSocketAddress (DimensionOrderedAddress::ConvertFrom(m_txAddress), PORT));
    m_txSocket->Bind (DimensionOrderedSocketAddress(dimOrdered->GetAddress (DimensionOrdered::X_POS).GetLocal (), PORT));
    NS_LOG_INFO ("Connecting to server address " << DimensionOrderedSocketAddress::ConvertFrom (txAddress).GetDimensionOrderedAddress ());
    if (m_txSocket->Connect (txAddress) == -1)
        NS_LOG_ERROR ("Socket connect failed");
    m_txSocket->SetConnectCallback (MakeCallback (&DimensionOrderedUdpClient::HandleConnectionSucceeded, this),
                                MakeCallback (&DimensionOrderedUdpClient::HandleConnectionFailed, this));
    m_txSocket->SetRecvCallback (MakeCallback (&DimensionOrderedUdpClient::HandleRead, this));
    
    // Send packet to server
    SendPacket ();
}

void
DimensionOrderedUdpClient::StopApplication (void)
{
    NS_LOG_FUNCTION (this);

    m_running = false;
    m_txSocket->Close ();
    m_txSocket = 0;
}

void
DimensionOrderedUdpClient::SendPacket (void)
{
    NS_LOG_FUNCTION (this);

    // Create packet
    Ptr<Packet> packet = Create<Packet> (PACKET_SIZE);
    m_txSocket->Send (packet);

    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " sent packet to server:\n" <<
                 "      Source: " << GetNode ()->GetObject<DimensionOrdered> ()->GetAddress (DimensionOrdered::X_POS).GetLocal () << "\n" << 
                 "      Destination: " << DimensionOrderedAddress::ConvertFrom(m_txAddress) << "\n" <<
                 "      Time: " << Simulator::Now());
}

void
DimensionOrderedUdpClient::HandleConnectionSucceeded (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    socket->SetRecvCallback (MakeCallback (&DimensionOrderedUdpClient::HandleRead, this));
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " connection successful:\n" <<
                 "      Time: " << Simulator::Now ());
}

void
DimensionOrderedUdpClient::HandleConnectionFailed (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_ERROR ("Node " << GetNode ()->GetId () << " connection failed:\n" <<
                  "     Time: " << Simulator::Now());
}

void
DimensionOrderedUdpClient::HandleRead (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);

    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom (from)))
    {
        if (packet->GetSize () > 0)
        {
            DimensionOrderedSocketAddress fromSocketAddr = DimensionOrderedSocketAddress::ConvertFrom (from);
            DimensionOrderedAddress fromAddr = fromSocketAddr.GetDimensionOrderedAddress ();
            if (fromAddr == m_txAddress)
                NS_LOG_INFO ("Node " << GetNode ()->GetId () << " received response from server:\n" <<
                             "      Source : " << DimensionOrderedSocketAddress::ConvertFrom (from).GetDimensionOrderedAddress () << "\n" <<
                             "      Destination: " << GetNode ()->GetObject<DimensionOrdered> ()->GetAddress (DimensionOrdered::X_POS).GetLocal () << "\n" <<
                             "      Packet Size: " << packet->GetSize () << " bytes\n" <<  
                             "      Time: " << Simulator::Now ());
        }
    }
}
