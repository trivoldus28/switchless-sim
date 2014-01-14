/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/boolean.h"
#include "ns3/object-vector.h"
#include "ns3/dim-ordered.h"
#include "ns3/dim-ordered-header.h"

#include "do-udp-l4-protocol.h"
#include "do-udp-header.h"
#include "do-udp-socket-factory-impl.h"
#include "dim-ordered-end-point-demux.h"
#include "dim-ordered-end-point.h"
#include "dim-ordered-l3-protocol.h"
#include "do-udp-socket-impl.h"

NS_LOG_COMPONENT_DEFINE ("DoUdpL4Protocol");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoUdpL4Protocol);

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t DoUdpL4Protocol::PROT_NUMBER = 18;

TypeId 
DoUdpL4Protocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoUdpL4Protocol")
    .SetParent<DimensionOrderedL4Protocol> ()
    .AddConstructor<DoUdpL4Protocol> ()
    .AddAttribute ("SocketList", "The list of sockets associated to this protocol.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&DoUdpL4Protocol::m_sockets),
                   MakeObjectVectorChecker<DoUdpSocketImpl> ())
  ;
  return tid;
}

DoUdpL4Protocol::DoUdpL4Protocol ()
  : m_endPoints (new DimensionOrderedEndPointDemux ())
{
  NS_LOG_FUNCTION_NOARGS ();
}

DoUdpL4Protocol::~DoUdpL4Protocol ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void 
DoUdpL4Protocol::SetNode (Ptr<Node> node)
{
  m_node = node;
}

/*
 * This method is called by AddAgregate and completes the aggregation
 * by setting the node in the udp stack and link it to the ipv4 object
 * present in the node along with the socket factory
 */
void
DoUdpL4Protocol::NotifyNewAggregate ()
{
  NS_LOG_FUNCTION (this);
  Ptr<Node> node = this->GetObject<Node> ();
  Ptr<DimensionOrdered> dimOrdered = this->GetObject<DimensionOrdered> ();

  if (m_node == 0)
    {
      if ((node != 0) && (dimOrdered != 0))
        {
          this->SetNode (node);
          Ptr<DoUdpSocketFactoryImpl> udpFactory = CreateObject<DoUdpSocketFactoryImpl> ();
          udpFactory->SetUdp (this);
          node->AggregateObject (udpFactory);
        }
    }
  
  // We set at least one of our 2 down targets to the IPv4/IPv6 send
  // functions.  Since these functions have different prototypes, we
  // need to keep track of whether we are connected to an IPv4 or
  // IPv6 lower layer and call the appropriate one.
  
  if (dimOrdered != 0 && m_downTarget.IsNull())
    {
      dimOrdered->Insert (this);
      this->SetDownTarget (MakeCallback (&DimensionOrdered::Send, dimOrdered));
    }
  Object::NotifyNewAggregate ();
}

int 
DoUdpL4Protocol::GetProtocolNumber (void) const
{
  return PROT_NUMBER;
}


void
DoUdpL4Protocol::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  for (std::vector<Ptr<DoUdpSocketImpl> >::iterator i = m_sockets.begin (); i != m_sockets.end (); i++)
    {
      *i = 0;
    }
  m_sockets.clear ();

  if (m_endPoints != 0)
    {
      delete m_endPoints;
      m_endPoints = 0;
    }
  m_node = 0;
  m_downTarget.Nullify ();
/*
 = MakeNullCallback<void,Ptr<Packet>, DimensionOrderedAddress, DimensionOrderedAddress, uint8_t> ();
*/
  DimensionOrderedL4Protocol::DoDispose ();
}

Ptr<Socket>
DoUdpL4Protocol::CreateSocket (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<DoUdpSocketImpl> socket = CreateObject<DoUdpSocketImpl> ();
  socket->SetNode (m_node);
  socket->SetUdp (this);
  m_sockets.push_back (socket);
  return socket;
}

DimensionOrderedEndPoint *
DoUdpL4Protocol::Allocate (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_endPoints->Allocate ();
}

DimensionOrderedEndPoint *
DoUdpL4Protocol::Allocate (DimensionOrderedAddress address)
{
  NS_LOG_FUNCTION (this << address);
  return m_endPoints->Allocate (address);
}

