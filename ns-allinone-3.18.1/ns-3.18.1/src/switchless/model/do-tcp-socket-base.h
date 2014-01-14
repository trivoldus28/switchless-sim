/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DO_TCP_SOCKET_BASE_H
#define DO_TCP_SOCKET_BASE_H

#include <stdint.h>
#include <queue>
#include "ns3/callback.h"
#include "ns3/traced-value.h"
#include "ns3/do-tcp-socket.h"
#include "ns3/ptr.h"
#include "ns3/dim-ordered-address.h"
#include "ns3/dim-ordered-header.h"
#include "ns3/dim-ordered-interface.h"
#include "ns3/event-id.h"
#include "ns3/do-tcp-tx-buffer.h"
#include "ns3/do-tcp-rx-buffer.h"
#include "ns3/rtt-estimator.h"

namespace ns3 {

class DimensionOrderedEndPoint;
class Node;
class Packet;
class DoTcpL4Protocol;
class DoTcpHeader;

/**
 * \ingroup socket
 * \ingroup tcp
 *
 * \brief A base class for implementation of a stream socket using TCP in DimensionOrdered stack.
 *
 * This class contains the essential components of TCP, as well as a sockets
 * interface for upper layers to call. This serves as a base for other TCP
 * functions where the sliding window mechanism is handled here. This class
 * provides connection orientation and sliding window flow control. Part of
 * this class is modified from the original NS-3 TCP socket implementation
 * (DoTcpSocketImpl) by Raj Bhattacharjea <raj.b@gatech.edu> of Georgia Tech.
 */
class DoTcpSocketBase : public DoTcpSocket
{
public:
  static TypeId GetTypeId (void);
  /**
   * Create an unbound TCP socket
   */
  DoTcpSocketBase (void);

  /**
   * Clone a TCP socket, for use upon receiving a connection request in LISTEN state
   */
  DoTcpSocketBase (const DoTcpSocketBase& sock);
  virtual ~DoTcpSocketBase (void);

  // Set associated Node, DoTcpL4Protocol, RttEstimator to this socket
  virtual void SetNode (Ptr<Node> node);
  virtual void SetTcp (Ptr<DoTcpL4Protocol> tcp);
  virtual void SetRtt (Ptr<RttEstimator> rtt);

  // Necessary implementations of null functions from ns3::Socket
  virtual enum SocketErrno GetErrno (void) const;    // returns m_errno
  virtual enum SocketType GetSocketType (void) const; // returns socket type
  virtual Ptr<Node> GetNode (void) const;            // returns m_node
  virtual int Bind (void);    // Bind a socket by setting up endpoint in DoTcpL4Protocol
  virtual int Bind6 (void);    // Bind a socket by setting up endpoint in DoTcpL4Protocol
  virtual int Bind (const Address &address);         // ... endpoint of specific addr or port
  virtual int Connect (const Address &address);      // Setup endpoint and call ProcessAction() to connect
  virtual int Listen (void);  // Verify the socket is in a correct state and call ProcessAction() to listen
  virtual int Close (void);   // Close by app: Kill socket upon tx buffer emptied
  virtual int ShutdownSend (void);    // Assert the m_shutdownSend flag to prevent send to network
  virtual int ShutdownRecv (void);    // Assert the m_shutdownRecv flag to prevent forward to app
  virtual int Send (Ptr<Packet> p, uint32_t flags);  // Call by app to send data to network
  virtual int SendTo (Ptr<Packet> p, uint32_t flags, const Address &toAddress); // Same as Send(), toAddress is insignificant
  virtual Ptr<Packet> Recv (uint32_t maxSize, uint32_t flags); // Return a packet to be forwarded to app
  virtual Ptr<Packet> RecvFrom (uint32_t maxSize, uint32_t flags, Address &fromAddress); // ... and write the remote address at fromAddress
  virtual uint32_t GetTxAvailable (void) const; // Available Tx buffer size
  virtual uint32_t GetRxAvailable (void) const; // Available-to-read data size, i.e. value of m_rxAvailable
  virtual int GetSockName (Address &address) const; // Return local addr:port in address
  virtual void BindToNetDevice (Ptr<NetDevice> netdevice); // NetDevice with my m_endPoint

protected:
  // Implementing ns3::DoTcpSocket -- Attribute get/set
  virtual void     SetSndBufSize (uint32_t size);
  virtual uint32_t GetSndBufSize (void) const;
  virtual void     SetRcvBufSize (uint32_t size);
  virtual uint32_t GetRcvBufSize (void) const;
  virtual void     SetSegSize (uint32_t size);
  virtual uint32_t GetSegSize (void) const;
  virtual void     SetSSThresh (uint32_t threshold) = 0;
  virtual uint32_t GetSSThresh (void) const = 0;
  virtual void     SetInitialCwnd (uint32_t cwnd) = 0;
  virtual uint32_t GetInitialCwnd (void) const = 0;
  virtual void     SetConnTimeout (Time timeout);
  virtual Time     GetConnTimeout (void) const;
  virtual void     SetConnCount (uint32_t count);
  virtual uint32_t GetConnCount (void) const;
  virtual void     SetDelAckTimeout (Time timeout);
  virtual Time     GetDelAckTimeout (void) const;
  virtual void     SetDelAckMaxCount (uint32_t count);
  virtual uint32_t GetDelAckMaxCount (void) const;
  virtual void     SetTcpNoDelay (bool noDelay);
  virtual bool     GetTcpNoDelay (void) const;
  virtual void     SetPersistTimeout (Time timeout);
  virtual Time     GetPersistTimeout (void) const;
  virtual bool     SetAllowBroadcast (bool allowBroadcast);
  virtual bool     GetAllowBroadcast (void) const;

