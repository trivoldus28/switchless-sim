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

#ifndef POINT_TO_POINT_MESH_HELPER_H
#define POINT_TO_POINT_MESH_HELPER_H

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
class PointToPoint2DMeshHelper : public PointToPointTopoHelper 
{
public: 
  /**
   * Create a PointToPoint2DMeshHelper in order to easily create
   * grid topologies using p2p links
   *
   * \param nRows total number of rows in the grid
   *
   * \param nCols total number of colums in the grid
   *
   * \param pointToPoint the PointToPointHelper which is used 
   *                     to connect all of the nodes together 
   *                     in the grid
   */
  PointToPoint2DMeshHelper (uint32_t nRows, 
                          uint32_t nCols, bool isTorus,
                          PointToPointHelper pointToPoint);

  ~PointToPoint2DMeshHelper ();

  /**
   * \param row the row address of the node desired
   *
   * \param col the column address of the node desired
   *
   * \returns a pointer to the node specified by the 
   *          (row, col) address
   */
  Ptr<Node> GetNode (uint32_t row, uint32_t col);
  Ptr<Node> GetNode (uint32_t nodeid);

  /**
   * This returns an Ipv4 address at the node specified by 
   * the (row, col) address.  Technically, a node will have 
   * multiple interfaces in the grid; therefore, it also has 
   * multiple Ipv4 addresses.  This method only returns one of 
   * the addresses. If you picture the grid, the address returned 
   * is the left row device of all the nodes, except the left-most 
   * grid nodes, which returns the right row device.
   *
   * \param row the row address of the node desired
   *
   * \param col the column address of the node desired
   *
   * \returns Ipv4Address of one of the interfaces of the node 
   *          specified by the (row, col) address
   */
  Ipv4Address GetIpv4Address (uint32_t row, uint32_t col);
  Ipv4Address GetIpv4Address (uint32_t nodeid);
  Address GetAddress (uint32_t nodeid);
  /**
   * \param stack an InternetStackHelper which is used to install 
   *              on every node in the grid
   */
  void InstallStack (InternetStackHelper stack);

  void AssignIpv4Addresses (Ipv4AddressHelper ip, Ipv4AddressHelper link_ip);

  /**
   * Assigns Ipv6 addresses to all the row and column interfaces
   *
   * \param network an IPv6 address representing the network portion
   *                of the IPv6 Address
   * \param prefix the prefix length
   */

protected:
  uint32_t m_xSize;
  uint32_t m_ySize;
  std::vector<NetDeviceContainer> m_rowDevices;
  std::vector<NetDeviceContainer> m_colDevices;
  std::vector<NetDeviceContainer> m_hubDevices;
  std::vector<Ipv4InterfaceContainer> m_Interfaces;
  std::vector<NodeContainer> m_nodes;
  std::vector<NodeContainer> m_hubs;
};

} // namespace ns3

#endif /* POINT_TO_POINT_MESH_HELPER_H */
