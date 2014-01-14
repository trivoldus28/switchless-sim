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
 * Code exerpted from Adrian S. Tam <adrian.sw.tam@gmail.com> & Fan Wang <amywangfan1985@yahoo.com.cn>
 * Author: Tri Nguyen
 */

#include <math.h>

#include "ns3/internet-stack-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/string.h"
#include "ns3/vector.h"
#include "ns3/log.h"
#include "ns3/ipv6-address-generator.h"
#include "ns3/ipv4-address-generator.h"
// #include "ns3/ptr.h"
// #include "ns3/net-device.h"
#include "ns3/point-to-point-module.h"
#include "p2p-fattree.h"
 
NS_LOG_COMPONENT_DEFINE ("PointToPointFattreeHelper");


namespace ns3 {

Node * makeOneNode(NodeContainer & container){
  container.Create(1);
  return &(*(container.Get(container.GetN()-1)));
}

// void
// PointToPointFattreeHelper::recursiveMakeTree(Node * root, unsigned group_size, unsigned router_fanout, unsigned tree_depth, uint64_t base_datarate,
//                           PointToPointHelper p2p_host_to_router)
// {
//   if (tree_depth == 0){
//     for (unsigned i = 0; i < group_size; i++){
//       Node * leaf = makeOneNode(m_nodes);
//       m_node_devices.Add(p2p_host_to_router.Install(leaf, root));
//     }
//   }
//   else{
//     for (unsigned i = 0; i < router_fanout; i++){
//       Node * router = makeOneNode(m_routers);
//       uint64_t fat_datarate = base_datarate * group_size * pow(router_fanout,tree_depth - 1);
//       //std::cout << "Data Rate: " << fat_datarate << std::endl;
//       p2p_host_to_router.SetDeviceAttribute("DataRate", DataRateValue(fat_datarate));
//       m_router_devices.Add(p2p_host_to_router.Install(root, router));
//       recursiveMakeTree(router, group_size, router_fanout, tree_depth-1, base_datarate, p2p_host_to_router);
//     }
//   }
// }

PointToPointFattreeHelper::PointToPointFattreeHelper(unsigned num_node,
                          PointToPointHelper p2phelper)
{
  m_node.Create(num_node + 1);
  p2phelper.SetDeviceAttribute ("DataRate", StringValue ("100Gbps")); // 100Gbps is 10Gbps for some reason
  p2phelper.SetChannelAttribute ("Delay", StringValue ("1500ns")); // .5us * 3
  // connect node
  for(int i = 0; i < num_node; i++){
    m_node_devices.Add(p2phelper.Install(m_node.Get(i), m_node.Get(num_node)));
  }
}

PointToPointFattreeHelper::~PointToPointFattreeHelper ()
{
}

void
PointToPointFattreeHelper::InstallStack (InternetStackHelper stack)
{
  stack.Install(m_node);
}

void
PointToPointFattreeHelper::AssignIpv4Addresses (Ipv4AddressHelper node_ip, Ipv4AddressHelper link_ip)
{
  for (uint32_t i = 0; i < m_node_devices.GetN(); i+=2){
    m_interfaces.Add (node_ip.Assign (m_node_devices.Get (i))); 
    // m_interfaces.Add (node_ip.Assign (m_node_devices.Get (i+1)));
    node_ip.Assign (m_node_devices.Get (i+1));
    node_ip.NewNetwork ();
  }

  // for (uint32_t i = 0; i < m_router_devices.GetN(); i++){
  //   link_ip.Assign(m_router_devices.Get(i));
  // }
}

Ptr<Node> 
PointToPointFattreeHelper::GetNode (unsigned nodeid)
{
  return (m_node.Get(nodeid));
}

Address
PointToPointFattreeHelper::GetAddress (unsigned nodeid)
{
  return (m_interfaces.GetAddress(nodeid));
}

Ipv4Address
PointToPointFattreeHelper::GetIpv4Address (unsigned nodeid)
{
  return (m_interfaces.GetAddress(nodeid));
}


// void
// PointToPointFattreeHelper::AssignIP (Ptr<NetDevice> c, uint32_t address, Ipv4InterfaceContainer &con)
// {
//   NS_LOG_FUNCTION_NOARGS ();

//   Ptr<Node> node = c->GetNode ();
//   NS_ASSERT_MSG (node, "PointToPointFattreeHelper::AssignIP(): Bad node");

//   Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
//   NS_ASSERT_MSG (ipv4, "PointToPointFattreeHelper::AssignIP(): Bad ipv4");

//   int32_t ifIndex = ipv4->GetInterfaceForDevice (c);
//   if (ifIndex == -1) {
//     ifIndex = ipv4->AddInterface (c);
//   };
//   NS_ASSERT_MSG (ifIndex >= 0, "PointToPointFattreeHelper::AssignIP(): Interface index not found");

//   Ipv4Address addr(address);
//   Ipv4InterfaceAddress ifaddr(addr, 0xFFFFFFFF);
//   ipv4->AddAddress (ifIndex, ifaddr);
//   ipv4->SetMetric (ifIndex, 1);
//   ipv4->SetUp (ifIndex);
//   con.Add (ipv4, ifIndex);
//   Ipv4AddressGenerator::AddAllocated (addr);
// }

} // namespace ns3
