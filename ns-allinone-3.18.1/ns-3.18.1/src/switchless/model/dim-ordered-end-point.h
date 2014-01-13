/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_END_POINT_H
#define DIM_ORDERED_END_POINT_H

// C/C++ includes

// NS3 includes
#include "ns3/net-device.h"
#include "ns3/simulator.h"

// Switchless includes
#include "ns3/dim-ordered-address.h"
#include "ns3/dim-ordered-header.h"
#include "ns3/dim-ordered-interface.h"

namespace ns3 {

/**
 * \brief A representation of a dimension ordered enpoint/connection
 * 
 * This class provides a dimension ordered four-tuple (source and destination
 * ports and addresses).  These are used in the ns3::DimensionOrderedEndPointDemux
 * as targets of lookups.  This class also has a callback for notification to higher
 * layers that a packet from a lower layer was received. In the ns3 dimensionordered
 * stack, these notifications are automatically registered to be received
 * by the corresponding socket.
 */
class DimensionOrderedEndPoint 
{
public:
  DimensionOrderedEndPoint (DimensionOrderedAddress address, uint16_t port);
  ~DimensionOrderedEndPoint ();

  DimensionOrderedAddress GetLocalAddress (void);
  void SetLocalAddress (DimensionOrderedAddress address);
  uint16_t GetLocalPort (void);
  DimensionOrderedAddress GetPeerAddress (void);
  uint16_t GetPeerPort (void);

  void SetPeer (DimensionOrderedAddress, uint16_t port);

  void BindToNetDevice (Ptr<NetDevice> device);
  Ptr<NetDevice> GetBoundNetDevice (void);

  // Called from socket implementations to get notified about important events
  void SetRxCallback (Callback<void, Ptr<Packet>, DimensionOrderedHeader, uint16_t, Ptr<DimensionOrderedInterface> > callback);
  void SetDestroyCallback (Callback<void> callback);

  // Called from an L4Protocol implementation to notify an enpoint of a packet reception
  void ForwardUp (Ptr<Packet> p, const DimensionOrderedHeader& header, uint16_t sport,
                  Ptr<DimensionOrderedInterface> incomingInterface);
  
private:
  void DoForwardUp (Ptr<Packet> p, const DimensionOrderedHeader& header, uint16_t sport,
                    Ptr<DimensionOrderedInterface> incomingInterface);

  DimensionOrderedAddress m_localAddr;
  uint16_t m_localPort;
  DimensionOrderedAddress m_peerAddr;
  uint16_t m_peerPort;
  Ptr<NetDevice> m_boundnetdevice;
  Callback<void, Ptr<Packet>, DimensionOrderedHeader, uint16_t, Ptr<DimensionOrderedInterface> > m_rxCallback;
  Callback<void> m_destroyCallback;
};

} // namespace ns3

#endif /* DIM_ORDERED_END_POINT_H */