  // Helper functions: Connection set up
  int SetupCallback (void);        // Common part of the two Bind(), i.e. set callback and remembering local addr:port
  int DoConnect (void);            // Sending a SYN packet to make a connection if the state allows
  void ConnectionSucceeded (void); // Schedule-friendly wrapper for Socket::NotifyConnectionSucceeded()
  int SetupEndpoint (void);        // Configure m_endpoint for local addr for given remote addr
  void CompleteFork (Ptr<Packet>, const DoTcpHeader&, const Address& fromAddress, const Address& toAdress);

  // Helper functions: Transfer operation
  void ForwardUp (Ptr<Packet> packet, DimensionOrderedHeader header, uint16_t port, Ptr<DimensionOrderedInterface> incomingInterface);
  virtual void DoForwardUp (Ptr<Packet> packet, DimensionOrderedHeader header, uint16_t port, Ptr<DimensionOrderedInterface> incomingInterface); //Get a pkt from L3
  bool SendPendingData (bool withAck = false); // Send as much as the window allows
  uint32_t SendDataPacket (SequenceNumber32 seq, uint32_t maxSize, bool withAck); // Send a data packet
  void SendEmptyPacket (uint8_t flags); // Send a empty packet that carries a flag, e.g. ACK
  void SendRST (void); // Send reset and tear down this socket
  bool OutOfRange (SequenceNumber32 head, SequenceNumber32 tail) const; // Check if a sequence number range is within the rx window

  // Helper functions: Connection close
  int DoClose (void); // Close a socket by sending RST, FIN, or FIN+ACK, depend on the current state
  void CloseAndNotify (void); // To CLOSED state, notify upper layer, and deallocate end point
  void Destroy (void); // Kill this socket by zeroing its attributes
  void DeallocateEndPoint (void); // Deallocate m_endPoint
  void PeerClose (Ptr<Packet>, const DoTcpHeader&); // Received a FIN from peer, notify rx buffer
  void DoPeerClose (void); // FIN is in sequence, notify app and respond with a FIN
  void CancelAllTimers (void); // Cancel all timer when endpoint is deleted
  void TimeWait (void);  // Move from CLOSING or FIN_WAIT_2 to TIME_WAIT state

  // State transition functions
  void ProcessEstablished (Ptr<Packet>, const DoTcpHeader&); // Received a packet upon ESTABLISHED state
  void ProcessListen (Ptr<Packet>, const DoTcpHeader&, const Address&, const Address&); // Process the newly received ACK
  void ProcessSynSent (Ptr<Packet>, const DoTcpHeader&); // Received a packet upon SYN_SENT
  void ProcessSynRcvd (Ptr<Packet>, const DoTcpHeader&, const Address&, const Address&); // Received a packet upon SYN_RCVD
  void ProcessWait (Ptr<Packet>, const DoTcpHeader&); // Received a packet upon CLOSE_WAIT, FIN_WAIT_1, FIN_WAIT_2
  void ProcessClosing (Ptr<Packet>, const DoTcpHeader&); // Received a packet upon CLOSING
  void ProcessLastAck (Ptr<Packet>, const DoTcpHeader&); // Received a packet upon LAST_ACK

