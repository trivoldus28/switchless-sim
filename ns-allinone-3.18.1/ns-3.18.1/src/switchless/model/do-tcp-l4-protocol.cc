/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/object-vector.h"

#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/simulator.h"

#include "do-tcp-l4-protocol.h"
#include "do-tcp-header.h"
#include "dim-ordered-end-point-demux.h"
#include "dim-ordered-end-point.h"
#include "dim-ordered-l3-protocol.h"
#include "do-tcp-socket-factory-impl.h"
#include "do-tcp-newreno.h"
#include "ns3/rtt-estimator.h"

#include <vector>
#include <sstream>
#include <iomanip>

NS_LOG_COMPONENT_DEFINE ("DoTcpL4Protocol");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DoTcpL4Protocol);

//DoTcpL4Protocol stuff----------------------------------------------------------

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT                                   \
  if (m_node) { std::clog << Simulator::Now ().GetSeconds () << " [node " << m_node->GetId () << "] "; } 

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t DoTcpL4Protocol::PROT_NUMBER = 7;

TypeId 
DoTcpL4Protocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DoTcpL4Protocol")
    .SetParent<DimensionOrderedL4Protocol> ()
    .AddConstructor<DoTcpL4Protocol> ()
    .AddAttribute ("RttEstimatorType",
                   "Type of RttEstimator objects.",
                   TypeIdValue (RttMeanDeviation::GetTypeId ()),
                   MakeTypeIdAccessor (&DoTcpL4Protocol::m_rttTypeId),
                   MakeTypeIdChecker ())
    .AddAttribute ("SocketType",
                   "Socket type of TCP objects.",
                   TypeIdValue (DoTcpNewReno::GetTypeId ()),
                   MakeTypeIdAccessor (&DoTcpL4Protocol::m_socketTypeId),
                   MakeTypeIdChecker ())
    .AddAttribute ("SocketList", "The list of sockets associated to this protocol.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&DoTcpL4Protocol::m_sockets),
                   MakeObjectVectorChecker<DoTcpSocketBase> ())
  ;
  return tid;
}

DoTcpL4Protocol::DoTcpL4Protocol ()
  : m_endPoints (new DimensionOrderedEndPointDemux ())
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("Made a DoTcpL4Protocol "<<this);
}

DoTcpL4Protocol::~DoTcpL4Protocol ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void 
DoTcpL4Protocol::SetNode (Ptr<Node> node)
{
  m_node = node;
}

/* 
 * This method is called by AddAgregate and completes the aggregation
 * by setting the node in the TCP stack, link it to the DimensionOrdered stack and 
 * adding TCP socket factory to the node.
 */
void
DoTcpL4Protocol::NotifyNewAggregate ()
{
  Ptr<Node> node = this->GetObject<Node> ();
  Ptr<DimensionOrdered> dimOrdered = this->GetObject<DimensionOrdered> ();

  if (m_node == 0)
    {
      if ((node != 0) && (dimOrdered != 0))
        {
          this->SetNode (node);
          Ptr<DoTcpSocketFactoryImpl> tcpFactory = CreateObject<DoTcpSocketFactoryImpl> ();
          tcpFactory->SetTcp (this);
          node->AggregateObject (tcpFactory);
        }
    }

  if (dimOrdered != 0 && m_downTarget.IsNull ())
    {
      dimOrdered->Insert(this);
      this->SetDownTarget(MakeCallback(&DimensionOrdered::Send, dimOrdered));
    }
  Object::NotifyNewAggregate ();
}

int 
DoTcpL4Protocol::GetProtocolNumber (void) const
{
  return PROT_NUMBER;
}

void
DoTcpL4Protocol::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sockets.clear ();

  if (m_endPoints != 0)
    {
      delete m_endPoints;
      m_endPoints = 0;
    }

  m_node = 0;
  m_downTarget.Nullify ();
  DimensionOrderedL4Protocol::DoDispose ();
}

Ptr<Socket>
DoTcpL4Protocol::CreateSocket (TypeId socketTypeId)
{
  NS_LOG_FUNCTION_NOARGS ();
  ObjectFactory rttFactory;
  ObjectFactory socketFactory;
  rttFactory.SetTypeId (m_rttTypeId);
  socketFactory.SetTypeId (socketTypeId);
  Ptr<RttEstimator> rtt = rttFactory.Create<RttEstimator> ();
  Ptr<DoTcpSocketBase> socket = socketFactory.Create<DoTcpSocketBase> ();
  socket->SetNode (m_node);
  socket->SetTcp (this);
  socket->SetRtt (rtt);
  m_sockets.push_back (socket);
  return socket;
}

