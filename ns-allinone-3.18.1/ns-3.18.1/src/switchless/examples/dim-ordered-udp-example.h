/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_UDP_EXAMPLE_H
#define DIM_ORDERED_UDP_EXAMPLE_H

// C/C++ includes

// NS3 includes
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

// Switchless includes
#include "ns3/switchless-module.h"
#include "dim-ordered-udp-client.h"
#include "dim-ordered-udp-server.h"

// Helper functions
ns3::DimensionOrderedAddressHelper::AddressAssignmentList SetupTopology (ns3::NodeContainer &nodes);

#endif /* DIM_ORDERED_UDP_EXAMPLE_H */
