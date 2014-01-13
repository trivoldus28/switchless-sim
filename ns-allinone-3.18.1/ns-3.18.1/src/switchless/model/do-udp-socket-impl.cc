/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/dim-ordered-socket-address.h"
#include "ns3/dim-ordered.h"
#include "ns3/dim-ordered-header.h"
#include "ns3/do-udp-socket-factory.h"
#include "ns3/trace-source-accessor.h"
#include "do-udp-socket-impl.h"
#include "dim-ordered-end-point.h"
#include <limits>

NS_LOG_COMPONENT_DEFINE ("DoUdpSocketImpl");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoUdpSocketImpl);

static const uint32_t MAX_DIMENSION_ORDERED_UDP_DATAGRAM_SIZE = 65507;

// Add attributes generic to all UdpSockets to base class UdpSocket
TypeId
DoUdpSocketImpl::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoUdpSocketImpl")
    .SetParent<DoUdpSocket> ()
    .AddConstructor<DoUdpSocketImpl> ()
    .AddTraceSource ("Drop", "Drop UDP packet due to receive buffer overflow",
                     MakeTraceSourceAccessor (&DoUdpSocketImpl::m_dropTrace))
  ;
  return tid;
}

DoUdpSocketImpl::DoUdpSocketImpl ()
  : m_endPoint (0),
    m_node (0),
    m_udp (0),
    m_errno (ERROR_NOTERROR),
    m_shutdownSend (false),
    m_shutdownRecv (false),
    m_connected (false),
    m_rxAvailable (0)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_allowBroadcast = false;
}

DoUdpSocketImpl::~DoUdpSocketImpl ()
{
  NS_LOG_FUNCTION_NOARGS ();

  /// \todo  leave any multicast groups that have been joined
  m_node = 0;
  /**
   * Note: actually this function is called AFTER
   * DoUdpSocketImpl::Destroy or DoUdpSocketImpl::Destroy6
   * so the code below is unnecessary in normal operations
   */
  if (m_endPoint != 0)
    {
      NS_ASSERT (m_udp != 0);
      /**
       * Note that this piece of code is a bit tricky:
       * when DeAllocate is called, it will call into
       * Ipv4EndPointDemux::Deallocate which triggers
       * a delete of the associated endPoint which triggers
       * in turn a call to the method DoUdpSocketImpl::Destroy below
       * will will zero the m_endPoint field.
       */
      NS_ASSERT (m_endPoint != 0);
      m_udp->DeAllocate (m_endPoint);
      NS_ASSERT (m_endPoint == 0);
    }
  m_udp = 0;
}

void 
DoUdpSocketImpl::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = node;

}
void 
DoUdpSocketImpl::SetUdp (Ptr<DoUdpL4Protocol> udp)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_udp = udp;
}


enum Socket::SocketErrno
DoUdpSocketImpl::GetErrno (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_errno;
}

enum Socket::SocketType
DoUdpSocketImpl::GetSocketType (void) const
{
  return NS3_SOCK_DGRAM;
}

Ptr<Node>
DoUdpSocketImpl::GetNode (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_node;
}

void 
DoUdpSocketImpl::Destroy (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_endPoint = 0;
}

int
DoUdpSocketImpl::FinishBind (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  bool done = false;
  if (m_endPoint != 0)
    {
      m_endPoint->SetRxCallback (MakeCallback (&DoUdpSocketImpl::ForwardUp, Ptr<DoUdpSocketImpl> (this)));
      m_endPoint->SetDestroyCallback (MakeCallback (&DoUdpSocketImpl::Destroy, Ptr<DoUdpSocketImpl> (this)));
      done = true;
    }
  if (done)
    {
      return 0;
    }
  return -1;
}

int
DoUdpSocketImpl::Bind (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_endPoint = m_udp->Allocate ();
  return FinishBind ();
}

int
DoUdpSocketImpl::Bind6 (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return -1;
}

