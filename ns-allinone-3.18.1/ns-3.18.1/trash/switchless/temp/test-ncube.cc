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

#include "p2p-ncube.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  unsigned nMary = 2;
  unsigned nNcube = 9;
  bool isTorus = false;

  CommandLine cmd;
  cmd.AddValue ("nMary", "Number of nodes in one dimension", nMary);
  cmd.AddValue ("nNcube", "Number of dimensions", nNcube);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("isTorus", "Whether the topology is torus", isTorus);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  std::cout << "Setting up simulation" << std::endl;

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1000Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  PointToPointNcubeHelper grid(nMary, nNcube, isTorus, pointToPoint);


  InternetStackHelper stack;
  grid.InstallStack(stack);

  Ipv4AddressHelper nodeAddresses;
  Ipv4AddressHelper linkAddresses;
  nodeAddresses.SetBase ("0.0.0.0", "255.255.255.0");
  linkAddresses.SetBase ("1.0.0.0", "255.0.0.0");
  grid.AssignIpv4Addresses(nodeAddresses, linkAddresses);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (grid.GetNode(0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (grid.GetIpv4Address(0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  unsigned max_id = pow(nMary,nNcube);
  std::cout << "max_id " << max_id << std::endl;
  max_id -= 1;
  ApplicationContainer clientApps = echoClient.Install (grid.GetNode(max_id));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  std::cout << "Populating routing table" << std::endl;

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  std::cout << "Done populating routing table" << std::endl;

  // pointToPoint.EnablePcapAll ("second");
  // csma.EnablePcap ("second", csmaDevices.Get (1), true);

  std::cout << "Running simulation" << std::endl;

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
