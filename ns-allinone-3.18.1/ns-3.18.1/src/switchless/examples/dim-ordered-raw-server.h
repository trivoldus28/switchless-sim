/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_RAW_SERVER_H
#define DIM_ORDERED_RAW_SERVER_H

// C/C++ includes

// NS3 includes
#include "ns3/applications-module.h"

// Switchless Includes
#include "ns3/switchless-module.h"

using namespace ns3;

class DimensionOrderedRawServer : public Application
{
public:
  // Constructor/Destructor
  DimensionOrderedRawServer ();
  virtual ~DimensionOrderedRawServer ();

  bool Setup ();
private:
  static const uint16_t PORT = 8080;
  static const uint16_t PACKET_SIZE = 8;

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void SendResponsePacket (Ptr<Socket> socket, Address& to);

  bool HandleConnectionRequest (Ptr<Socket> socket, const Address& from);
  void HandleAccept (Ptr<Socket> socket, const Address& from);
  void HandleRead (Ptr<Socket> socket);
  void HandleClose (Ptr<Socket> socket);
  void HandleError (Ptr<Socket> socket);

  bool                      m_setup;
  bool                      m_running;
  Ptr<Socket>               m_rxSocket;
  std::vector<Ptr<Socket> > m_acceptSockets;
};

#endif /* DIM_ORDERED_RAW_SERVER_H */
