/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-l3-protocol.h"

NS_LOG_COMPONENT_DEFINE ("DimensionOrderedL3Protocol");

namespace ns3 {

const uint16_t DimensionOrderedL3Protocol::PROT_NUMBER =0x0900;

NS_OBJECT_ENSURE_REGISTERED (DimensionOrderedL3Protocol);

TypeId
DimensionOrderedL3Protocol::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::DimensionOrderedL3Protocol")
      .SetParent<DimensionOrdered> ()
      .AddConstructor<DimensionOrderedL3Protocol> ()
      .AddTraceSource ("Tx", "Send DimensionOrdered packet to outgoing interface.",
                       MakeTraceSourceAccessor (&DimensionOrderedL3Protocol::m_txTrace))
      .AddTraceSource ("Rx", "Receive DimensionOrdered packet from incoming interface.",
                       MakeTraceSourceAccessor (&DimensionOrderedL3Protocol::m_rxTrace))
      .AddTraceSource ("Drop", "Drop DimensionOrdered packet",
                       MakeTraceSourceAccessor (&DimensionOrderedL3Protocol::m_dropTrace))
      //TODO: Can this be fixed?
      //.AddAttribute ("InterfaceList", "The set of DimensionOrdered interfaces associated to this DimensionOrdered stack.",
      //               ObjectVectorValue (),
      //               MakeObjectVectorAccessor (&DimensionOrderedL3Protocol::m_interfaces),
      //               MakeObjectVectorChecker<DimensionOrderedInterface> ())
      .AddTraceSource ("SendOutgoing", "A newly-generated packet by this node is about to be queued for transmission",
                       MakeTraceSourceAccessor (&DimensionOrderedL3Protocol::m_sendOutgoingTrace))
      .AddTraceSource ("UnicastForward", "A unicast DimensionOrdered packet was received by this node and is being forwarded to another node",
                       MakeTraceSourceAccessor (&DimensionOrderedL3Protocol::m_unicastForwardTrace))
      .AddTraceSource ("LocalDeliver", "A DimensionOrdered packet was received by/for this node, and it is being forward up the stack",
                       MakeTraceSourceAccessor (&DimensionOrderedL3Protocol::m_localDeliverTrace))
      ;
      return tid;
}

DimensionOrderedL3Protocol::DimensionOrderedL3Protocol ()
  : m_protocols (),
    m_interfaces  (),
    m_origin (0,0,0),
    m_dimsMax (0,0,0),
    m_node (0),
    m_sendOutgoingTrace (),
    m_unicastForwardTrace (),
    m_localDeliverTrace (),
    m_txTrace (),
    m_rxTrace (),
    m_dropTrace (),
    m_sockets ()
{
    NS_LOG_FUNCTION (this);
    // Initialize interface slots to 0
    for (int i = 0; i < NUM_DIRS; i++)
        m_interfaces[i] = 0;
}

DimensionOrderedL3Protocol::~DimensionOrderedL3Protocol ()
{
    NS_LOG_FUNCTION (this);
}

void
DimensionOrderedL3Protocol::Insert (Ptr<DimensionOrderedL4Protocol> protocol)
{
    NS_LOG_FUNCTION (this << protocol);
    m_protocols.push_back (protocol);
}

Ptr<DimensionOrderedL4Protocol>
DimensionOrderedL3Protocol::GetProtocol (int protocolNumber) const
{
    NS_LOG_FUNCTION (this << protocolNumber);
    for (L4List_t::const_iterator i = m_protocols.begin(); i != m_protocols.end (); ++i)
    {
        if ((*i)->GetProtocolNumber () == protocolNumber)
        {
            return *i;
        }
    }
    return 0;
}

void
DimensionOrderedL3Protocol::Remove (Ptr<DimensionOrderedL4Protocol> protocol)
{
    NS_LOG_FUNCTION (this << protocol);
    m_protocols.remove (protocol);
}

