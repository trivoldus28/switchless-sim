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

#ifndef POINT_TO_POINT_TOPO_HELPER_H
#define POINT_TO_POINT_TOPO_HELPER_H

namespace ns3 {

/**
 * \ingroup pointtopointlayout
 *
 * \brief A helper to make it easier to create a grid topology
 * with p2p links
 */
class PointToPointTopoHelper 
{
public: 
  /**
   * Create a PointToPointTopoHelper in order to easily create
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
  // PointToPointTopoHelper ();

  // ~PointToPointTopoHelper ();

  virtual Ptr<Node> GetNode (uint32_t nodeid) = 0;

  virtual Ipv4Address GetIpv4Address (unsigned nodeid) = 0;

  virtual void InstallStack (InternetStackHelper stack) = 0;

  virtual void AssignIpv4Addresses (Ipv4AddressHelper ip, Ipv4AddressHelper link_ip) = 0;
};

} // namespace ns3

#endif /* POINT_TO_POINT_TOPO_HELPER_H */
