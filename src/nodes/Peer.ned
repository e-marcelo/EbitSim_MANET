// EbitSim - Enhanced BitTorrent Simulation
// This program is under the terms of the Creative Commons
// Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
//
// You are free:
//
//    to Share - to copy, distribute and transmit the work
//    to Remix - to adapt the work
//
// Under the following conditions:
//
//    Attribution - You must attribute the work in the manner specified by the
//    author or licensor (but not in any way that suggests that they endorse you
//    or your use of the work).
//
//    Noncommercial - You may not use this work for commercial purposes.
//
//    Share Alike - If you alter, transform, or build upon this work, you may
//    distribute the resulting work only under the same or similar license to
//    this one.
//
// With the understanding that:
//
//    Waiver - Any of the above conditions can be waived if you get permission
//    from the copyright holder.
//
//    Public Domain - Where the work or any of its elements is in the public
//    domain under applicable law, that status is in no way affected by the
//    license.
//
//    Other Rights - In no way are any of the following rights affected by the
//    license:
//        - Your fair dealing or fair use rights, or other applicable copyright
//          exceptions and limitations;
//        - The author's moral rights;
//        - Rights other persons may have either in the work itself or in how
//          the work is used, such as publicity or privacy rights.
//
//    Notice - For any reuse or distribution, you must make clear to others the
//    license terms of this work. The best way to do this is with a link to this
//    web page. <http://creativecommons.org/licenses/by-nc-sa/3.0/>
//
// Author:
//     Pedro Manoel Fabiano Alves Evangelista <pevangelista@larc.usp.br>
//     Supervised by Prof Tereza Cristina M. B. Carvalho <carvalho@larc.usp.br>
//     Graduate Student at Escola Politecnica of University of Sao Paulo, Brazil
//
// Contributors:
//     Marcelo Carneiro do Amaral <mamaral@larc.usp.br>
//     Victor Souza <victor.souza@ericsson.com>
//
// Disclaimer:
//     This work is part of a Master Thesis developed by:
//        Pedro Evangelista, graduate student at
//        Laboratory of Computer Networks and Architecture
//        Escola Politecnica
//        University of Sao Paulo
//        Brazil
//     and supported by:
//        Innovation Center
//        Ericsson Telecomunicacoes S.A., Brazil.
//
// UNLESS OTHERWISE MUTUALLY AGREED TO BY THE PARTIES IN WRITING AND TO THE
// FULLEST EXTENT PERMITTED BY APPLICABLE LAW, LICENSOR OFFERS THE WORK AS-IS
// AND MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND CONCERNING THE WORK,
// EXPRESS, IMPLIED, STATUTORY OR OTHERWISE, INCLUDING, WITHOUT LIMITATION,
// WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
// NONINFRINGEMENT, OR THE ABSENCE OF LATENT OR OTHER DEFECTS, ACCURACY, OR THE
// PRESENCE OF ABSENCE OF ERRORS, WHETHER OR NOT DISCOVERABLE. SOME
// JURISDICTIONS DO NOT ALLOW THE EXCLUSION OF IMPLIED WARRANTIES, SO THIS
// EXCLUSION MAY NOT APPLY TO YOU.

package br.larc.usp.nodes;

import br.larc.usp.client.BitTorrentApp;
//import inet.mobility.models.Slaw;

//import inet.base.IHook;
//import inet.base.NotificationBoard;
////EAM :: import inet.linklayer.IWirelessNic;
//import inet.linklayer.ethernet.EthernetInterface;
////EAM :: import inet.linklayer.mfcore.WirelessMac;
//import inet.mobility.models.MassMobility;
//import inet.networklayer.common.InterfaceTable;
//import inet.networklayer.ipv4.RoutingTable;
//import inet.networklayer.manetrouting.AODVUU;
//import inet.nodes.inet.AdhocHost;
import inet.transport.tcp.TCP;
//import inet.util.TCPDump; //I.R.
import inet.nodes.inet.NetworkLayer;
//import ned.IdealChannel;
//
//
//import inet.applications.pingapp.PingApp;
////EAM :: import inet.applications.tcpapp.TCPApp;
////EAM :: import inet.applications.udpapp.UDPApp;
//import inet.applications.ITCPApp;
//import inet.applications.IUDPApp;
//import inet.base.NotificationBoard;
////EAM :: import inet.linklayer.ieee80211.Ieee80211NicAdhoc;
////EAM :: import inet.mobility.BasicMobility;
//import inet.networklayer.common.InterfaceTable;
//import inet.networklayer.ipv4.RoutingTable;
//import inet.nodes.inet.NetworkLayer;
//import inet.transport.tcp.TCP;
//import inet.transport.udp.UDP;
//
//
////EAM ->
//import inet.util.PcapRecorder;
//import inet.networklayer.ipv4.RoutingTable;
//import inet.networklayer.common.InterfaceTable;
//import inet.mobility.IMobility;
//import inet.linklayer.IWirelessNic;
//import inet.linklayer.IWiredNic;
//import inet.linklayer.IExternalNic;
//import inet.base.NotificationBoard;
import inet.networklayer.IManetRouting;