void
DimensionOrderedL3Protocol::SetNode (Ptr<Node> node)
{
    NS_LOG_FUNCTION (this << node);
    m_node = node;
    // Add a LoopbackNetDevice if needed, and an DimensionOrderedInterface on top of it
    SetupLoopback ();
}

Ptr<Socket>
DimensionOrderedL3Protocol::CreateRawSocket (void)
{
    NS_LOG_FUNCTION (this);
    Ptr<DimensionOrderedRawSocketImpl> socket = CreateObject<DimensionOrderedRawSocketImpl> ();
    socket->SetNode (m_node);
    m_sockets.push_back (socket);
    return socket;
}

void
DimensionOrderedL3Protocol::DeleteRawSocket (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    for (SocketList::iterator i = m_sockets.begin (); i != m_sockets.end (); ++i)
    {
        if ((*i) == socket)
        {
            m_sockets.erase (i);
            return;
        }
    }
    return;
}

/*
 * This method is called by AddAgregate and completes the aggregation
 * by setting the node in the DimensionOrdered stack
 */
void
DimensionOrderedL3Protocol::NotifyNewAggregate ()
{
    NS_LOG_FUNCTION (this);
    if (m_node == 0)
    {
        Ptr<Node> node = this->GetObject<Node> ();
        //verify that it's a valid node and that
        // the node has not been set before
        if (node != 0)
            this->SetNode (node);
    }
    Object::NotifyNewAggregate ();
}

void
DimensionOrderedL3Protocol::DoDispose (void)
{
    NS_LOG_FUNCTION (this);
    for (L4List_t::iterator i = m_protocols.begin(); i != m_protocols.end (); ++i)
        *i = 0;
    m_protocols.clear ();

    for (int i = 0; i < NUM_DIRS; i++)
        m_interfaces[i] = 0;
    m_sockets.clear ();
    m_node = 0;
    
    Object::DoDispose ();
}

void
DimensionOrderedL3Protocol::SetupLoopback (void)
{
    NS_LOG_FUNCTION (this);

    Ptr<DimensionOrderedInterface> interface = CreateObject<DimensionOrderedInterface> ();
    Ptr<LoopbackNetDevice> device = 0;
    // First check whether an existing LoopbackNetDevice exists on the node
    for (uint32_t i = 0; i < m_node->GetNDevices (); i++)
    {
        if ((device = DynamicCast<LoopbackNetDevice> (m_node->GetDevice (i))))
            break;
    }
    if (device == 0)
    {
        device = CreateObject<LoopbackNetDevice> ();
        m_node->AddDevice (device);
    }
    interface->SetDevice (device);
    interface->SetNode (m_node);
    DimensionOrderedInterfaceAddress ifaceAddr = DimensionOrderedInterfaceAddress (DimensionOrderedAddress::GetLoopback ());
    interface->SetAddress (ifaceAddr);
    AddDimensionOrderedInterface (interface, LOOPBACK);
    Ptr<Node> node = GetObject<Node> ();
    node->RegisterProtocolHandler (MakeCallback (&DimensionOrderedL3Protocol::Receive, this),
                                   DimensionOrderedL3Protocol::PROT_NUMBER, device);
    interface->SetUp ();
}

void
DimensionOrderedL3Protocol::AddInterface (Ptr<NetDevice> device, InterfaceDirection dir)
{
    NS_LOG_FUNCTION (this << device << dir);

    Ptr<Node> node = GetObject<Node> ();
    node->RegisterProtocolHandler (MakeCallback (&DimensionOrderedL3Protocol::Receive, this),
                                   DimensionOrderedL3Protocol::PROT_NUMBER, device);
    Ptr<DimensionOrderedInterface> interface = CreateObject<DimensionOrderedInterface> ();
    interface->SetNode (m_node);
    interface->SetDevice (device);
    AddDimensionOrderedInterface (interface, dir);
    return;
}

void
DimensionOrderedL3Protocol::AddDimensionOrderedInterface (Ptr<DimensionOrderedInterface> interface,
                                                          InterfaceDirection dir)
{
    NS_LOG_FUNCTION (this << interface << dir);
    if (dir < NUM_DIRS)
        m_interfaces[dir] = interface;
    return;
}

