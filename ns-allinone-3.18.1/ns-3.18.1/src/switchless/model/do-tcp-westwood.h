/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_TCP_WESTWOOD_H
#define DO_TCP_WESTWOOD_H

#include "do-tcp-socket-base.h"
#include "ns3/packet.h"

namespace ns3 {

/**
 * \ingroup socket
 * \ingroup tcp
 *
 * \brief An implementation of a stream socket using TCP.
 *
 * This class contains the implementation of TCP Westwood and Westwood+.
 *
 * Westwood and Westwood+ employ the AIAD (Additive Increase/Adaptive Decrease) 
 * congestion control paradigm. When a congestion episode happens, 
 * instead of halving the cwnd, these protocols try to estimate the network's
 * bandwidth and use the estimated value to adjust the cwnd. 
 * While Westwood performs the bandwidth sampling every ACK reception, 
 * Westwood+ samples the bandwidth every RTT.
 *
 * The two main methods in the implementation are the CountAck (const TCPHeader&)
 * and the EstimateBW (int, const, Time). The CountAck method calculates
 * the number of acknowledged segments on the receipt of an ACK.
 * The EstimateBW estimates the bandwidth based on the value returned by CountAck
 * and the sampling interval (last ACK inter-arrival time for Westwood and last RTT for Westwood+).   
 */
class DoTcpWestwood : public DoTcpSocketBase
{
public:
  static TypeId GetTypeId (void);

  DoTcpWestwood (void);
  DoTcpWestwood (const DoTcpWestwood& sock);
  virtual ~DoTcpWestwood (void);

  enum ProtocolType 
  {
    WESTWOOD,
    WESTWOODPLUS
  };

  enum FilterType 
  {
    NONE,
    TUSTIN
  };

  // From DoTcpSocketBase
  virtual int Connect (const Address &address);
  virtual int Listen (void);

protected:
  /**
   * Limit the size of outstanding data based on the cwnd and the receiver's advertised window
   *
   * \return the max. possible number of unacked bytes
   */  
  virtual uint32_t Window (void);

  /**
   * Call CopyObject<DoTcpWestwood> to clone me
   */  
  virtual Ptr<DoTcpSocketBase> Fork (void);

  /**
   * Process the newly received ACK
   *
   * \param packet the received ACK packet
   * \param tcpHeader the header attached to the ACK packet
   */  
  virtual void ReceivedAck (Ptr<Packet> packet, const DoTcpHeader& tcpHeader);
  
  /**
   * Adjust the cwnd based on the current congestion control phase,
   * and then call the DoTcpSocketBase::NewAck() to complete the processing
   *
   * \param seq the acknowledgment number
   */  
  virtual void NewAck (SequenceNumber32 const& seq);

  /**
   * Adjust the cwnd using the currently estimated bandwidth,
   * retransmit the missing packet, and enter fast recovery if 3 DUPACKs are received
   *
   * \param header the TCP header of the ACK packet
   * \param count the number of DUPACKs
   */  
  virtual void DupAck (const DoTcpHeader& header, uint32_t count);

  /**
   * Upon an RTO event, adjust the cwnd using the currently estimated bandwidth,
   * retransmit the missing packet, and exit fast recovery
   */  
  virtual void Retransmit (void);

  /**
   * Estimate the RTT, record the minimum value,
   * and run a clock on the RTT to trigger Westwood+ bandwidth sampling
   */
  virtual void EstimateRtt (const DoTcpHeader& header);

  // Implementing ns3::DoTcpSocket -- Attribute get/set
  /**
   * \param size the segment size to be used in a connection
   */  
  virtual void SetSegSize (uint32_t size);

  /**
   * \param the slow-start threshold
   */  
  virtual void SetSSThresh (uint32_t threshold);

  /**
   * \return the slow-start threshold
   */  
  virtual uint32_t GetSSThresh (void) const;

  /**
   * \param cwnd the initial cwnd
   */  
  virtual void SetInitialCwnd (uint32_t cwnd);

  /**
   * \return the initial cwnd
   */ 
  virtual uint32_t GetInitialCwnd (void) const;

private:
  /**
   * Initialize cwnd at the beginning of a connection
   */
  void InitializeCwnd (void);

  /**
   * Calculate the number of acknowledged packets upon the receipt of an ACK packet
   *
   * \param tcpHeader the header of the received ACK packet
   * \return the number of ACKed packets
   */
  int CountAck (const DoTcpHeader& tcpHeader);

  /**
   * Update the total number of acknowledged packets during the current RTT
   *
   * \param acked the number of packets the currently received ACK acknowledges
   */
  void UpdateAckedSegments (int acked);

  /**
   * Estimate the network's bandwidth
   *
   * \param acked the number of acknowledged packets returned by CountAck
   * \param tcpHeader the header of the packet
   * \param rtt the RTT estimation
   */
  void EstimateBW (int acked, const DoTcpHeader& tcpHeader, Time rtt);

  /**
   * Tustin filter
   */
  void Filtering (void);

protected:
  TracedValue<uint32_t>  m_cWnd;                   //< Congestion window
  uint32_t               m_ssThresh;               //< Slow Start Threshold
  uint32_t               m_initialCWnd;            //< Initial cWnd value
  bool                   m_inFastRec;              //< Currently in fast recovery if TRUE

  TracedValue<double>    m_currentBW;              //< Current value of the estimated BW
  double                 m_lastSampleBW;           //< Last bandwidth sample
  double                 m_lastBW;                 //< Last bandwidth sample after being filtered
  Time                   m_minRtt;                 //< Minimum RTT
  double                 m_lastAck;                //< The time last ACK was received
  SequenceNumber32       m_prevAckNo;              //< Previously received ACK number
  int                    m_accountedFor;           //< The number of received DUPACKs
  enum ProtocolType      m_pType;                  //< 0 for Westwood, 1 for Westwood+
  enum FilterType        m_fType;                  //< 0 for none, 1 for Tustin

  int                    m_ackedSegments;          //< The number of segments ACKed between RTTs
  bool                   m_IsCount;                //< Start keeping track of m_ackedSegments for Westwood+ if TRUE
  EventId                m_bwEstimateEvent;        //< The BW estimation event for Westwood+

};

} // namespace ns3

#endif /* TCP_WESTWOOD_H */