int 
DoUdpSocketImpl::Bind (const Address &address)
{
  NS_LOG_FUNCTION (this << address);

  if (DimensionOrderedSocketAddress::IsMatchingType (address))
    {
      DimensionOrderedSocketAddress transport = DimensionOrderedSocketAddress::ConvertFrom (address);
      DimensionOrderedAddress dimOrdered = transport.GetDimensionOrderedAddress ();
      uint16_t port = transport.GetPort ();
      if (dimOrdered == DimensionOrderedAddress::GetAny () && port == 0)
        {
          m_endPoint = m_udp->Allocate ();
        }
      else if (dimOrdered == DimensionOrderedAddress::GetAny () && port != 0)
        {
          m_endPoint = m_udp->Allocate (port);
        }
      else if (dimOrdered != DimensionOrderedAddress::GetAny () && port == 0)
        {
          m_endPoint = m_udp->Allocate (dimOrdered);
        }
      else if (dimOrdered != DimensionOrderedAddress::GetAny () && port != 0)
        {
          m_endPoint = m_udp->Allocate (dimOrdered, port);
        }
    }
  else
    {
      NS_LOG_ERROR ("Not IsMatchingType");
      m_errno = ERROR_INVAL;
      return -1;
    }

  return FinishBind ();
}

int 
DoUdpSocketImpl::ShutdownSend (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_shutdownSend = true;
  return 0;
}

int 
DoUdpSocketImpl::ShutdownRecv (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_shutdownRecv = true;
  return 0;
}

int
DoUdpSocketImpl::Close (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (m_shutdownRecv == true && m_shutdownSend == true)
    {
      m_errno = Socket::ERROR_BADF;
      return -1;
    }
  m_shutdownRecv = true;
  m_shutdownSend = true;
  return 0;
}

int
DoUdpSocketImpl::Connect (const Address & address)
{
  NS_LOG_FUNCTION (this << address);
  if (DimensionOrderedSocketAddress::IsMatchingType(address) == true)
    {
      DimensionOrderedSocketAddress transport = DimensionOrderedSocketAddress::ConvertFrom (address);
      m_defaultAddress = Address(transport.GetDimensionOrderedAddress ());
      m_defaultPort = transport.GetPort ();
      m_connected = true;
      NotifyConnectionSucceeded ();
    }
  else
    {
      return -1;
    }

  return 0;
}

int 
DoUdpSocketImpl::Listen (void)
{
  m_errno = Socket::ERROR_OPNOTSUPP;
  return -1;
}

int 
DoUdpSocketImpl::Send (Ptr<Packet> p, uint32_t flags)
{
  NS_LOG_FUNCTION (this << p << flags);

  if (!m_connected)
    {
      m_errno = ERROR_NOTCONN;
      return -1;
    }

  return DoSend (p);
}

int 
DoUdpSocketImpl::DoSend (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  if ((m_endPoint == 0) && (DimensionOrderedSocketAddress::IsMatchingType(m_defaultAddress) == true))
    {
      if (Bind () == -1)
        {
          NS_ASSERT (m_endPoint == 0);
          return -1;
        }
      NS_ASSERT (m_endPoint != 0);
    }
  if (m_shutdownSend)
    {
      m_errno = ERROR_SHUTDOWN;
      return -1;
    } 

  return DoSendTo (p, (const Address)m_defaultAddress);
}

int
DoUdpSocketImpl::DoSendTo (Ptr<Packet> p, const Address &address)
{
  NS_LOG_FUNCTION (this << p << address);

  if (!m_connected)
    {
      NS_LOG_LOGIC ("Not connected");
      if (DimensionOrderedSocketAddress::IsMatchingType(address) == true)
        {
          DimensionOrderedSocketAddress transport = DimensionOrderedSocketAddress::ConvertFrom (address);
          DimensionOrderedAddress dimOrdered = transport.GetDimensionOrderedAddress ();
          uint16_t port = transport.GetPort ();
          return DoSendTo (p, dimOrdered, port);
        }
      else
        {
          return -1;
        }
    }
  else
    {
      // connected UDP socket must use default addresses
      NS_LOG_LOGIC ("Connected");
      if (DimensionOrderedAddress::IsMatchingType(m_defaultAddress))
        {
          return DoSendTo (p, DimensionOrderedAddress::ConvertFrom(m_defaultAddress), m_defaultPort);
        }
    }
  m_errno = ERROR_AFNOSUPPORT;
  return(-1);
}

