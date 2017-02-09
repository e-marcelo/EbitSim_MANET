
package br.larc.usp.controller;

//
// TODO auto-generated module
//
simple ClientController
{
    parameters:
//        @signal[ClientController_EnterTimeSignal];
//        @statistic[EnterTime](
//            source=ClientController_EnterTimeSignal;
//            title="Enter Time";
//            record=vector;
//        );
        @display("i=block/user");
        bool debugFlag = default(false);
        xml profile;
        int numSeeders; // The number of initial seeders in each swarm
        //EAM :: double startTime @unit(s) = default(10s); // the entry time of the first peer
        double startTime @unit(s) = default(1500s); // the entry time of the first peer
        string trackerAddress = default("tracker");  // tracker address
        int trackerPort = default(8082); //The port number to connect to
}
