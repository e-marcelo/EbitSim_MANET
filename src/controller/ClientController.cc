

#include "ClientController.h"

//EAM :: #include <IPAddressResolver.h>
#include <IPvXAddressResolver.h>
#include <cxmlelement.h>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <ctopology.h>
#include <boost/tokenizer.hpp>
#include "SwarmManager.h"
#include "UserCommand_m.h"
#include <boost/foreach.hpp>

// Dumb fix because of the CDT parser (https://bugs.eclipse.org/bugs/show_bug.cgi?id=332278)
#ifdef __CDT_PARSER__
#undef BOOST_FOREACH
#define BOOST_FOREACH(a, b) for(a; b; )
#endif


Define_Module(ClientController);

// helper functions in anonymous workspace
namespace {


std::vector<int> newSeeds;

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

LeaveSwarmCommand createDefaultControlInfoLeave(int infoHash) {
    // these parameters are default for all announce messages sent
    LeaveSwarmCommand leaveSwarmCommand;
    leaveSwarmCommand.setInfoHash(infoHash);
    return leaveSwarmCommand;
}



//! Create the announce message that will be sent to the passed SwarmManager.
cMessage * createAnnounceMsg(EnterSwarmCommand const& defaultControlInfo,
        bool seeder, SwarmManager * swarmManager,int idDisplay) {
    // create the message and set the control info
    cMessage *msg = new cMessage("Enter Swarm");
    msg->setContextPointer(swarmManager);
    msg->setKind(USER_COMMAND_ENTER_SWARM);


    // Each new message needs a new control info
    // Update the seeder status in the control info
    EnterSwarmCommand * enterSwarmCommand = defaultControlInfo.dup();
    enterSwarmCommand->setSeeder(seeder);
    enterSwarmCommand->setIdDisplay(idDisplay);
    msg->setControlInfo(enterSwarmCommand);
    return msg;
}

cMessage * createAnnounceMsgReNewSwarm(EnterSwarmCommand const& defaultControlInfo,
        bool seeder, SwarmManager * swarmManager,int idDisplay) {
    // create the message and set the control info
    cMessage *msg = new cMessage("Renew Swarm");
    msg->setContextPointer(swarmManager);
    msg->setKind(USER_COMMAND_RENEW_SWARM);


    // Each new message needs a new control info
    // Update the seeder status in the control info
    EnterSwarmCommand * enterSwarmCommand = defaultControlInfo.dup();
    enterSwarmCommand->setSeeder(seeder);
    enterSwarmCommand->setIdDisplay(idDisplay);
    msg->setControlInfo(enterSwarmCommand);
    return msg;
}

cMessage * createAnnounceMsgLeave(LeaveSwarmCommand const& defaultControlInfo,
        SwarmManager * swarmManager) {
    // create the message and set the control info
    cMessage *msg = new cMessage("Leave Swarm");
    msg->setContextPointer(swarmManager);
    msg->setKind(USER_COMMAND_LEAVE_SWARM);


    // Each new message needs a new control info
    // Update the seeder status in the control info
    LeaveSwarmCommand * enterSwarmCommand = defaultControlInfo.dup();
    msg->setControlInfo(enterSwarmCommand);
    return msg;
}


//! Schedule the announce messages to all BitTorrent applications.
void scheduleLeaveMessages(ClientController * self, simtime_t const& leaveTime,
       LeaveSwarmCommand const& defaultControlInfo) {
    cTopology topo;
    topo.extractByProperty("peer");
    //simtime_t leaveTime_ = leaveTime;

    for (int i = 0; i < topo.getNumNodes(); ++i) {
        SwarmManager * swarmManager = check_and_cast<SwarmManager *>(
                topo.getNode(i)->getModule()->getSubmodule("swarmManager"));

              cMessage *msg = createAnnounceMsgLeave(defaultControlInfo,swarmManager);
              //Calendarizando salida del enjambre de todos los pares!
              self->scheduleAt(leaveTime,msg);

    }
}
//Schedule the re-enter to swarm

void scheduleEnterSwarmMessages(ClientController * self, simtime_t const& startTime,
        simtime_t const& interarrivalTime, EnterSwarmCommand const& defaultControlInfo) {
    cTopology topo;
    topo.extractByProperty("peer");

    simtime_t RenterTime = startTime;
    for (int i = 0; i < topo.getNumNodes(); ++i) {
        SwarmManager * swarmManager = check_and_cast<SwarmManager *>(
                topo.getNode(i)->getModule()->getSubmodule("swarmManager"));


        //Considerando que todos los nodos no son semilla al inicio
        bool seeder = false;
        // The first numSeeders will start immediately
        BOOST_FOREACH(int seedNew, newSeeds){
               if(i == seedNew){
                  seeder = true;
                  //std::cerr << "-> Semilla \n";
                  continue;
               }
        }

        cMessage *msg = createAnnounceMsgReNewSwarm(defaultControlInfo, seeder,swarmManager,i);

//        self->scheduleAt(enterTime, msg);
//        if(!seeder)
//            self->emitEnterTime(enterTime);
        // The first peers set to seeders and start imediatelly
        self->scheduleAt(RenterTime, msg);

    }
}




//! Schedule the announce messages to all BitTorrent applications.
void scheduleStartMessages(ClientController * self, simtime_t const& startTime,
        simtime_t const& interarrivalTime, long const numSeeders,
        EnterSwarmCommand const& defaultControlInfo) {
    cTopology topo;
    topo.extractByProperty("peer");

    simtime_t enterTime = startTime;


//    int idModule;

    for (int i = 0; i < topo.getNumNodes(); ++i) {
        SwarmManager * swarmManager = check_and_cast<SwarmManager *>(
                topo.getNode(i)->getModule()->getSubmodule("swarmManager"));

        // The first numSeeders will start immediately

        bool seeder = i < numSeeders;
        cMessage *msg = createAnnounceMsg(defaultControlInfo, seeder,swarmManager,i);
//        self->scheduleAt(enterTime, msg);
//        if(!seeder)
//            self->emitEnterTime(enterTime);
        // The first peers set to seeders and start imediatelly


        if (seeder) {

              std::string opt;
              //std::string newArg;
              std::ostringstream numNode;
/*
              int count = 0;*/
              //Construcción del identificador del par (en modo gráfico)
              opt = std::string("peer[");
              newSeeds.push_back(i);
              numNode << i;
              opt.append(numNode.str());
              opt.append("]");

              std::string pos = simulation.getModuleByPath(opt.c_str())->getDisplayString().str();
              pos.append(";i=old/x_green");
//              simulation.getModuleByPath(opt.c_str())->getDisplayString().parse(";i=old/x_green");
//              idModule = topo.getNode(i)->getModuleId();
//              std::cerr << "-* Mi posición :: " << simulation.getModuleByPath(opt.c_str())->getProperties()->info().c_str();
/*
              std::cerr << "-* Mi posición :: " << pos.c_str() <<"\n";
              boost::char_separator<char> sep(";");
              typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
              tokenizer mytokenizer(pos,sep);

              for(auto& token: mytokenizer){
//                  std::cerr << "\n\t + " << token << "\n";
                  if(count == 3){
                      newArg = token;
                      break;
                  }
                  count++;

              }
//              pos = newArg.c_str();

//              std::cerr << "- Mi posición :: " <<pos.c_str() <<newArg.c_str() << "\n";
//            simulation.getModule(idModule)->getDisplayString().parse("is=vs;i=old/x_red");
//              opt.append(";i=old/x_red");
              */
              simulation.getModuleByPath(opt.c_str())->getDisplayString().parse(pos.c_str());
              //std::cerr << "-X* Mi posición :: " << simulation.getModuleByPath(opt.c_str())->getDisplayString().str() <<"\n";

////EAM::            self->scheduleAt(simTime(), msg);

//              topo.getNode(i)->getModule()->getDisplayString().parse();

              self->scheduleAt(enterTime,msg);

//            self->scheduleAt(enterTime,msg);
              opt.clear();
              numNode.str("");

        //self->scheduleAt(enterTime, msg);
        //if(!seeder)
            //self->emitEnterTime(enterTime);
        // The first peers set to seeders and start imediatelly
//
        } else {
            self->emitEnterTime(enterTime);
//            //EAM :: self->scheduleAt(enterTime,msg);
            self->scheduleAt(enterTime+1, msg); //<-- Con movilidad todos los pares inician al mismo tiempo (no podemos perder la oportunidad de compartir y obtener contenido)
//            //EAM :: self->scheduleAt(enterTime, msg);
//            //EAM :: enterTime += exponential(interarrivalTime);
        }
//        enterTime += exponential(interarrivalTime);

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
void ClientController::renewSwarmInit(){
    // get the parameters
    std::string sTrackerAddress = par("trackerAddress").stringValue();
    int trackerPort = par("trackerPort").longValue();
    //int numSeeders = par("numSeeders").longValue();
    //simtime_t leaveTime = par("leaveTime").doubleValue();

    //simtime_t pauseEnterSwarm = par("pauseTimeEnterSwarm").doubleValue();


    cXMLElement * profile = par("profile").xmlValue();

    // verify parameters
    cXMLElementList contentList = profile->getChildrenByTagName("content");
    if (contentList.empty()) {
        throw cException("List of contents is empty. Check the xml file");
    }
    // throw an error if unable to resolve
    IPvXAddress trackerAddress = IPvXAddressResolver().resolve(sTrackerAddress.c_str(), IPvXAddressResolver::ADDR_IPv4);

    sTrackerAddress.append(".tcpApp[0]");

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


                scheduleEnterSwarmMessages(this, simTime(), interarrival,
                                          defaultControlInfo);

                //Horas antes de terminar las ocho horas de descarga volvemos intentar reiniciar la descarga
                //scheduleLeaveMessages(this,leaveTimeLast,defaultControlInfoLeave);

                //Último intento de terminar la descarga antes de agotar el tiempo de descarga!
                //scheduleEnterSwarmMessages(this, leaveTimeLast+pauseEnterSwarm, interarrival,
                //                           defaultControlInfo);

            }
            //Contador de pares que terminan la descarga
            //this->endPeerDownload = 0;
            cTopology topo;
            topo.extractByProperty("peer");
            //this->numNodesTotal = topo.getNumNodes() - newSeeds.size();
            std::cerr << "Numero de sanguijuelas :: " << topo.getNumNodes() - newSeeds.size() << "\n";
            this->updateStatusString();


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
        //bool debugFlag = par("debugFlag").boolValue();
        int numSeeders = par("numSeeders").longValue();
        simtime_t startTime = par("startTime").doubleValue();
        simtime_t leaveTime = par("leaveTime").doubleValue();
        simtime_t renewTimeSwarm = par("renewTimeSwarm").doubleValue();
        simtime_t pauseEnterSwarm = par("pauseTimeEnterSwarm").doubleValue();


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
            LeaveSwarmCommand const& defaultControlInfoLeave =
                                           createDefaultControlInfoLeave(torrentMetadata.infoHash);

            //*Inicia primera ronda de la descarga
            scheduleStartMessages(this, startTime, interarrival,
                               numSeeders, defaultControlInfo);

            //Estadísticamente la descarga se estanca en un periodo muy visible
            scheduleLeaveMessages(this,leaveTime,defaultControlInfoLeave);




            //*Ronda 2
            cMessage * resetSwarm = new cMessage("renewSwarm");
            resetSwarm->setContextPointer(this);
            this->scheduleAt(leaveTime+pauseEnterSwarm, resetSwarm);

            scheduleLeaveMessages(this,(leaveTime+(renewTimeSwarm*2)),defaultControlInfoLeave);


            //Ronda 3
            cMessage * resetSwarm2 = new cMessage("renewSwarm");
                        resetSwarm2->setContextPointer(this);
            this->scheduleAt((leaveTime+(renewTimeSwarm*2))+pauseEnterSwarm, resetSwarm2);

            scheduleLeaveMessages(this,leaveTime+(3*renewTimeSwarm),defaultControlInfoLeave);

            //Ronda 4
            cMessage * resetSwarm3 = new cMessage("renewSwarm");
            resetSwarm3->setContextPointer(this);

            this->scheduleAt((leaveTime+(3*renewTimeSwarm))+pauseEnterSwarm, resetSwarm3);

            scheduleLeaveMessages(this,leaveTime+(4*renewTimeSwarm),defaultControlInfoLeave);

            //Ronda 5
            cMessage * resetSwarm4 = new cMessage("renewSwarm");
            resetSwarm4->setContextPointer(this);

            this->scheduleAt((leaveTime+(4*renewTimeSwarm))+pauseEnterSwarm, resetSwarm4);

            scheduleLeaveMessages(this,leaveTime+(5*renewTimeSwarm),defaultControlInfoLeave);



            //Ronda 6
            cMessage * resetSwarm5 = new cMessage("renewSwarm");
            resetSwarm5->setContextPointer(this);

            this->scheduleAt((leaveTime+(5*renewTimeSwarm))+pauseEnterSwarm, resetSwarm5);

            scheduleLeaveMessages(this,leaveTime+(6*renewTimeSwarm),defaultControlInfoLeave);

            //Ronda 7

            cMessage * resetSwarm6 = new cMessage("renewSwarm");
            resetSwarm6->setContextPointer(this);

            this->scheduleAt((leaveTime+(6*renewTimeSwarm))+pauseEnterSwarm, resetSwarm6);

            scheduleLeaveMessages(this,leaveTime+(7*renewTimeSwarm),defaultControlInfoLeave);

            //Ronda final (8)
            cMessage * resetSwarm7 = new cMessage("renewSwarm");
            resetSwarm7->setContextPointer(this);

            this->scheduleAt((leaveTime+(7*renewTimeSwarm))+pauseEnterSwarm, resetSwarm7);


            //Calendarizar en un momento posterior!
            //scheduleEnterSwarmMessages(this, leaveTime+pauseEnterSwarm, interarrival,
            //                          defaultControlInfo);

            //Horas antes de terminar las ocho horas de descarga volvemos intentar reiniciar la descarga
            //scheduleLeaveMessages(this,leaveTimeLast,defaultControlInfoLeave);

            //Último intento de terminar la descarga antes de agotar el tiempo de descarga!
            //scheduleEnterSwarmMessages(this, leaveTimeLast+pauseEnterSwarm, interarrival,
            //                           defaultControlInfo);

        }
        //Contador de pares que terminan la descarga
        this->endPeerDownload = 0;
        cTopology topo;
        topo.extractByProperty("peer");
        this->numNodesTotal = topo.getNumNodes() - numSeeders;
        std::cerr << "Numero de sanguijuelas :: " << this->numNodesTotal << "\n";
        this->updateStatusString();
    }
}
// Private methods
void ClientController::printDebugMsg(std::string s) {
    if (this->debugFlag) {
        // debug "header"
        std::cerr << simulation.getEventNumber() << " (T=";
        std::cerr << simulation.getSimTime() << ")(ClientController) - ";
        //EAM :: std::cerr << "Peer " << this->localPeerId << ": ";
        std::cerr << s << "\n";
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

void ClientController::endUserDownload(cMessage *msg)
{
    if(msg->getKind() == 333){ //Tipo de dato con el identificador para terminar la simulación
        //std::cerr << "-> Semilla nueva (modo gráfico) :: Peer[ " << msg->getSrcProcId() <<"] \n";
        //Vector con la referencia de los pares que son semillas dentro de la simulación!
        newSeeds.push_back(msg->getSrcProcId());
        //std::string *newSeed = static_cast<std::string *>(msg->getContextPointer());
        //if(newSeed != NULL)
            //std::cerr << "-> Datos de la nueva semilla :: " << newSeed <<"\n";
        this->endPeerDownload++;
        std::cerr << "[clientController]* Pares que reportan descarga completa :: "<< this->endPeerDownload << " \n";
        if(this->endPeerDownload >= this->numNodesTotal){
            std::cerr << "*** Termina simulación [Condición de finalización aceptada]! \n";
            endSimulation(); //Esperamos que el histograma se guarde por defecto (en el tiempo establecido)
        }
        //Enviar a todos la información de la nueva semilla (como en la inicialización, con un código diferente)
        //Envio a todos menos a las semillas previas y la actual -> Revisar el comportamiento del tracker original
        //Lo recibe el swarm y actualizamos la lista de pares no conectados -> prioridad a las semillas

    }
}

// Protected methods
// TODO add startTimer (to tell when the Client will enter the swarm)
// TODO make creation of Peers dynamic? Research memory consumption gains
// TODO add stopTimer (to tell when the Client will leave the swarm)
// TODO add seedTimer (to tell how long the Client will be seeding). Maybe utilize the stopTimer.
void ClientController::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("userController")) {
        this->endUserDownload(msg);
    }else{
        if (!msg->isSelfMessage()) {
            throw cException("This module doesn't process messages");
        }
        if(msg->isName("renewSwarm")){
               //std::cerr << "\n*** Vamos a entrar de nuevo al enjambre :: " << simTime() << "\n";
               renewSwarmInit();
               delete msg;
        }else{
            SwarmManager * swarmManager = check_and_cast<SwarmManager *>(
            (cModule *) msg->getContextPointer());
            // Send the scheduled message directly to the swarm manager module
            sendDirect(msg, swarmManager, "userCommand");
        }


    }
}
