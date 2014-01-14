/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_TCP_TAHOE_H
#define DO_TCP_TAHOE_H

#include "do-tcp-socket-base.h"

namespace ns3 {

/**
 * \ingroup socket
 * \ingroup tcp
 *
 * \brief An implementation of a stream socket using TCP.
 *
 * This class contains the Tahoe implementation of TCP. Tahoe is not officially
 * published in RFC. The reference for implementing this is based on
 * Kevin Fall and Sally Floyd, "Simulation-based Comparisons of Tahoe, Reno, and SACK TCP", CCR, 1996
 * http://inst.eecs.berkeley.edu/~ee122/fa05/projects/Project2/proj2_spec_files/sacks.pdf
 * In summary, we have slow start, congestion avoidance, and fast retransmit.
 * The implementation of these algorithms are based on W. R. Stevens's book and
 * also \RFC{2001}.
 */
class DoTcpTahoe : public DoTcpSocketBase
{
public:
  static TypeId GetTypeId (void);
  /**
   * Create an unbound tcp socket.
   */
  DoTcpTahoe (void);
  DoTcpTahoe (const DoTcpTahoe& sock);
  virtual ~DoTcpTahoe (void);

  // From DoTcpSocketBase
  virtual int Connect (const Address &address);
  virtual int Listen (void);

protected:
  virtual uint32_t Window (void); // Return the max possible number of unacked bytes
  virtual Ptr<DoTcpSocketBase> Fork (void); // Call CopyObject<DoTcpTahoe> to clone me
  virtual void NewAck (SequenceNumber32 const& seq); // Inc cwnd and call NewAck() of parent
  virtual void DupAck (const DoTcpHeader& t, uint32_t count);  // Treat 3 dupack as timeout
  virtual void Retransmit (void); // Retransmit time out

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
};

} // namespace ns3

#endif /* TCP_TAHOE_H */
