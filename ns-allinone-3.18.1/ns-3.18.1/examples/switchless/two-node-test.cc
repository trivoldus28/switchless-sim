/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 *  Author: Michael McKeown
 */

// C/C++ Includes

// NS-3 Includes
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

// Switchless Includes
#include "data-center-app.h"

#define UDP_IP 0
#define TCP_IP 1
#define UDP_DO 2
#define TCP_DO 3

#define STACK UDP_DO

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TwoNodeTest");

Ipv4InterfaceContainer SetupInternetStack (NodeContainer &nodes, NetDeviceContainer &devices);
DimensionOrderedInterfaceContainer SetupDimensionOrderedStack (NodeContainer &nodes, NetDeviceContainer &devices);

int
main (int argc, char * argv[])
{
#if STACK == UDP_IP
    DataCenterApp::NETWORK_STACK stack = DataCenterApp::UDP_IP_STACK;
#elif STACK == TCP_IP
    DataCenterApp::NETWORK_STACK stack = DataCenterApp::TCP_IP_STACK;
#elif STACK == UDP_DO
    DataCenterApp::NETWORK_STACK stack = DataCenterApp::UDP_DO_STACK;
#elif STACK == TCP_DO
    DataCenterApp::NETWORK_STACK stack = DataCenterApp::TCP_DO_STACK;
#endif

    Time::SetResolution (Time::NS);
    LogComponentEnable ("DataCenterApp", LOG_LEVEL_ALL);

    NodeContainer nodes;
    nodes.Create (2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install (nodes);

    // TODO: Do we want a RateErrorModel to model errors to cause retransmits?

#if STACK == UDP_IP || STACK == TCP_IP
    Ipv4InterfaceContainer interfaces = SetupInternetStack (nodes, devices);
#elif STACK == UDP_DO || STACK == TCP_DO
    DimensionOrderedInterfaceContainer interfaces = SetupDimensionOrderedStack (nodes, devices);
#endif
      
    Ptr<DataCenterApp> app0 = CreateObject<DataCenterApp> (); 
    DataCenterApp::SendParams app0Params;
    app0Params.m_sending = true;
    app0Params.m_nodes.push_back (interfaces.GetAddress (1));
    app0Params.m_receivers = DataCenterApp::ALL_IN_LIST;
    app0Params.m_nReceivers = 1;
    app0Params.m_sendPattern = DataCenterApp::FIXED_INTERVAL;
    app0Params.m_sendInterval = MilliSeconds (500.);
    app0Params.m_maxSendInterval = MilliSeconds (500.);
    app0Params.m_minSendInterval = MilliSeconds (100.);
    app0Params.m_packetSize = 512;
    app0Params.m_nIterations = 10;    
    app0->Setup(app0Params, 0, stack, true); 
    nodes.Get (0)->AddApplication (app0);
    app0->SetStartTime (Seconds(0.));
    app0->SetStopTime (Seconds(20.));

    Ptr<DataCenterApp> app1 = CreateObject<DataCenterApp> ();
    DataCenterApp::SendParams app1Params;
    app1Params.m_sending = true;
    app1Params.m_nodes.push_back (interfaces.GetAddress (0));
    app1Params.m_receivers = DataCenterApp::ALL_IN_LIST;
    app1Params.m_nReceivers = 1;
    app1Params.m_sendPattern = DataCenterApp::FIXED_INTERVAL;
    app1Params.m_sendInterval = MilliSeconds (500.);
    app1Params.m_maxSendInterval = MilliSeconds (500.);
    app1Params.m_minSendInterval = MilliSeconds (100.);
    app1Params.m_packetSize = 512;
    app1Params.m_nIterations = 10;
    app1->Setup(app1Params, 1, stack, true);
    nodes.Get (1)->AddApplication (app1);
    app1->SetStartTime (Seconds(0.));
    app1->SetStopTime (Seconds(20.));

    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}

Ipv4InterfaceContainer
SetupInternetStack (NodeContainer &nodes, NetDeviceContainer &devices)
{
    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");

    return address.Assign (devices);
}

DimensionOrderedInterfaceContainer 
SetupDimensionOrderedStack (NodeContainer &nodes, NetDeviceContainer &devices)
{
    DimensionOrderedStackHelper stack;
    stack.Install (nodes, std::make_tuple(1,1,0), std::make_tuple(2,1,0));

    DimensionOrderedAddressHelper::AddressAssignmentList assignList;

    assignList.push_back (std::make_tuple (devices.Get (0),
                                           DimensionOrderedAddress (1,1,0),
                                           DimensionOrdered::X_POS));
    assignList.push_back (std::make_tuple (devices.Get (1),
                                           DimensionOrderedAddress (2,1,0),
                                           DimensionOrdered::X_NEG));

    DimensionOrderedAddressHelper address;
    return address.Assign (assignList);

}