Ptr<DimensionOrderedInterface>
DimensionOrderedL3Protocol::GetInterface (InterfaceDirection dir) const
{
    NS_LOG_FUNCTION (this << dir);
    if (dir < NUM_DIRS)
        return m_interfaces[dir];
    return 0;
}

uint32_t
DimensionOrderedL3Protocol::GetNInterfaces (void) const
{
    NS_LOG_FUNCTION (this);
    return NUM_DIRS;
}

DimensionOrdered::InterfaceDirection
DimensionOrderedL3Protocol::GetInterfaceForAddress (DimensionOrderedAddress address) const
{
    NS_LOG_FUNCTION (this << address);
    for (uint32_t i = 0; i < NUM_DIRS; i++)
    {
        if (m_interfaces[i])
        {
            if (m_interfaces[i]->GetAddress ().GetLocal () == address)
                return static_cast<InterfaceDirection> (i);
        }
    }
    return INVALID_DIR;
}

DimensionOrdered::InterfaceDirection
DimensionOrderedL3Protocol::GetInterfaceForDevice (Ptr<const NetDevice> device) const
{
    NS_LOG_FUNCTION (this << device);
    for (uint32_t i = 0; i < NUM_DIRS; i++)
    {
        if (m_interfaces[i])
        {
            if (m_interfaces[i]->GetDevice () == device)
                return static_cast<InterfaceDirection> (i);
        }
    }
    return INVALID_DIR;
}

bool
DimensionOrderedL3Protocol::IsDestinationAddress (DimensionOrderedAddress address, 
                                                  InterfaceDirection ifd) const
{
    NS_LOG_FUNCTION (this << address << ifd);
    //First check the incoming interface for a unicast address match
    if (GetAddress (ifd).GetLocal () == address)
    {
        NS_LOG_LOGIC ("For me (destination " << address << " match)");
        return true;
    }
    if (address.IsBroadcast ())
    {
        NS_LOG_LOGIC ("For me (DimensionOrderedAddress broadcast address)");
        return true;
    }
    if (address == DimensionOrderedAddress::GetLoopback ())
    {
        NS_LOG_LOGIC ("For me (DimensionOrderedAddress loopback address)");
    }
    return false;    
}

void
DimensionOrderedL3Protocol::Receive (Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol,
                                     const Address &from, const Address &to, NetDevice::PacketType packetType)
{
    NS_LOG_FUNCTION (this << device << p << protocol << from << to << packetType);

    NS_LOG_LOGIC ("Packet from " << from << " received on node " <<
                  m_node->GetId ());

    Ptr<Packet> packet = p->Copy ();

    Ptr<DimensionOrderedInterface> dimensionOrderedInterface;
    for (uint32_t i = 0; i < NUM_DIRS; i++)
    {
        dimensionOrderedInterface = m_interfaces[i];
        if (dimensionOrderedInterface)
        {
            if (dimensionOrderedInterface->GetDevice () == device)
            {
                if (dimensionOrderedInterface->IsUp ())
                {
                    m_rxTrace (packet, m_node->GetObject<DimensionOrdered> (), 
                               static_cast<InterfaceDirection> (i));
                    break;
                }
                else
                {
                    NS_LOG_LOGIC ("Dropping received packet -- interface is down");
                    DimensionOrderedHeader header;
                    packet->RemoveHeader (header);
                    m_dropTrace (header, packet, DROP_INTERFACE_DOWN, m_node->GetObject <DimensionOrdered> (),
                                 static_cast<InterfaceDirection> (i));
                    return;
                }
            }
        }
    }
    DimensionOrderedHeader header;
    packet->RemoveHeader (header);

    NS_LOG_LOGIC ("Packet from " << header.GetSource () << " destined to " << header.GetDestination ());

    // Trim any residual frame padding from underlying devices
    if (header.GetPayloadSize () < packet->GetSize ())
        packet->RemoveAtEnd (packet->GetSize() - header.GetPayloadSize ());

    for (SocketList::iterator i = m_sockets.begin (); i != m_sockets.end (); ++i)
    {
        NS_LOG_LOGIC ("Fowarding to raw socket");
        Ptr<DimensionOrderedRawSocketImpl> socket = *i;
        socket->ForwardUp (packet, header, dimensionOrderedInterface);
    }

    // Figure out where to route this input packet
    InterfaceDirection ifd = GetInterfaceForDevice (device);
    if (header.GetDestination ().IsBroadcast ())
    {
        NS_LOG_LOGIC ("For me (DimensionOrderedAddress broadcast address)");
        LocalDeliver (packet, header, ifd);
        Forward (packet, header);
        return;
    }

    for (uint32_t i = 0; i < NUM_DIRS; i++)
    {
        if (m_interfaces[i])
        {
            InterfaceDirection dir = static_cast<InterfaceDirection> (i);
            DimensionOrderedInterfaceAddress iaddr = GetAddress (dir);
            DimensionOrderedAddress addr = iaddr.GetLocal ();
            if (addr.IsEqual (header.GetDestination ()))
            {
                if (dir == ifd)
                    NS_LOG_LOGIC ("For me (destination " << addr << " match)");
                else
                    NS_LOG_LOGIC ("For me (destination " << addr << " match) on another interface " << header.GetDestination ());
                LocalDeliver (packet, header, ifd);
                return;
            }
            NS_LOG_LOGIC ("Address " << addr << " not a match for " << header.GetDestination ());
        }
    }
    
    // If we are here, we found this packet was not destined for this node
    // need to forward
    Forward (packet, header);
}

