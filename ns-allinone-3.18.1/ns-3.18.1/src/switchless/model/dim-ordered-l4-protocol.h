/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_L4_PROTOCOL_H
#define DIM_ORDERED_L4_PROTOCOL_H

// C/C++ includes

// NS3 includes
#include "ns3/object.h"
#include "ns3/uinteger.h"

// Switchless includes
#include "ns3/dim-ordered-interface.h"
#include "ns3/dim-ordered-header.h"

namespace ns3 {

/**
 * \brief L4 Protocol abstract base class
 * 
 * This is an abstract base class for layer four protocols which use 
 * DimensionOrdered as the network layer.
 */
class DimensionOrderedL4Protocol : public Object
{
public:
  enum RxStatus {
      RX_OK,
      RX_CSUM_FAILED,
      RX_ENDPOINT_CLOSED,
      RX_ENDPOINT_UNREACH
  };

  static TypeId GetTypeId (void);

  virtual ~DimensionOrderedL4Protocol ();

  /**
   * \returns the protocol number of this protocol.
   */
  virtual int GetProtocolNumber (void) const = 0;

  /**
   * \param p packet to forward up
   * \param header DimensionOrderedHeader information
   * \param incomingInterface the DimensionOrderedInterface on which the packet arrived
   *
   * Called from lower-level layers to send the packet up in the stack.
   */
  virtual enum RxStatus Receive (Ptr<Packet> p,
                                 DimensionOrderedHeader const &header,
                                 Ptr<DimensionOrderedInterface> incomingInterface) = 0;
                                
  typedef Callback<void,Ptr<Packet>, DimensionOrderedAddress, DimensionOrderedAddress, uint8_t> DownTargetCallback;
  /**
   * This method allows a caller to set the current down target callback
   * set for this L4 protocol
   *
   * \param cb current Callback for the L4 protocol
   */
   virtual void SetDownTarget (DownTargetCallback cb) = 0;

   /**
    * This method allows a caller to get the current down target callback
    * set for this L4 protocol, for
    * 
    * \return current Callback for the L4 protocol
    */
   virtual DownTargetCallback GetDownTarget (void) const = 0;
};

}  // namespace ns3

#endif /* DIM_ORDERED_L4_PROTOCOL_H */

