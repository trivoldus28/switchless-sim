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
#include "p2p-hierarchical.h"
 
NS_LOG_COMPONENT_DEFINE ("PointToPointHierarchicalHelper");


namespace ns3 {


PointToPointHierarchicalHelper::PointToPointHierarchicalHelper(unsigned num_node, unsigned num_edge,
                          unsigned num_agg, unsigned num_repl1, unsigned num_repl2,
                          PointToPointHelper p2phelper)
{
  unsigned num_node_per_edge = num_node / num_edge;
  if (num_node % num_edge) num_node_per_edge++;
  unsigned num_edge_per_agg = num_edge / num_agg;
  if (num_edge % num_agg) num_edge_per_agg++;
  //unsigned num_agg_per_core = num_agg;
  unsigned num_core = (num_agg == 1) ? 0 : 1;

  // num_core *= num_repl1 * num_repl2;
  // num_agg *= num_repl1;

  m_host.Create(num_node);
  m_edge.Create(num_edge);
  m_agg.Create(num_agg * num_repl1);
  m_core.Create(num_core * num_repl1 * num_repl2);

  // connect host to edge
  for(int i = 0; i < num_node; i++){
    m_node_devices.Add(p2phelper.Install(m_host.Get(i), m_edge.Get(i/num_node_per_edge)));
  }
  // connect edge to agg
  for(int i = 0; i < num_edge; i++){
    for (int j = 0; j < num_repl1; j++){
      unsigned offset = j * num_agg;
      m_router_devices.Add(p2phelper.Install(m_edge.Get(i), m_agg.Get((i/num_edge_per_agg) + offset)));
    }
  }
  // connect agg to core
  for (int i = 0; i < num_agg; i++){
    for (int repl1 = 0; repl1 < num_repl1; repl1++){
      unsigned offsetagg = num_agg * repl1;
      for (int repl2 = 0; repl2 < num_repl2; repl2++){
        unsigned offsetcore = num_repl2 * repl1 + repl2;
        // std::cout << 
        m_router_devices.Add(p2phelper.Install(m_agg.Get(i + offsetagg), m_core.Get(offsetcore)));
      }
    }
  }
}

PointToPointHierarchicalHelper::~PointToPointHierarchicalHelper ()
{
}

void
PointToPointHierarchicalHelper::InstallStack (InternetStackHelper stack)
{
  stack.Install(m_host);
  stack.Install(m_edge);
  stack.Install(m_agg);
  stack.Install(m_core);
}

void
PointToPointHierarchicalHelper::AssignIpv4Addresses (Ipv4AddressHelper node_ip, Ipv4AddressHelper link_ip)
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
PointToPointHierarchicalHelper::GetNode (unsigned nodeid)
{
  return (m_host.Get(nodeid));
}

Ipv4Address
PointToPointHierarchicalHelper::GetIpv4Address (unsigned nodeid)
{
  return (m_interfaces.GetAddress(nodeid * 2));
}


} // namespace ns3