bool
DimensionOrderedL3Protocol::IsUnicast (DimensionOrderedAddress ad) const
{
    NS_LOG_FUNCTION (this << ad);
    return !ad.IsBroadcast ();
}

void
DimensionOrderedL3Protocol::SendWithHeader (Ptr<Packet> packet, DimensionOrderedHeader header)
{
    NS_LOG_FUNCTION (this << packet << header);
    Send (packet, header.GetSource (), header.GetDestination(), header.GetProtocol ());
}

void
DimensionOrderedL3Protocol::Send (Ptr<Packet> packet,
                                  DimensionOrderedAddress source,
                                  DimensionOrderedAddress destination,
                                  uint8_t protocol)
{
    NS_LOG_FUNCTION (this << packet << source << destination << uint32_t (protocol));

    DimensionOrderedHeader header;
    header = BuildHeader (source, destination, protocol, packet->GetSize());

    if (destination.IsBroadcast ())
    {
        NS_LOG_LOGIC ("DimensionOrderedL3Protocol::Send case 1: broadcast");
        for (uint32_t i = 0; i < NUM_DIRS; i++)
        {
            if (m_interfaces[i])
            {
                m_sendOutgoingTrace (header, packet, static_cast<InterfaceDirection> (i));
                SendRealOut(static_cast<InterfaceDirection> (i), packet, header);
            }
        }
        return;
    }
   
    NS_LOG_LOGIC ("DimensionOrderedL3Protocol::Send case 2: unicast to " << destination);
    InterfaceDirection destDir = FindRoute (destination);
    if (destDir < NUM_DIRS)
    {
        m_sendOutgoingTrace (header, packet, destDir);
        SendRealOut (destDir, packet, header);
    }
    else
    {
        NS_LOG_WARN ("No route to host. Drop.");
        m_dropTrace (header, packet, DROP_NO_ROUTE, m_node->GetObject<DimensionOrdered> (), INVALID_DIR);
    }
}

DimensionOrderedHeader
DimensionOrderedL3Protocol::BuildHeader (DimensionOrderedAddress source,
                                         DimensionOrderedAddress destination,
                                         uint8_t protocol,
                                         uint16_t payloadSize)
{
    NS_LOG_FUNCTION (this << source << destination << (uint16_t) protocol << payloadSize);
    DimensionOrderedHeader header;
    header.SetSource (source);
    header.SetDestination (destination);
    header.SetProtocol (protocol);
    header.SetPayloadSize (payloadSize);
    return header;
}

