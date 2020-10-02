/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

// Default Network Topology
//
//       192.168.2.0
// n0 ----------------- n1
//      point-to-point
//
 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  // Declare nodes that would be linked to a point-point cable
  NodeContainer p2pnodes;
  p2pnodes.Create(2);
  
  
  PointToPointHelper pointToPoint;
  
  // Modify the data-rate to 10Mbps as required in the assignment
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // Install peripheral cards for point-point link defined above
  NetDeviceContainer device_nodes;
  device_nodes = pointToPoint.Install (p2pnodes);
  
  // Install default UDP network stack on p2p nodes 
  InternetStackHelper stack;
  stack.Install (p2pnodes);

  // Assign an IPv4 address block as below on those nodes
  Ipv4AddressHelper address;
  address.SetBase ("192.168.2.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (device_nodes);
  
  // Echo server listens on port 63
  UdpEchoServerHelper echoServer (63);

  ApplicationContainer serverApps = echoServer.Install (p2pnodes.Get(1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 63);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (256));

  ApplicationContainer clientApps = echoClient.Install (p2pnodes.Get(0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  
  pointToPoint.EnablePcapAll ("l1_q1");
  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
