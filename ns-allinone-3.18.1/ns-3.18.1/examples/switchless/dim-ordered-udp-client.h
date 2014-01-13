/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DIM_ORDERED_UDP_CLIENT_H
#define DIM_ORDERED_UDP_CLIENT_H

// C/C++ includes

// NS3 includes
#include "ns3/applications-module.h"

// Switchless Includes
#include "ns3/switchless-module.h"

using namespace ns3;

class DimensionOrderedUdpClient : public Application
{
public:
  // Constructor/Destructor
  DimensionOrderedUdpClient ();
  virtual ~DimensionOrderedUdpClient ();

  bool Setup (const Address &serverAddress);
private:
  static const uint16_t PORT = 8080;
  static const uint16_t PACKET_SIZE = 8;

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void SendPacket (void);

  void HandleConnectionSucceeded (Ptr<Socket> socket);
  void HandleConnectionFailed (Ptr<Socket> socket);
  void HandleRead (Ptr<Socket> socket);

  bool          m_setup;
  bool          m_running;
  Address       m_txAddress;
  Ptr<Socket>   m_txSocket;
};

#endif /* DIM_ORDERED_UDP_CLIENT_H */
