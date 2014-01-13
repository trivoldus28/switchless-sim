/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-end-point-demux.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedEndPointDemux");

DimensionOrderedEndPointDemux::DimensionOrderedEndPointDemux ()
  : m_ephemeral (49152), 
    m_portLast (65535), 
    m_portFirst (49152)
{
    NS_LOG_FUNCTION (this);
}

DimensionOrderedEndPointDemux::~DimensionOrderedEndPointDemux ()
{
    NS_LOG_FUNCTION (this);
    for (EndPointsI i = m_endPoints.begin (); i != m_endPoints.end (); i++)
    {
        DimensionOrderedEndPoint *endPoint = *i;
        delete endPoint;
    }
    m_endPoints.clear ();
}

bool
DimensionOrderedEndPointDemux::LookupPortLocal (uint16_t port)
{
    NS_LOG_FUNCTION (this << port);
    for (EndPointsI i = m_endPoints.begin (); i != m_endPoints.end (); i++)
    {
        if ((*i)->GetLocalPort () == port)
            return true;
    }
    return false;
}

bool
DimensionOrderedEndPointDemux::LookupLocal (DimensionOrderedAddress addr, uint16_t port)
{
    NS_LOG_FUNCTION (this << addr << port);
    for (EndPointsI i = m_endPoints.begin (); i != m_endPoints.end (); i++)
    {
        if ((*i)->GetLocalPort () == port &&
            (*i)->GetLocalAddress () == addr)
            return true;
    }
    return false;
}

DimensionOrderedEndPoint *
DimensionOrderedEndPointDemux::Allocate (void)
{
    NS_LOG_FUNCTION (this);
    uint16_t port = AllocateEphemeralPort ();
    if (port == 0)
    {
        NS_LOG_WARN ("Ephemeral port allocation failed.");
        return 0;
    }
    DimensionOrderedEndPoint *endPoint = new DimensionOrderedEndPoint (DimensionOrderedAddress::GetAny (), port);
    m_endPoints.push_back (endPoint);
    NS_LOG_DEBUG ("Now have >>" << m_endPoints.size () << "<< endpoints.");
    return endPoint;
}

DimensionOrderedEndPoint *
DimensionOrderedEndPointDemux::Allocate (DimensionOrderedAddress address)
{
    NS_LOG_FUNCTION (this << address);
    uint16_t port = AllocateEphemeralPort ();
    if (port == 0)
    {
        NS_LOG_WARN ("Ephemeral port allocation failed.");
        return 0;
    }
    DimensionOrderedEndPoint *endPoint = new DimensionOrderedEndPoint (address, port);
    m_endPoints.push_back (endPoint);
    NS_LOG_DEBUG ("Now have >>" << m_endPoints.size () << "<< endpoints.");
    return endPoint;
}

DimensionOrderedEndPoint *
DimensionOrderedEndPointDemux::Allocate (uint16_t port)
{
    NS_LOG_FUNCTION (this << port);

    return Allocate (DimensionOrderedAddress::GetAny (), port);
}

DimensionOrderedEndPoint *
DimensionOrderedEndPointDemux::Allocate (DimensionOrderedAddress address, uint16_t port)
{
    NS_LOG_FUNCTION (this << address << port);
    if (LookupLocal (address, port))
    {
        NS_LOG_WARN ("Duplicate address/port; failing.");
        return 0;
    }
    DimensionOrderedEndPoint *endPoint = new DimensionOrderedEndPoint (address, port);
    m_endPoints.push_back (endPoint);
    NS_LOG_DEBUG ("Now have >>" << m_endPoints.size () << "<< endpoints.");
    return endPoint;
}

