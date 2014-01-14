/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-stack-helper.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedStackHelper");

namespace ns3 {

DimensionOrderedStackHelper::DimensionOrderedStackHelper ()
{
    Initialize ();
}

// private method called by both constructor and Reset ()
void
DimensionOrderedStackHelper::Initialize ()
{
    SetTcp ("ns3::DoTcpL4Protocol");
}

DimensionOrderedStackHelper::~DimensionOrderedStackHelper ()
{
}

DimensionOrderedStackHelper::DimensionOrderedStackHelper (const DimensionOrderedStackHelper &o)
{
    m_tcpFactory = o.m_tcpFactory;
}

DimensionOrderedStackHelper &
DimensionOrderedStackHelper::operator = (const DimensionOrderedStackHelper &o)
{
    return *this;
}

void
DimensionOrderedStackHelper::Reset (void)
{
    Initialize ();
}

void
DimensionOrderedStackHelper::SetTcp (const std::string tid)
{
    m_tcpFactory.SetTypeId (tid);
}

void
DimensionOrderedStackHelper::SetTcp (std::string tid, std::string n0, const AttributeValue &v0)
{
    m_tcpFactory.SetTypeId (tid);
    m_tcpFactory.Set (n0, v0);
}

void
DimensionOrderedStackHelper::Install (NodeContainer c, std::tuple<uint8_t, uint8_t, uint8_t> origin,
                                      std::tuple<uint8_t, uint8_t, uint8_t> dimsMax) const
{
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
        Install (*i, origin, dimsMax);
}

void
DimensionOrderedStackHelper::InstallAll (std::tuple<uint8_t, uint8_t, uint8_t> origin,
                                         std::tuple<uint8_t, uint8_t, uint8_t> dimsMax) const
{
    Install (NodeContainer::GetGlobal (), origin, dimsMax);
}

void
DimensionOrderedStackHelper::CreateAndAggregateObjectFromTypeId (Ptr<Node> node, const std::string typeId)
{
    ObjectFactory factory;
    factory.SetTypeId (typeId);
    Ptr<Object> protocol = factory.Create <Object> ();
    node->AggregateObject (protocol);
}

void
DimensionOrderedStackHelper::Install (Ptr<Node> node, std::tuple<uint8_t, uint8_t, uint8_t> origin,
                                      std::tuple<uint8_t, uint8_t, uint8_t> dimsMax) const
{
    if (node->GetObject<DimensionOrdered> () != 0)
    {
        NS_FATAL_ERROR ("DimensionOrderedStackHelper::Install () Aggregating "
                        "an InternetStack to a node with an existing DimensionOrdered object");
        return;
    }

    CreateAndAggregateObjectFromTypeId (node, "ns3::DimensionOrderedL3Protocol");

    Ptr<DimensionOrdered> dimOrdered = node->GetObject<DimensionOrdered> ();
    dimOrdered->SetOrigin (origin);
    dimOrdered->SetDimensionsMax (dimsMax);

    CreateAndAggregateObjectFromTypeId (node, "ns3::DoUdpL4Protocol");
    node->AggregateObject (m_tcpFactory.Create<Object> ());
    Ptr<PacketSocketFactory> factory = CreateObject<PacketSocketFactory> ();
    node->AggregateObject (factory);
}

void
DimensionOrderedStackHelper::Install (std::string nodeName, std::tuple<uint8_t, uint8_t, uint8_t> origin,
                                      std::tuple<uint8_t, uint8_t, uint8_t> dimsMax) const
{
    Ptr<Node> node = Names::Find<Node> (nodeName);
    Install (node, origin, dimsMax);
}

} // namespace ns3