Ptr<Socket>
DoTcpL4Protocol::CreateSocket (void)
{
  return CreateSocket (m_socketTypeId);
}

DimensionOrderedEndPoint *
DoTcpL4Protocol::Allocate (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_endPoints->Allocate ();
}

DimensionOrderedEndPoint *
DoTcpL4Protocol::Allocate (DimensionOrderedAddress address)
{
  NS_LOG_FUNCTION (this << address);
  return m_endPoints->Allocate (address);
}

DimensionOrderedEndPoint *
DoTcpL4Protocol::Allocate (uint16_t port)
{
  NS_LOG_FUNCTION (this << port);
  return m_endPoints->Allocate (port);
}

DimensionOrderedEndPoint *
DoTcpL4Protocol::Allocate (DimensionOrderedAddress address, uint16_t port)
{
  NS_LOG_FUNCTION (this << address << port);
  return m_endPoints->Allocate (address, port);
}

DimensionOrderedEndPoint *
DoTcpL4Protocol::Allocate (DimensionOrderedAddress localAddress, uint16_t localPort,
                         DimensionOrderedAddress peerAddress, uint16_t peerPort)
{
  NS_LOG_FUNCTION (this << localAddress << localPort << peerAddress << peerPort);
  return m_endPoints->Allocate (localAddress, localPort,
                                peerAddress, peerPort);
}

void 
DoTcpL4Protocol::DeAllocate (DimensionOrderedEndPoint *endPoint)
{
  NS_LOG_FUNCTION (this << endPoint);
  m_endPoints->DeAllocate (endPoint);
}

enum DimensionOrderedL4Protocol::RxStatus
DoTcpL4Protocol::Receive (Ptr<Packet> packet,
                        DimensionOrderedHeader const &doHeader,
                        Ptr<DimensionOrderedInterface> incomingInterface)
{
  NS_LOG_FUNCTION (this << packet << doHeader << incomingInterface);

  DoTcpHeader tcpHeader;
  if(Node::ChecksumEnabled ())
    {
      tcpHeader.EnableChecksums ();
      tcpHeader.InitializeChecksum (doHeader.GetSource (), doHeader.GetDestination (), PROT_NUMBER);
    }

  packet->PeekHeader (tcpHeader);

  NS_LOG_LOGIC ("DoTcpL4Protocol " << this
                                 << " receiving seq " << tcpHeader.GetSequenceNumber ()
                                 << " ack " << tcpHeader.GetAckNumber ()
                                 << " flags "<< std::hex << (int)tcpHeader.GetFlags () << std::dec
                                 << " data size " << packet->GetSize ());

  if(!tcpHeader.IsChecksumOk ())
    {
      NS_LOG_INFO ("Bad checksum, dropping packet!");
      return DimensionOrderedL4Protocol::RX_CSUM_FAILED;
    }

  NS_LOG_LOGIC ("DoTcpL4Protocol "<<this<<" received a packet");
  DimensionOrderedEndPointDemux::EndPoints endPoints =
    m_endPoints->Lookup (doHeader.GetDestination (), tcpHeader.GetDestinationPort (),
                         doHeader.GetSource (), tcpHeader.GetSourcePort (),incomingInterface);
  if (endPoints.empty ())
    {
      NS_LOG_LOGIC ("  No endpoints matched on DoTcpL4Protocol "<<this);
      std::ostringstream oss;
      oss<<"  destination IP: ";
      doHeader.GetDestination ().Print (oss);
      oss<<" destination port: "<< tcpHeader.GetDestinationPort ()<<" source IP: ";
      doHeader.GetSource ().Print (oss);
      oss<<" source port: "<<tcpHeader.GetSourcePort ();
      NS_LOG_LOGIC (oss.str ());

      if (!(tcpHeader.GetFlags () & DoTcpHeader::RST))
        {
          // build a RST packet and send
          Ptr<Packet> rstPacket = Create<Packet> ();
          DoTcpHeader header;
          if (tcpHeader.GetFlags () & DoTcpHeader::ACK)
            {
              // ACK bit was set
              header.SetFlags (DoTcpHeader::RST);
              header.SetSequenceNumber (header.GetAckNumber ());
            }
          else
            {
              header.SetFlags (DoTcpHeader::RST | DoTcpHeader::ACK);
              header.SetSequenceNumber (SequenceNumber32 (0));
              header.SetAckNumber (header.GetSequenceNumber () + SequenceNumber32 (1));
            }
          header.SetSourcePort (tcpHeader.GetDestinationPort ());
          header.SetDestinationPort (tcpHeader.GetSourcePort ());
          SendPacket (rstPacket, header, doHeader.GetDestination (), doHeader.GetSource ());
          return DimensionOrderedL4Protocol::RX_ENDPOINT_CLOSED;
        }
      else
        {
          return DimensionOrderedL4Protocol::RX_ENDPOINT_CLOSED;
        }
    }
  NS_ASSERT_MSG (endPoints.size () == 1, "Demux returned more than one endpoint");
  NS_LOG_LOGIC ("DoTcpL4Protocol "<<this<<" forwarding up to endpoint/socket");
  (*endPoints.begin ())->ForwardUp (packet, doHeader, tcpHeader.GetSourcePort (), 
                                    incomingInterface);
  return DimensionOrderedL4Protocol::RX_OK;
}

