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

import inet.base.IHook;
import inet.base.NotificationBoard;
//EAM :: import inet.linklayer.IWirelessNic;
import inet.linklayer.ethernet.EthernetInterface;
//EAM :: import inet.linklayer.mfcore.WirelessMac;
import inet.mobility.models.MassMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.manetrouting.AODVUU;
import inet.nodes.inet.AdhocHost;
import inet.transport.tcp.TCP;
import inet.util.TCPDump; //I.R.
import inet.nodes.inet.NetworkLayer;
import ned.IdealChannel;


import inet.applications.pingapp.PingApp;
//EAM :: import inet.applications.tcpapp.TCPApp;
//EAM :: import inet.applications.udpapp.UDPApp;
import inet.applications.ITCPApp;
import inet.applications.IUDPApp;
import inet.base.NotificationBoard;
//EAM :: import inet.linklayer.ieee80211.Ieee80211NicAdhoc;
//EAM :: import inet.mobility.BasicMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.transport.tcp.TCP;
import inet.transport.udp.UDP;


//EAM ->
import inet.util.PcapRecorder;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.mobility.IMobility;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IWiredNic;
import inet.linklayer.IExternalNic;
import inet.base.NotificationBoard;


//
// Peer module.
//
module Peer like PeerInterface
{
    parameters:
        @node();
        //Parametro de mobilidad
        string mobilityType = default("StationaryMobility");

        //EAM :: BasicTopology --> wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");  // use adhoc management
        bool IPForward = default(false);
        int namid = default(-1);
        string routingFile = default("");
        @labels(node,wireless-node);
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        string tcpAppType = default("");
        string udpAppType = default("");
        //bool IPForard = default(false);
        //string routingFile = default("");
        //EAM :: string mobilityType = default("NullMobility");
        @display("i=device/pocketpc_s");
    gates:
        input radioIn @directIn;
        //input ethg;
    submodules:
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=132,230");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,150");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward;
                routerId = "";
                routingFile = routingFile;
                @display("p=60,230");
        }
        bitTorrentApp: BitTorrentApp {
            parameters:
                @display("p=163,67");
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

        //EAM :: wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
        //EAM ::     parameters:
        //EAM ::         @display("p=159,386;q=queue");
        //EAM :: }

        wlan: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=248,349;q=queue");
        }
//        mobility: MassMobility {
//            parameters:
//                @display("p=149,307");
//        }

    //        aodvuu: AODVUU {
    //            @display("p=255,164");
    //        }
    connections allowunconnected:
        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;

        // connections to network outside
        radioIn --> wlan.radioIn;

        //EAM :: Cambio ++ --> upperLayerIn
        wlan.upperLayerOut --> networkLayer.ifIn++;
        wlan.upperLayerIn <-- networkLayer.ifOut++;
        // Connect application to TCP layer
        bitTorrentApp.tcpOut[0] --> tcp.appIn++;
        bitTorrentApp.tcpOut[1] --> tcp.appIn++;
        bitTorrentApp.tcpIn[0] <-- tcp.appOut++;
        bitTorrentApp.tcpIn[1] <-- tcp.appOut++;
		//ethg --> wlan.radioIn;

		//ethg --> wlan.radioIn;
		//wlan.radioIn <--> ethg;
		//eth.phys <--> ethg;

		//Protocolo de encaminamiento AODV
//		aodvuu.to_ip --> networkLayer.tcpIn;
//		aodvuu.from_ip <-- networkLayer.tcpOut;

//        aodvuu.to_ip --> networkLayer.manetIn;
//        networkLayer.manetOut --> aodvuu.from_ip;

}