int
DoUdpSocketImpl::DoSendTo (Ptr<Packet> p, DimensionOrderedAddress dest, uint16_t port)
{
  NS_LOG_FUNCTION (this << p << dest << port);
  if (m_boundnetdevice)
    {
      NS_LOG_LOGIC ("Bound interface number " << m_boundnetdevice->GetIfIndex ());
    }
  if (m_endPoint == 0)
    {
      if (Bind () == -1)
        {
          NS_ASSERT (m_endPoint == 0);
          return -1;
        }
      NS_ASSERT (m_endPoint != 0);
    }
  if (m_shutdownSend)
    {
      m_errno = ERROR_SHUTDOWN;
      return -1;
    }

  if (p->GetSize () > GetTxAvailable () )
    {
      m_errno = ERROR_MSGSIZE;
      return -1;
    }

  if (IsManualIpTos ())
    {
      SocketIpTosTag ipTosTag;
      ipTosTag.SetTos (GetIpTos ());
      p->AddPacketTag (ipTosTag);
    }

  Ptr<DimensionOrdered> dimOrdered = m_node->GetObject<DimensionOrdered> ();

  {
    SocketSetDontFragmentTag tag;
    bool found = p->RemovePacketTag (tag);
    if (!found)
      {
        if (m_mtuDiscover)
          {
            tag.Enable ();
          }
        else
          {
            tag.Disable ();
          }
        p->AddPacketTag (tag);
      }
  }
  //
  // If dest is set to the limited broadcast address (all ones),
  // convert it to send a copy of the packet out of every 
  // interface as a subnet-directed broadcast.
  // Exception:  if the interface has a /32 address, there is no
  // valid subnet-directed broadcast, so send it as limited broadcast
  // Note also that some systems will only send limited broadcast packets
  // out of the "default" interface; here we send it out all interfaces
  //
  if (dest.IsBroadcast ())
    {
      if (!m_allowBroadcast)
        {
          m_errno = ERROR_OPNOTSUPP;
          return -1;
        }
      NS_LOG_LOGIC ("Limited broadcast start.");
      for (uint32_t i = 0; i < dimOrdered->GetNInterfaces (); i++ )
        {
          DimensionOrdered::InterfaceDirection dir = static_cast<DimensionOrdered::InterfaceDirection> (i);
          if (dimOrdered->GetInterface(dir) != 0)
          {
            // Get the primary address
            DimensionOrderedInterfaceAddress iaddr = dimOrdered->GetAddress (dir);
            DimensionOrderedAddress addri = iaddr.GetLocal ();
            if (addri == DimensionOrderedAddress::GetLoopback())
                continue;
            // Check if interface-bound socket
            if (m_boundnetdevice) 
            {
              if (dimOrdered->GetNetDevice (dir) != 
                  m_boundnetdevice)
                continue;
            }
            NS_LOG_LOGIC ("Sending one copy from " << addri);
            m_udp->Send (p->Copy (), addri, dest,
                         m_endPoint->GetLocalPort (), port);
            NotifyDataSent (p->GetSize ());
            NotifySend (GetTxAvailable ());
          }
        }
      NS_LOG_LOGIC ("Limited broadcast end.");
      return p->GetSize ();
    }
  else if (m_endPoint->GetLocalAddress () != DimensionOrderedAddress::GetAny ())
    {
      m_udp->Send (p->Copy (), m_endPoint->GetLocalAddress (), dest,
                   m_endPoint->GetLocalPort (), port);
      NotifyDataSent (p->GetSize ());
      NotifySend (GetTxAvailable ());
      return p->GetSize ();
    }
  else
    {
      DimensionOrderedHeader header;
      header.SetDestination (dest);
      header.SetProtocol (DoUdpL4Protocol::PROT_NUMBER);
      // Get the address for this node
      Ptr<DimensionOrdered> dimOrdered = m_node->GetObject<DimensionOrdered> ();
      DimensionOrderedAddress src = dimOrdered->GetAddress(DimensionOrdered::X_POS).GetLocal ();
      header.SetSource (src);
      m_udp->Send (p->Copy (), header.GetSource (), header.GetDestination (),
                   m_endPoint->GetLocalPort (), port);
      NotifyDataSent (p->GetSize ());
      return p->GetSize ();
    }

  return 0;
}

//  maximum message size for UDP broadcast is limited by MTU
// size of underlying link; we are not checking that now.
/// \todo Check MTU size of underlying link
uint32_t
DoUdpSocketImpl::GetTxAvailable (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  // No finite send buffer is modelled, but we must respect
  // the maximum size of an DimensionOrdered datagram (65535 bytes - headers).
  return MAX_DIMENSION_ORDERED_UDP_DATAGRAM_SIZE;
}

