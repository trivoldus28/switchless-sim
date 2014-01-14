/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dim-ordered-tcp-example.h"

using namespace ns3;

int 
main (int argc, char *argv[])
{
    Time::SetResolution (Time::NS);
    //LogComponentEnable ("DoTcpL4Protocol", LOG_LEVEL_ALL);
    //LogComponentEnable ("DimensionOrderedL3Protocol", LOG_LEVEL_LOGIC);
    LogComponentEnable ("DimensionOrderedTcpClient", LOG_LEVEL_INFO);
    LogComponentEnable ("DimensionOrderedTcpServer", LOG_LEVEL_INFO);

    // Create 9 nodes to be configured in a torus
    NodeContainer nodes;
    nodes.Create (9);

    // Install DimensionOrdered stack
    DimensionOrderedStackHelper stack;
    stack.Install (nodes, std::make_tuple(1,1,0), std::make_tuple(3,3,0));

    // Setup topology
    DimensionOrderedAddressHelper::AddressAssignmentList assignList;
    assignList = SetupTopology (nodes);

    // Assign addresses to interfaces and interfaces to devices
    DimensionOrderedInterfaceContainer interfaces;
    interfaces = DimensionOrderedAddressHelper::Assign (assignList);

    // Install client application on node 8
    Ptr<DimensionOrderedTcpClient> client0 = CreateObject<DimensionOrderedTcpClient> ();
    // Send to Node 0
    client0->Setup (DimensionOrderedAddress(1,1,0));
    // Send from Node 8
    nodes.Get (8)->AddApplication (client0);
    client0->SetStartTime (Seconds (0.));
    client0->SetStopTime (Seconds (20.));

    // Install server application on node 0
    Ptr<DimensionOrderedTcpServer> server0 = CreateObject<DimensionOrderedTcpServer> ();
    server0->Setup ();
    // Install on node 0
    nodes.Get (0)->AddApplication (server0);
    server0->SetStartTime (Seconds (0.));
    server0->SetStopTime (Seconds (20.));

    // Install client application on node 7
    Ptr<DimensionOrderedTcpClient> client1 = CreateObject<DimensionOrderedTcpClient> ();
    // Send to Node 2
    client1->Setup (DimensionOrderedAddress (3,1,0));
    // Send from Node 7
    nodes.Get (7)->AddApplication (client1);
    client1->SetStartTime (Seconds (0.));
    client1->SetStopTime (Seconds (20.));
    
    // Install sever application on node 2
    Ptr<DimensionOrderedTcpServer> server1 = CreateObject<DimensionOrderedTcpServer> ();
    server1->Setup ();
    // Install on node 2
    nodes.Get (2)->AddApplication (server1);
    server1->SetStartTime (Seconds (0.));
    server1->SetStopTime (Seconds (20.));

    // Install client application on node 5
    Ptr<DimensionOrderedTcpClient> client2 = CreateObject<DimensionOrderedTcpClient> ();
    // Send to node 6
    client2->Setup (DimensionOrderedAddress (1,3,0));
    // Send from node 5
    nodes.Get (5)->AddApplication (client2);
    client2->SetStartTime (Seconds (0.));
    client2->SetStopTime (Seconds (20.));

    // Install server application on node 6
    Ptr<DimensionOrderedTcpServer> server2 = CreateObject<DimensionOrderedTcpServer> ();
    server2->Setup ();
    // Install on node 6
    nodes.Get (6)->AddApplication (server2);
    server2->SetStartTime (Seconds (0.));
    server2->SetStopTime (Seconds (20.));

    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}

