/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_TCP_SOCKET_H
#define DO_TCP_SOCKET_H

#include "ns3/socket.h"
#include "ns3/traced-callback.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/tcp-socket.h"

namespace ns3 {

class Node;
class Packet;

// ALREADY DECLARED IN TCP SOCKET, JUST USE THAT
/* Names of the 11 TCP states */
/*typedef enum {
  CLOSED,       // 0
  LISTEN,       // 1
  SYN_SENT,     // 2
  SYN_RCVD,     // 3
  ESTABLISHED,  // 4
  CLOSE_WAIT,   // 5
  LAST_ACK,     // 6
  FIN_WAIT_1,   // 7
  FIN_WAIT_2,   // 8
  CLOSING,      // 9
  TIME_WAIT,   // 10
  LAST_STATE
} DoTcpStates_t;*/

/**
 * \ingroup socket
 *
 * \brief (abstract) base class of all DoTcpSockets
 *
 * This class exists solely for hosting DoTcpSocket attributes that can
 * be reused across different implementations.
 */
class DoTcpSocket : public Socket
{
public:
  static TypeId GetTypeId (void);
 
  DoTcpSocket (void);
  virtual ~DoTcpSocket (void);

  // Literal names of TCP states for use in log messages */
  static const char* const TcpStateName[LAST_STATE];

private:
  // Indirect the attribute setting and getting through private virtual methods
  virtual void SetSndBufSize (uint32_t size) = 0;
  virtual uint32_t GetSndBufSize (void) const = 0;
  virtual void SetRcvBufSize (uint32_t size) = 0;
  virtual uint32_t GetRcvBufSize (void) const = 0;
  virtual void SetSegSize (uint32_t size) = 0;
  virtual uint32_t GetSegSize (void) const = 0;
  virtual void SetSSThresh (uint32_t threshold) = 0;
  virtual uint32_t GetSSThresh (void) const = 0;
  virtual void SetInitialCwnd (uint32_t count) = 0;
  virtual uint32_t GetInitialCwnd (void) const = 0;
  virtual void SetConnTimeout (Time timeout) = 0;
  virtual Time GetConnTimeout (void) const = 0;
  virtual void SetConnCount (uint32_t count) = 0;
  virtual uint32_t GetConnCount (void) const = 0;
  virtual void SetDelAckTimeout (Time timeout) = 0;
  virtual Time GetDelAckTimeout (void) const = 0;
  virtual void SetDelAckMaxCount (uint32_t count) = 0;
  virtual uint32_t GetDelAckMaxCount (void) const = 0;
  virtual void SetTcpNoDelay (bool noDelay) = 0;
  virtual bool GetTcpNoDelay (void) const = 0;
  virtual void SetPersistTimeout (Time timeout) = 0;
  virtual Time GetPersistTimeout (void) const = 0;

};

} // namespace ns3

#endif /* TCP_SOCKET_H */


