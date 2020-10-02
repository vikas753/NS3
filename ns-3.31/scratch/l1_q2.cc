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
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// Default Network Topology
//
//               192.168.3.0
// n0   n1   n2 -------------- n3   n4   n5
//  |    |   |  point-to-point  |   |    | 
//  ==========                  ===========
// LAN 192.168.1.0              LAN 192.168.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 2;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  nCsma = nCsma == 0 ? 1 : nCsma;

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  NodeContainer csmaNodes1, csmaNodes2;
  csmaNodes1.Add (p2pNodes.Get (0));
  csmaNodes2.Add (p2pNodes.Get (1));
  csmaNodes1.Create (nCsma);
  csmaNodes2.Create (nCsma);
        
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (10000)));

  NetDeviceContainer csmaDevices1, csmaDevices2;
  csmaDevices1 = csma.Install (csmaNodes1);
  csmaDevices2 = csma.Install (csmaNodes2);

  InternetStackHelper stack;
  //stack.Install (p2pNodes);
  stack.Install (csmaNodes1);
  stack.Install (csmaNodes2);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.3.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("192.168.1.0", "255.255.255.0");
  
  Ipv4InterfaceContainer csmaInterfaces1, csmaInterfaces2;
  csmaInterfaces1 = address.Assign (csmaDevices1);

  address.SetBase ("192.168.2.0", "255.255.255.0");
  csmaInterfaces2 = address.Assign (csmaDevices2);
   
  UdpEchoServerHelper echoServer (21);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes1.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaInterfaces1.GetAddress (1), 21);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (2));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (3.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (csmaNodes2.Get(2));
  clientApps.Start (Seconds (4.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //pointToPoint.EnablePcapAll ("second");
  csma.EnablePcap ("node2", csmaDevices1.Get (2), true);
   //csma.EnablePcap ("second", csmaDevices1.Get (0), true);
    //csma.EnablePcap ("second", csmaDevices1.Get (1), true);
  csma.EnablePcap ("node4", csmaDevices2.Get (1), true);
  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