DimensionOrderedEndPoint *
DimensionOrderedEndPointDemux::Allocate (DimensionOrderedAddress localAddress, uint16_t localPort,
                                         DimensionOrderedAddress peerAddress, uint16_t peerPort)
{
    NS_LOG_FUNCTION (this << localAddress << localPort << peerAddress << peerPort);
    for (EndPointsI i = m_endPoints.begin (); i != m_endPoints.end (); i++)
    {
        if ((*i)->GetLocalPort () == localPort &&
            (*i)->GetLocalAddress () == localAddress &&
            (*i)->GetPeerPort () == peerPort &&
            (*i)->GetPeerAddress () == peerAddress)
        {
            NS_LOG_WARN ("No way we can allocate this end-point.");
            return 0;
        }
    }
    DimensionOrderedEndPoint *endPoint = new DimensionOrderedEndPoint (localAddress, localPort);
    endPoint->SetPeer (peerAddress, peerPort);
    m_endPoints.push_back (endPoint);

    NS_LOG_DEBUG ("Now have >>" << m_endPoints.size () << "<< endpoints.");

    return endPoint;
}

void
DimensionOrderedEndPointDemux::DeAllocate (DimensionOrderedEndPoint *endPoint)
{
    NS_LOG_FUNCTION (this << endPoint);
    for (EndPointsI i = m_endPoints.begin (); i != m_endPoints.end (); i++)
    {
        if (*i == endPoint)
        {
            delete endPoint;
            m_endPoints.erase (i);
            break;
        }
    }
}

/*
 * return list of all available Endpoints
 */
DimensionOrderedEndPointDemux::EndPoints
DimensionOrderedEndPointDemux::GetAllEndPoints (void)
{
    NS_LOG_FUNCTION (this);
    EndPoints ret;

    for (EndPointsI i = m_endPoints.begin (); i != m_endPoints.end (); i++)
    {
        DimensionOrderedEndPoint* endP = *i;
        ret.push_back (endP);
    }
    return ret;
}

/*
 * If we have an exact match, we return it.
 * Otherwise, if we find a generic match, we return it.
 * Otherwise, we return 0.
 */
DimensionOrderedEndPointDemux::EndPoints
DimensionOrderedEndPointDemux::Lookup (DimensionOrderedAddress daddr, uint16_t dport,
                                       DimensionOrderedAddress saddr, uint16_t sport,
                                       Ptr<DimensionOrderedInterface> incomingInterface)
{
    NS_LOG_FUNCTION (this << daddr << dport << saddr << sport << incomingInterface);

    EndPoints retval1; // Matches exact on local port, wildcards on others
    EndPoints retval2; // Matches exact on local port/addr, wildcards on others
    EndPoints retval3; // Matches all but local address
    EndPoints retval4; // Exact match on all 4

    NS_LOG_DEBUG ("Looking up endpoint for destination address " << daddr);
    for (EndPointsI i = m_endPoints.begin (); i != m_endPoints.end (); i++)
    {
        DimensionOrderedEndPoint* endP = *i;
        NS_LOG_DEBUG ("Looking at endpoint dport=" << endP->GetLocalPort ()
                                                   << " daddr=" << endP->GetLocalAddress ()
                                                   << " sport=" << endP->GetPeerPort ()
                                                   << " saddr=" << endP->GetPeerAddress ());

        if (endP->GetLocalPort () != dport)
        {
            NS_LOG_LOGIC ("Skipping endpoint " << &endP
                                             << " because endpoint dport "
                                             << endP->GetLocalPort ()
                                             << " does not match packet dport " << dport);
            continue;
        }
        if (endP->GetBoundNetDevice ())
        {
            if (endP->GetBoundNetDevice () != incomingInterface->GetDevice())
            {
                NS_LOG_LOGIC ("Skipping endpoint " << &endP
                                                 << " because endpoint is bound to specific device and"
                                                 << endP->GetBoundNetDevice ()
                                                 << " does not match packet device " << incomingInterface->GetDevice ());
                continue;
            }
        }
        DimensionOrderedAddress incomingInterfaceAddr = daddr; // may be a broadcast
        bool isBroadcast = daddr.IsBroadcast ();
        NS_LOG_DEBUG ("dest addr " << daddr << " broadcast? " << isBroadcast);
        bool localAddressMatchesWildCard =
          endP->GetLocalAddress () == DimensionOrderedAddress::GetAny ();
        bool localAddressMatchesExact = endP->GetLocalAddress () == daddr;
        
        if (isBroadcast)
            NS_LOG_DEBUG ("Found bcast, localaddr " << endP->GetLocalAddress ());

        if (isBroadcast && (endP->GetLocalAddress () != DimensionOrderedAddress::GetAny ()))
            localAddressMatchesExact = (endP->GetLocalAddress () == incomingInterfaceAddr);

        // if no match here, keep looking
        if (!(localAddressMatchesExact || localAddressMatchesWildCard))
            continue;
        bool remotePeerMatchesExact = endP->GetPeerPort () == sport;
        bool remotePeerMatchesWildCard = endP->GetPeerPort () == 0;
        bool remoteAddressMatchesExact = endP->GetPeerAddress () == saddr;
        bool remoteAddressMatchesWildCard = endP->GetPeerAddress () ==
            DimensionOrderedAddress::GetAny ();
        
        // If remote does not match either with exact or wildcard,
        // skip this one
        if (!(remotePeerMatchesExact || remotePeerMatchesWildCard))
            continue;
        if (!(remoteAddressMatchesExact || remoteAddressMatchesWildCard))
            continue;

        // Now figure out which return list to add this one to
        if (localAddressMatchesWildCard &&
            remotePeerMatchesWildCard &&
            remoteAddressMatchesWildCard)
        { // Only local port matches exactly
            retval1.push_back (endP);
        }
        if ((localAddressMatchesExact || (isBroadcast && localAddressMatchesWildCard))&&
            remotePeerMatchesWildCard &&
            remoteAddressMatchesWildCard)
        { // Only local port and local address matches exactly
            retval2.push_back (endP);
        }
        if (localAddressMatchesWildCard &&
            remotePeerMatchesExact &&
            remoteAddressMatchesExact)
        { // All but local address
            retval3.push_back (endP);
        }
        if (localAddressMatchesExact &&
            remotePeerMatchesExact &&
            remoteAddressMatchesExact)
        { // All 4 match
            retval4.push_back (endP);
        }
    }
    
    // Here we find the most exact match
    if (!retval4.empty ()) return retval4;
    if (!retval3.empty ()) return retval3;
    if (!retval2.empty ()) return retval2;
    return retval1;  // might be empty if no matches
}

