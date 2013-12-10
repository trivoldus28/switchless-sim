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

#include "ns3/point-to-point-module.h"
#include "p2p-fattree.h"
 
NS_LOG_COMPONENT_DEFINE ("PointToPointFattreeHelper");


namespace ns3 {

Node * makeOneNode(NodeContainer & container){
  container.Create(1);
  return &(*(container.Get(container.GetN()-1)));
}

void
PointToPointFattreeHelper::recursiveMakeTree(Node * root, unsigned group_size, unsigned router_fanout, unsigned tree_depth, uint64_t base_datarate,
                          PointToPointHelper p2p_host_to_router)
{
  if (tree_depth == 0){
    for (unsigned i = 0; i < group_size; i++){
      Node * leaf = makeOneNode(m_nodes);
      m_node_devices.Add(p2p_host_to_router.Install(leaf, root));
    }
  }
  else{
    for (unsigned i = 0; i < router_fanout; i++){
      Node * router = makeOneNode(m_routers);
      uint64_t fat_datarate = base_datarate * group_size * pow(router_fanout,tree_depth - 1);
      p2p_host_to_router.SetDeviceAttribute("DataRate", DataRateValue(fat_datarate));
      m_router_devices.Add(p2p_host_to_router.Install(root, router));
      recursiveMakeTree(router, group_size, router_fanout, tree_depth-1, base_datarate, p2p_host_to_router);
    }
  }
}

PointToPointFattreeHelper::PointToPointFattreeHelper(unsigned group_size, unsigned router_fanout, unsigned tree_depth, uint64_t base_datarate,
                          PointToPointHelper p2p_host_to_router)
{
  Node * root_router = makeOneNode(m_routers);
  recursiveMakeTree(root_router, group_size, router_fanout, tree_depth, base_datarate, p2p_host_to_router);

  // std::cout << "Nodes built: " << m_nodes.GetN() << std::endl;
  // std::cout << "Routers built: " << m_routers.GetN() << std::endl;
}

PointToPointFattreeHelper::~PointToPointFattreeHelper ()
{
}

void
PointToPointFattreeHelper::InstallStack (InternetStackHelper stack)
{
  for (uint32_t i = 0; i < m_nodes.GetN(); i++){
    stack.Install(m_nodes.Get(i));
  }
  for (uint32_t i = 0; i < m_routers.GetN(); i++){
    stack.Install(m_routers.Get(i));
  }
}

void
PointToPointFattreeHelper::AssignIpv4Addresses (Ipv4AddressHelper node_ip, Ipv4AddressHelper link_ip)
{
  for (uint32_t i = 0; i < m_node_devices.GetN(); i+=2){
    m_interfaces.Add (node_ip.Assign (m_node_devices.Get (i))); 
    m_interfaces.Add (node_ip.Assign (m_node_devices.Get (i+1)));
    node_ip.NewNetwork ();
  }

  for (uint32_t i = 0; i < m_router_devices.GetN(); i++){
    link_ip.Assign(m_router_devices.Get(i));
  }
}

Ptr<Node> 
PointToPointFattreeHelper::GetNode (unsigned nodeid)
{
  return (m_nodes.Get(nodeid));
}

Ipv4Address
PointToPointFattreeHelper::GetIpv4Address (unsigned nodeid)
{
  return (m_interfaces.GetAddress(nodeid*2));
}

} // namespace ns3
