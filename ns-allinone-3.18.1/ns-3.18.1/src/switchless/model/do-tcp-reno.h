/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_TCP_RENO_H
#define DO_TCP_RENO_H

#include "do-tcp-socket-base.h"

namespace ns3 {

/**
 * \ingroup socket
 * \ingroup tcp
 *
 * \brief An implementation of a stream socket using TCP.
 *
 * This class contains the Reno implementation of TCP, according to \RFC{2581},
 * except sec.4.1 "re-starting idle connections", which we do not detect for
 * idleness and thus no slow start upon resumption.
 */
class DoTcpReno : public DoTcpSocketBase
{
public:
  static TypeId GetTypeId (void);
  /**
   * Create an unbound tcp socket.
   */
  DoTcpReno (void);
  DoTcpReno (const DoTcpReno& sock);
  virtual ~DoTcpReno (void);

  // From DoTcpSocketBase
  virtual int Connect (const Address &address);
  virtual int Listen (void);

protected:
  virtual uint32_t Window (void); // Return the max possible number of unacked bytes
  virtual Ptr<DoTcpSocketBase> Fork (void); // Call CopyObject<DoTcpReno> to clone me
  virtual void NewAck (const SequenceNumber32& seq); // Inc cwnd and call NewAck() of parent
  virtual void DupAck (const DoTcpHeader& t, uint32_t count);  // Fast retransmit
  virtual void Retransmit (void); // Retransmit timeout

  // Implementing ns3::DoTcpSocket -- Attribute get/set
  virtual void     SetSegSize (uint32_t size);
  virtual void     SetSSThresh (uint32_t threshold);
  virtual uint32_t GetSSThresh (void) const;
  virtual void     SetInitialCwnd (uint32_t cwnd);
  virtual uint32_t GetInitialCwnd (void) const;
private:
  void InitializeCwnd (void);            // set m_cWnd when connection starts

protected:
  TracedValue<uint32_t>  m_cWnd;         //< Congestion window
  uint32_t               m_ssThresh;     //< Slow Start Threshold
  uint32_t               m_initialCWnd;  //< Initial cWnd value
  uint32_t               m_retxThresh;   //< Fast Retransmit threshold
  bool                   m_inFastRec;    //< currently in fast recovery
};

} // namespace ns3

#endif /* DO_TCP_RENO_H */
