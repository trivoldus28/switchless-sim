/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Josh Pelkey <jpelkey@gatech.edu>
 */

#include "ns3/internet-stack-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/string.h"
#include "ns3/vector.h"
#include "ns3/log.h"
#include "ns3/ipv6-address-generator.h"
#include "ns3/switchless-module.h"

#include "p2p-cube-dimordered.h"
 
NS_LOG_COMPONENT_DEFINE ("PointToPointCubeDimorderedHelper");


namespace ns3 {

PointToPointCubeDimorderedHelper::PointToPointCubeDimorderedHelper (unsigned x, unsigned y, unsigned z, bool isTorus,
                                                PointToPointHelper pointToPoint)
{

  // unsigned num_nodes = pow(nMary,nNcube);
  unsigned num_nodes = x * y * z;
  m_total_nodes = num_nodes;
  m_nodes.Create(num_nodes);

  // for (unsigned i = 0; i < num_nodes; i++){
  //   m_hub_bridge_devs.Add(reallyfastbus.Install(m_nodes.Get(i), m_hubs.Get(i)));
  // }

  // Install DimensionOrdered stack (This is just for the Raw implementation,
  // implementations that use the full stack should use the DimensionOrderedStackHelper)
  // for (NodeContainer::Iterator it = m_nodes.Begin (); it != m_nodes.End (); it++)
  // {
  //     Ptr<Node> node = *it;
  //     // Setup DimensionOrdered L3 Protocol
  //     CreateAndAggregateObjectFromTypeId (node, "ns3::DimensionOrderedL3Protocol");
  //     Ptr<DimensionOrdered> dimOrdered = node->GetObject<DimensionOrdered> ();
  //     dimOrdered->SetOrigin (std::make_tuple (1,1,1));
  //     dimOrdered->SetDimensionsMax (std::make_tuple (x,y,z));
      
  //     // Setup raw socket factory
  //     Ptr<DimensionOrderedRawSocketFactoryImpl> rawFactory = CreateObject<DimensionOrderedRawSocketFactoryImpl> ();
  //     node->AggregateObject (rawFactory);
  // }

  DimensionOrderedStackHelper stack;
  stack.Install (m_nodes, std::make_tuple(1,1,1), std::make_tuple(x,y,z));

  // Setup topology
  DimensionOrderedAddressHelper::AddressAssignmentList assignList;
  NetDeviceContainer devices;

  for (int zi = 0; zi < z; zi++){
    for (int yi = 0; yi < y; yi++){
      for (int xi = 0; xi < x; xi++){
        unsigned nodeid = xi + x*yi + x*y*zi;
        if (xi != 0){
          devices = pointToPoint.Install(m_nodes.Get(nodeid), m_nodes.Get(nodeid - 1));
          assignList.push_back (std::make_tuple (devices.Get (0), 
                                                 DimensionOrderedAddress (xi+1, yi+1, zi+1), 
                                                 DimensionOrdered::X_NEG));
          assignList.push_back (std::make_tuple (devices.Get (1),
                                                 DimensionOrderedAddress (xi, yi+1, zi+1), 
                                                 DimensionOrdered::X_POS));
        }
        else if (isTorus){
          devices = pointToPoint.Install(m_nodes.Get(nodeid), m_nodes.Get(nodeid + x - 1));
          assignList.push_back (std::make_tuple (devices.Get (0), 
                                                 DimensionOrderedAddress (xi+1, yi+1, zi+1), 
                                                 DimensionOrdered::X_NEG));
          assignList.push_back (std::make_tuple (devices.Get (1),
                                                 DimensionOrderedAddress (xi+x, yi+1, zi+1), 
                                                 DimensionOrdered::X_POS));
        }
        if (yi != 0){
          devices = pointToPoint.Install(m_nodes.Get(nodeid), m_nodes.Get(nodeid - x));
          assignList.push_back (std::make_tuple (devices.Get (0), 
                                                 DimensionOrderedAddress (xi+1, yi+1, zi+1), 
                                                 DimensionOrdered::Y_NEG));
          assignList.push_back (std::make_tuple (devices.Get (1),
                                                 DimensionOrderedAddress (xi+1, yi, zi+1), 
                                                 DimensionOrdered::Y_POS));
        }
        else if (isTorus){
          devices = pointToPoint.Install(m_nodes.Get(nodeid), m_nodes.Get(nodeid + (y-1)*x));
          assignList.push_back (std::make_tuple (devices.Get (0), 
                                                 DimensionOrderedAddress (xi+1, yi+1, zi+1), 
                                                 DimensionOrdered::Y_NEG));
          assignList.push_back (std::make_tuple (devices.Get (1),
                                                 DimensionOrderedAddress (xi+1, yi+y, zi+1), 
                                                 DimensionOrdered::Y_POS));
        }
        if (zi != 0){
          devices = pointToPoint.Install(m_nodes.Get(nodeid), m_nodes.Get(nodeid - x*y));
          assignList.push_back (std::make_tuple (devices.Get (0), 
                                                 DimensionOrderedAddress (xi+1, yi+1, zi+1), 
                                                 DimensionOrdered::Z_NEG));
          assignList.push_back (std::make_tuple (devices.Get (1),
                                                 DimensionOrderedAddress (xi+1, yi+1, zi), 
                                                 DimensionOrdered::Z_POS));
        }
        else if (isTorus){
          devices = pointToPoint.Install(m_nodes.Get(nodeid), m_nodes.Get(nodeid + (z-1)*y*x));
          assignList.push_back (std::make_tuple (devices.Get (0), 
                                                 DimensionOrderedAddress (xi+1, yi+1, zi+1), 
                                                 DimensionOrdered::Z_NEG));
          assignList.push_back (std::make_tuple (devices.Get (1),
                                                 DimensionOrderedAddress (xi+1, yi+1, zi+z), 
                                                 DimensionOrdered::Z_POS));
        }
      }
    }
  }
  // DimensionOrderedInterfaceContainer interfaces;
  m_diminterfaces = DimensionOrderedAddressHelper::Assign (assignList);
}

PointToPointCubeDimorderedHelper::~PointToPointCubeDimorderedHelper ()
{
}

void
PointToPointCubeDimorderedHelper::InstallStack (InternetStackHelper stack)
{
  // for (uint32_t i = 0; i < m_total_nodes; i++){
  //   stack.Install(m_nodes.Get(i));
  //   stack.Install(m_hubs.Get(i));
  // }
}

void
PointToPointCubeDimorderedHelper::AssignIpv4Addresses (Ipv4AddressHelper node_ip, Ipv4AddressHelper link_ip)
{
  // for (uint32_t i = 0; i < m_hub_bridge_devs.GetN(); i+=2){
  //   m_Interfaces.Add (node_ip.Assign (m_hub_bridge_devs.Get (i))); 
  //   m_Interfaces.Add (node_ip.Assign (m_hub_bridge_devs.Get (i+1)));
  //   node_ip.NewNetwork ();
  // }

  // for (uint32_t i = 0; i < m_devices.GetN(); i++){
  //   link_ip.Assign(m_devices.Get(i));
  // }
}

Ptr<Node> 
PointToPointCubeDimorderedHelper::GetNode (unsigned nodeid)
{
  return (m_nodes.Get(nodeid));
}

Ipv4Address
PointToPointCubeDimorderedHelper::GetIpv4Address (unsigned nodeid)
{
  // return (m_Interfaces.GetAddress(nodeid*2));
  return 0;
}

Address
PointToPointCubeDimorderedHelper::GetAddress (unsigned nodeid)
{
  return (m_diminterfaces.GetAddress(nodeid));
}

} // namespace ns3