void
DimensionOrderedL3Protocol::SendRealOut (InterfaceDirection dir, Ptr<Packet> packet,
                                         DimensionOrderedHeader const &header)
{
    NS_LOG_FUNCTION (this << dir << packet << &header);

    NS_ASSERT (dir < NUM_DIRS);
    
    packet->AddHeader (header);

    Ptr<DimensionOrderedInterface> outInterface = 0;
    outInterface = m_interfaces[dir];
    NS_ASSERT_MSG (outInterface, "DimensionOrdered:3Protocol::SendRealOut(): Sending over NULL interface");
    NS_LOG_LOGIC ("Send via NetDevice ifIndex " << outInterface->GetDevice ()->GetIfIndex ()
                  << " DimensionOrderedInterface direction " << InterfaceDirectionToAscii (dir));

    if (outInterface->IsUp ())
    {
        NS_LOG_LOGIC ("Send to destination " << header.GetDestination ());
        //NS_ASSERT (packet->GetSize () <= outInterface->GetDevice ()->GetMtu ());

        m_txTrace (packet, m_node->GetObject<DimensionOrdered> (), dir);
        outInterface->Send (packet, header.GetDestination ());
    }
    else
    {
        NS_LOG_LOGIC ("Dropping -- outgoing interface is down: " << header.GetDestination ());
        m_dropTrace (header, packet, DROP_INTERFACE_DOWN, m_node->GetObject<DimensionOrdered> (), dir);
    }
}

void
DimensionOrderedL3Protocol::Forward (Ptr<const Packet> p, const DimensionOrderedHeader &header)
{
    NS_LOG_FUNCTION (this << p << header);
    NS_LOG_LOGIC ("Forwarding logic for node: " << m_node->GetId ());

    //Forwarding
    Ptr<Packet> packet = p->Copy ();
    
    DimensionOrderedAddress destination = header.GetDestination ();

    if (destination.IsBroadcast ())
    {
        NS_LOG_LOGIC ("DimensionOrderedL3Protocol::Forward case 1: broadcast");
        for (uint32_t i = 0; i < NUM_DIRS; i++)
        {
            if (m_interfaces[i])
                SendRealOut(static_cast<InterfaceDirection> (i), packet, header);
        }
        return; 
    }
    
    NS_LOG_LOGIC ("DimensionOrderedL3Protocol::Forward case 2: unicast to " << destination);
    InterfaceDirection destDir = FindRoute (destination);
    if (destDir < NUM_DIRS)
    {
        m_unicastForwardTrace (header, packet, destDir);
        SendRealOut (destDir, packet, header);
    }
    else
    {
        NS_LOG_WARN ("No route to host. Drop.");
        m_dropTrace (header, packet, DROP_NO_ROUTE, m_node->GetObject<DimensionOrdered> (), INVALID_DIR);
    }    

}

