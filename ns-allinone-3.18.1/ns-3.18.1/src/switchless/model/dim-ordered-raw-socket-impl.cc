/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-raw-socket-impl.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedRawSocketImpl");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DimensionOrderedRawSocketImpl);

TypeId
DimensionOrderedRawSocketImpl::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::DimensionOrderedRawSocketImpl")
      .SetParent<Socket> ()
      .AddAttribute ("Protocol", "Protocol number to match.",
                     UintegerValue (0),
                     MakeUintegerAccessor (&DimensionOrderedRawSocketImpl::m_protocol),
                     MakeUintegerChecker<uint16_t> ())
      // TODO: implement icmp?
      //.AddAttribute ("IcmpFilter",
      //               "Any icmp header whose type field matches a bit in this filter is dropped. Type must be less than 32.",
      //               UintegerValue (0),
      //               MakeUintegerAccessor (&DimensionOrderedRawSocketImpl::m_icmpFilter),
      //               MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("DimensionOrderedHeaderInclude",
                     "Include DimensionOrdered Header information.",
                     BooleanValue (false),
                     MakeBooleanAccessor (&DimensionOrderedRawSocketImpl::m_dohdrincl),
                     MakeBooleanChecker ())
      ;
      return tid;
}

DimensionOrderedRawSocketImpl::DimensionOrderedRawSocketImpl ()
  : m_err (Socket::ERROR_NOTERROR),
    m_node (0),
    m_src (DimensionOrderedAddress::GetAny ()),
    m_dst (DimensionOrderedAddress::GetAny ()),
    m_protocol (0),
    m_shutdownSend (false),
    m_shutdownRecv (false)
{
    NS_LOG_FUNCTION (this);
}

void
DimensionOrderedRawSocketImpl::SetNode (Ptr<Node> node)
{
    NS_LOG_FUNCTION (this << node);
    m_node = node;
}

void
DimensionOrderedRawSocketImpl::DoDispose (void)
{
    NS_LOG_FUNCTION (this);
    m_node = 0;
    Socket::DoDispose ();
}

enum Socket::SocketErrno
DimensionOrderedRawSocketImpl::GetErrno (void) const
{
    NS_LOG_FUNCTION (this);
    return m_err;
}

enum Socket::SocketType
DimensionOrderedRawSocketImpl::GetSocketType (void) const
{
    NS_LOG_FUNCTION (this);
    return NS3_SOCK_RAW;
}

Ptr<Node>
DimensionOrderedRawSocketImpl::GetNode (void) const
{
    NS_LOG_FUNCTION (this);
    return m_node;
}

int
DimensionOrderedRawSocketImpl::Bind (const Address &address)
{
    NS_LOG_FUNCTION (this << address);
    if (!DimensionOrderedSocketAddress::IsMatchingType (address))
    {
        m_err = Socket::ERROR_INVAL;
        return -1;
    }
    DimensionOrderedSocketAddress ad = DimensionOrderedSocketAddress::ConvertFrom (address);
    m_src = ad.GetDimensionOrderedAddress ();
    return 0; 
}

int
DimensionOrderedRawSocketImpl::Bind (void)
{
    NS_LOG_FUNCTION (this);
    m_src = DimensionOrderedAddress::GetAny ();
    return 0;
}

int
DimensionOrderedRawSocketImpl::Bind6 (void)
{
    NS_LOG_FUNCTION (this);
    return (-1);
}

int
DimensionOrderedRawSocketImpl::GetSockName (Address &address) const
{
    NS_LOG_FUNCTION (this << address);
    address = DimensionOrderedSocketAddress (m_src, 0);
    return 0;
}

int
DimensionOrderedRawSocketImpl::Close (void)
{
    NS_LOG_FUNCTION (this);
    Ptr<DimensionOrdered> dimensionordered = m_node->GetObject<DimensionOrdered> ();
    if (dimensionordered != 0)
        dimensionordered->DeleteRawSocket (this);
    return 0;
}

int
DimensionOrderedRawSocketImpl::ShutdownSend (void)
{
    NS_LOG_FUNCTION (this);
    m_shutdownSend = true;
    return 0;
}

int
DimensionOrderedRawSocketImpl::ShutdownRecv (void)
{
    NS_LOG_FUNCTION (this);
    m_shutdownRecv = true;
    return 0;
}

int
DimensionOrderedRawSocketImpl::Connect (const Address &address)
{
    NS_LOG_FUNCTION (this << address);
    if (!DimensionOrderedSocketAddress::IsMatchingType (address))
    {
        m_err = Socket::ERROR_INVAL;
        return -1;
    }
    DimensionOrderedSocketAddress ad = DimensionOrderedSocketAddress::ConvertFrom (address);
    m_dst = ad.GetDimensionOrderedAddress();
    return 0;
}

int
DimensionOrderedRawSocketImpl::Listen (void)
{
    NS_LOG_FUNCTION (this);
    m_err = Socket::ERROR_OPNOTSUPP;
    return -1;
}

uint32_t
DimensionOrderedRawSocketImpl::GetTxAvailable (void) const
{
    NS_LOG_FUNCTION (this);
    return 0xffffffff;
}

int
DimensionOrderedRawSocketImpl::Send (Ptr<Packet> p, uint32_t flags)
{
    NS_LOG_FUNCTION (this << p << flags);
    DimensionOrderedSocketAddress to = DimensionOrderedSocketAddress (m_dst, m_protocol);
    return SendTo (p, flags, to);
}

