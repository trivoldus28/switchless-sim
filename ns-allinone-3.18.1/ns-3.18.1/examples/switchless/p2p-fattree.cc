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
  // Node * root_router = makeOneNode(m_routers);
  // recursiveMakeTree(root_router, group_size, router_fanout, tree_depth, base_datarate, p2p_host_to_router);

  m_num_node = num_node;

  const unsigned N = ceil(pow(num_node, .33333333333f));
  const unsigned numST = 2*N;
  const unsigned numCore = N*N;
  const unsigned numAggr = numST * N;
  const unsigned numEdge = numST * N;
  const unsigned numHost = numEdge * N;
  const unsigned numTotal= numCore + numAggr + numEdge + numHost;
  NS_LOG_LOGIC ("Creating fat-tree nodes.");
  m_node.Create(numTotal);

  for(unsigned j=0;j<2*N;j++) { // For every subtree
    for(unsigned i=j*2*N; i<=j*2*N+N-1; i++) { // First N nodes
      m_edge.Add(m_node.Get(i));
    }
    for(unsigned i=j*2*N+N; i<=j*2*N+2*N-1; i++) { // Last N nodes
      m_aggr.Add(m_node.Get(i));
    }
  };
  for(unsigned i=4*N*N; i<5*N*N; i++) {
    m_core.Add(m_node.Get(i));
  };
  for(unsigned i=5*N*N; i<numTotal; i++) {
    m_host.Add(m_node.Get(i));
  };

  InternetStackHelper stack;
  stack.Install(m_node);

  /*
   * Connect nodes by adding netdevice and set up IP addresses to them.
   *
   * The formula is as follows. We have a fat tree of parameter N, and
   * there are six categories of netdevice, namely, (1) on host;
   * (2) edge towards host; (3) edge towards aggr; (4) aggr towards
   * edge; (5) aggr towards core; (6) on core. There are 2N^3 devices
   * in each category which makes up to 12N^3 netdevices. The IP addrs
   * are assigned in the subnet 10.0.0.0/8 with the 24 bits filled as
   * follows: (Assume N is representable in 6 bits)
   *
   * Address         Scheme
   *               | 7 bit      | 1 bit |  6 bit  | 2 bit | 8 bit   |
   * Host (to edge)| Subtree ID |   0   | Edge ID |  00   | Host ID |
   * Edge (to host)| Subtree ID |   0   | Edge ID |  10   | Host ID |
   * Edge (to aggr)| Subtree ID |   0   | Edge ID |  11   | Aggr ID |
   * Agg. (to edge)| Subtree ID |   0   | Edge ID |  01   | Aggr ID |
   *
   * Address         Scheme
   *               | 7 bit  | 1 bit | 2 bit |  6 bit  | 8 bit   |
   * Agg. (to core)| Subtree ID |   1   |  00   | Aggr ID | Core ID |
   * Core (to aggr)| Subtree ID |   1   |  01   | Core ID | Aggr ID |
   *
   * All ID are numbered from 0 onward. Subtree ID is numbered from left to
   * right according to the fat-tree diagram. Host ID is numbered from
   * left to right within the same attached edge switch. Edge and aggr
   * ID are numbered within the same subtree. Core ID is numbered with a
   * mod-N from left to right according to the fat-tree diagram.
   */
   
  for (unsigned j=0; j<numST; j++) { // For each subtree
    for(unsigned i=0; i<N; i++) { // For each edge
      for(unsigned m=0; m<N; m++) { // For each port of edge
        // Connect edge to host
        Ptr<Node> eNode = m_edge.Get(j*N+i);
        Ptr<Node> hNode = m_host.Get(j*N*N+i*N+m);
        NetDeviceContainer devices = p2phelper.Install(eNode, hNode);
        // m_node_devices.Add(p2p.Install(eNode,hNode));
        // Set IP address for end host
        uint32_t address = (((((((10<<7)+j)<<7)+i)<<2)+0x0)<<8)+m;
        // std::cout << "h: " << address << std::endl;
        AssignIP(devices.Get(1), address, m_hostIface);
        // Set IP address for edge switch
        address = (((((((10<<7)+j)<<7)+i)<<2)+0x2)<<8)+m;
        // std::cout << "e: " << address << std::endl;
        AssignIP(devices.Get(0), address, m_edgeIface);
        // m_hostIface.add(devices.Get(1));
        // m_edgeIface.add(devices.Get(0));
      };
    };
  };
  // std::cout << "Nodes built: " << m_nodes.GetN() << std::endl;
  // std::cout << "Routers built: " << m_routers.GetN() << std::endl;
  for (unsigned j=0; j<numST; j++) { // For each subtree
    for(unsigned i=0; i<N; i++) { // For each edge
      for(unsigned m=0; m<N; m++) { // For each aggregation
        // Connect edge to aggregation
        Ptr<Node> aNode = m_aggr.Get(j*N+m);
        Ptr<Node> eNode = m_edge.Get(j*N+i);
        NetDeviceContainer devices = p2phelper.Install(aNode, eNode);
        // Set IP address for aggregation switch
        uint32_t address = (((((((10<<7)+j)<<7)+i)<<2)+0x1)<<8)+m;
        AssignIP(devices.Get(0), address, m_aggrIface);
        // Set IP address for edge switch
        address = (((((((10<<7)+j)<<7)+i)<<2)+0x3)<<8)+m;
        AssignIP(devices.Get(1), address, m_edgeIface);
        // m_edgeIface.add(devices.Get(1));
        // m_aggrIface.add(devices.Get(0));;
      } ;
    };
  };
  for(unsigned j=0; j<numST; j++) { // For each subtree
    for(unsigned i=0; i<N; i++) { // For each aggr
      for(unsigned m=0; m<N; m++) { // For each port of aggr
        // Connect aggregation to core
        Ptr<Node> cNode = m_core.Get(i*N+m);
        Ptr<Node> aNode = m_aggr.Get(j*N+i);
        NetDeviceContainer devices = p2phelper.Install(cNode, aNode);
        // Set IP address for aggregation switch
        uint32_t address = (((((((10<<7)+j)<<3)+0x4)<<6)+i)<<8)+m;
        AssignIP(devices.Get(1), address, m_aggrIface);
        // Set IP address for core switch
        address = (((((((10<<7)+j)<<3)+0x5)<<6)+m)<<8)+i;
        AssignIP(devices.Get(0), address, m_coreIface);
        // m_aggrIface.add(devices.Get(1));
        // m_coreIface.add(devices.Get(0));;
      };
    };
  };
}