DimensionOrdered::InterfaceDirection
DimensionOrderedL3Protocol::FindRoute (DimensionOrderedAddress destination)
{
    NS_LOG_FUNCTION (this << destination);
   
    // Assume all addresses of all interfaces are equal, so assert that first
    bool addressInit = false;
    DimensionOrderedAddress nodeAddress;
    for (uint32_t i = 0; i < NUM_DIRS; i++)
    {
        if (static_cast<InterfaceDirection> (i) == LOOPBACK)
            continue;
        else if (m_interfaces[i])
        {
            if (!addressInit)
            {
                nodeAddress = m_interfaces[i]->GetAddress ().GetLocal ();
                addressInit = true;
            }
            else
            {
                if (m_interfaces[i]->GetAddress ().GetLocal () != nodeAddress)
                    NS_ASSERT_MSG (false, 
                                   "DimensionOrderedL3Protocol::FindRoute: Not all interface addresses match");
            }
        }
    }

    // Check for loopback or sending to the nodeAddress
    if (destination == DimensionOrderedAddress::GetLoopback () || destination == nodeAddress)
        return LOOPBACK;
    
    // Check if we need to route in X dimension still 
    if (destination.GetAddressX () != nodeAddress.GetAddressX ())
    {
        //
        // Find shortest path to correct x destination
        //
        // Cast addresses to signed values for use with subtraction
        int32_t destXAddr = static_cast<int32_t> (destination.GetAddressX ());
        int32_t nodeXAddr = static_cast<int32_t> (nodeAddress.GetAddressX ());
        int32_t originXAddr = static_cast<int32_t> (std::get<0> (m_origin));
        int32_t maxXAddr = static_cast<int32_t> (std::get<0> (m_dimsMax));
        // Get distance by subtraction
        int32_t distance = destXAddr - nodeXAddr;
        // Destnation is in X_NEG direction, but this
        // does not mean X_NEG is the shortest path to take
        if (distance < 0)
        {
           int32_t xNegDistance = -distance; 
           int32_t xPosDistance = (maxXAddr - nodeXAddr) + (destXAddr - originXAddr) + 1;
           if (xPosDistance < xNegDistance && m_interfaces[X_POS])
               return X_POS;
           else if (xPosDistance > xNegDistance && m_interfaces[X_NEG])
               return X_NEG;
           else if (m_interfaces[X_NEG])
               return X_NEG;
           else if (m_interfaces[X_POS])
               return X_POS;

        }
        // Destination is in the X_POS direction, but this
        // does not mean X_POS is the shortest path to take
        else
        {
            int32_t xPosDistance = distance;
            int32_t xNegDistance = (nodeXAddr - originXAddr) + (maxXAddr - destXAddr) + 1;
            if (xPosDistance < xNegDistance && m_interfaces[X_POS])
                return X_POS;
            else if (xPosDistance > xNegDistance && m_interfaces[X_NEG])
                return X_NEG;
            else if (m_interfaces[X_POS])
                return X_POS;
            else if (m_interfaces[X_NEG])
                return X_NEG;
        }
         
    }
    else if (destination.GetAddressY () != nodeAddress.GetAddressY ())
    {
        //
        // Find shortest path to correct y destination
        //
        // Cast addresses to signed values for use with subtraction
        int32_t destYAddr = static_cast<int32_t> (destination.GetAddressY ());
        int32_t nodeYAddr = static_cast<int32_t> (nodeAddress.GetAddressY ());
        int32_t originYAddr = static_cast<int32_t> (std::get<1> (m_origin));
        int32_t maxYAddr = static_cast<int32_t> (std::get<1> (m_dimsMax));
        // Get distance by subtraction
        int32_t distance = destYAddr - nodeYAddr;
        // Destnation is in Y_NEG direction, but this
        // does not mean Y_NEG is the shortest path to take
        if (distance < 0)
        {
           int32_t yNegDistance = -distance;
           int32_t yPosDistance = (maxYAddr - nodeYAddr) + (destYAddr - originYAddr) + 1;
           if (yPosDistance < yNegDistance && m_interfaces[Y_POS])
               return Y_POS;
           else if (yPosDistance > yNegDistance && m_interfaces[Y_NEG])
               return Y_NEG;
           else if (m_interfaces[Y_NEG])
               return Y_NEG;
           else if (m_interfaces[Y_POS])
               return Y_POS;

        }
        // Destination is in the Y_POS direction, but this
        // does not mean Y_POS is the shortest path to take
        else
        {
            int32_t yPosDistance = distance;
            int32_t yNegDistance = (nodeYAddr - originYAddr) + (maxYAddr - destYAddr) + 1;
            if (yPosDistance < yNegDistance && m_interfaces[Y_POS])
                return Y_POS;
            else if (yPosDistance > yNegDistance && m_interfaces[Y_NEG])
                return Y_NEG;
            else if (m_interfaces[Y_POS])
                return Y_POS;
            else if (m_interfaces[Y_NEG])
                return Y_NEG;
        }
    }
    else if (destination.GetAddressZ () != nodeAddress.GetAddressZ ())
    {
        //
        // Find shortest path to correct z destination
        //
        // Cast addresses to signed values for use with subtraction
        int32_t destZAddr = static_cast<int32_t> (destination.GetAddressZ ());
        int32_t nodeZAddr = static_cast<int32_t> (nodeAddress.GetAddressZ ());
        int32_t originZAddr = static_cast<int32_t> (std::get<2> (m_origin));
        int32_t maxZAddr = static_cast<int32_t> (std::get<2> (m_dimsMax));
        // Get distance by subtraction
        int32_t distance = destZAddr - nodeZAddr;
        // Destnation is in Z_NEG direction, but this
        // does not mean Z_NEG is the shortest path to take
        if (distance < 0)
        {
           int32_t zNegDistance = -distance;
           int32_t zPosDistance = (maxZAddr - nodeZAddr) + (destZAddr - originZAddr) + 1;
           if (zPosDistance < zNegDistance && m_interfaces[Z_POS])
               return Z_POS;
           else if (zPosDistance > zNegDistance && m_interfaces[Z_NEG])
               return Z_NEG;
           else if (m_interfaces[Z_NEG])
               return Z_NEG;
           else if (m_interfaces[Z_POS])
               return Z_POS;

        }
        // Destination is in the Z_POS direction, but this
        // does not mean Z_POS is the shortest path to take
        else
        {
            int32_t zPosDistance = distance;
            int32_t zNegDistance = (nodeZAddr - originZAddr) + (maxZAddr - destZAddr) + 1;
            if (zPosDistance < zNegDistance && m_interfaces[Z_POS])
                return Z_POS;
            else if (zPosDistance > zNegDistance && m_interfaces[Z_NEG])
                return Z_NEG;
            else if (m_interfaces[Z_POS])
                return Z_POS;
            else if (m_interfaces[Z_NEG])
                return Z_NEG;
        }
    }

    return INVALID_DIR;
}