DimensionOrderedEndPoint *
DoUdpL4Protocol::Allocate (uint16_t port)
{
  NS_LOG_FUNCTION (this << port);
  return m_endPoints->Allocate (port);
}

DimensionOrderedEndPoint *
DoUdpL4Protocol::Allocate (DimensionOrderedAddress address, uint16_t port)
{
  NS_LOG_FUNCTION (this << address << port);
  return m_endPoints->Allocate (address, port);
}
DimensionOrderedEndPoint *
DoUdpL4Protocol::Allocate (DimensionOrderedAddress localAddress, uint16_t localPort,
                         DimensionOrderedAddress peerAddress, uint16_t peerPort)
{
  NS_LOG_FUNCTION (this << localAddress << localPort << peerAddress << peerPort);
  return m_endPoints->Allocate (localAddress, localPort,
                                peerAddress, peerPort);
}

void 
DoUdpL4Protocol::DeAllocate (DimensionOrderedEndPoint *endPoint)
{
  NS_LOG_FUNCTION (this << endPoint);
  m_endPoints->DeAllocate (endPoint);
}

enum DimensionOrderedL4Protocol::RxStatus
DoUdpL4Protocol::Receive (Ptr<Packet> packet,
                        DimensionOrderedHeader const &header,
                        Ptr<DimensionOrderedInterface> interface)
{
  NS_LOG_FUNCTION (this << packet << header);
  DoUdpHeader udpHeader;
  if(Node::ChecksumEnabled ())
    {
      udpHeader.EnableChecksums ();
    }

  udpHeader.InitializeChecksum (header.GetSource (), header.GetDestination (), PROT_NUMBER);

  // We only peek at the header for now (instead of removing it) so that it will be intact
  // if we have to pass it to a IPv6 endpoint via:
  // 
  //   DoUdpL4Protocol::Receive (Ptr<Packet> packet, Ipv6Address &src, Ipv6Address &dst, ...)

  packet->PeekHeader (udpHeader);

  if(!udpHeader.IsChecksumOk ())
    {
      NS_LOG_INFO ("Bad checksum : dropping packet!");
      return DimensionOrderedL4Protocol::RX_CSUM_FAILED;
    }

  NS_LOG_DEBUG ("Looking up dst " << header.GetDestination () << " port " << udpHeader.GetDestinationPort ()); 
  DimensionOrderedEndPointDemux::EndPoints endPoints =
    m_endPoints->Lookup (header.GetDestination (), udpHeader.GetDestinationPort (),
                         header.GetSource (), udpHeader.GetSourcePort (), interface);
  if (endPoints.empty ())
    {
      NS_LOG_LOGIC ("RX_ENDPOINT_UNREACH");
      return DimensionOrderedL4Protocol::RX_ENDPOINT_UNREACH;
    }

  packet->RemoveHeader(udpHeader);
  for (DimensionOrderedEndPointDemux::EndPointsI endPoint = endPoints.begin ();
       endPoint != endPoints.end (); endPoint++)
    {
      (*endPoint)->ForwardUp (packet->Copy (), header, udpHeader.GetSourcePort (), 
                              interface);
    }
  return DimensionOrderedL4Protocol::RX_OK;
}

void
DoUdpL4Protocol::Send (Ptr<Packet> packet, 
                     DimensionOrderedAddress saddr, DimensionOrderedAddress daddr, 
                     uint16_t sport, uint16_t dport)
{
  NS_LOG_FUNCTION (this << packet << saddr << daddr << sport << dport);

  DoUdpHeader udpHeader;
  if(Node::ChecksumEnabled ())
    {
      udpHeader.EnableChecksums ();
      udpHeader.InitializeChecksum (saddr,
                                    daddr,
                                    PROT_NUMBER);
    }
  udpHeader.SetDestinationPort (dport);
  udpHeader.SetSourcePort (sport);

  packet->AddHeader (udpHeader);

  m_downTarget (packet, saddr, daddr, PROT_NUMBER);
}

void
DoUdpL4Protocol::SetDownTarget (DimensionOrderedL4Protocol::DownTargetCallback callback)
{
  NS_LOG_FUNCTION (this);
  m_downTarget = callback;
}

DimensionOrderedL4Protocol::DownTargetCallback
DoUdpL4Protocol::GetDownTarget (void) const
{
  return m_downTarget;
}

} // namespace ns3

