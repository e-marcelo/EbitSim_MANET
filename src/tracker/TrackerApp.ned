
package br.larc.usp.tracker;

import inet.applications.tcpapp.TCPSrvHostApp;

//
// TODO auto-generated module
//
simple TrackerApp extends TCPSrvHostApp
{
    parameters:
        @class(TrackerApp); // tell omnet++ to use this class instead of TCPSrvHostApp
        localPort = default(8082); // port number to listen on
        serverThreadClass = "PeerConnectionThread";

        xml contents; // xml with all the infohashes managed by this Tracker

        // Tracker parameters
        bool debugFlag = default(false);
        double interval @unit(s) = default(600s);
        int maxListSize = default(50);
        double processTime @unit(s) = default(0s);
}
