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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/switchless-module.h"
#include "dim-ordered-udp-client.h"
#include "dim-ordered-udp-server.h"

#include "p2p-cube-dimordered.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  // unsigned nMary = 2;
  // unsigned nNcube = 9;
  bool isTorus = true;
  unsigned nXdim = 6;
  unsigned nYdim = 6;
  unsigned nZdim = 1;

  CommandLine cmd;
  // cmd.AddValue ("nMary", "Number of nodes in one dimension", nMary);
  // cmd.AddValue ("nNcube", "Number of dimensions", nNcube);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("isTorus", "Whether the topology is torus", isTorus);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      // LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      // LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO)
    LogComponentEnable ("DimensionOrderedUdpClient", LOG_LEVEL_INFO);
    LogComponentEnable ("DimensionOrderedUdpServer", LOG_LEVEL_INFO);
    }

  std::cout << "Setting up simulation" << std::endl;

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1000Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  PointToPointCubeDimorderedHelper grid(nXdim, nYdim, nZdim, isTorus, pointToPoint);


  // InternetStackHelper stack;
  // grid.InstallStack(stack);

  // Ipv4AddressHelper nodeAddresses;
  // Ipv4AddressHelper linkAddresses;
  // nodeAddresses.SetBase ("0.0.0.0", "255.255.255.0");
  // linkAddresses.SetBase ("1.0.0.0", "255.0.0.0");
  // grid.AssignIpv4Addresses(nodeAddresses, linkAddresses);


  Ptr<DimensionOrderedUdpClient> client0 = CreateObject<DimensionOrderedUdpClient> ();
  // Send to Node 0
  client0->Setup (DimensionOrderedAddress(3,3,1));
  // Send from Node 8
  grid.GetNode (0)->AddApplication (client0);
  client0->SetStartTime (Seconds (0.));
  client0->SetStopTime (Seconds (20.));


  Ptr<DimensionOrderedUdpClient> client1 = CreateObject<DimensionOrderedUdpClient> ();
  // Send to Node 0
  client1->Setup (DimensionOrderedAddress(2,2,1));
  // Send from Node 8
  grid.GetNode (0)->AddApplication (client1);
  client1->SetStartTime (Seconds (0.));
  client1->SetStopTime (Seconds (20.));

  // Install server application on node 0
  for (int i = 0; i < nXdim*nYdim*nZdim; i++){
    Ptr<DimensionOrderedUdpServer> server0 = CreateObject<DimensionOrderedUdpServer> ();
    server0->Setup ();
    grid.GetNode (i)->AddApplication (server0);
    server0->SetStartTime (Seconds (0.));
    server0->SetStopTime (Seconds (20.));
  }

  std::cout << "Run simulation" << std::endl;

    Time::SetResolution (Time::MS);
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
