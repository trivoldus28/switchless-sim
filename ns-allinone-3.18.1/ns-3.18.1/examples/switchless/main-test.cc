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
#include <utility> // std::pair, std::make_pair

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

    LogComponentEnable ("DataCenterApp", LOG_DEBUG);
    unsigned nRackSize = 0;
    unsigned nTreeFanout = 0;
    bool bTorus = true; // not parsed
    std::string sSenderChoice = "random";  // not parsed
    std::string sReceiverChoice = "random"; // not parsed
    bool bFixedInterval = false;
    bool bRandomInterval = false;
    bool bSynchronized = false;
    unsigned meshNumRow = 0;
    unsigned meshNumCol = 0;

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
    int sChoice=0;
    int rChoice=0;
    CommandLine cmd;
    int debuglog = 0;
    cmd.AddValue("debug", "Debug", debuglog);
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
    cmd.AddValue("sChoice", "", sChoice); //0 for random
    cmd.AddValue("rChoice", "", rChoice); //0 for random
    cmd.AddValue("iter", "", nIterations);
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
    if(topologytype == MESH)
    {
        meshNumRow = topo_sub1;
        meshNumCol = topo_sub2;
    }
    else if (topologytype == TREE)
    {   
        nRackSize = topo_sub1;
        nTreeFanout = topo_sub2;
    }
    else if (topologytype == CUBE)
    {
        m_cube= topo_sub1;
        n_cube= topo_sub2;
    }
    else{
        std::cout << "Invalid topo\n";
        NS_ASSERT(false);
    }


    Config::SetDefault ("ns3::DropTailQueue::Mode", StringValue ("QUEUE_MODE_PACKETS"));
    Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue (500));
    // common variables
    PointToPointHelper pointToPoint;


    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Gbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("100ns"));

    std::cout << "Making topology\n";
    PointToPointTopoHelper * topology;
    // switch statements
    if (topologytype == TREE){
        NS_ASSERT(nNodes % nRackSize == 0);
        unsigned depth = log(nNodes / nRackSize) / log(nTreeFanout);
        NS_ASSERT(nNodes/nRackSize == pow(nTreeFanout, depth));
        uint64_t linkDataRate = 1000LL*1000*1000*10; // 1Gbps
        topology = new PointToPointFattreeHelper(nRackSize, nTreeFanout, depth, linkDataRate, pointToPoint);
    }
    else if (topologytype == MESH){
        NS_ASSERT(nNodes == (meshNumRow * meshNumCol));
        topology = new PointToPoint2DMeshHelper(meshNumRow, meshNumCol, bTorus, pointToPoint);
    }
    else if (topologytype == CUBE){
        NS_ASSERT(nNodes == pow(m_cube, n_cube));
        topology = new PointToPointNcubeHelper(m_cube, n_cube, bTorus, pointToPoint);
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
        std::vector <Ipv4Address> receiverNodeList;

        if (sReceiverChoice == "random"){
            for(int i=0;i<nNodes;i++)
            {
                if(i!=*it)
                {
                    Ipv4Address t = topology->GetIpv4Address(i);
                    receiverNodeList.push_back(t);
                }
            }

            params.m_nReceivers = nReceiver;
            params.m_receivers = DataCenterApp::RANDOM_SUBSET;
        }
        else{
             std::unordered_set<int> receiverSet;
             if (topologytype == TREE){
                int nodeid = *it;
                float logval = log2(nNeighbor); // 
                int logv = ceil(logval); //
                int rem1 = nodeid % (int)(pow(2,(logv-1))); //
                int rem2 = nodeid % (int)(pow(2,(logv))); //
                int base1 = nodeid- rem1;
                for (int i=0; i< pow(2,(logv-1)); i++)
                {
                    receiverSet.insert(base1+i);
                }
                int remainings = nNeighbor-pow(2,(logv-1));
                int base2;
                if(rem2> pow(2,(logv-1)))
                {
                    base2 = nodeid - rem2;
                }
                else
                {
                    base2 = nodeid - rem2 + pow(2,(logv-1));
                }
                while (remainings !=0)
                {
                    int randid = rand() % (int)(pow(2,(logv-1)));
                    receiverSet.insert(base2+randid);
                    remainings --;
                }
             }
             else if(topologytype == MESH){
                // Basically we will have a box to draw random, clustered nodes from
                // Box boundaries are 1,4,9,16,25,...
                // centered around this particular send node (*it)
                unsigned senderY = *it / meshNumCol;
                unsigned senderX = *it % meshNumCol;
                unsigned mindistance =1;
                int failSafeCounter = 0;
                while (receiverSet .size() < nNeighbor){
                    int randx = rand() % (mindistance*2+1);
                    int randy = rand() % (mindistance*2+1);
                    randx = randx-mindistance;
                    randy = randy-mindistance;
                    int distance = abs(randx)+abs(randy);
                    if(mindistance == distance)
                    {
                        int poty = randy + senderY;
                        int potx = randx + senderX;
                        NS_ASSERT(bTorus == true);
                        if (potx < 0)
                            potx += meshNumCol;
                        if (poty < 0)
                            poty += meshNumRow;
                        if (potx ==0 && poty == 0)
                        {
                            continue;
                        }
                        if (potx >= meshNumCol || poty >=meshNumRow) 
                        {
                            continue;
                        }
                        unsigned coord = poty* meshNumCol + potx;
                        receiverSet.insert(coord);


                    }
                    if(receiverSet.size() == 2*mindistance*(mindistance+1))
                    {
                        mindistance++;
                    }
                    failSafeCounter++;
                    if(failSafeCounter==10000)
                    {
                        mindistance++;
                    }
                }
             }
             else if(topologytype == CUBE){
                // NS_ASSERT(false);
                unsigned maxHammingDistance = 1;
                unsigned failSafeCounter = 0;
                std::unordered_set<uint64_t> recvCoordSet;
                while(recvCoordSet.size() < nNeighbor){
                    unsigned distance = 0;
                    uint64_t flatCoord = 0;
                    for (int i = 0; i < n_cube; i++){
                        int coord = rand() % (maxHammingDistance*2+1);
                        coord -= maxHammingDistance;
                        distance += abs(coord);
                        if (coord < 0) coord += m_cube; // implied torus
                        flatCoord += (pow(m_cube, i) * coord);
                    }
                    if (distance == 0) continue;
                    if (distance <= maxHammingDistance)
                        recvCoordSet.insert(flatCoord);
                    unsigned maxCapacity = (4*pow(maxHammingDistance,n_cube-1)) + 2*(n_cube-2);
                    if (recvCoordSet.size() >= maxCapacity){
                        maxHammingDistance++;
                        failSafeCounter = 0;
                    }
                    // std::cout << "D " << distance << std::endl;
                    // std::cout << "H " << maxHammingDistance << std::endl;
                    // std::cout << "S " << recvCoordSet.size() << std::endl;
                    // std::cout << "M " << maxCapacity << std::endl;
                    failSafeCounter++;
                    if (failSafeCounter > 10000){
                        maxHammingDistance++;
                        failSafeCounter = 0;
                    }
                }

                // get the sender coords
                std::vector<unsigned> senderCoords;
                uint64_t senderFlatCoord = *it;
                for (int i = 0; i < n_cube; i++){
                    unsigned coord = (senderFlatCoord / (unsigned)pow(m_cube,i)) % m_cube;
                    senderCoords.push_back(coord);
                }

                // re-center the receiver coordinates and append to list
                for (auto it = recvCoordSet.begin(); it != recvCoordSet.end(); it++){
                    uint64_t flatCoord = *it;
                    uint64_t adjustedFlatCoord = 0;
                    // std::vector<unsigned> coords;
                    for (int i = 0; i < n_cube; i++){
                        unsigned coord = (flatCoord / (unsigned)pow(m_cube,i)) % m_cube;
                        // coords.push_back(coord);
                        unsigned adjustedCoord = coord + senderCoords[i];
                        adjustedCoord %= m_cube;
                        adjustedFlatCoord += (pow(m_cube, i) * adjustedCoord);
                    }
                    receiverSet.insert(adjustedFlatCoord);
                }
             }

             for (auto it = receiverSet.begin(); it != receiverSet.end(); it++)
             {
                Ipv4Address t = topology->GetIpv4Address(*it);
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
        Ptr<DataCenterApp> app = CreateObject<DataCenterApp>();
        app->Setup(params, *it, DEBUG); 
        topology->GetNode(*it)->AddApplication(app);

        app->SetStartTime (Seconds(0.));
        app->SetStopTime (Seconds(100000.));
    }
    for (std::unordered_set<int>::iterator it = nonsenderSet.begin(); it != nonsenderSet.end(); it++){
        DataCenterApp::SendParams params;
        Ptr<DataCenterApp> app = CreateObject<DataCenterApp>();
        app->Setup(params, *it, DEBUG); 
        topology->GetNode(*it)->AddApplication(app);
        app->SetStartTime (Seconds(0.));
        app->SetStopTime (Seconds(100000.));
    }



    std::cout << "Populating routing table\n";
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    std::cout << "Running simulation\n";
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
