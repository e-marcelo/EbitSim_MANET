//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
package br.larc.usp.simulations;

import br.larc.usp.controller.ClientController;
import br.larc.usp.nodes.Peer;
import br.larc.usp.nodes.PeerInterface;

import inet.examples.mobility.MobileHost;
//EAM :: import inet.examples.mobility.MobileHost;
//EAM :: import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;
import inet.nodes.ethernet.EtherHost;
import inet.nodes.inet.AdhocHost;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;

//EAM :: import inet.networklayer.autorouting.FlatNetworkConfigurator;
//EAM :: import inet.nodes.adhoc.MobileHost;
//EAM :: import inet.world.ChannelControl;
import inet.world.radio.ChannelControl;

//Modelo de movilidad
import inet.mobility.models.Slaw;
//
// Basic ethernet topology with one router and everybody connected
//
network BasicTopology extends BasicNetwork
{
    parameters:
        string mobilityModel;
        //double playgroundSizeX;
        //double playgroundSizeY;

        @display("bgb=1000,1000,white;bgg=100,2,grey80");
        @signal[BitTorrentClient_PeerWireBytesSent];
        @signal[BitTorrentClient_ContentBytesSent];
        @statistic[PeerWireBytesSent](source=BitTorrentClient_PeerWireBytesSent; title="Total bytes of PeerWire messages sent"; record=sum);
        @statistic[ContentBytesSent](source=BitTorrentClient_ContentBytesSent; title="Total bytes of content sent"; record=sum);

        @signal[BitTorrentClient_BitFieldSent];
        @signal[BitTorrentClient_CancelSent];
        @signal[BitTorrentClient_ChokeSent];
        @signal[BitTorrentClient_HandshakeSent];
        @signal[BitTorrentClient_HaveSent];
        @signal[BitTorrentClient_InterestedSent];
        @signal[BitTorrentClient_KeepAliveSent];
        @signal[BitTorrentClient_NotInterestedSent];
        @signal[BitTorrentClient_PieceSent];
        @signal[BitTorrentClient_RequestSent];
        @signal[BitTorrentClient_UnchokeSent];
        @statistic[BitFieldSent](source=BitTorrentClient_BitFieldSent; title="Sum of BitFields sent"; record=sum);
        @statistic[CancelSent](source=BitTorrentClient_CancelSent; title="Sum of Cancels sent"; record=sum);
        @statistic[ChokeSent](source=BitTorrentClient_ChokeSent; title="Sum of Chokes sent"; record=sum);
        @statistic[HandshakeSent](source=BitTorrentClient_HandshakeSent; title="Sum of Handshakes sent"; record=sum);
        @statistic[HaveSent](source=BitTorrentClient_HaveSent; title="Sum of Haves sent"; record=sum);
        @statistic[InterestedSent](source=BitTorrentClient_InterestedSent; title="Sum of Interesteds sent"; record=sum);
        @statistic[KeepAliveSent](source=BitTorrentClient_KeepAliveSent; title="Sum of KeepAlives sent "; record=sum);
        @statistic[NotInterestedSent](source=BitTorrentClient_NotInterestedSent; title="Sum of NotInteresteds sent"; record=sum);
        @statistic[PieceSent](source=BitTorrentClient_PieceSent; title="Sum of Pieces sent"; record=sum);
        @statistic[RequestSent](source=BitTorrentClient_RequestSent; title="Sum of Requests sent"; record=sum);
        @statistic[UnchokeSent](source=BitTorrentClient_UnchokeSent; title="Sum of Unchokes sent"; record=sum);


        @signal[SwarmManager_EnterSwarm];
        @signal[SwarmManager_LeaveSwarm];
        @signal[SwarmManager_EmittedPeerId];
        @statistic[EnterSwarm](source=SwarmManager_EnterSwarm; title="Enter Swarm Time"; record=vector; );
        @statistic[LeaveSwarm](source=SwarmManager_LeaveSwarm; title="Leave Swarm Time"; record=vector; );
        @statistic[SwarmEmittedPeerId](source=SwarmManager_EmittedPeerId; title="Peer Swarm Signal Emitted"; record=vector; );

        //        @signal[ContentManager_BecameSeeder];
        //        @signal[ContentManager_PieceDownloadTime];
        //        @signal[ContentManager_PieceDownloaded];
        //        @signal[ContentManager_TotalBytesDownloaded];
        //        @signal[ContentManager_TotalBytesUploaded];
        //        @signal[ContentManager_25_percentDownloadTime];
        //        @signal[ContentManager_50_percentDownloadTime];
        //        @signal[ContentManager_75_percentDownloadTime];
        @signal[ContentManager_100_percentDownloadTime];
        @statistic[DownloadComplete](source=ContentManager_100_percentDownloadTime; title="Download Complete"; unit=s; record=vector,histogram);
        @signal[ContentManager_EmittedPeerId];
        @statistic[DownloadEmittedPeerId](source=ContentManager_EmittedPeerId; title="Peer Download Signal Emitted"; record=vector);

    submodules:
        //        tracker: Tracker;

        peer[numPeers]: <> like PeerInterface{
            @display("is=vs");
            
        }
        
        flatNetworkConfigurator: FlatNetworkConfigurator {

            @display("p=48,125");
        }

        clientController: ClientController {
            @display("p=48,204");
        }
        channelControl: ChannelControl {
            parameters:
                //EAM :: playgroundSizeX = playgroundSizeX;
                //EAM :: playgroundSizeY = playgroundSizeY;
                @display("p=48,50");
        }
        slaw: Slaw if mobilityModel == "SlawMobility" {
            @display("p=48,264;i=block/cogwheel_s");
        }

		tracker: AdhocHost {
            //IPForward = default(true); 
            //wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");  // use adhoc management
            mobilityType = mobilityModel;

            numTcpApps = 1;
            //EAM :: numTcpApps = 1;
            //EAM :: tcpAppType = "TrackerApp"; **Revisar version 5
            //@display("r=,,#707070;p=308,211");
            //EAM :: @display("p=450,490");
			//tcpAppType = "TrackerApp";
            //@address("192.168.0.133");
            //@display("p=392,293;i=device/server2");
            @display("is=vs");
            //@display("p=500,500");
        }
		

    connections:
        //for i=0..numPeers-1 {
        //    peer[i].ethg <--> ethernetline <--> router.ethg++;
        //}
        //tracker.ethg++ <--> ethernetline <--> router.ethg++;
}
