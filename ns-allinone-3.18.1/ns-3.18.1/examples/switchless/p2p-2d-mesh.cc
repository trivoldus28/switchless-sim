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
                                                uint32_t nCols, 
                                                PointToPointHelper pointToPoint)
  : m_xSize (nCols), m_ySize (nRows)
{
  // Bounds check
  if (m_xSize < 1 || m_ySize < 1 || (m_xSize < 2 && m_ySize < 2))
    {
      NS_FATAL_ERROR ("Need more nodes for grid.");
    }

  InternetStackHelper stack;

  for (uint32_t y = 0; y < nRows; ++y)
    {
      NodeContainer rowNodes;
      NodeContainer rowRouters;
      NetDeviceContainer rowDevices;
      NetDeviceContainer colDevices;
      NetDeviceContainer hubDevices;

      for (uint32_t x = 0; x < nCols; ++x)
        {
          rowRouters.Create (1);
          rowNodes.Create(1);
          hubDevices.Add (pointToPoint.
                          Install (rowNodes.Get(x), rowRouters.Get (x)));;

          // install p2p links across the row
          if (x > 0)
            {
              rowDevices.Add (pointToPoint.
                              Install (rowRouters.Get (x-1), rowRouters.Get (x)));
            }

          // install vertical p2p links
          if (y > 0)
            {
              colDevices.Add (pointToPoint.
                              Install ((m_routers.at (y-1)).Get (x), rowRouters.Get (x)));
            }
        }

      m_routers.push_back (rowRouters);
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
      NodeContainer rowRouters = m_routers[i];
      for (uint32_t j = 0; j < rowNodes.GetN (); ++j)
        {
          stack.Install (rowNodes.Get (j));
          stack.Install (rowRouters.Get (j));
        }
    }
}

void
PointToPoint2DMeshHelper::AssignIpv4Addresses (Ipv4AddressHelper rowIp)
{
  // Assign addresses to all row devices in the grid.
  // These devices are stored in a vector.  Each row 
  // of the grid has all the row devices in one entry 
  // of the vector.  These entries come in pairs.
  for (uint32_t i = 0; i < m_rowDevices.size (); ++i)
    {
      Ipv4InterfaceContainer rowInterfaces; 
      Ipv4InterfaceContainer hubInterfaces; 
      NetDeviceContainer rowContainer = m_rowDevices[i];
      NetDeviceContainer hubContainer = m_hubDevices[i];
      for (uint32_t j = 0; j < rowContainer.GetN (); j+=2)
        {
          rowInterfaces.Add (rowIp.Assign (rowContainer.Get (j))); 
          rowInterfaces.Add (rowIp.Assign (rowContainer.Get (j+1)));
          rowIp.NewNetwork ();
        }
      for (uint32_t j = 0; j < hubContainer.GetN (); j+=2)
        {
          hubInterfaces.Add (rowIp.Assign (hubContainer.Get (j))); 
          hubInterfaces.Add (rowIp.Assign (hubContainer.Get (j+1)));
          rowIp.NewNetwork ();
        }
      // m_rowInterfaces.push_back (rowInterfaces);
      m_Interfaces.push_back(hubInterfaces);
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
          colInterfaces.Add (rowIp.Assign (colContainer.Get (j))); 
          colInterfaces.Add (rowIp.Assign (colContainer.Get (j+1)));
          rowIp.NewNetwork ();
        }
      // m_colInterfaces.push_back (colInterfaces);
    }
}

