

#include "ClientController.h"

//EAM :: #include <IPAddressResolver.h>
#include <IPvXAddressResolver.h>
#include <cxmlelement.h>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <ctopology.h>

#include "SwarmManager.h"
#include "UserCommand_m.h"

Define_Module(ClientController);

// helper functions in anonymous workspace
namespace {
//! Create ControlInfo object common to all announce messages.
EnterSwarmCommand createDefaultControlInfo(
        TorrentMetadata const& torrentMetadata,
        IPvXAddress const& trackerAddress, int trackerPort) {
    // these parameters are default for all announce messages sent
    EnterSwarmCommand enterSwarmCommand;
    enterSwarmCommand.setTorrentMetadata(torrentMetadata);
    enterSwarmCommand.setTrackerAddress(trackerAddress);
    enterSwarmCommand.setTrackerPort(trackerPort);

    return enterSwarmCommand;
}

//! Create the announce message that will be sent to the passed SwarmManager.
cMessage * createAnnounceMsg(EnterSwarmCommand const& defaultControlInfo,
        bool seeder, SwarmManager * swarmManager) {
    // create the message and set the control info
    cMessage *msg = new cMessage("Enter Swarm");
    msg->setContextPointer(swarmManager);
    msg->setKind(USER_COMMAND_ENTER_SWARM);

    // Each new message needs a new control info
    // Update the seeder status in the control info
    EnterSwarmCommand * enterSwarmCommand = defaultControlInfo.dup();
    enterSwarmCommand->setSeeder(seeder);
    msg->setControlInfo(enterSwarmCommand);
    return msg;
}

//! Schedule the announce messages to all BitTorrent applications.
void scheduleStartMessages(ClientController * self, simtime_t const& startTime,
        simtime_t const& interarrivalTime, long const numSeeders,
        EnterSwarmCommand const& defaultControlInfo) {
    cTopology topo;
    topo.extractByProperty("peer");

//    int numNodes = topo.getNumNodes();

    simtime_t enterTime = startTime;

    for (int i = 0; i < topo.getNumNodes(); ++i) {
        SwarmManager * swarmManager = check_and_cast<SwarmManager *>(
                topo.getNode(i)->getModule()->getSubmodule("swarmManager"));

        // The first numSeeders will start immediately

        bool seeder = i < numSeeders;
        cMessage *msg = createAnnounceMsg(defaultControlInfo, seeder,
                swarmManager);

        // The first peers set to seeders and start imediatelly
        if (seeder) {
//EAM::            self->scheduleAt(simTime(), msg);
            self->scheduleAt(enterTime,msg);
        } else {
            self->emitEnterTime(enterTime);
            self->scheduleAt(enterTime,msg);
            //EAM :: self->scheduleAt(simTime(), msg);
            //EAM :: self->scheduleAt(enterTime, msg);
            //EAM :: enterTime += exponential(interarrivalTime);
        }
    }
}
}
// cListener
void ClientController::receiveSignal(cComponent *source, simsignal_t signalID,
        long l) {

}
// public methods
ClientController::ClientController() :
        debugFlag(false) {
}

ClientController::~ClientController() {
}

void ClientController::emitEnterTime(simtime_t enterTime) {
    emit(this->enterTime_Signal, enterTime);
}

// Private methods
int ClientController::numInitStages() const {
    return 4;
}

// Starting point of the simulation
void ClientController::initialize(int stage) {
    if (stage == 0) {
        this->enterTime_Signal = registerSignal(
                "ClientController_EnterTimeSignal");
    } else if (stage == 3) {
        // get the parameters
        std::string sTrackerAddress = par("trackerAddress").stringValue();
        int trackerPort = par("trackerPort").longValue();
        bool debugFlag = par("debugFlag").boolValue();
        int numSeeders = par("numSeeders").longValue();
        simtime_t startTime = par("startTime").doubleValue();
        cXMLElement * profile = par("profile").xmlValue();

        // verify parameters
        if (numSeeders < 1) {
            throw cException("The number of seeders must be larger than 1");
        }
        cXMLElementList contentList = profile->getChildrenByTagName("content");
        if (contentList.empty()) {
            throw cException("List of contents is empty. Check the xml file");
        }
        // throw an error if unable to resolve
        IPvXAddress trackerAddress = IPvXAddressResolver().resolve(sTrackerAddress.c_str(), IPvXAddressResolver::ADDR_IPv4);

        sTrackerAddress.append(".tcpApp[0]");
        //EV <<"\n[Tracker ***] -> ";
        //sTrackerAddress.c_str()
        //EV << simulation.getModuleByPath("tracker.tcpApp[0]");
        //EV << "<-- \n";
        // get references to other modules
        TrackerApp* trackerApp = check_and_cast<TrackerApp *>(
                simulation.getModuleByPath(sTrackerAddress.c_str()));

        // For each content in the profile, schedule the start messages.
        cXMLElementList::iterator it = contentList.begin();
        for (int num = 0; it != contentList.end(); ++it, ++num) {
            using boost::lexical_cast;

            char const* contentName =
                    (*it)->getFirstChildWithTag("name")->getNodeValue();
            simtime_t interarrival =
                    lexical_cast<double>(
                            (*it)->getFirstChildWithTag("interarrival")->getNodeValue());

            // This simulates the phase where the Client get the the .torrent
            // files from the torrent repository server.
            TorrentMetadata const& torrentMetadata =
                    trackerApp->getTorrentMetaData(contentName);

            EnterSwarmCommand const& defaultControlInfo =
                    createDefaultControlInfo(torrentMetadata, trackerAddress,
                            trackerPort);

            scheduleStartMessages(this, startTime, interarrival,
                    numSeeders, defaultControlInfo);

        }

        this->updateStatusString();
    }
}
// Private methods
void ClientController::printDebugMsg(std::string s) {
    if (this->debugFlag) {
//        // debug "header"
//        std::cerr << simulation.getEventNumber() << " (T=";
//        std::cerr << simulation.getSimTime() << ")(ClientController) - ";
//        std::cerr << "Peer " << this->localPeerId << ": ";
//        std::cerr << s << "\n";
    }
}
void ClientController::updateStatusString() {
    if (ev.isGUI()) {
//        std::ostringstream out;
//        out << "peerId: " << this->localPeerId;
//        getDisplayString().setTagArg("t", 0, out.str().c_str());
    }
}
void ClientController::subscribeToSignals() {
    subscribe("ContentManager_BecameSeeder", this);
}

// Protected methods
// TODO add startTimer (to tell when the Client will enter the swarm)
// TODO make creation of Peers dynamic? Research memory consumption gains
// TODO add stopTimer (to tell when the Client will leave the swarm)
// TODO add seedTimer (to tell how long the Client will be seeding). Maybe utilize the stopTimer.
void ClientController::handleMessage(cMessage *msg) {
    if (!msg->isSelfMessage()) {
        throw cException("This module doesn't process messages");
    }
    SwarmManager * swarmManager = check_and_cast<SwarmManager *>(
            (cModule *) msg->getContextPointer());
    // Send the scheduled message directly to the swarm manager module
    sendDirect(msg, swarmManager, "userCommand");
}