  // Window management
  virtual uint32_t UnAckDataCount (void);       // Return count of number of unacked bytes
  virtual uint32_t BytesInFlight (void);        // Return total bytes in flight
  virtual uint32_t Window (void);               // Return the max possible number of unacked bytes
  virtual uint32_t AvailableWindow (void);      // Return unfilled portion of window
  virtual uint16_t AdvertisedWindowSize (void); // The amount of Rx window announced to the peer

  // Manage data tx/rx
  virtual Ptr<DoTcpSocketBase> Fork (void) = 0; // Call CopyObject<> to clone me
  virtual void ReceivedAck (Ptr<Packet>, const DoTcpHeader&); // Received an ACK packet
  virtual void ReceivedData (Ptr<Packet>, const DoTcpHeader&); // Recv of a data, put into buffer, call L7 to get it if necessary
  virtual void EstimateRtt (const DoTcpHeader&); // RTT accounting
  virtual void NewAck (SequenceNumber32 const& seq); // Update buffers w.r.t. ACK
  virtual void DupAck (const DoTcpHeader& t, uint32_t count) = 0; // Received dupack
  virtual void ReTxTimeout (void); // Call Retransmit() upon RTO event
  virtual void Retransmit (void); // Halving cwnd and call DoRetransmit()
  virtual void DelAckTimeout (void);  // Action upon delay ACK timeout, i.e. send an ACK
  virtual void LastAckTimeout (void); // Timeout at LAST_ACK, close the connection
  virtual void PersistTimeout (void); // Send 1 byte probe to get an updated window size
  virtual void DoRetransmit (void); // Retransmit the oldest packet
  virtual void ReadOptions (const DoTcpHeader&); // Read option from incoming packets
  virtual void AddOptions (DoTcpHeader&); // Add option to outgoing packets

protected:
  // Counters and events
  EventId           m_retxEvent;       //< Retransmission event
  EventId           m_lastAckEvent;    //< Last ACK timeout event
  EventId           m_delAckEvent;     //< Delayed ACK timeout event
  EventId           m_persistEvent;    //< Persist event: Send 1 byte to probe for a non-zero Rx window
  EventId           m_timewaitEvent;   //< TIME_WAIT expiration event: Move this socket to CLOSED state
  uint32_t          m_dupAckCount;     //< Dupack counter
  uint32_t          m_delAckCount;     //< Delayed ACK counter
  uint32_t          m_delAckMaxCount;  //< Number of packet to fire an ACK before delay timeout
  bool              m_noDelay;         //< Set to true to disable Nagle's algorithm
  uint32_t          m_cnCount;         //< Count of remaining connection retries
  uint32_t          m_cnRetries;       //< Number of connection retries before giving up
  TracedValue<Time> m_rto;             //< Retransmit timeout
  TracedValue<Time> m_lastRtt;         //< Last RTT sample collected
  Time              m_delAckTimeout;   //< Time to delay an ACK
  Time              m_persistTimeout;  //< Time between sending 1-byte probes
  Time              m_cnTimeout;       //< Timeout for connection retry

  // Connections to other layers of TCP/IP
  DimensionOrderedEndPoint*       m_endPoint;
  Ptr<Node>           m_node;
  Ptr<DoTcpL4Protocol>  m_tcp;

  // Round trip time estimation
  Ptr<RttEstimator> m_rtt;

  // Rx and Tx buffer management
  TracedValue<SequenceNumber32> m_nextTxSequence; //< Next seqnum to be sent (SND.NXT), ReTx pushes it back
  TracedValue<SequenceNumber32> m_highTxMark;     //< Highest seqno ever sent, regardless of ReTx
  DoTcpRxBuffer                   m_rxBuffer;       //< Rx buffer (reordering buffer)
  DoTcpTxBuffer                   m_txBuffer;       //< Tx buffer

  // State-related attributes
  TracedValue<TcpStates_t> m_state;         //< TCP state
  enum SocketErrno         m_errno;         //< Socket error code
  bool                     m_closeNotified; //< Told app to close socket
  bool                     m_closeOnEmpty;  //< Close socket upon tx buffer emptied
  bool                     m_shutdownSend;  //< Send no longer allowed
  bool                     m_shutdownRecv;  //< Receive no longer allowed
  bool                     m_connected;     //< Connection established
  double                   m_msl;           //< Max segment lifetime

  // Window management
  uint32_t              m_segmentSize; //< Segment size
  uint16_t              m_maxWinSize;  //< Maximum window size to advertise
  TracedValue<uint32_t> m_rWnd;        //< Flow control window at remote side
};

} // namespace ns3

#endif /* DO_TCP_SOCKET_BASE_H */
