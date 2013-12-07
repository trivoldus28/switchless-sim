/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 *  Author: Michael McKeown
 */

// NS-3 Includes
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

// Switchless Includes
#include "data-center-app.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TwoNodeTest");

int
main (int argc, char * argv[])
{
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

    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces = address.Assign (devices);
    
    Ptr<DataCenterApp> app0 = CreateObject<DataCenterApp> (); 
    DataCenterApp::SendParams app0Params;
    app0Params.m_sending = true;
    Ipv4Address app0NodeList[] = {interfaces.GetAddress (1)};
    app0Params.m_nodes = app0NodeList;
    app0Params.m_receivers = DataCenterApp::ALL_IN_LIST;
    app0Params.m_nReceivers = 1;
    app0Params.m_sendPattern = DataCenterApp::FIXED_INTERVAL;
    app0Params.m_sendInterval = MilliSeconds (500.);
    app0Params.m_packetSize = 1024;
    app0Params.m_nPackets = 10;    
    app0->Setup(app0Params, true); 
    nodes.Get (0)->AddApplication (app0);
    app0->SetStartTime (Seconds(0.));
    app0->SetStopTime (Seconds(20.));

    Ptr<DataCenterApp> app1 = CreateObject<DataCenterApp> ();
    DataCenterApp::SendParams app1Params;
    app1Params.m_sending = true;
    Ipv4Address app1NodeList[] = {interfaces.GetAddress (0)};
    app1Params.m_nodes = app1NodeList;
    app1Params.m_receivers = DataCenterApp::ALL_IN_LIST;
    app1Params.m_nReceivers = 1;
    app1Params.m_sendPattern = DataCenterApp::FIXED_INTERVAL;
    app1Params.m_sendInterval = MilliSeconds (500.);
    app1Params.m_packetSize = 1024;
    app1Params.m_nPackets = 10;
    app1->Setup(app1Params, true);
    nodes.Get (1)->AddApplication (app1);
    app1->SetStartTime (Seconds(0.));
    app1->SetStopTime (Seconds(20.));

    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