void
DimensionOrderedL3Protocol::LocalDeliver (Ptr<const Packet> packet, DimensionOrderedHeader const &header, 
                                          InterfaceDirection ifd)
{
    NS_LOG_FUNCTION (this << packet << &header << ifd);
    Ptr<Packet> p = packet->Copy (); // need to pass a non-const packet up

    m_localDeliverTrace (header, packet, ifd);

    Ptr<DimensionOrderedL4Protocol> protocol = GetProtocol (header.GetProtocol ());
    if (protocol != 0)
    {
        // we need to make a copy in the unlikely event we hit the
        // RX_ENPOINT_UNREACH codepath
        Ptr<Packet> copy = p->Copy ();
        enum DimensionOrderedL4Protocol::RxStatus status = protocol->Receive (p, header, GetInterface(ifd));
        switch (status)
        {
            case DimensionOrderedL4Protocol::RX_OK:
                // fall through
            case DimensionOrderedL4Protocol::RX_ENDPOINT_CLOSED:
                // fall through
            case DimensionOrderedL4Protocol::RX_CSUM_FAILED:
                break;
            case DimensionOrderedL4Protocol::RX_ENDPOINT_UNREACH:
                if (header.GetDestination ().IsBroadcast ())
                    break;
                //TODO: implement icmp?
                //GetIcmp ()->SendDestUnreachPort (ip, copy);
                break;
        }
    }
}

bool
DimensionOrderedL3Protocol::SetAddress (InterfaceDirection dir, DimensionOrderedInterfaceAddress address)
{
    NS_LOG_FUNCTION (this << dir << address);
    Ptr<DimensionOrderedInterface> interface = GetInterface(dir);
    if (interface == 0)
        return false;
    bool retVal = interface->SetAddress (address);
    return retVal;
}

DimensionOrderedInterfaceAddress
DimensionOrderedL3Protocol::GetAddress (InterfaceDirection dir) const
{
    NS_LOG_FUNCTION (this << dir);
    Ptr<DimensionOrderedInterface> interface = GetInterface (dir);
    // TODO: returning zero is not the best option since this is the same as loopback
    if (interface == 0)
        return DimensionOrderedInterfaceAddress(DimensionOrderedAddress::GetZero());
    return interface->GetAddress ();
}