void
PointToPoint2DMeshHelper::AssignIpv6Addresses(Ipv6Address addrBase, Ipv6Prefix prefix)
{
  Ipv6AddressGenerator::Init(addrBase, prefix);
  Ipv6Address v6network;
  Ipv6AddressHelper addrHelper;

  // Assign addresses to all row devices in the grid.
  // These devices are stored in a vector.  Each row 
  // of the grid has all the row devices in one entry 
  // of the vector.  These entries come in pairs.
  for (uint32_t i = 0; i < m_rowDevices.size (); ++i)
    {
      Ipv6InterfaceContainer rowInterfaces; 
      NetDeviceContainer rowContainer = m_rowDevices[i];
      for (uint32_t j = 0; j < rowContainer.GetN (); j+=2)
        {
          v6network = Ipv6AddressGenerator::GetNetwork (prefix);
          addrHelper.SetBase(v6network, prefix);
          Ipv6InterfaceContainer ic = addrHelper.Assign (rowContainer.Get (j));
          rowInterfaces.Add (ic);
          ic = addrHelper.Assign (rowContainer.Get (j+1));
          rowInterfaces.Add (ic);
          Ipv6AddressGenerator::NextNetwork (prefix);
        }
      m_rowInterfaces6.push_back (rowInterfaces);
    }

  // Assign addresses to all col devices in the grid.
  // These devices are stored in a vector.  Each col 
  // of the grid has all the col devices in one entry 
  // of the vector.  These entries come in pairs.
  for (uint32_t i = 0; i < m_colDevices.size (); ++i)
    {
      Ipv6InterfaceContainer colInterfaces; 
      NetDeviceContainer colContainer = m_colDevices[i];
      for (uint32_t j = 0; j < colContainer.GetN (); j+=2)
        {
          v6network = Ipv6AddressGenerator::GetNetwork (prefix);
          addrHelper.SetBase(v6network, prefix);
          Ipv6InterfaceContainer ic = addrHelper.Assign (colContainer.Get (j));
          colInterfaces.Add (ic);
          ic = addrHelper.Assign (colContainer.Get (j+1));
          colInterfaces.Add (ic);
          Ipv6AddressGenerator::NextNetwork (prefix);
        }
      m_colInterfaces6.push_back (colInterfaces);
    }
}

void
PointToPoint2DMeshHelper::BoundingBox (double ulx, double uly,
                                     double lrx, double lry)
{
  double xDist; 
  double yDist; 
  if (lrx > ulx)
    {
      xDist = lrx - ulx;
    }
  else
    {
      xDist = ulx - lrx;
    }
  if (lry > uly)
    {
      yDist = lry - uly;
    }
  else
    {
      yDist = uly - lry;
    }
  double xAdder = xDist / m_xSize;
  double yAdder = yDist / m_ySize;
  double yLoc = yDist / 2;
  for (uint32_t i = 0; i < m_ySize; ++i)
    {
      double xLoc = xDist / 2;
      for (uint32_t j = 0; j < m_xSize; ++j)
        {
          Ptr<Node> node = GetNode (i, j);
          Ptr<ConstantPositionMobilityModel> loc = node->GetObject<ConstantPositionMobilityModel> ();
          if (loc ==0)
            {
              loc = CreateObject<ConstantPositionMobilityModel> ();
              node->AggregateObject (loc);
            }
          Vector locVec (xLoc, yLoc, 0);
          loc->SetPosition (locVec);

          xLoc += xAdder;
        }
      yLoc += yAdder;
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

Ipv6Address
PointToPoint2DMeshHelper::GetIpv6Address (uint32_t row, uint32_t col)
{
  if (row > m_nodes.size () - 1 ||
      col > m_nodes.at (row).GetN () - 1)
    {
      NS_FATAL_ERROR ("Index out of bounds in PointToPoint2DMeshHelper::GetIpv4Address.");
    }

  // Right now this just gets one of the addresses of the
  // specified node.  The exact device can't be specified.
  // If you picture the grid, the address returned is the
  // address of the left (row) device of all nodes, with
  // the exception of the left-most nodes in the grid;
  // in which case the right (row) device address is
  // returned
  if (col == 0)
    {
      return (m_rowInterfaces6.at (row)).GetAddress (0, 1);
    }
  else
    {
      return (m_rowInterfaces6.at (row)).GetAddress ((2*col)-1, 1);
    }
}

} // namespace ns3
