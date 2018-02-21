
package br.larc.usp.controller;

//
// TODO auto-generated module
//
simple ClientController
{
    parameters:
        @controller(); // used by the Tracker to find all Peers in the simulation
        @signal[ClientController_EnterTimeSignal];
        @statistic[EnterTime](
            source=ClientController_EnterTimeSignal;
            title="Enter Time";
            record=vector;
        );
        @display("i=block/user");
        bool debugFlag = default(false);
        xml profile;
        int numSeeders; // The number of initial seeders in each swarm
        //EAM :: double startTime @unit(s) = default(10s); // the entry time of the first peer
        double startTime @unit(s) = default(259200s); // the entry time of the first peer
        //Calibrar para el caso de BTMANET Clasico!
        double leaveTime @unit(s) = default(262200s); // the leave time of the all peers
        double renewTimeSwarm @unit(s) = default(3000s); // the leave time of the all peers
        double pauseTimeEnterSwarm @unit(s) = default(300s); //Pause for re-enter swarm 6 minutes
        string trackerAddress = default("tracker");  // tracker address
        int trackerPort = default(8082); //The port number to connect to
    gates:
        input userController @directIn;
}
