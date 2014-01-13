/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_END_POINT_DEMUX_H
#define DIM_ORDERED_END_POINT_DEMUX_H

// C/C++ includes
#include <list>

// NS3 includes

// Switchless includes
#include "ns3/dim-ordered-address.h"
#include "ns3/dim-ordered-end-point.h"

namespace ns3 {

/**
 * \brief Demultiplexes packets to various transport layer endpoints
 *
 * This class serves as a lookup table to match partial or full information
 * about a four-tuple to an ns3::DimensionOrderedEndPoint.  It internally
 * contains a list of endpoints, and has APIs to add and find endpoints in this
 * demux.  This code is shared in common to TCP and UDP protocols in ns3.  This
 * demux sits betweens ns3's layer 4 and the socket layer
 */
class DimensionOrderedEndPointDemux
{
public:
  typedef std::list<DimensionOrderedEndPoint *> EndPoints;
  typedef std::list<DimensionOrderedEndPoint *>::iterator EndPointsI;

  DimensionOrderedEndPointDemux ();
  ~DimensionOrderedEndPointDemux ();

  EndPoints GetAllEndPoints (void);
  bool LookupPortLocal (uint16_t);
  bool LookupLocal (DimensionOrderedAddress addr, uint16_t port);
  EndPoints Lookup (DimensionOrderedAddress daddr,
                    uint16_t dport,
                    DimensionOrderedAddress saddr,
                    uint16_t sport,
                    Ptr<DimensionOrderedInterface> incomingInterface);

  DimensionOrderedEndPoint *SimpleLookup (DimensionOrderedAddress daddr,
                                          uint16_t dport,
                                          DimensionOrderedAddress saddr,
                                          uint16_t sport);

  DimensionOrderedEndPoint *Allocate (void);
  DimensionOrderedEndPoint *Allocate (DimensionOrderedAddress address);
  DimensionOrderedEndPoint *Allocate (uint16_t port);
  DimensionOrderedEndPoint *Allocate (DimensionOrderedAddress address, uint16_t port);
  DimensionOrderedEndPoint *Allocate (DimensionOrderedAddress localAddress,
                                      uint16_t localPort,
                                      DimensionOrderedAddress peerAddress,
                                      uint16_t peerPort);

  void DeAllocate (DimensionOrderedEndPoint *endPoint);

private:
  uint16_t AllocateEphemeralPort (void);

  uint16_t m_ephemeral;
  uint16_t m_portLast;
  uint16_t m_portFirst;
  EndPoints m_endPoints;

};

} // namespace ns3

#endif /* DIM_ORDERED_END_POINT_DEMUX_H */