DimensionOrderedAddress
DimensionOrderedL3Protocol::SelectSourceAddress (Ptr<const NetDevice> device,
                                                 DimensionOrderedAddress dst,
                                                 DimensionOrderedInterfaceAddress::InterfaceAddressScope_e scope)
{
    NS_LOG_FUNCTION (this << device << dst << scope);
    DimensionOrderedAddress addr = DimensionOrderedAddress::GetZero ();
    DimensionOrderedInterfaceAddress iaddr;
    bool found = false;

    if (device != 0)
    {
        InterfaceDirection i = GetInterfaceForDevice (device);
        NS_ASSERT_MSG (i >= 0 && i < NUM_DIRS, "No device found on node");
        iaddr = GetAddress (i);
        if (!iaddr.IsSecondary () && iaddr.GetScope () <= scope)
        {
            addr = iaddr.GetLocal ();
            found = true;
        }
    }

    if (found)
        return addr;
    
    // Iterate among all interfaces
    for (uint32_t i = 0; i < NUM_DIRS; i++)
    {
        iaddr = GetAddress (static_cast<InterfaceDirection> (i));
        if (!iaddr.IsSecondary () && iaddr.GetScope () != DimensionOrderedInterfaceAddress::LINK && 
            iaddr.GetScope () <= scope)
            return iaddr.GetLocal ();
    }

    NS_LOG_WARN ("Could not find source address for " << dst << " and scope " << scope << ", returning 0");
    return addr;
}

uint16_t
DimensionOrderedL3Protocol::GetMtu (InterfaceDirection dir) const
{
    NS_LOG_FUNCTION (this << dir);
    Ptr<DimensionOrderedInterface> interface = GetInterface (dir);
    if (interface == 0)
        return 0;
    return interface->GetDevice ()->GetMtu ();
}

bool
DimensionOrderedL3Protocol::IsUp (InterfaceDirection dir) const
{
    NS_LOG_FUNCTION (this << dir);
    Ptr<DimensionOrderedInterface> interface = GetInterface (dir);
    if (interface == 0)
        return false;
    return interface->IsUp ();
}

void
DimensionOrderedL3Protocol::SetUp (InterfaceDirection dir)
{
    NS_LOG_FUNCTION (this << dir);
    Ptr<DimensionOrderedInterface> interface = GetInterface (dir);
    if (interface)
        interface->SetUp ();
}

void
DimensionOrderedL3Protocol::SetDown (InterfaceDirection dir)
{
    NS_LOG_FUNCTION (this << dir);
    Ptr<DimensionOrderedInterface> interface = GetInterface (dir);
    if (interface)
        interface->SetDown ();
}

Ptr<NetDevice>
DimensionOrderedL3Protocol::GetNetDevice (InterfaceDirection dir)
{
    NS_LOG_FUNCTION (this << dir);
    Ptr<DimensionOrderedInterface> interface = GetInterface (dir);
    if (interface == 0)
        return 0;
    return interface->GetDevice ();
}

void
DimensionOrderedL3Protocol::SetOrigin (std::tuple<uint8_t, uint8_t, uint8_t> origin)
{
    NS_LOG_FUNCTION (this << &origin);
    m_origin = origin;
}

std::tuple<uint8_t, uint8_t, uint8_t>
DimensionOrderedL3Protocol::GetOrigin (void) const
{
    NS_LOG_FUNCTION (this);
    return m_origin;
}

void
DimensionOrderedL3Protocol::SetDimensionsMax (std::tuple<uint8_t, uint8_t, uint8_t> dimsMax)
{
    NS_LOG_FUNCTION (this << &dimsMax);
    m_dimsMax = dimsMax;
}

std::tuple<uint8_t, uint8_t, uint8_t>
DimensionOrderedL3Protocol::GetDimensionsMax (void) const
{
    NS_LOG_FUNCTION (this);
    return m_dimsMax;
}

} // namespace ns3