DimensionOrderedEndPoint *
DimensionOrderedEndPointDemux::SimpleLookup (DimensionOrderedAddress daddr,
                                             uint16_t dport,
                                             DimensionOrderedAddress saddr,
                                             uint16_t sport)
{
    NS_LOG_FUNCTION (this << daddr << dport << saddr << sport);

    // this code is a copy/paste version of an old BSD ip stack lookup function
    uint32_t genericity = 3;
    DimensionOrderedEndPoint *generic = 0;
    for (EndPointsI i = m_endPoints.begin (); i != m_endPoints.end (); i++)
    {
        if ((*i)->GetLocalPort () != dport)
            continue;
        if ((*i)->GetLocalAddress () == daddr &&
            (*i)->GetPeerPort () == sport &&
            (*i)->GetPeerAddress () == saddr)
            return *i;
        uint32_t tmp = 0;
        if ((*i)->GetLocalAddress () == DimensionOrderedAddress::GetAny ())
            tmp++;
        if ((*i)->GetPeerAddress () == DimensionOrderedAddress::GetAny ())
            tmp++;
        if (tmp < genericity)
        {
            generic = (*i);
            genericity = tmp;
        }
    }
    return generic;
}

uint16_t
DimensionOrderedEndPointDemux::AllocateEphemeralPort (void)
{
    // Similar to countint up logic in netinet/in_pcb.c
    NS_LOG_FUNCTION (this);
    uint16_t port = m_ephemeral;
    int count = m_portLast - m_portFirst;
    do
    {
        if (count-- < 0)
            return 0;
        ++port;
        if (port < m_portFirst || port > m_portLast)
            port = m_portFirst;
    } while (LookupPortLocal (port));
    m_ephemeral = port;
    return port;
}

} // namespace ns3
