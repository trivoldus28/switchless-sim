/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_INTERFACE_CONTAINER_H
#define DIM_ORDERED_INTERFACE_CONTAINER_H

// C/C++ includes
#include <vector>

// NS3 includes
#include "ns3/names.h"

// Switchless includes
#include "ns3/dim-ordered.h"

namespace ns3 {

/**
 * \brief Holds a vector of std::pair of Ptr<DimensionOrdered> and InterfaceDirection
 */
class DimensionOrderedInterfaceContainer
{
public:
  typedef std::vector<std::pair<Ptr<DimensionOrdered>, DimensionOrdered::InterfaceDirection> >::const_iterator Iterator;

  /**
   * Create an empty DimensionOrderedInterfaceContainer
   */
  DimensionOrderedInterfaceContainer ();

  /**
   * Concatenates the entries in other container with ours.
   * \param other container
   */
  void Add (DimensionOrderedInterfaceContainer other);

  /**
   * \brief Get an iterator which refers to the first pair in the container
   *
   * Pairs can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   *
   * \returns an interator which refers to the first pair in teh container.
   */
  Iterator Begin (void) const;

  /**
   * \brief Get an iterator which indicates past-the-last Node in the container
   *
   * Nodes can be retrieved from the contaienr in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   *
   * \returns an iterator which indicates an ending condition for a loop
   */
  Iterator End (void) const;

  /**
   * \returns the number of pairs stored in this container
   */
  uint32_t GetN (void) const;

  /**
   * \param i index of pair in container
   * \returns the DimensionOrderedAddress of the pair with windex i.
   */
  DimensionOrderedAddress GetAddress (uint32_t i) const;

  /**
   * Manually add an entry to the container consisting of the individual parts
   * of an entry std::pair.
   *
   * \param dimensionOrdered pointer to DimensionOrdered object
   * \param dir interface direction of the DimensionOrderedInterface to add to the container
   */
  void Add (Ptr<DimensionOrdered> dimensionOrdered, DimensionOrdered::InterfaceDirection dir);

  /**
   * Manually add an entry to thje container consisting of a previously composed
   * entry std::pair
   *
   * \param pair the pair of a pointer to DimensionOrdered object and interface direction 
   * of the DimensionOrderedInterface to add to the container
   */
  void Add (std::pair<Ptr<DimensionOrdered>, DimensionOrdered::InterfaceDirection>  pair);

  /**
   * Manually add an entry to the container consisting of the individual parts
   * of an entry std::pair
   * 
   * \param dimensionOrderedName std::string referring to the saved name of a DimensionOrdered object
   * that has been previously named using the Object Name Service.
   * \param dir interface direction of the DimensionOrderedInterface to add to this container
   */
  void Add (std::string dimensionOrderedName, DimensionOrdered::InterfaceDirection);

  /**
   * Get the std::pair of an Ptr<DimensionOrdered> and interface stored at the location specified
   * by the direction.
   * 
   * \param i the index of the netry to retrieve.
   */
  std::pair<Ptr<DimensionOrdered>, DimensionOrdered::InterfaceDirection> Get (uint32_t i) const;

private:
  typedef std::vector<std::pair<Ptr<DimensionOrdered>,DimensionOrdered::InterfaceDirection> > InterfaceVector;
  InterfaceVector m_interfaces; 
};

} // namespace ns3

#endif /* DIM_ORDERED_INTERFACE_CONTAINER_H */

