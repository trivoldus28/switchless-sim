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

#ifndef POINT_TO_POINT_FATTREE_HELPER_H
#define POINT_TO_POINT_FATTREE_HELPER_H

#include <vector>

#include "ns3/internet-stack-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/ipv6-interface-container.h"
#include "ns3/net-device-container.h"

#include "p2p-topology-interface.h"
namespace ns3 {

class PointToPointFattreeHelper  : public PointToPointTopoHelper 
{
public: 
  PointToPointFattreeHelper (unsigned num_node, 
                          PointToPointHelper p2p_host_to_router);

  ~PointToPointFattreeHelper ();

  Ptr<Node> GetNode (unsigned nodeid);

  Ipv4Address GetIpv4Address (unsigned nodeid);

  void InstallStack (InternetStackHelper stack);

  void AssignIpv4Addresses (Ipv4AddressHelper ip, Ipv4AddressHelper router_ip);
  Address GetAddress(unsigned nodeid);

private:
  void recursiveMakeTree(Node * root, unsigned group_size, unsigned router_fanout, unsigned tree_depth, uint64_t base_datarate,
                          PointToPointHelper p2p_host_to_router);
  void AssignIP (Ptr<NetDevice> c, uint32_t address, Ipv4InterfaceContainer &con);

  NodeContainer m_node;
  NodeContainer m_edge;
  NodeContainer m_aggr;
  NodeContainer m_core;
  NodeContainer m_host;
  NetDeviceContainer m_node_devices;
  NetDeviceContainer m_router_devices;
  Ipv4InterfaceContainer m_hostIface;
  Ipv4InterfaceContainer m_edgeIface;
  Ipv4InterfaceContainer m_aggrIface;
  Ipv4InterfaceContainer m_coreIface;

  unsigned m_num_node;
};

} // namespace ns3

#endif /* POINT_TO_POINT_FATTREE_HELPER_H */
