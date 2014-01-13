/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_STACK_HELPER_H
#define DIM_ORDERED_STACK_HELPER_H

// C/C++ includes
#include <string>
#include <tuple>

// NS3 includes
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/log.h"
#include "ns3/packet-socket-factory.h"
#include "ns3/names.h"

// Switchless includes
#include "ns3/dim-ordered.h"

namespace ns3 {

/**
 * \brief aggregate DimensionOrdered/TCP/UDP functionality to existing Nodes.
 *
 * This helper enables pcap and ascii tracing of events in the stack
 * associatted with a node.  This is substantially similar to the tracing
 * that happens in device hlpers, but the important difference is that, well,
 * there is no device.  This means that the creation of output file names will
 * change, and also the user-visibile methods will not reference devices
 * and therefore the nubmer of trace enable methods is reduced.
 *
 * This class aggregates instances of these objects, by default, to each node:
 *   - ns3::DimensionOrderedL3Protocol
 *   - ns3::DoUdpL4Protocol
 *   - a TCP based on the TCP factory provided
 *   - a DoPacketSocketFactory
 */
class DimensionOrderedStackHelper
{
public:
  /*
   * Create a new DimensionOrderedStackHelper
   */
  DimensionOrderedStackHelper (void);

  /*
   * Destroy the DimensionOrderedStackHelper
   */
  virtual ~DimensionOrderedStackHelper (void);
  DimensionOrderedStackHelper (const DimensionOrderedStackHelper &o);
  DimensionOrderedStackHelper &operator = (const DimensionOrderedStackHelper &o);

  /*
   * Return helper internal state to that of a new constructed one
   */
  void Reset (void);

  /**
   * Aggregate implementations of the ns3::Ipv4, ns3::DoUdp, and ns3::DoTcp classes
   * onto the provided node.  This method will assert if called on a node that
   * already has DimensionOrdered object aggregated to it.
   *
   * \param nodeName the name of the node on which to install the stack
   * \param origin The origin in the DimensionOrdered topology
   * \param dimsMax The max value of the DimensionOrdered topology dimension
   */
  void Install (std::string nodeName, std::tuple<uint8_t, uint8_t, uint8_t> origin,
                std::tuple<uint8_t, uint8_t, uint8_t> dimsMax) const;

  /**
   * Aggregate implementations of the ns3::Ipv4, ns3::DoUdp, and ns3::DoTcp classes
   * onto the provided node.  This method will assert if called on a node that
   * already has DimensionOrdered object aggregated to it.
   *
   * \param node the node on which to install the stack
   * \param origin The origin in the DimensionOrdered topology
   * \param dimsMax The max value of the DimensionOrdered topology dimension
   */
  void Install (Ptr<Node> node, std::tuple<uint8_t, uint8_t, uint8_t> origin,
                std::tuple<uint8_t, uint8_t, uint8_t> dimsMax) const;

  /**
   * For each node in the input container, aggregate implementations of the 
   * ns3::Ipv4, ns3::DoUdp, and ns3::DoTcp classes onto the provided node.  
   * This method will assert if called on a container with a node that
   * already has a DimensionOrdered object aggregated to it.
   *
   * \param c NodeContainer that holds the set of nodes on which to install the new stacks
   * \param origin The origin in the DimensionOrdered topology
   * \param dimsMax The max value of the DimensionOrdered topology dimension
   */
  void Install (NodeContainer c, std::tuple<uint8_t, uint8_t, uint8_t> origin,
                std::tuple<uint8_t, uint8_t, uint8_t> dimsMax) const;

  /**
   * Aggregate DimensionOrdered, DoUdp, and DoTCP stacks to all nodes in the simulation
   */
  void InstallAll (std::tuple<uint8_t, uint8_t, uint8_t> origin, 
                   std::tuple<uint8_t, uint8_t, uint8_t> dimsMax) const;

  /**
   * \brief Set the DoTcp stack which will not need any other parameter.
   *
   * This function sets up the tcp stack to the given TypeId. It should not be 
   * used for NSC stack setup because the nsc stack needs the Library attribute
   * to be setup, please use instead the version that requires an attribute
   * and a value. If you choose to use this function anyways to set nsc stack
   * the default value for the linux library will be used: "liblinux2.6.26.so".
   *
   * \param tid the type id, typically it is set to  "ns3::DoTcpL4Protocol"
   */
  void SetTcp (std::string tid); 

  /**
   * \brief This function is used to setup the Network Simulation Cradle stack with library value.
   * 
   * Give the NSC stack a shared library file name to use when creating the 
   * stack implementation.  The attr string is actually the attribute name to 
   * be setup and val is its value. The attribute is the stack implementation 
   * to be used and the value is the shared library name.
   * 
   * \param tid The type id, for the case of nsc it would be "ns3::DoNscTcpL4Protocol" 
   * \param attr The attribute name that must be setup, for example "Library"
   * \param val The attribute value, which will be in fact the shared library name (example:"liblinux2.6.26.so")
   */
  void SetTcp (std::string tid, std::string attr, const AttributeValue &val);

private:
  void Initialize (void);
  ObjectFactory m_tcpFactory;
  
  static void CreateAndAggregateObjectFromTypeId (Ptr<Node> node, const std::string typeId);
  
  static void Cleanup (void);
};

} // namespace ns3

#endif /* DIM_ORDERED_STACK_HELPER_H */
