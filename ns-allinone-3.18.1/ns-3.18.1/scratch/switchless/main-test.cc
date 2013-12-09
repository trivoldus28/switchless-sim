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
#include "p2p-topology-interface.h"
#include "p2p-2d-mesh.h"
#include "p2p-fattree.h"
#include "p2p-ncube.h"

#include <unordered_set>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MainProgram");

#define DEBUG (true)
#define TREE 1
#define MESH 2
#define CUBE 3
#define NO_TOPO 4
#define RANDOM 1
#define FIXED 2

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

int
main (int argc, char * argv[])
{
    unsigned nRackSize = 0;
    unsigned nTreeFanout = 0;
    unsigned nRow = 0;
    unsigned nCol = 0;
    //bool bTorus = false; // not parsed
    std::string sSenderTopology = "random";  // not parsed
    std::string sReceiverTopology = "random"; // not parsed
    bool bFixedInterval = false;
    bool bRandomInterval = false;
    bool bSynchronized = false;

    int topologytype = 0;
    int topo_sub1 = 0;
    int topo_sub2 = 0;
    int nNodes = 0;
    int nIterations = 0;
    int nPacketSize = 0;
    int nSender = 0;
    int nReceiver = 0;
    int intervaltype = 0;
    int nintervalsize = 0;
    int nMininterval = 0;
    int nMaxinterval = 0;
    int synchronized = 0;
    CommandLine cmd;
    cmd.AddValue("tp", "Topology",topologytype); // 1 or 2 or 3
    cmd.AddValue("t1", "",topo_sub1); // leaf-fan-out or row or m
    cmd.AddValue("t2", "",topo_sub2);  //non-leaf-fan-out or column or n
    cmd.AddValue("ncount", "", nNodes);
    cmd.AddValue("psize", "", nPacketSize);
    cmd.AddValue("scount", "",nSender);
    cmd.AddValue("rcount", "",nReceiver);
    cmd.AddValue("itype", "",intervaltype); // 1 or 2
    cmd.AddValue("isize", "",nintervalsize); // only for fixed
    cmd.AddValue("minint", "",nMininterval); // only for random 
    cmd.AddValue("maxint", "",nMaxinterval); // only for random
    cmd.AddValue("sync", "",synchronized);
    cmd.Parse (argc, argv);
    if(intervaltype==RANDOM)
    {
        bFixedInterval = false;
        bRandomInterval = true;
    }
    else
    {
        bFixedInterval = true;
        bRandomInterval = false;
    }
    bSynchronized = synchronized;
    if(topologytype == MESH)
    {
        nRow = topo_sub1;
        nCol = topo_sub2;
    }
    else if (topologytype == TREE)
    {   
        nRackSize = topo_sub1;
        nTreeFanout = topo_sub2;
    }
    else
    {
        //m-ary n-cube
    }


    // LogComponentEnable ("DataCenterApp", LOG_LEVEL_ALL);
    // Parsing

    // common variables
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1000Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    PointToPointTopoHelper * topology;
    // switch statements
    if (topologytype == TREE){
        unsigned depth = log(nNodes) / log(nTreeFanout);
        uint64_t linkDataRate = 1024*1024*1024; // 1Gbps
        topology = new PointToPointFattreeHelper(nRackSize, nTreeFanout, depth, linkDataRate, pointToPoint);
    }
    else if (topologytype == MESH){
        std::cout << "Not implemented\n";
        return 0;
    }
    else if (topologytype == CUBE){
        std::cout << "Not implemented\n";
        return 0;
    }
    else{
        std::cout << "No such topo\n";
        return 0;
    }

    Time::SetResolution (Time::NS);

    InternetStackHelper stack;
    topology->InstallStack(stack);

    Ipv4AddressHelper nodeAddresses;
    Ipv4AddressHelper linkAddresses;
    nodeAddresses.SetBase ("0.0.0.0", "255.255.0.0");
    linkAddresses.SetBase ("1.0.0.0", "255.255.0.0");
    topology->AssignIpv4Addresses(nodeAddresses, linkAddresses); 

    // application installation

    // std::vector <int> senderSet;
    std::unordered_set<int> senderSet;
    if (sSenderTopology == "random"){
        unsigned randid = 1;
        for (int i = 0; i < nSender; i++){
            randid = randid * 1103515245 + 12345;
            randid %= nNodes;
            senderSet.insert(randid);
            // senderSet.push_back(randid);
            // TODO: need to check for uniqueness
            // check to see if unordered_set is C++11 supported
        }
    }
    else if (sSenderTopology == "set"){
        // For now let's just get the first n nodes
        for (int i = 0; i < nSender; i++){
            // senderSet.push_back(i);
            senderSet.insert(i);
            // TODO: need to check for uniqueness
            // check to see if unordered_set is C++11 supported
        } 
    }

    // vector <int> receiverSet;
    // if (sReceiverTopology == "random"){
    //     unsigned randid = 10;
    //     for (int i = 0; i < nSender; i++){
    //         randid = randid * 1103515245 + 12345;
    //         randid %= nNodes;
    //         receiverSet.push_back(randid);
    //         // TODO: need to check for uniqueness
    //         // check to see if unordered_set is C++11 supported
    //     }
    // }
    // else if (sReceiverTopology == "set"){
    //     // For now let's just get the first n nodes
    //     for (int i = nSender - 1; i >= 0; i--){
    //         receiverSet.push_back(i);
    //         // TODO: need to check for uniqueness
    //         // check to see if unordered_set is C++11 supported
    //     } 
    // }

    for (std::unordered_set<int>::iterator it = senderSet.begin(); it != senderSet.end(); it++){
        DataCenterApp::SendParams params;
        params.m_sending = true;
        std::vector <Ipv4Address> receiverNodeList;

        if (sSenderTopology == "random"){
            for (int i = 0; i < nNodes; i++){
                Ipv4Address t = topology->GetIpv4Address(i);
                receiverNodeList.push_back(t);
            }
            // params.m_nodes = &receiverNodeList; // TODO: enable this when interface is changed
            params.m_nNodes = nNodes;
            params.m_nReceivers = nReceiver;
            params.m_receivers = DataCenterApp::RANDOM_SUBSET;
        }
        else{
            std::cout << "Unimplemented1\n" << std::endl;
        }

        if (bFixedInterval && bSynchronized){
            params.m_sendPattern = DataCenterApp::FIXED_INTERVAL;
            params.m_sendInterval = MilliSeconds (500.);
        }
        else{
            std::cout << "Unimplemented2\n" << std::endl;
        }
        
        params.m_packetSize = 1024;
        params.m_nPackets = nIterations; 
        // TODO: MISSING MAX/MIN

        Ptr<DataCenterApp> app = CreateObject<DataCenterApp>();
        app->Setup(params, *it, DEBUG); 
        topology->GetNode(*it)->AddApplication(app);

        // TODO: set time
        app->SetStartTime (Seconds(0.));
        app->SetStopTime (Seconds(20.));
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
