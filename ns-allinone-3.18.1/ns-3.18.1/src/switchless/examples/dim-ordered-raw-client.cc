/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-raw-client.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedRawClient");
NS_OBJECT_ENSURE_REGISTERED (DimensionOrderedRawClient);

DimensionOrderedRawClient::DimensionOrderedRawClient ()
  : m_setup (false),
    m_running (false),
    m_txAddress (),
    m_txSocket (0)
{
    NS_LOG_FUNCTION (this);
}

DimensionOrderedRawClient::~DimensionOrderedRawClient ()
{
    NS_LOG_FUNCTION (this);
}

bool
DimensionOrderedRawClient::Setup (const Address &serverAddress)
{
    NS_LOG_FUNCTION (this << serverAddress);

    m_txAddress = serverAddress;
    m_setup = true;

    return true;
}

void
DimensionOrderedRawClient::StartApplication (void)
{
    NS_LOG_FUNCTION (this);

    if (!m_setup)
        NS_LOG_WARN ("Application started before calling DimensionOrderedRawClient::Setup. Using defaults.");

    m_running = true;
    // Create socket and connect to server
    Ptr<DimensionOrdered> dimOrdered = GetNode ()->GetObject<DimensionOrdered> ();
    NS_ASSERT_MSG (dimOrdered, "Application started without a DimensionOrdered stack installed on the node");
    m_txSocket = Socket::CreateSocket (GetNode (), DimensionOrderedRawSocketFactory::GetTypeId ());
    Address txAddress (DimensionOrderedSocketAddress (DimensionOrderedAddress::ConvertFrom(m_txAddress), PORT));
    m_txSocket->Bind ();
    NS_LOG_INFO ("Connecting to server address " << DimensionOrderedSocketAddress::ConvertFrom (txAddress).GetDimensionOrderedAddress ());
    m_txSocket->Connect (DimensionOrderedSocketAddress::ConvertFrom (txAddress));
    m_txSocket->SetConnectCallback (MakeCallback (&DimensionOrderedRawClient::HandleConnectionSucceeded, this),
                                MakeCallback (&DimensionOrderedRawClient::HandleConnectionFailed, this));
    m_txSocket->SetRecvCallback (MakeCallback (&DimensionOrderedRawClient::HandleRead, this));
    
    // Send packet to server
    SendPacket ();
}

void
DimensionOrderedRawClient::StopApplication (void)
{
    NS_LOG_FUNCTION (this);

    m_running = false;
    m_txSocket->Close ();
    m_txSocket = 0;
}

void
DimensionOrderedRawClient::SendPacket (void)
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
DimensionOrderedRawClient::HandleConnectionSucceeded (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    socket->SetRecvCallback (MakeCallback (&DimensionOrderedRawClient::HandleRead, this));
    NS_LOG_INFO ("Node " << GetNode ()->GetId () << " connection successful:\n" <<
                 "      Time: " << Simulator::Now ());
}

void
DimensionOrderedRawClient::HandleConnectionFailed (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_ERROR ("Node " << GetNode ()->GetId () << " connection failed:\n" <<
                  "     Time: " << Simulator::Now());
}

void
DimensionOrderedRawClient::HandleRead (Ptr<Socket> socket)
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