//Nuevo intento Ad-hoc
import inet.util.PcapRecorder;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.mobility.IMobility;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IWiredNic;
import inet.linklayer.IExternalNic;
import inet.base.NotificationBoard;


//StandardHost
import inet.applications.IPingApp;
import inet.applications.ISCTPApp;
import inet.applications.ITCPApp;
import inet.applications.IUDPApp;
import inet.transport.ISCTP;
import inet.transport.ITCP;
import inet.transport.IUDP;


//
// Peer module.
//
module Peer like PeerInterface
{
    parameters:
        @display("i=device/pocketpc_s");
        @display("bgb=,448");
        @node;
        @labels(node,wireless-node);
        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the router. by default no wireless
        int numPcapRecorders = default(0); // no of PcapRecorders.
        string mobilityType = default(numRadios > 0 ? "StationaryMobility" : "");
        string routingFile = default("");
        bool IPForward = default(true);
        bool forwardMulticast = default(false);
        int numTcpApps = default(0);  // no of TCP apps. Specify the app types in INI file with tcpApp[0..1].typename="TCPEchoApp" syntax
        int numUdpApps = default(0);  // no of UDP apps. Specify the app types in INI file with udpApp[0..1].typename="UDPVideoStreamCli" syntax
        int numSctpApps = default(0); // no of SCTP apps. Specify the app types in INI file with sctpApp[0..1].typename="SCTPServer" syntax
        int numPingApps = default(0);  // no of PING apps. Specify the app types in INI file with pingApp[0..1].typename="PingApp" syntax
        bool hasTcp = default(numTcpApps>0);
        bool hasUdp = default(numUdpApps>0);
        bool hasSctp = default(numSctpApps>0);
        string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailable("UDP","UDP_None"));
        string sctpType = default(firstAvailable("SCTP","SCTP_None"));
        //EAM :: IPForward = default(false);  // disable routing by default
//        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");  // use adhoc management
        string routingProtocol @enum("AODVUU","DYMOUM","DYMO","DSRUU","OLSR","OLSR_ETX","DSDV_2","Batman") = default("AODVUU");  // used mobile routing protocol. see: inet.networklayer.manetrouting
//        IPForward = default(true);
		
    gates:
        input radioIn @directIn;

    submodules:
