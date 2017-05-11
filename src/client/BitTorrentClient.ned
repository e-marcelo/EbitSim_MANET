
package br.larc.usp.client;

import inet.applications.tcpapp.TCPSrvHostApp;
//
// Manage the interaction between the client and other peers.
//
simple BitTorrentClient extends TCPSrvHostApp
{
    parameters:
        @signal[BitTorrentClient_DownloadComplete_Test];
        
        
        @statistic[DownloadComplete_Test](
            source=BitTorrentClient_DownloadComplete_Test;
            title="Descargas completas (prueba)";
            record=vector
        );
        
        @signal[BitTorrentClient_NumUnconnected];
        @signal[BitTorrentClient_NumConnected];
        @signal[BitTorrentClient_ProcessingTime];
    
        @signal[BitTorrentClient_PeerWireBytesSent];
        @signal[BitTorrentClient_PeerWireBytesReceived];
        @signal[BitTorrentClient_ContentBytesSent];
        @signal[BitTorrentClient_ContentBytesReceived];
    
        @signal[BitTorrentClient_BitFieldSent];
        @signal[BitTorrentClient_BitFieldReceived];
        @signal[BitTorrentClient_CancelSent];
        @signal[BitTorrentClient_CancelReceived];
        @signal[BitTorrentClient_ChokeSent];
        @signal[BitTorrentClient_ChokeReceived];
        @signal[BitTorrentClient_HandshakeSent];
        @signal[BitTorrentClient_HandshakeReceived];
        @signal[BitTorrentClient_HaveSent];
        @signal[BitTorrentClient_HaveReceived];
        @signal[BitTorrentClient_InterestedSent];
        @signal[BitTorrentClient_InterestedReceived];
        @signal[BitTorrentClient_KeepAliveSent];
        @signal[BitTorrentClient_KeepAliveReceived];
        @signal[BitTorrentClient_NotInterestedSent];
        @signal[BitTorrentClient_NotInterestedReceived];
        @signal[BitTorrentClient_PieceSent];
        @signal[BitTorrentClient_PieceReceived];
        @signal[BitTorrentClient_RequestSent];
        @signal[BitTorrentClient_RequestReceived];
        @signal[BitTorrentClient_UnchokeSent];
        @signal[BitTorrentClient_UnchokeReceived];
        
        @display("i=misc/globe,green");
        @class(BitTorrentClient); // tell omnet++ to use this class instead of TCPSrvHostApp
        // class name of "thread" objects to launch on incoming connections
        serverThreadClass = "n/a";
        //EAM :: Adaptación del contenido
        xml contents; // xml with all the infohashes managed by this Tracker
        //Numero de pares en la simulación
        //EAM :: port
        localPort = default(6881); // port the client is listening
        bool debugFlag = default(false);
        bool subModulesDebugFlag = default(false);
        // file-saved histogram with the processing time
        string processingTimeHistogram;
        double downloadRateInterval @unit(s) = default(1s);
        double keepAliveInterval @unit(s) = default(60s);
        double oldUnchokeInterval @unit(s) = default(20s);
        double snubbedInterval @unit(s) = default(30s);
        double timeoutInterval @unit(s) = default(120s);
        double uploadRateInterval @unit(s) = default(1s);
        int globalNumberOfPeers = default(0);
        int numberOfPeers = default(0);
        int numberOfActivePeers = default(8);
        int numberOfPassivePeers = default(4);
        // The number of Peers the Client requests to the Tracker
        int numWant = default(50);
        int communicationRange = default(50);
        double timerAskMorePeers @unit(s) = default(300s); // 5 minutos
        double timerExtraPeers  @unit(s) = default(390s); // 5 minutos
        //Tamaño del escenario
        int sizeX = default(500);
        int sizeY = default(500);
}
