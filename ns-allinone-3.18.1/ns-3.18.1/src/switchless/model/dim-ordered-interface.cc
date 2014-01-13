/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-interface.h"
#include "dim-ordered-l3-protocol.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedInterface");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DimensionOrderedInterface);

TypeId
DimensionOrderedInterface::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::DimensionOrderedInterface")
        .SetParent<Object> ();
    return tid;
}

/**
 * By default, the DimensionOrderedInterface are created in the "down" state
 * with no DimensionOrderedAddress. Before becoming useable, the user must
 * invoke the SetUp on them once an address has been set.
 */
DimensionOrderedInterface::DimensionOrderedInterface ()
  : m_ifup (false),
    m_address (),
    m_node (0),
    m_device (0)
{
    NS_LOG_FUNCTION (this);
}

DimensionOrderedInterface::~DimensionOrderedInterface()
{
    NS_LOG_FUNCTION (this);
}

void
DimensionOrderedInterface::DoDispose (void)
{
    NS_LOG_FUNCTION (this);
    m_node = 0;
    m_device = 0;
    Object::DoDispose ();
}

void
DimensionOrderedInterface::SetNode (Ptr<Node> node)
{
    NS_LOG_FUNCTION (this << node);
    m_node = node;
}

void
DimensionOrderedInterface::SetDevice (Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION (this << device);
    m_device = device;
}

Ptr<NetDevice>
DimensionOrderedInterface::GetDevice (void) const
{
    NS_LOG_FUNCTION (this);
    return m_device;
}

bool
DimensionOrderedInterface::IsUp (void) const
{
    NS_LOG_FUNCTION (this);
    return m_ifup;
}

bool
DimensionOrderedInterface::IsDown (void) const
{
    NS_LOG_FUNCTION (this);
    return !m_ifup;
}

void
DimensionOrderedInterface::SetUp (void)
{
    NS_LOG_FUNCTION (this);
    m_ifup = true;
}

void
DimensionOrderedInterface::SetDown (void)
{
    NS_LOG_FUNCTION (this);
    m_ifup = false;
}

void
DimensionOrderedInterface::Send (Ptr<Packet> p, DimensionOrderedAddress dest)
{
    NS_LOG_FUNCTION (this << *p << dest);

    if (!IsUp ())
        return;

    // Check for a loopback device
    if (DynamicCast<LoopbackNetDevice> (m_device))
    {
        m_device->Send (p, m_device->GetBroadcast (), DimensionOrderedL3Protocol::PROT_NUMBER);
        return;
    }
    // Is this packet aimed at a local interface?
    else if (dest == m_address)
    {
        Ptr<DimensionOrderedL3Protocol> dimordered = m_node->GetObject<DimensionOrderedL3Protocol> ();
        dimordered->Receive (m_device, p, DimensionOrderedL3Protocol::PROT_NUMBER,
                             m_device->GetBroadcast (), 
                             m_device->GetBroadcast (),
                             NetDevice::PACKET_HOST);
        return;
    }
    else
        m_device->Send (p, m_device->GetBroadcast (), DimensionOrderedL3Protocol::PROT_NUMBER);
}

bool
DimensionOrderedInterface::SetAddress (DimensionOrderedInterfaceAddress address)
{
    NS_LOG_FUNCTION (this << address);
    m_address = address;
    return true;
}

DimensionOrderedInterfaceAddress
DimensionOrderedInterface::GetAddress () const
{
    NS_LOG_FUNCTION (this);
    return m_address;
}

} // namespace ns3