PointToPointFattreeHelper::~PointToPointFattreeHelper ()
{
}

void
PointToPointFattreeHelper::InstallStack (InternetStackHelper stack)
{
  // stack.Install(m_node);
}

void
PointToPointFattreeHelper::AssignIpv4Addresses (Ipv4AddressHelper node_ip, Ipv4AddressHelper link_ip)
{
  // for (uint32_t i = 0; i < m_node_devices.GetN(); i+=2){
  //   m_interfaces.Add (node_ip.Assign (m_node_devices.Get (i))); 
  //   m_interfaces.Add (node_ip.Assign (m_node_devices.Get (i+1)));
  //   node_ip.NewNetwork ();
  // }

  // for (uint32_t i = 0; i < m_router_devices.GetN(); i++){
  //   link_ip.Assign(m_router_devices.Get(i));
  // }
}

Ptr<Node> 
PointToPointFattreeHelper::GetNode (unsigned nodeid)
{
  return (m_host.Get(nodeid));
}

Ipv4Address
PointToPointFattreeHelper::GetIpv4Address (unsigned nodeid)
{
  return (m_hostIface.GetAddress(nodeid));
}


void
PointToPointFattreeHelper::AssignIP (Ptr<NetDevice> c, uint32_t address, Ipv4InterfaceContainer &con)
{
  NS_LOG_FUNCTION_NOARGS ();

  Ptr<Node> node = c->GetNode ();
  NS_ASSERT_MSG (node, "PointToPointFattreeHelper::AssignIP(): Bad node");

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ipv4, "PointToPointFattreeHelper::AssignIP(): Bad ipv4");

  int32_t ifIndex = ipv4->GetInterfaceForDevice (c);
  if (ifIndex == -1) {
    ifIndex = ipv4->AddInterface (c);
  };
  NS_ASSERT_MSG (ifIndex >= 0, "PointToPointFattreeHelper::AssignIP(): Interface index not found");

  Ipv4Address addr(address);
  Ipv4InterfaceAddress ifaddr(addr, 0xFFFFFFFF);
  ipv4->AddAddress (ifIndex, ifaddr);
  ipv4->SetMetric (ifIndex, 1);
  ipv4->SetUp (ifIndex);
  con.Add (ipv4, ifIndex);
  Ipv4AddressGenerator::AddAllocated (addr);
}

} // namespace ns3
