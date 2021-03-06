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
// #include "p2p-2d-mesh.h"
#include "p2p-fattree.h"
#include "p2p-cube.h"
#include "p2p-hierarchical.h"
#include "p2p-cube-dimordered.h"

#include <unordered_set>
#include <utility> // std::pair, std::make_pair

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MainProgram");

#define DEBUG (true)
#define FATTREE 1
#define MESH_DEPRECATED 2
#define CUBE 3
#define HIERARCHICAL 4
#define RANDOM 1
#define FIXED 2
#define CUBE_DIMORDERED 5
#define NO_TOPO 8

#define L4_TCP 1
#define L4_UDP 2


int
main (int argc, char * argv[])
{

    LogComponentEnable ("DataCenterApp", LOG_DEBUG);
    // LogComponentEnable ("DataCenterApp", LOG_LEVEL_ALL);
    // LogComponentEnable ("DimensionOrderedL3Protocol", LOG_LEVEL_ALL);
    // LogComponentEnable ("DimensionOrderedL3Protocol", LOG_LEVEL_ALL);
    // unsigned nRackSize = 0;
    // unsigned nTreeFanout = 0;
    bool bTorus = true; // not parsed
    std::string sSenderChoice = "random";  // not parsed
    std::string sReceiverChoice = "random"; // not parsed
    bool bFixedInterval = false;
    bool bRandomInterval = false;
    bool bSynchronized = false;
    // unsigned nYdim = 0;
    // unsigned nXdim = 0;
    DataCenterApp::NETWORK_STACK network_stack_type = DataCenterApp::INVALID_STACK;
    unsigned l4_type = 0;

    // parameters for hierarchical
    unsigned nEdge = 0;
    unsigned nAgg = 0;
    unsigned nRepl1 = 0;
    unsigned nRepl2 = 0;
    //unsigned nCore = 1; // always 1

    // parameters for cube
    unsigned nXdim;
    unsigned nYdim;
    unsigned nZdim;
    nXdim = nYdim = nZdim = 0;

    int topologytype = 0;
    int topo_sub1 = 0;
    int topo_sub2 = 0;
    int topo_sub3 = 0;
    int topo_sub4 = 0;
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
    // int m_cube=0;
    // int n_cube=0;
    int nNeighbor=0;
    int sChoice=0;
    int rChoice=0;
    CommandLine cmd;
    int debuglog = 0;
    cmd.AddValue("debug", "Debug", debuglog);
    cmd.AddValue("tp", "Topology",topologytype); // 1 or 2 or 3
    cmd.AddValue("t1", "",topo_sub1); // leaf-fan-out or row or m
    cmd.AddValue("t2", "",topo_sub2);  //non-leaf-fan-out or column or n
    cmd.AddValue("t3", "",topo_sub3); // leaf-fan-out or row or m
    cmd.AddValue("t4", "",topo_sub4);  //non-leaf-fan-out or column or n
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
    cmd.AddValue("sChoice", "", sChoice); //0 for random
    cmd.AddValue("rChoice", "", rChoice); //0 for random
    cmd.AddValue("iter", "", nIterations);
    cmd.AddValue("l4type", "", l4_type);
    cmd.Parse (argc, argv);
    if(debuglog==1)
    {
        LogComponentEnable ("DataCenterApp", LOG_INFO);
        LogComponentDisable("DataCenterApp", LOG_DEBUG);
    }
    if(sChoice != 0)
    {  
        //sSenderChoice = "set";
        //We don't support it
    }
    // if(rChoice != 0)
    if (nNeighbor != 0)
    {  
        sReceiverChoice = "set";
        NS_ASSERT(nReceiver == 0);
    }
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

    // if(topologytype == MESH)
    // {
    //     nXdim = topo_sub1;
    //     nYdim = topo_sub2;
    // }
    //else 
    if (topologytype == HIERARCHICAL)
    {   
        nEdge = topo_sub1;
        nAgg = topo_sub2;
        nRepl1 = topo_sub3;
        nRepl2 = topo_sub4;
    }
    else if (topologytype == CUBE || topologytype == CUBE_DIMORDERED)
    {
        nXdim= topo_sub1;
        nYdim= topo_sub2;
        nZdim= topo_sub3;
    }
    else if (topologytype == FATTREE)
    {
    }
    else{
        std::cout << "Invalid topo\n";
        NS_ASSERT(false);
    }

    NS_ASSERT(l4_type != 0);


    Config::SetDefault ("ns3::DropTailQueue::Mode", StringValue ("QUEUE_MODE_PACKETS"));
    Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue (20000));
    // common variables
    PointToPointHelper pointToPoint;


    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Gbps")); // 100Gbps is 10Gbps for some reason
    // pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("500ns")); // .5us

    std::cout << "Making topology\n";
    PointToPointTopoHelper * topology;
    // switch statements
    if (topologytype == FATTREE){
        // NS_ASSERT(nNodes % nRackSize == 0);
        // unsigned depth = log(nNodes / nRackSize) / log(nTreeFanout);
        // NS_ASSERT(nNodes/nRackSize == pow(nTreeFanout, depth));
        topology = new PointToPointFattreeHelper(nNodes, pointToPoint);
        if (l4_type == L4_UDP)
            network_stack_type = DataCenterApp::UDP_IP_STACK;
        else
            network_stack_type = DataCenterApp::TCP_IP_STACK;
    }
    // else if (topologytype == MESH){
    //     NS_ASSERT(nNodes <= (nYdim * nXdim));
    //     topology = new PointToPoint2DMeshHelper(nYdim, nXdim, bTorus, pointToPoint);
    // }
    else if (topologytype == CUBE || topologytype == CUBE_DIMORDERED){
        NS_ASSERT(nNodes <= (nXdim * nYdim * nZdim));
        if (topologytype == CUBE){
            topology = new PointToPointCubeHelper(nXdim, nYdim, nZdim, bTorus, pointToPoint);
            if (l4_type == L4_UDP)
                network_stack_type = DataCenterApp::UDP_IP_STACK;
            else
                network_stack_type = DataCenterApp::TCP_IP_STACK;
        }
        else{
            topology = new PointToPointCubeDimorderedHelper(nXdim, nYdim, nZdim, bTorus, pointToPoint);
            if (l4_type == L4_UDP)
                network_stack_type = DataCenterApp::UDP_DO_STACK;
            else
                network_stack_type = DataCenterApp::TCP_DO_STACK;
        }
    }
    else if (topologytype == HIERARCHICAL){
        topology = new PointToPointHierarchicalHelper(nNodes, nEdge, nAgg, nRepl1, nRepl2, pointToPoint);
        if (l4_type == L4_UDP)
            network_stack_type = DataCenterApp::UDP_IP_STACK;
        else
            network_stack_type = DataCenterApp::TCP_IP_STACK;
    }
    else{
        std::cout << "Invalid topo\n";
        return 0;
    }

    Time::SetResolution (Time::NS);

    InternetStackHelper stack;
    topology->InstallStack(stack);
    Ipv4AddressHelper nodeAddresses;
    Ipv4AddressHelper linkAddresses;
    nodeAddresses.SetBase ("0.0.0.0", "255.255.0.0");
    linkAddresses.SetBase ("128.0.0.0", "255.255.0.0");
    //std::cout << "Point 1\n";
    topology->AssignIpv4Addresses(nodeAddresses, linkAddresses); 
    std::cout << "Finished assigning IP addresses\n";
    //std::cout << "Point 2\n";
    // Random Seed 
    srand(100);
    std::unordered_set<int> senderSet;
    std::unordered_set<int> nonsenderSet;
    for (int i=0; i<nNodes; i++)
    {
        nonsenderSet.insert(i);
    }
    if (sSenderChoice == "random"){
        unsigned randid = 0;
        while (senderSet.size() < nSender){
            randid = rand();
            randid %= nNodes;
            senderSet.insert(randid);
            nonsenderSet.erase(randid);
        }

    }
    else
        NS_ASSERT(sSenderChoice != "random");

    std::cout << "Making application parameters\n";
    for (std::unordered_set<int>::iterator it = senderSet.begin(); it != senderSet.end(); it++){
        DataCenterApp::SendParams params;
        params.m_sending = true;
        std::vector <Address> receiverNodeList;

        if (sReceiverChoice == "random"){
            // std::cout << "Random Receiver" << std::endl;
            for(int i=0;i<nNodes;i++)
            {
                if(i!=*it)
                {
                    Address t = topology->GetAddress(i);
                    receiverNodeList.push_back(t);
                }
            }

            params.m_nReceivers = nReceiver;
            params.m_receivers = DataCenterApp::RANDOM_SUBSET;
        }
        else{
            // std::cout << "Set Receiver" << std::endl;
            // std::cout << nNeighbor << std::endl;
            // std::cout << nNodes << std::endl;

             std::unordered_set<int> receiverSet;
             if (topologytype == FATTREE || topologytype == HIERARCHICAL){
                int nodeid = *it;
                // float logval = log2(nNeighbor); // 
                // int logv = ceil(logval); //
                // int rem1 = nodeid % (int)(pow(2,(logv-1))); //
                // int rem2 = nodeid % (int)(pow(2,(logv))); //
                // int base1 = nodeid- rem1;
                // for (int i=0; i< pow(2,(logv-1)); i++)
                // {
                //     receiverSet.insert(base1+i);
                // }
                // int remainings = nNeighbor-pow(2,(logv-1));
                // int base2;
                // if(rem2> pow(2,(logv-1)))
                // {
                //     base2 = nodeid - rem2;
                // }
                // else
                // {
                //     base2 = nodeid - rem2 + pow(2,(logv-1));
                // }
                // while (remainings !=0)
                // {
                //     int randid = rand() % (int)(pow(2,(logv-1)));
                //     receiverSet.insert(base2+randid);
                //     remainings --;
                // }

                // int remaincount = nNeighbor;
                int startid = nodeid - nNeighbor/2; // centering
                startid = nodeid - (nodeid % 16);
                while (startid < 0) startid += 16;
                while ((startid + nNeighbor) > nNodes) startid -= 16;
                for (int i = startid; i < startid + nNeighbor; i++){
                    receiverSet.insert(i);
                }
             }
             // else if(topologytype == MESH){
             //    // Basically we will have a box to draw random, clustered nodes from
             //    // Box boundaries are 1,4,9,16,25,...
             //    // centered around this particular send node (*it)
             //    unsigned senderY = *it / nXdim;
             //    unsigned senderX = *it % nXdim;
             //    unsigned mindistance =1;
             //    int failSafeCounter = 0;
             //    while (receiverSet .size() < nNeighbor){
             //        int randx = rand() % (mindistance*2+1);
             //        int randy = rand() % (mindistance*2+1);
             //        randx = randx-mindistance;
             //        randy = randy-mindistance;
             //        int distance = abs(randx)+abs(randy);
             //        if(mindistance == distance)
             //        {
             //            int poty = randy + senderY;
             //            int potx = randx + senderX;
             //            NS_ASSERT(bTorus == true);
             //            if (potx < 0)
             //                potx += nXdim;
             //            if (poty < 0)
             //                poty += nYdim;
             //            if (potx ==0 && poty == 0)
             //            {
             //                continue;
             //            }
             //            if (potx >= nXdim || poty >=nYdim) 
             //            {
             //                continue;
             //            }
             //            unsigned coord = poty* nXdim + potx;
             //            receiverSet.insert(coord);
             //        }
             //        if(receiverSet.size() == 2*mindistance*(mindistance+1))
             //        {
             //            mindistance++;
             //        }
             //        failSafeCounter++;
             //        if(failSafeCounter==10000)
             //        {
             //            failSafeCounter =0 ;
             //            mindistance++;
             //        }
             //    }
             // }
             else if(topologytype == CUBE || topologytype == CUBE_DIMORDERED){
                unsigned senderZ = *it / (nXdim * nYdim);
                unsigned senderY = *it / nXdim;
                unsigned senderX = *it % nXdim;
                unsigned mindistance =1;
                int failSafeCounter = 0;
                while (receiverSet .size() < nNeighbor){
                    int randx = rand() % (mindistance*2+1);
                    int randy = rand() % (mindistance*2+1);
                    int randz = rand() % (mindistance*2+1);
                    randx = randx-mindistance;
                    randy = randy-mindistance;
                    randz = randz-mindistance;
                    int distance = abs(randx)+abs(randy)+abs(randz);
                    if(distance <= mindistance && distance != 0)
                    {
                        int potz = randz + senderZ;
                        int poty = randy + senderY;
                        int potx = randx + senderX;
                        NS_ASSERT(bTorus == true);
                        if (potx < 0)
                            potx += nXdim;
                        if (poty < 0)
                            poty += nYdim;
                        if (potz < 0)
                            potz += nZdim;
                        if (potx ==0 && poty == 0 && potz == 0)
                        {
                            continue;
                        }
                        if (potx >= nXdim || poty >=nYdim || potz >= nZdim) 
                        {
                            continue;
                        }
                        unsigned coord = (potz * nYdim * nXdim) + (poty * nXdim) + potx;
                        receiverSet.insert(coord);
                    }
                    if(receiverSet.size() == 2*mindistance*(mindistance+1))
                    {
                        mindistance++;
                    }
                    failSafeCounter++;
                    if(failSafeCounter==10000)
                    {
                        failSafeCounter =0 ;
                        mindistance++;
                    }
                }
             }

             for (std::unordered_set<int>::iterator it = receiverSet.begin(); it != receiverSet.end(); it++)
             {
                Address t = topology->GetAddress(*it);
                receiverNodeList.push_back(t);
             }
            params.m_nReceivers = nNeighbor;
            params.m_receivers = DataCenterApp::ALL_IN_LIST;
        }

        // Finally putting the IP list to the parameters
        params.m_nodes = receiverNodeList; 
        if (bFixedInterval && bSynchronized){
            params.m_sendPattern = DataCenterApp::FIXED_INTERVAL;
            params.m_sendInterval = MicroSeconds (nintervalsize);
        }
        else if(bFixedInterval && !bSynchronized)
        {
            params.m_sendPattern = DataCenterApp::FIXED_SPORADIC;
            params.m_sendInterval = MicroSeconds (nintervalsize);
            params.m_maxSendInterval = MicroSeconds(nMaxinterval);
            params.m_minSendInterval = MicroSeconds(nMininterval);
        }
        else if(!bFixedInterval && bSynchronized)
        {
            params.m_sendPattern = DataCenterApp::RANDOM_INTERVAL;
            params.m_maxSendInterval = MicroSeconds(nMaxinterval);
            params.m_minSendInterval = MicroSeconds(nMininterval);
        }
        else
        {
            params.m_sendPattern = DataCenterApp::RANDOM_SPORADIC;
            params.m_maxSendInterval = MicroSeconds(nMaxinterval);
            params.m_minSendInterval = MicroSeconds(nMininterval);
        }
        params.m_packetSize = nPacketSize;
        params.m_nIterations = nIterations; 
        // std::cout << "Number of receivers: " <<  params.m_nReceivers<< std::endl;
        // std::cout << "Number of nodes in set: " <<  params.m_nodes.size() << std::endl;
        Ptr<DataCenterApp> app = CreateObject<DataCenterApp>();
        bool ret = app->Setup(params, *it, network_stack_type, DEBUG);
        if (!ret){
            std::cout << "Setup senders failed" << std::endl;
            exit(1);
        }
        topology->GetNode(*it)->AddApplication(app);

        app->SetStartTime (Seconds(0.));
        app->SetStopTime (Seconds(100000.));
    }
    for (std::unordered_set<int>::iterator it = nonsenderSet.begin(); it != nonsenderSet.end(); it++){
        DataCenterApp::SendParams params;
        params.m_sending = false;
        Ptr<DataCenterApp> app = CreateObject<DataCenterApp>();
        bool ret = app->Setup(params, *it, network_stack_type, DEBUG); 
        if (!ret){
            std::cout << "Setup receivers failed" << std::endl;
            exit(1);
        }
        topology->GetNode(*it)->AddApplication(app);
        app->SetStartTime (Seconds(0.));
        app->SetStopTime (Seconds(100000.));
    }


    //Config::SetDefault ("ns3::ArpCache::PendingQueueSize", UintegerValue (MAX_BURST_SIZE/L2MTU*3));
    if (network_stack_type == DataCenterApp::UDP_IP_STACK || network_stack_type == DataCenterApp::TCP_IP_STACK){
        std::cout << "Populating routing table\n";
        Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    }

    std::cout << "Running simulation\n";
    Simulator::Run ();
    Simulator::Destroy ();

    std::cout << "Simulation finished\n";

    return 0;
}
