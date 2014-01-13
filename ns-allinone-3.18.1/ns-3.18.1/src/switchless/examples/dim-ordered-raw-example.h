/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_RAW_EXAMPLE_H
#define DIM_ORDERED_RAW_EXAMPLE_H

// C/C++ includes

// NS3 includes
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

// Switchless includes
#include "ns3/switchless-module.h"
#include "dim-ordered-raw-client.h"
#include "dim-ordered-raw-server.h"

// Helper functions
ns3::DimensionOrderedAddressHelper::AddressAssignmentList SetupTopology (ns3::NodeContainer &nodes);
void CreateAndAggregateObjectFromTypeId (ns3::Ptr<ns3::Node> node, const std::string typeId);

#endif /* DIM_ORDERED_RAW_EXAMPLE_H */