int
DimensionOrderedRawSocketImpl::SendTo (Ptr<Packet> p, uint32_t flags, const Address &toAddress)
{
    NS_LOG_FUNCTION (this << p << flags << toAddress);
    if (!DimensionOrderedSocketAddress::IsMatchingType (toAddress))
    {
        m_err = Socket::ERROR_INVAL;
        return -1;
    }
    if (m_shutdownSend)
        return 0;
    DimensionOrderedSocketAddress ad = DimensionOrderedSocketAddress::ConvertFrom (toAddress);
    Ptr<DimensionOrdered> dimordered = m_node->GetObject<DimensionOrdered> ();
    DimensionOrderedAddress dst = ad.GetDimensionOrderedAddress ();
    DimensionOrderedAddress src = m_src;
    DimensionOrderedHeader header;
    if (!m_dohdrincl)
    {
        header.SetDestination (dst);
        header.SetProtocol (m_protocol);
    }
    else
    {
        p->RemoveHeader (header);
        dst = header.GetDestination ();
        src = header.GetSource ();
    }
    Ptr<NetDevice> oif = m_boundnetdevice; //specify non-zero if bound to a source address
    if (!oif && src != DimensionOrderedAddress::GetAny ())
    {
        DimensionOrdered::InterfaceDirection dir = dimordered->GetInterfaceForAddress (src);
        NS_ASSERT (dir < DimensionOrdered::NUM_DIRS);
        oif = dimordered->GetNetDevice (dir);
        NS_LOG_LOGIC ("Set index " << oif << " from source " << src);
    }

    // Get address of this node
    Ptr<DimensionOrdered> dimOrdered = m_node->GetObject<DimensionOrdered> ();
    src = dimOrdered->GetAddress (DimensionOrdered::X_POS).GetLocal ();
    if (!m_dohdrincl)
        dimordered->Send (p, src, dst, m_protocol);
    else
        dimordered->SendWithHeader (p, header);
    NotifyDataSent (p->GetSize ());
    NotifySend (GetTxAvailable ());
    return p->GetSize ();
}

uint32_t
DimensionOrderedRawSocketImpl::GetRxAvailable (void) const
{
    NS_LOG_FUNCTION (this);
    uint32_t rx = 0;
    for (std::list<Data>::const_iterator i = m_recv.begin (); i != m_recv.end (); ++i)
        rx += (i->packet)->GetSize ();

    return rx;
}

Ptr<Packet>
DimensionOrderedRawSocketImpl::Recv (uint32_t maxSize, uint32_t flags)
{
    NS_LOG_FUNCTION (this << maxSize << flags);
    Address tmp;
    return RecvFrom (maxSize, flags, tmp);
}

Ptr<Packet>
DimensionOrderedRawSocketImpl::RecvFrom (uint32_t maxSize, uint32_t flags, Address &fromAddress)
{
    NS_LOG_FUNCTION (this << maxSize << flags << fromAddress);
    if (m_recv.empty())
        return 0;

    struct Data data = m_recv.front ();
    m_recv.pop_front();
    DimensionOrderedSocketAddress addr = DimensionOrderedSocketAddress (data.fromAddress, data.fromProtocol);
    fromAddress = addr;
    if (data.packet->GetSize () > maxSize)
    {
        Ptr<Packet> first = data.packet->CreateFragment (0, maxSize);
        if (!(flags & MSG_PEEK))
            data.packet->RemoveAtStart (maxSize);
        m_recv.push_front (data);
        return first;
    }
    return data.packet;
}

void
DimensionOrderedRawSocketImpl::SetProtocol (uint16_t protocol)
{
    NS_LOG_FUNCTION (this << protocol);
    m_protocol = protocol;
}

bool
DimensionOrderedRawSocketImpl::ForwardUp (Ptr<const Packet> p, DimensionOrderedHeader header,
                                          Ptr<DimensionOrderedInterface> incomingInterface)
{
    NS_LOG_FUNCTION (this << *p << header << incomingInterface);
    if (m_shutdownRecv)
        return false;

    Ptr<NetDevice> boundNetDevice = Socket::GetBoundNetDevice();
    if (boundNetDevice)
    {
        if (boundNetDevice != incomingInterface->GetDevice())
            return false;
    }

    NS_LOG_LOGIC ("src = " << m_src << " dst = " << m_dst);
    if ((m_src == DimensionOrderedAddress::GetAny() || header.GetDestination () == m_src) &&
        (m_dst == DimensionOrderedAddress::GetAny() || header.GetSource () == m_dst) &&
        header.GetProtocol () == m_protocol)
    {
        Ptr<Packet> copy = p->Copy ();
        // TODO: Implement icmp??
        /*if (m_protocol == 1)
        {
            Icmpv4Header icmpHeader;
            copy->PeekHeader (icmpHeader);
            uint8_t type = icmpHeader.GetType ();
            // filter out icmp packet
            if (type < 32 && ((uint32_t(1) << type) & m_icmpFilter))
                return false;
        }*/
        //copy->AddHeader (header);
        struct Data data;
        data.packet = copy;
        data.fromAddress = header.GetSource ();
        data.fromProtocol = header.GetProtocol ();
        m_recv.push_back (data);
        NotifyDataRecv ();
        return true;
    }
    return false;
}

bool
DimensionOrderedRawSocketImpl::SetAllowBroadcast (bool allowBroadcast)
{
    NS_LOG_FUNCTION (this << allowBroadcast);
    if (!allowBroadcast)
        return false;
    return true;
}

bool
DimensionOrderedRawSocketImpl::GetAllowBroadcast () const
{
    NS_LOG_FUNCTION (this);
    return true;
}

} // namespace ns3