int 
DoUdpSocketImpl::SendTo (Ptr<Packet> p, uint32_t flags, const Address &address)
{
  NS_LOG_FUNCTION (this << p << flags << address);
  if (DimensionOrderedSocketAddress::IsMatchingType (address))
    {
      if (IsManualIpTos ())
        {
          SocketIpTosTag ipTosTag;
          ipTosTag.SetTos (GetIpTos ());
          p->AddPacketTag (ipTosTag);
        }

      DimensionOrderedSocketAddress transport = DimensionOrderedSocketAddress::ConvertFrom (address);
      DimensionOrderedAddress dimOrdered = transport.GetDimensionOrderedAddress ();
      uint16_t port = transport.GetPort ();
      return DoSendTo (p, dimOrdered, port);
    }
  return -1;
}

uint32_t
DoUdpSocketImpl::GetRxAvailable (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  // We separately maintain this state to avoid walking the queue 
  // every time this might be called
  return m_rxAvailable;
}

Ptr<Packet>
DoUdpSocketImpl::Recv (uint32_t maxSize, uint32_t flags)
{
  NS_LOG_FUNCTION (this << maxSize << flags);
  if (m_deliveryQueue.empty () )
    {
      m_errno = ERROR_AGAIN;
      return 0;
    }
  Ptr<Packet> p = m_deliveryQueue.front ();
  if (p->GetSize () <= maxSize) 
    {
      m_deliveryQueue.pop ();
      m_rxAvailable -= p->GetSize ();
    }
  else
    {
      p = 0; 
    }
  return p;
}

Ptr<Packet>
DoUdpSocketImpl::RecvFrom (uint32_t maxSize, uint32_t flags, 
                         Address &fromAddress)
{
  NS_LOG_FUNCTION (this << maxSize << flags);
  Ptr<Packet> packet = Recv (maxSize, flags);
  if (packet != 0)
    {
      SocketAddressTag tag;
      bool found;
      found = packet->PeekPacketTag (tag);
      NS_ASSERT (found);
      fromAddress = tag.GetAddress ();
    }
  return packet;
}

int
DoUdpSocketImpl::GetSockName (Address &address) const
{
  NS_LOG_FUNCTION_NOARGS ();
  if (m_endPoint != 0)
    {
      address = DimensionOrderedSocketAddress (m_endPoint->GetLocalAddress (), m_endPoint->GetLocalPort ());
    }
  return 0;
}

void
DoUdpSocketImpl::BindToNetDevice (Ptr<NetDevice> netdevice)
{
  NS_LOG_FUNCTION (netdevice);
  Socket::BindToNetDevice (netdevice); // Includes sanity check
  if (m_endPoint == 0)
    {
      if (Bind () == -1)
        {
          NS_ASSERT (m_endPoint == 0);
          return;
        }
      NS_ASSERT (m_endPoint != 0);
    }
  m_endPoint->BindToNetDevice (netdevice);
  return;
}

void 
DoUdpSocketImpl::ForwardUp (Ptr<Packet> packet, DimensionOrderedHeader header, uint16_t port,
                          Ptr<DimensionOrderedInterface> incomingInterface)
{
  NS_LOG_FUNCTION (this << packet << header << port);

  if (m_shutdownRecv)
    {
      return;
    }

  if ((m_rxAvailable + packet->GetSize ()) <= m_rcvBufSize)
    {
      Address address = DimensionOrderedSocketAddress (header.GetSource (), port);
      SocketAddressTag tag;
      tag.SetAddress (address);
      packet->AddPacketTag (tag);
      m_deliveryQueue.push (packet);
      m_rxAvailable += packet->GetSize ();
      NotifyDataRecv ();
    }
  else
    {
      // In general, this case should not occur unless the
      // receiving application reads data from this socket slowly
      // in comparison to the arrival rate
      //
      // drop and trace packet
      NS_LOG_WARN ("No receive buffer space available.  Drop.");
      m_dropTrace (packet);
    }
}

void 
DoUdpSocketImpl::SetRcvBufSize (uint32_t size)
{
  m_rcvBufSize = size;
}

uint32_t 
DoUdpSocketImpl::GetRcvBufSize (void) const
{
  return m_rcvBufSize;
}

void 
DoUdpSocketImpl::SetMtuDiscover (bool discover)
{
  m_mtuDiscover = discover;
}
bool 
DoUdpSocketImpl::GetMtuDiscover (void) const
{
  return m_mtuDiscover;
}

bool
DoUdpSocketImpl::SetAllowBroadcast (bool allowBroadcast)
{
  m_allowBroadcast = allowBroadcast;
  return true;
}

bool
DoUdpSocketImpl::GetAllowBroadcast () const
{
  return m_allowBroadcast;
}


} // namespace ns3
