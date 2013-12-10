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

#include "p2p-2d-mesh.h"
 
NS_LOG_COMPONENT_DEFINE ("PointToPoint2DMeshHelper");

namespace ns3 {

PointToPoint2DMeshHelper::PointToPoint2DMeshHelper (uint32_t nRows, 
                                                uint32_t nCols, bool isTorus,
                                                PointToPointHelper pointToPoint)
  : m_xSize (nCols), m_ySize (nRows)
{
  // Bounds check
  if (m_xSize < 1 || m_ySize < 1 || (m_xSize < 2 && m_ySize < 2))
    {
      NS_FATAL_ERROR ("Need more nodes for grid.");
    }

  InternetStackHelper stack;

  // for communication between machine and "virtual hub"
  PointToPointHelper p2pHub;
  p2pHub.SetDeviceAttribute ("DataRate", StringValue ("100000Mbps"));
  p2pHub.SetChannelAttribute ("Delay", StringValue ("0ms"));

  for (uint32_t y = 0; y < nRows; ++y)
    {
      NodeContainer rowNodes;
      NodeContainer rowHubs;
      NetDeviceContainer rowDevices;
      NetDeviceContainer colDevices;
      NetDeviceContainer hubDevices;

      for (uint32_t x = 0; x < nCols; ++x)
        {
          rowHubs.Create (1);
          rowNodes.Create(1);
          hubDevices.Add (p2pHub.Install (rowNodes.Get(x), rowHubs.Get (x)));;

          // install p2p links across the row
          if (x > 0)
            {
              rowDevices.Add (pointToPoint.
                              Install (rowHubs.Get (x-1), rowHubs.Get (x)));
            }
            

          // install vertical p2p links
          if (y > 0)
            {
              colDevices.Add (pointToPoint.
                              Install ((m_hubs.at (y-1)).Get (x), rowHubs.Get (x)));
            }

          if (isTorus){
            if (x == nCols - 1 && nCols > 1)
              {
                rowDevices.Add (pointToPoint.
                                Install (rowHubs.Get (x), rowHubs.Get (0)));
              }

            if (y == nRows - 1 && nRows > 1)
              {
                colDevices.Add (pointToPoint.
                                Install ((m_hubs.at (0)).Get (x), rowHubs.Get (x)));
              }
            }
        }

      m_hubs.push_back (rowHubs);
      m_nodes.push_back (rowNodes);
      m_rowDevices.push_back (rowDevices);
      m_hubDevices.push_back (hubDevices);

      if (y > 0)
        m_colDevices.push_back (colDevices);
    }
}

PointToPoint2DMeshHelper::~PointToPoint2DMeshHelper ()
{
}

void
PointToPoint2DMeshHelper::InstallStack (InternetStackHelper stack)
{
  for (uint32_t i = 0; i < m_nodes.size (); ++i)
    {
      NodeContainer rowNodes = m_nodes[i];
      NodeContainer rowRouters = m_hubs[i];
      for (uint32_t j = 0; j < rowNodes.GetN (); ++j)
        {
          stack.Install (rowNodes.Get (j));
          stack.Install (rowRouters.Get (j));
        }
    }
}

void
PointToPoint2DMeshHelper::AssignIpv4Addresses (Ipv4AddressHelper nodeIp, Ipv4AddressHelper linkIp)
{
  for (uint32_t i = 0; i < m_hubDevices.size (); ++i)
    {
      Ipv4InterfaceContainer hubInterfaces; 
      NetDeviceContainer hubContainer = m_hubDevices[i];
      for (uint32_t j = 0; j < hubContainer.GetN (); j+=2)
        {
          hubInterfaces.Add (nodeIp.Assign (hubContainer.Get (j))); 
          hubInterfaces.Add (nodeIp.Assign (hubContainer.Get (j+1)));
          nodeIp.NewNetwork ();
        }
      m_Interfaces.push_back(hubInterfaces);
    }

  // Assign addresses to all row devices in the grid.
  // These devices are stored in a vector.  Each row 
  // of the grid has all the row devices in one entry 
  // of the vector.  These entries come in pairs.
  for (uint32_t i = 0; i < m_rowDevices.size (); ++i)
    {
      Ipv4InterfaceContainer rowInterfaces; 
      NetDeviceContainer rowContainer = m_rowDevices[i];
      NetDeviceContainer hubContainer = m_hubDevices[i];
      for (uint32_t j = 0; j < rowContainer.GetN (); j+=2)
        {
          rowInterfaces.Add (linkIp.Assign (rowContainer.Get (j))); 
          rowInterfaces.Add (linkIp.Assign (rowContainer.Get (j+1)));
          // linkIp.NewNetwork ();
        }
    }

  // Assign addresses to all col devices in the grid.
  // These devices are stored in a vector.  Each col 
  // of the grid has all the col devices in one entry 
  // of the vector.  These entries come in pairs.
  for (uint32_t i = 0; i < m_colDevices.size (); ++i)
    {
      Ipv4InterfaceContainer colInterfaces; 
      NetDeviceContainer colContainer = m_colDevices[i];
      for (uint32_t j = 0; j < colContainer.GetN (); j+=2)
        {
          colInterfaces.Add (linkIp.Assign (colContainer.Get (j))); 
          colInterfaces.Add (linkIp.Assign (colContainer.Get (j+1)));
          // linkIp.NewNetwork ();
        }
      // m_colInterfaces.push_back (colInterfaces);
    }
}

Ptr<Node> 
PointToPoint2DMeshHelper::GetNode (uint32_t row, uint32_t col)
{
  if (row > m_nodes.size () - 1 || 
      col > m_nodes.at (row).GetN () - 1) 
    {
      NS_FATAL_ERROR ("Index out of bounds in PointToPoint2DMeshHelper::GetNode.");
    }

  return (m_nodes.at (row)).Get (col);
}

Ptr<Node> 
PointToPoint2DMeshHelper::GetNode (unsigned nodeid)
{
  unsigned row = nodeid / m_xSize;
  unsigned col = nodeid % m_xSize;
  return (m_nodes.at (row)).Get (col);
}

Ipv4Address
PointToPoint2DMeshHelper::GetIpv4Address (uint32_t row, uint32_t col)
{
  if (row > m_nodes.size () - 1 || 
      col > m_nodes.at (row).GetN () - 1) 
    {
      NS_FATAL_ERROR ("Index out of bounds in PointToPoint2DMeshHelper::GetIpv4Address.");
    }

  return (m_Interfaces.at (row)).GetAddress (col);
}

Ipv4Address
PointToPoint2DMeshHelper::GetIpv4Address (unsigned nodeid){
  unsigned row = nodeid / m_xSize;
  unsigned col = nodeid % m_xSize;
  if (row > m_nodes.size () - 1 || 
      col > m_nodes.at (row).GetN () - 1) 
    {
      NS_FATAL_ERROR ("Index out of bounds in PointToPoint2DMeshHelper::GetIpv4Address.");
    }

  return (m_Interfaces.at (row)).GetAddress (col);
}

} // namespace ns3
