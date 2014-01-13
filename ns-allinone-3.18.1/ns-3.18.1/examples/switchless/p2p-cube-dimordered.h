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

#ifndef POINT_TO_POINT_NCUBE_DIMORDERED_HELPER_H
#define POINT_TO_POINT_NCUBE_DIMORDERED_HELPER_H

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

/**
 * \ingroup pointtopointlayout
 *
 * \brief A helper to make it easier to create a grid topology
 * with p2p links
 */
class PointToPointCubeDimorderedHelper : public PointToPointTopoHelper
{
public: 
  PointToPointCubeDimorderedHelper (unsigned x, unsigned y, unsigned z, bool isTorus,
                          PointToPointHelper pointToPoint);

  ~PointToPointCubeDimorderedHelper ();

  Ptr<Node> GetNode (unsigned nodeid);
  Ipv4Address GetIpv4Address (unsigned nodeid);
  void InstallStack (InternetStackHelper stack);
  void AssignIpv4Addresses (Ipv4AddressHelper ip, Ipv4AddressHelper link_ip);
  Address GetAddress(unsigned nodeid);

private:
  unsigned m_total_nodes;

  NodeContainer m_nodes;
  NetDeviceContainer m_devices;
  NetDeviceContainer m_hub_bridge_devs;
  Ipv4InterfaceContainer m_Interfaces;
  DimensionOrderedInterfaceContainer m_diminterfaces;
};

} // namespace ns3

#endif /* POINT_TO_POINT_NCUBE_DIMORDERED_HELPER_H */