DimensionOrderedAddressHelper::AddressAssignmentList SetupTopology (NodeContainer &nodes)
{
    // Setup a point to point helper to connect topology
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    // List of assignments to make
    DimensionOrderedAddressHelper::AddressAssignmentList assignList;
    
    // Temporary device container
    NetDeviceContainer devices;

    // Create topology (with warp around not shown):
    // 0 -- 1 -- 2
    // |    |    |
    // 3 -- 4 -- 5
    // |    |    |
    // 6 -- 7 -- 8
    devices = pointToPoint.Install (nodes.Get (0), nodes.Get (1));
    assignList.push_back (std::make_tuple (devices.Get (0), 
                                           DimensionOrderedAddress (1,1,0), 
                                           DimensionOrdered::X_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (2,1,0), 
                                           DimensionOrdered::X_NEG));

    devices = pointToPoint.Install (nodes.Get (0), nodes.Get (2));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (1,1,0),
                                           DimensionOrdered::X_NEG));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (3,1,0),
                                           DimensionOrdered::X_POS));

    devices = pointToPoint.Install (nodes.Get (0), nodes.Get (3));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (1,1,0),
                                           DimensionOrdered::Y_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (1,2,0),
                                           DimensionOrdered::Y_NEG));

    devices = pointToPoint.Install (nodes.Get (0), nodes.Get (6));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (1,1,0),
                                           DimensionOrdered::Y_NEG));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (1,3,0),
                                           DimensionOrdered::Y_POS));

    devices = pointToPoint.Install (nodes.Get (1), nodes.Get (2));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (2,1,0),
                                           DimensionOrdered::X_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (3,1,0),
                                           DimensionOrdered::X_NEG)); 

    devices = pointToPoint.Install (nodes.Get (1), nodes.Get (4));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (2,1,0),
                                           DimensionOrdered::Y_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (2,2,0),
                                           DimensionOrdered::Y_NEG));

    devices = pointToPoint.Install (nodes.Get (1), nodes.Get (7));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (2,1,0),
                                           DimensionOrdered::Y_NEG));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (2,3,0),
                                           DimensionOrdered::Y_POS));

    devices = pointToPoint.Install (nodes.Get (2), nodes.Get (5));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (3,1,0),
                                           DimensionOrdered::Y_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (3,2,0),
                                           DimensionOrdered::Y_NEG));

    devices = pointToPoint.Install (nodes.Get (2), nodes.Get (8));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (3,1,0),
                                           DimensionOrdered::Y_NEG));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (3,3,0),
                                           DimensionOrdered::Y_POS));

    devices = pointToPoint.Install (nodes.Get (3), nodes.Get (4));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (1,2,0),
                                           DimensionOrdered::X_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (2,2,0),
                                           DimensionOrdered::X_NEG));

    devices = pointToPoint.Install (nodes.Get (3), nodes.Get (5));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (1,2,0),
                                           DimensionOrdered::X_NEG));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (3,2,0),
                                           DimensionOrdered::X_POS));

    devices = pointToPoint.Install (nodes.Get (3), nodes.Get (6));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (1,2,0),
                                           DimensionOrdered::Y_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (1,3,0),
                                           DimensionOrdered::Y_NEG));

    devices = pointToPoint.Install (nodes.Get (4), nodes.Get (5));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (2,2,0),
                                           DimensionOrdered::X_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (3,2,0),
                                           DimensionOrdered::X_NEG));

    devices = pointToPoint.Install (nodes.Get (4), nodes.Get (7));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (2,2,0),
                                           DimensionOrdered::Y_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (2,3,0),
                                           DimensionOrdered::Y_NEG));

    devices = pointToPoint.Install (nodes.Get (5), nodes.Get (8));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (3,2,0),
                                           DimensionOrdered::Y_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (3,3,0),
                                           DimensionOrdered::Y_NEG));

    devices = pointToPoint.Install (nodes.Get (6), nodes.Get (7));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (1,3,0),
                                           DimensionOrdered::X_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (2,3,0),
                                           DimensionOrdered::X_NEG));

    devices = pointToPoint.Install (nodes.Get (6), nodes.Get (8));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (1,3,0),
                                           DimensionOrdered::X_NEG));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (3,3,0),
                                           DimensionOrdered::X_POS));

    devices = pointToPoint.Install (nodes.Get (7), nodes.Get (8));
    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (2,3,0),
                                           DimensionOrdered::X_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (3,3,0),
                                           DimensionOrdered::X_NEG));

    return assignList; 
}
