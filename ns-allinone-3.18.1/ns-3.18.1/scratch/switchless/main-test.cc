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


int
main (int argc, char * argv[])
{
    unsigned nRackSize = 0;
    unsigned nTreeFanout = 0;
    //bool bTorus = false; // not parsed
    std::string sSenderChoice = "random";  // not parsed
    std::string sReceiverChoice = "random"; // not parsed
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
    int m_cube=0;
    int n_cube=0;
    int nNeighbor=0;
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
    cmd.AddValue("neighborcount","",nNeighbor);
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
        // Only n^2
    }
    else if (topologytype == TREE)
    {   
        nRackSize = topo_sub1;
        nTreeFanout = topo_sub2;
    }
    else
    {
        m_cube= topo_sub1;
        n_cube= topo_sub2;
    }


    // LogComponentEnable ("DataCenterApp", LOG_LEVEL_ALL);

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
        //TODO: DO IT
        return 0;
    }
    else if (topologytype == CUBE){
        std::cout << "Not implemented\n";
        //TODO: DO IT
        return 0;
    }
    else{
        std::cout << "No such topo\n";
        //TODO: DO IT
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


    // Random Seed 
    srand(100);

    std::unordered_set<int> senderSet;
    if (sSenderChoice == "random"){
        unsigned randid = 0;
        for (int i = 0; i < nSender; i++){
            randid = rand();
            randid %= nNodes;
            if(senderSet.find(randid) != senderSet.end())
            {
                senderSet.insert(randid);
            }
            else
            {
                i--;
            }
        }
    }
    else if (sSenderChoice == "set"){
        // For now let's just get the first n nodes
        for (int i = 0; i < nSender; i++){
            senderSet.insert(i);
        } 
    }

    for (std::unordered_set<int>::iterator it = senderSet.begin(); it != senderSet.end(); it++){
        DataCenterApp::SendParams params;
        params.m_sending = true;
        std::vector <Ipv4Address> receiverNodeList;

        if (sReceiverChoice == "random"){
            unsigned randid=0;
            for (int i = 0; i < nReceiver; i++){
                randid = rand();
                randid %= nNodes;
                if(std::find(receiverNodeList.begin(), receiverNodeList.end(), topology->GetIpv4Address(randid))!=receiverNodeList.end())
                {
                     Ipv4Address t = topology->GetIpv4Address(randid);
                     receiverNodeList.push_back(t);
                }
                else
                {
                    i--;
                }
              
            }
            //params.m_nodes = &receiverNodeList; // TODO: enable this when interface is changed // So who's method are we using.
            params.m_nNodes = nNodes;
            params.m_nReceivers = nReceiver;
            params.m_receivers = DataCenterApp::RANDOM_SUBSET;
        }
        else{
             std::cout << "Unimplemented1\n" << std::endl; // Neighborhood Case
             if (topologytype == TREE){
                int nodeid = *it;
                if(nodeid >= (nNeighbor/2) && nodeid <= nNodes-1-(nNeighbor/2))
                {
                    for(int i=1; i <= nNeighbor/2; i++)
                    {
                        Ipv4Address t = topology->GetIpv4Address(nodeid-i);
                        receiverNodeList.push_back(t);
                        t= topology->GetIpv4Address(nodeid+i);
                        receiverNodeList.push_back(t);
                    }
                }
                else if(nodeid >= (nNeighbor/2) && nodeid > nNodes-1-(nNeighbor/2))
                {
                    int getFirst = nodeid-nNodes+1+(nNeighbor/2);
                    for(int i=1; i<=getFirst; i++)
                    {
                        Ipv4Address t = topology->GetIpv4Address(i);
                        receiverNodeList.push_back(t);
                    }
                    for(int i=1; i <= nNeighbor/2; i++)
                    {
                        Ipv4Address t = topology->GetIpv4Address(nodeid-i);
                        receiverNodeList.push_back(t);
                    }
                    for (int i=1; i<= nNeighbor/2 - getFirst ; i++)
                    {
                        Ipv4Address t = topology->GetIpv4Address(nodeid+i);
                        receiverNodeList.push_back(t);   
                    }

                }
                else if(nodeid < (nNeighbor/2) && nodeid <= nNodes-1-(nNeighbor/2))
                {
                    int getLast = nNeighbor/2 - nodeid;
                    for(int i=1; i<=getLast; i++)
                    {
                        Ipv4Address t = topology->GetIpv4Address(nNodes-i);
                        receiverNodeList.push_back(t);
                    }
                    for(int i=1; i <= nNeighbor/2; i++)
                    {
                        Ipv4Address t = topology->GetIpv4Address(nodeid+i);
                        receiverNodeList.push_back(t);
                    }
                    for (int i=1; i<= nNeighbor/2 - getLast ; i++)
                    {
                        Ipv4Address t = topology->GetIpv4Address(nodeid-i);
                        receiverNodeList.push_back(t);   
                    }
                }
             }
             else if(topologytype == MESH){
             }
             else if(topologytype == CUBE){
             }
        }

        if (bFixedInterval && bSynchronized){
            params.m_sendPattern = DataCenterApp::FIXED_INTERVAL;
            params.m_sendInterval = MilliSeconds (500.);
        }
        else if(bFixedInterval && !bSynchronized)
        {
            std::cout << "Unimplemented2\n" << std::endl; // Implement Parameter Setting
        }
        else if(!bFixedInterval && bSynchronized)
        {
            std::cout << "Unimplemented2\n" << std::endl; // Implement Parameter Setting
        }
        else
        {
            std::cout << "Unimplemented2\n" << std::endl; // Implement Parameter Setting
        }
        params.m_packetSize = nPacketSize;
        params.m_nPackets = nIterations; 
        Ptr<DataCenterApp> app = CreateObject<DataCenterApp>();
        app->Setup(params, *it, DEBUG); 
        topology->GetNode(*it)->AddApplication(app);

        // TODO: set time
        app->SetStartTime (Seconds(0.));
        app->SetStopTime (Seconds(100.));
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
