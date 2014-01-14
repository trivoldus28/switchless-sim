/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_TCP_RFC793_H
#define DO_TCP_RFC793_H

#include "do-tcp-socket-base.h"

namespace ns3 {

/**
 * \ingroup socket
 * \ingroup tcp
 *
 * \brief An implementation of a stream socket using TCP.
 *
 * This class contains an \RFC{793} implementation of TCP, as well as a sockets
 * interface for talking to TCP.  This serves as a base for other TCP functions
 * where the sliding window mechanism is handled here.  This class provides
 * connection orientation and sliding window flow control.
 */
class DoTcpRfc793 : public DoTcpSocketBase
{
public:
  static TypeId GetTypeId (void);
  /**
   * Create an unbound tcp socket.
   */
  DoTcpRfc793 (void);
  DoTcpRfc793 (const DoTcpRfc793& sock);
  virtual ~DoTcpRfc793 (void);

protected:
  virtual Ptr<DoTcpSocketBase> Fork (); // Call CopyObject<DoTcpRfc793> to clone me
  virtual void DupAck (const DoTcpHeader& t, uint32_t count);
  virtual void     SetSSThresh (uint32_t threshold);
  virtual uint32_t GetSSThresh (void) const;
  virtual void     SetInitialCwnd (uint32_t cwnd);
  virtual uint32_t GetInitialCwnd (void) const;
};

} // namespace ns3

#endif /* DO_TCP_RFC793_H */
