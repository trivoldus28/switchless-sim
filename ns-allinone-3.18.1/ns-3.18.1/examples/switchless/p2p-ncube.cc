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

#include "p2p-ncube.h"
 
NS_LOG_COMPONENT_DEFINE ("PointToPointNcubeHelper");

unsigned getLesserInDimension(unsigned dimension, unsigned current_node, unsigned mary){
  unsigned scale = 1;
  for (int i = 0; i < dimension; i++){
    scale *= mary;
  }
  unsigned num = current_node - scale;
  return num;
}

unsigned getLeastInDimension(unsigned dimension, unsigned current_node, unsigned mary){
  unsigned pos = current_node;
  for (int i = 0; i < dimension; i++){
    pos /= mary;
  }
  pos = (pos % mary);

  unsigned scale = 1;
  for (int i = 0; i < dimension; i++){
    scale *= mary;
  }
  unsigned num = current_node - (scale * pos);
  return num;
}

unsigned isLeastInDimension(unsigned dimension, unsigned current_node, unsigned mary){
  unsigned num = current_node;
  for (int i = 0; i < dimension; i++){
    num /= mary;
  }
  return (num % mary) == 0;
}

unsigned isGreatestInDimension(unsigned dimension, unsigned current_node, unsigned mary){
  unsigned num = current_node;
  for (int i = 0; i < dimension; i++){
    num /= mary;
  }
  return (num % mary) == (mary - 1);
}

namespace ns3 {

PointToPointNcubeHelper::PointToPointNcubeHelper (uint32_t nMary, unsigned nNcube, bool isTorus,
                                                PointToPointHelper pointToPoint)
  : m_mary (nMary), m_ncube(nNcube), m_isTorus(isTorus)
{

  unsigned num_nodes = pow(nMary,nNcube);
  m_total_nodes = num_nodes;
  m_nodes.Create(num_nodes);
  m_hubs.Create(num_nodes);

  // for communication between machine and "virtual hub"
  PointToPointHelper p2pbridge;
  p2pbridge.SetDeviceAttribute ("DataRate", StringValue ("100000Mbps"));
  p2pbridge.SetChannelAttribute ("Delay", StringValue ("0ms"));

  for (unsigned i = 0; i < num_nodes; i++){
    m_hub_bridge_devs.Add(p2pbridge.Install(m_nodes.Get(i), m_hubs.Get(i)));
  }

  for (unsigned i = 0; i < num_nodes; i++){
    for (unsigned dim = 0; dim < nNcube; dim++){

      if (!isLeastInDimension(dim,i,nMary)){
        unsigned lesser_node_id = getLesserInDimension(dim, i, nMary);
        m_devices.Add(pointToPoint.Install(m_hubs.Get(i), m_hubs.Get(lesser_node_id)));
        if (isTorus){
          if (isGreatestInDimension(dim, i, nMary)){
            unsigned loopback_node_id = getLeastInDimension(dim, i, nMary);
            m_devices.Add(pointToPoint.Install(m_hubs.Get(i), m_hubs.Get(loopback_node_id)));
          }
        }
      }

    } // for each dim
  } // for each node
}

PointToPointNcubeHelper::~PointToPointNcubeHelper ()
{
}

void
PointToPointNcubeHelper::InstallStack (InternetStackHelper stack)
{
  for (uint32_t i = 0; i < m_total_nodes; i++){
    stack.Install(m_nodes.Get(i));
    stack.Install(m_hubs.Get(i));
  }
}

void
PointToPointNcubeHelper::AssignIpv4Addresses (Ipv4AddressHelper node_ip, Ipv4AddressHelper link_ip)
{
  for (uint32_t i = 0; i < m_hub_bridge_devs.GetN(); i+=2){
    m_Interfaces.Add (node_ip.Assign (m_hub_bridge_devs.Get (i))); 
    m_Interfaces.Add (node_ip.Assign (m_hub_bridge_devs.Get (i+1)));
    node_ip.NewNetwork ();
  }

  for (uint32_t i = 0; i < m_devices.GetN(); i++){
    link_ip.Assign(m_devices.Get(i));
  }
}

Ptr<Node> 
PointToPointNcubeHelper::GetNode (unsigned nodeid)
{
  return (m_nodes.Get(nodeid));
}

Ipv4Address
PointToPointNcubeHelper::GetIpv4Address (unsigned nodeid)
{
  return (m_Interfaces.GetAddress(nodeid*2));
}

} // namespace ns3
