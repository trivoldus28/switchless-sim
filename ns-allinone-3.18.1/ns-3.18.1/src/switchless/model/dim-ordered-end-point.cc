/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-end-point.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedEndpoint");

namespace ns3 {

DimensionOrderedEndPoint::DimensionOrderedEndPoint (DimensionOrderedAddress address, uint16_t port)
  : m_localAddr (address),
    m_localPort (port),
    m_peerAddr (DimensionOrderedAddress::GetAny ()),
    m_peerPort (0)
{
    NS_LOG_FUNCTION (this << address << port);
}

DimensionOrderedEndPoint::~DimensionOrderedEndPoint ()
{
    NS_LOG_FUNCTION (this);
    if (!m_destroyCallback.IsNull ())
        m_destroyCallback ();

    m_rxCallback.Nullify ();
    m_destroyCallback.Nullify ();
}

DimensionOrderedAddress
DimensionOrderedEndPoint::GetLocalAddress (void)
{
    NS_LOG_FUNCTION (this);
    return m_localAddr;
}

void
DimensionOrderedEndPoint::SetLocalAddress (DimensionOrderedAddress address)
{
    NS_LOG_FUNCTION (this << address);
    m_localAddr = address;
}

uint16_t
DimensionOrderedEndPoint::GetLocalPort (void)
{
    NS_LOG_FUNCTION (this);
    return m_localPort;
}

DimensionOrderedAddress
DimensionOrderedEndPoint::GetPeerAddress (void)
{
    NS_LOG_FUNCTION (this);
    return m_peerAddr;
}

uint16_t
DimensionOrderedEndPoint::GetPeerPort (void)
{
    NS_LOG_FUNCTION (this);
    return m_peerPort;
}

void
DimensionOrderedEndPoint::SetPeer (DimensionOrderedAddress address, uint16_t port)
{
    NS_LOG_FUNCTION (this << address << port);
    m_peerAddr = address;
    m_peerPort = port;
}

void
DimensionOrderedEndPoint::BindToNetDevice (Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION (this << device);
    m_boundnetdevice = device;
    return;
}

Ptr<NetDevice>
DimensionOrderedEndPoint::GetBoundNetDevice (void)
{
    NS_LOG_FUNCTION (this);
    return m_boundnetdevice;
}

void
DimensionOrderedEndPoint::SetRxCallback (
  Callback<void, Ptr<Packet>, DimensionOrderedHeader, uint16_t, Ptr<DimensionOrderedInterface> > callback)
{
    NS_LOG_FUNCTION (this << &callback);
    m_rxCallback = callback;
}

void
DimensionOrderedEndPoint::SetDestroyCallback (Callback<void> callback)
{
    NS_LOG_FUNCTION (this << &callback);
    m_destroyCallback = callback;
}

void
DimensionOrderedEndPoint::ForwardUp (Ptr<Packet> p, const DimensionOrderedHeader& header, uint16_t sport,
                                     Ptr<DimensionOrderedInterface> incomingInterface)
{
    NS_LOG_FUNCTION (this << p << &header << sport << incomingInterface);

    if (!m_rxCallback.IsNull ())
    {
        Simulator::ScheduleNow (&DimensionOrderedEndPoint::DoForwardUp, this, p, header, sport,
                                incomingInterface);
    }
}

void
DimensionOrderedEndPoint::DoForwardUp (Ptr<Packet> p, const DimensionOrderedHeader& header, uint16_t sport,
                                       Ptr<DimensionOrderedInterface> incomingInterface)
{
    NS_LOG_FUNCTION (this << p << &header << sport << incomingInterface);

    if (!m_rxCallback.IsNull ())
    {
        m_rxCallback (p, header, sport, incomingInterface);
    }
}

} // namespace ns3