void
DoTcpL4Protocol::Send (Ptr<Packet> packet, 
                     DimensionOrderedAddress saddr, DimensionOrderedAddress daddr,
                     uint16_t sport, uint16_t dport, Ptr<NetDevice> oif)
{
  NS_LOG_FUNCTION (this << packet << saddr << daddr << sport << dport << oif);

  DoTcpHeader tcpHeader;
  tcpHeader.SetDestinationPort (dport);
  tcpHeader.SetSourcePort (sport);
  if(Node::ChecksumEnabled ())
    {
      tcpHeader.EnableChecksums ();
    }
  tcpHeader.InitializeChecksum (saddr,
                                daddr,
                                PROT_NUMBER);
  tcpHeader.SetFlags (DoTcpHeader::ACK);
  tcpHeader.SetAckNumber (SequenceNumber32 (0));

  packet->AddHeader (tcpHeader);

  Ptr<DimensionOrdered> dimOrdered = m_node->GetObject<DimensionOrdered> ();
  if (dimOrdered != 0)
    {
      DimensionOrderedHeader doHeader;
      doHeader.SetDestination (daddr);
      doHeader.SetProtocol (PROT_NUMBER);
      dimOrdered->Send (packet, saddr, daddr, PROT_NUMBER);
    }
}

void
DoTcpL4Protocol::SendPacket (Ptr<Packet> packet, const DoTcpHeader &outgoing,
                           DimensionOrderedAddress saddr, DimensionOrderedAddress daddr, Ptr<NetDevice> oif)
{
  NS_LOG_LOGIC ("DoTcpL4Protocol " << this
                                 << " sending seq " << outgoing.GetSequenceNumber ()
                                 << " ack " << outgoing.GetAckNumber ()
                                 << " flags " << std::hex << (int)outgoing.GetFlags () << std::dec
                                 << " data size " << packet->GetSize ());
  NS_LOG_FUNCTION (this << packet << saddr << daddr << oif);
  // XXX outgoingHeader cannot be logged

  DoTcpHeader outgoingHeader = outgoing;
  outgoingHeader.SetLength (5); //header length in units of 32bit words
  /** \todo UrgentPointer */
  /* outgoingHeader.SetUrgentPointer (0); */
  if(Node::ChecksumEnabled ())
    {
      outgoingHeader.EnableChecksums ();
    }
  outgoingHeader.InitializeChecksum (saddr, daddr, PROT_NUMBER);

  packet->AddHeader (outgoingHeader);

  Ptr<DimensionOrdered> dimOrdered = 
    m_node->GetObject<DimensionOrdered> ();
  if (dimOrdered != 0)
    {
      DimensionOrderedHeader doHeader;
      doHeader.SetDestination (daddr);
      doHeader.SetProtocol (PROT_NUMBER);
      m_downTarget (packet, saddr, daddr, PROT_NUMBER);
    }
  else
    NS_FATAL_ERROR ("Trying to use Tcp on a node without an DimensionOrdered interface");
}

void
DoTcpL4Protocol::SetDownTarget (DimensionOrderedL4Protocol::DownTargetCallback callback)
{
  m_downTarget = callback;
}

DimensionOrderedL4Protocol::DownTargetCallback
DoTcpL4Protocol::GetDownTarget (void) const
{
  return m_downTarget;
}

} // namespace ns3