//        slaw: Slaw {
//            @display("p=48,264;i=block/cogwheel_s");
//        }
//        //StandardHost
//        tcpApp[numTcpApps]: <> like ITCPApp {
//            parameters:
//                @display("p=147,54,row,60");
//        }
//        tcp: <tcpType> like ITCP if hasTcp {
//            parameters:
//                @display("p=147,141");
//        }
//        udpApp[numUdpApps]: <> like IUDPApp {
//            parameters:
//                @display("p=329,54,row,60");
//        }
//        udp: <udpType> like IUDP if hasUdp {
//            parameters:
//                @display("p=329,141");
//        }
//        sctpApp[numSctpApps]: <> like ISCTPApp {
//            parameters:
//                @display("p=527,54,row,60");
//        }
//        sctp: <sctpType> like ISCTP if hasSctp {
//            @display("p=527,141");
//        }
//        pingApp[numPingApps]: <default("PingApp")> like IPingApp {
//            parameters:
//                @display("p=635,141,row,60");
//        }


        //Basic Node
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=53,155;is=s");
        }

        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if mobilityType != "" {
            parameters:
                @display("p=53,111");
        }


        routingTable: RoutingTable {
            parameters:
                @display("p=53,225;is=s");
                IPForward = IPForward;
                forwardMulticast = forwardMulticast;
                routingFile = routingFile;
        }

        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=53,287;is=s");
        }

        pcapRecorder[numPcapRecorders]: PcapRecorder {
            @display("p=53,338,r,10");
        }

        wlan: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=214,406,row,60;q=queue");
        }

        //    parameters: EbitSim [original]
        //        @node();
        //        //Parametro de mobilidad
        //        string mobilityType = default("StationaryMobility");
        //        bool IPForward = default(false);
        //        int namid = default(-1);
        //        string routingFile = default("");
        //        @labels(node,wireless-node);
        //        int numTcpApps = default(0);
        //        int numUdpApps = default(0);
        //        string tcpAppType = default("");
        //        string udpAppType = default("");
        //        @display("i=device/pocketpc_s");
        //    gates:
        //        input radioIn @directIn;
        //        //input ethg;
        //    submodules:
        //        mobility: <mobilityType> like IMobility {
        //            parameters:
        //                @display("p=132,230");
        //        }
        //        notificationBoard: NotificationBoard {
        //            parameters:
        //                @display("p=60,70");
        //        }
        //        interfaceTable: InterfaceTable {
        //            parameters:
        //                @display("p=60,150");
        //        }
        //        routingTable: RoutingTable {
        //            parameters:
        //                IPForward = IPForward;
        //                routerId = "";
        //                routingFile = routingFile;
        //                @display("p=60,230");
        //        }
        bitTorrentApp: BitTorrentApp {
            parameters:
                @display("p=237,39;i=abstract/penguin");
        }
        tcp: TCP {
            parameters:
                @display("p=163,154");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false;
                @display("p=248,247;q=queue");
            gates:
                ifIn[1];
                ifOut[1];
        }
        manetrouting: <routingProtocol> like IManetRouting if routingProtocol != "" {
            @display("p=522,307");
        }
    //
    //        wlan: <default("Ieee80211Nic")> like IWirelessNic {
    //            parameters:
    //                @display("p=248,349;q=queue");
    //        }
    connections allowunconnected:
        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;
        //
        //        // connections to network outside
        radioIn --> wlan.radioIn;
        //
        //        //EAM :: Cambio ++ --> upperLayerIn
        wlan.upperLayerOut --> networkLayer.ifIn++;
        wlan.upperLayerIn <-- networkLayer.ifOut++;
        //        // Connect application to TCP layer

        //
        //        //StandardHost
        //        for i=0..numTcpApps-1 {
        //            tcpApp[i].tcpOut --> tcp.appIn++;
        //            tcpApp[i].tcpIn <-- tcp.appOut++;
        //        }
        //El orden importa en las conexiones!!!
        bitTorrentApp.tcpOut[0] --> tcp.appIn++;
        bitTorrentApp.tcpOut[1] --> tcp.appIn++;
        bitTorrentApp.tcpIn[0] <-- tcp.appOut++;
        bitTorrentApp.tcpIn[1] <-- tcp.appOut++;
        
        networkLayer.manetOut --> manetrouting.from_ip if routingProtocol != "";
        networkLayer.manetIn <-- manetrouting.to_ip if routingProtocol != "";
//
//        tcp.ipOut --> networkLayer.tcpIn if hasTcp;
//        tcp.ipIn <-- networkLayer.tcpOut if hasTcp;
//
//        for i=0..numUdpApps-1 {
//            udpApp[i].udpOut --> udp.appIn++;
//            udpApp[i].udpIn <-- udp.appOut++;
//        }
//
//        udp.ipOut --> networkLayer.udpIn if hasUdp;
//        udp.ipIn <-- networkLayer.udpOut if hasUdp;
//
//        for i=0..numSctpApps-1 {
//            sctpApp[i].sctpOut --> sctp.from_appl++;
//            sctp.to_appl++ --> sctpApp[i].sctpIn;
//        }
//        sctp.to_ip --> networkLayer.sctpIn if hasSctp;
//        networkLayer.sctpOut --> sctp.from_ip if hasSctp;
//
//        for i=0..numPingApps-1 {
//            networkLayer.pingOut++ --> pingApp[i].pingIn;
//            networkLayer.pingIn++ <-- pingApp[i].pingOut;
//        }

		//Basic Node
   		// connections to network outside



}
