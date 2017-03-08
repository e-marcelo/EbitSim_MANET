// -> typedef std::map<int, Swarm> SwarmMap;
// -> typedef std::map<int, PeerStatus> PeerMap;
// -> std::map<std::string, TorrentMetadataBTM> contents; //The key is the content name from XML file
// -> std::map<int, SwarmPeerList> swarms; // The key is the infoHash

#include "BitTorrentClient.h"
#include <ctopology.h>
#include "controller/ClientController.h"
#include "UserCommand_m.h"

#include <boost/tokenizer.hpp>

#include <string.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
//EAM :: #include <IPAddressResolver.h>
#include <IPvXAddressResolver.h>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include "Application_m.h"
#include "PeerWire_m.h"
#include "PeerWireMsgBundle_m.h"
#include "PeerWireThread.h"
#include "Choker.h"
#include "ContentManager.h"
#include <boost/foreach.hpp>
#include "SwarmManager.h"
// random_shuffle example

#include <vector>       // std::vector
#include <ctime>        // std::time
#include <cstdlib>      //


// Dumb fix because of the CDT parser (https://bugs.eclipse.org/bugs/show_bug.cgi?id=332278)
#ifdef __CDT_PARSER__
#undef BOOST_FOREACH
#define BOOST_FOREACH(a, b) for(a; b; )
#endif

// Helper functions
namespace {
std::string toStr(int i) {
    return boost::lexical_cast<std::string>(i);
}
}

Define_Module(BitTorrentClient);



// Public Methods
BitTorrentClient::BitTorrentClient() :
    swarmManager(NULL), endOfProcessingTimer("End of processing"), doubleProcessingTimeHist(
        "Piece processing time histogram"), snubbedInterval(0), timeoutInterval(
        0), keepAliveInterval(0), downloadRateInterval(0), uploadRateInterval(
        0), localPort(-1), localPeerId(-1), debugFlag(false), globalNumberOfPeers(
        0), numActiveConn(0), numPassiveConn(0), prevNumUnconnected(0), prevNumConnected(
        0) {
}

BitTorrentClient::~BitTorrentClient() {
    this->cancelEvent(&this->endOfProcessingTimer);
    // delete all sockets
//    this->socketMap.deleteSockets();
    // delete all threads, which are not deleted when the sockets are destroyed.
    BOOST_FOREACH(PeerWireThread * thread, this->allThreads) {
        delete thread;
    }

    typedef std::pair<int, Swarm> map_t;
    BOOST_FOREACH(map_t s, this->swarmMap) {
        // stop and delete the dynamic modules
        s.second.contentManager->callFinish();
        s.second.contentManager->deleteModule();
        s.second.choker->callFinish();
        s.second.choker->deleteModule();
    }
}

// Method used by the SwarmManager::SwarmModules
void BitTorrentClient::addUnconnectedPeers(int infoHash,
    UnconnectedList & peers) {

    Enter_Method("addUnconnectedPeers(infoHash: %d, qtty: %d)", infoHash,peers.size());
    Swarm & swarm = this->getSwarm(infoHash);

    // if seeding or closing, don't add unconnected peers
    if (!(swarm.seeding || swarm.closing)) {
        UnconnectedList & unconnectedList = swarm.unconnectedList;

#ifdef DEBUG_MSG
        std::ostringstream out;
        out << "Prev list: ";
        BOOST_FOREACH(PeerConnInfo p, unconnectedList) {
            out << boost::get<0>(p) << ", ";
        }
#endif
        unconnectedList.splice(unconnectedList.end(), peers);
        // sort the unconnected list in order to remove the duplicates
        unconnectedList.sort();
        unconnectedList.unique();

#ifdef DEBUG_MSG
        out << " Adding " << peers.size() << " peers: ";
        BOOST_FOREACH(PeerConnInfo p, unconnectedList) {
            out << boost::get<0>(p) << ", ";
        }
        this->printDebugMsg(out.str());
#endif
        // try to connect to more peers, if possible
        this->attemptActiveConnections(swarm, infoHash);

        // if the number of unconnected peers changed, emit a signal
        if (this->prevNumUnconnected != unconnectedList.size()) {
            this->prevNumUnconnected = unconnectedList.size();
            emit(this->numUnconnected_Signal, this->prevNumUnconnected);
        }
    }
}

// Methods used by the Choker
void BitTorrentClient::chokePeer(int infoHash, int peerId) {
    Enter_Method("chokePeer(infoHash: %d, peerId: %d)", infoHash, peerId);

    PeerStatus & peer = this->getPeerStatus(infoHash, peerId);
    if (peer.isUnchoked()) {
        peer.setUnchoked(false, simTime());
        peer.getThread()->sendApplicationMessage(APP_CHOKE_PEER);
    }
}
PeerVector BitTorrentClient::getFastestToDownload(int infoHash) const {
    Enter_Method("getFastestToDownload(infoHash: %d)", infoHash);
    Swarm const& swarm = this->getSwarm(infoHash);
    PeerMap const& peerMap = swarm.peerMap;
    PeerVector orderedPeers;
    if (peerMap.size()) {
        orderedPeers.reserve(peerMap.size());
        PeerMapConstIt it = peerMap.begin();
//EAM ::        int i = 0;
        for (; it != peerMap.end(); ++it) {
            PeerStatus const* peerStatus = &it->second;
            orderedPeers.push_back(peerStatus);
        }

        // Order from lowest to largest, and we want the reverse order
        std::sort(orderedPeers.rbegin(), orderedPeers.rend(),
            PeerStatus::sortByDownloadRate);
    }

    return orderedPeers;
}
PeerVector BitTorrentClient::getFastestToUpload(int infoHash) const {
    Enter_Method("getFastestToUpload(infoHash: %d)", infoHash);
    Swarm const& swarm = this->getSwarm(infoHash);
    PeerMap const& peerMap = swarm.peerMap;
    int peerMapSize = peerMap.size();
    PeerVector orderedPeers;
    if (peerMapSize) {
        orderedPeers.reserve(peerMapSize);
        PeerMapConstIt it = peerMap.begin();
        for (; it != peerMap.end(); ++it) {
            PeerStatus const* peerStatus = &it->second;
            orderedPeers.push_back(peerStatus);
        }

        std::sort(orderedPeers.rbegin(), orderedPeers.rend(),
            PeerStatus::sortByUploadRate);
    }

    return orderedPeers;
}
void BitTorrentClient::unchokePeer(int infoHash, int peerId) {
    Enter_Method("unchokePeer(infoHash: %d, peerId: %d)", infoHash, peerId);

    PeerStatus & peer = this->getPeerStatus(infoHash, peerId);

    if (!peer.isUnchoked()) {
        peer.setUnchoked(true, simTime());
        peer.getThread()->sendApplicationMessage(APP_UNCHOKE_PEER);
    }
}

// Methods used by the ContentManager
void BitTorrentClient::closeConnection(int infoHash, int peerId) const {
    Enter_Method("closeConnection(infoHash: %d, peerId: %d)", infoHash, peerId);
//
    this->getPeerStatus(infoHash, peerId).getThread()->sendApplicationMessage(APP_CLOSE); //<---
}
void BitTorrentClient::finishedDownload(int infoHash) {
    Enter_Method("finishedDownload(infoHash: %d)", infoHash);
    Swarm & swarm = this->getSwarm(infoHash);
    swarm.seeding = true;
    swarm.choker->par("seeder") = true; //Evitamos cerrar conexion
    swarm.contentManager->par("seeder") = true; //Evitamos cerrar conexion
    // no more active downloads
    swarm.unconnectedList.clear(); //Evitamos cerrar conexion
    this->swarmManager->finishedDownload(infoHash);
    emit(this->numDownloadComplete_Signal,simTime());
    finishDownload();

}
void BitTorrentClient::peerInteresting(int infoHash, int peerId) const {
    Enter_Method("peerInteresting(infoHash: %d, peerId: %d)", infoHash, peerId);
    this->getPeerStatus(infoHash, peerId).getThread()->sendApplicationMessage(
        APP_PEER_INTERESTING);
}
void BitTorrentClient::peerNotInteresting(int infoHash, int peerId) const {
    Enter_Method("peerNotInteresting(infoHash: %d, peerId: %d)", infoHash,
        peerId);
    this->getPeerStatus(infoHash, peerId).getThread()->sendApplicationMessage(
        APP_PEER_NOT_INTERESTING);
}
void BitTorrentClient::sendHaveMessages(int infoHash, int pieceIndex) const {
    Enter_Method("sendHaveMessage(pieceIndex: %d)", pieceIndex);
    Swarm const& swarm = this->getSwarm(infoHash);
    PeerMap const& peerMap = swarm.peerMap;
    PeerMapConstIt it = peerMap.begin();

    // Send have message to all peers from the swarm connected with the Client
    while (it != peerMap.end()) {
        std::string name = "HaveMsg (" + toStr(pieceIndex) + ")";
        HaveMsg* haveMsg = new HaveMsg(name.c_str());
        haveMsg->setIndex(pieceIndex);
        (it->second).getThread()->sendPeerWireMsg(haveMsg);
        ++it;
    }
}
void BitTorrentClient::sendPieceMessage(int infoHash, int peerId) const {
    Enter_Method("sendPieceMessage(infoHash: %d, peerId: %d)", infoHash,
        peerId);

    this->getPeerStatus(infoHash, peerId).getThread()->sendApplicationMessage(
        APP_SEND_PIECE_MSG);
}
double BitTorrentClient::updateDownloadRate(int infoHash, int peerId,
    unsigned long totalDownloaded) {
    double downloadRate = 0;
    PeerStatus & peer = this->getPeerStatus(infoHash, peerId);
    peer.setBytesDownloaded(simTime().dbl(), totalDownloaded);
    downloadRate = peer.getDownloadRate();
    return downloadRate;
}
double BitTorrentClient::updateUploadRate(int infoHash, int peerId,
    unsigned long totalUploaded) {
    double uploadRate = 0;
    PeerStatus & peer = this->getPeerStatus(infoHash, peerId);
    peer.setBytesUploaded(simTime().dbl(), totalUploaded);
    uploadRate = peer.getUploadRate();
    return uploadRate;
}

// Methods used by the SwarmManager
int BitTorrentClient::getLocalPeerId() const {
    return this->localPeerId;
}
void BitTorrentClient::createSwarm(int infoHash, int numOfPieces,
    int numOfSubPieces, int subPieceSize, bool newSwarmSeeding, int idDisplay) {
    Enter_Method("addSwarm(infoHash: %d, %s)", infoHash,
        (newSwarmSeeding ? "seeding" : "leeching"));
    assert(!this->swarmMap.count(infoHash)); // Swarm must not exist <- Cuidar la validación

    // create Choker module
    cModule * choker;
    cModuleType *chokerManagerType = cModuleType::get("br.larc.usp.client."
        "Choker");
    std::string name_c = "choker_" + toStr(infoHash);
    choker = chokerManagerType->create(name_c.c_str(), this);
    choker->par("debugFlag") = this->subModulesDebugFlag;
    choker->par("infoHash") = infoHash;
    choker->par("seeder") = newSwarmSeeding;
    choker->finalizeParameters();
    choker->buildInside();
    choker->scheduleStart(simTime());
    choker->callInitialize();

    // create ContentManager module
    cModule * contentManager;
    cModuleType *contentManagerType = cModuleType::get("br.larc.usp.client."
        "ContentManager");
    std::string name_cm = "contentManager_" + toStr(infoHash);
    contentManager = contentManagerType->create(name_cm.c_str(), this);
    contentManager->par("numOfPieces") = numOfPieces;
    contentManager->par("numOfSubPieces") = numOfSubPieces;
    contentManager->par("subPieceSize") = subPieceSize;
    contentManager->par("debugFlag") = this->subModulesDebugFlag;
    contentManager->par("seeder") = newSwarmSeeding;
    contentManager->par("infoHash") = infoHash;
    contentManager->finalizeParameters();
    contentManager->buildInside();
    contentManager->scheduleStart(simTime());
    contentManager->callInitialize();

    // create swarm
    Swarm & swarm = this->swarmMap[infoHash];
    swarm.numActive = 0;
    swarm.numPassive = 0;
    swarm.seeding = newSwarmSeeding;
    swarm.closing = false;
    swarm.choker = static_cast<Choker*>(choker);
    swarm.contentManager = static_cast<ContentManager*>(contentManager);
    //Identificador del contenido a compartir
    this->infoHash_ = infoHash;
    //Referencia al identificador gráfico del nodo en la GUI
    this->localIdDisplay = idDisplay;
    this->seed = newSwarmSeeding;
    //EAM :: std::cerr << "[***] Pares en la lista" << peers.size() << "\n";
    //Iniciamos la descarga, obviando la consulta que previamente se realizaba consultando al Tracker.

    this->strCurrentNode = std::string("peer[");
    this->optNumtoStr << this->localIdDisplay;
    this->strCurrentNode.append(optNumtoStr.str());
    this->strCurrentNode.append("]");

    if(!newSwarmSeeding){ //Sino es semilla, el par actual solo adquiere pares de tu cuadrante
         selectListPeersRandom(); //Selección por cuadrantes, no al azar!!!
         if(this->peers.size()){
               std::cerr << "***[Enjambre] Lista de pares, nodo :: " << this->localIdDisplay << " -> " <<this->peers.size() <<  "\n";
        //          std::cerr << "[Renovación] Nueva lista de pares :: " << this->peers.size()<< "\n";
                addUnconnectedPeers(infoHash, this->peers);
         }else{
                std::cerr << "***[Enjambre] Lista de pares vacia, nodo :: " << this->localIdDisplay << " aislado :(\n";
         }
    }
    //else{
//        this->numWant = this->numberOfPeers; // Todos los nodos de mi cuadrante (para evitar errores en la conexion)
//        selectListPeersRandom(); //Selección por cuadrantes, no al azar!!!
//        if(this->peers.size()){
//            std::cerr << "***[Enjambre] Soy semilla, nodo :: " << this->idDisplay << " -> " <<this->peers.size() <<  "\n";
//        }else{
//            std::cerr << "***[Enjambre] Soy semilla, nodo :: " << this->idDisplay << " aislado :(\n";
//        }
//
//    }

/***
    if (peers.size()) {
//        std::cerr << "***[Enjambre] Lista peers :: " << peers.size() << "\n";
//        std::list<PeerConnInfo> listPeers;
//        std::memcpy(&peers,&listPeers,peers.size());
        this->addUnconnectedPeers(infoHash, peers);
    }
    ***/
}
void BitTorrentClient::deleteSwarm(int infoHash) {
    Enter_Method("removeSwarm(infoHash: %d)", infoHash);
#ifdef DEBUG_MSG
    this->printDebugMsg("Leaving swarm");
#endif
    Swarm & swarm = this->getSwarm(infoHash);
    // Since it is not possible to delete the swarm before closing the threads,
    // set the closing flag to true and delete the swarm when all peers disconnect.
    swarm.closing = true;
    swarm.unconnectedList.clear();

    // close the connection with all the peers
    typedef std::pair<int, PeerStatus> map_t;
    BOOST_FOREACH(map_t const& peer, swarm.peerMap) {
        PeerWireThread * thread = peer.second.getThread();
        thread->sendApplicationMessage(APP_CLOSE);
    }
}

// Methods used by the PeerWireThread
void BitTorrentClient::addPeerToSwarm(int infoHash, int peerId,
    PeerWireThread* thread, bool active) {
    Swarm & swarm = this->getSwarm(infoHash);
    PeerMap & peerMap = swarm.peerMap;

    // if the number of connected peers changed, emit a signal
    if (this->prevNumConnected != peerMap.size()) {
        this->prevNumConnected = peerMap.size();
        emit(this->numConnected_Signal, this->prevNumConnected);
    }

    // The active connections are counted when they are initiated, but passive
    // connections cannot be known before the Handshake
    if (thread->activeConnection == false) {
        ++swarm.numPassive;
    }

    // Can't connect with the same Peer twice
    assert(!peerMap.count(peerId));
    peerMap.insert(std::make_pair(peerId, PeerStatus(peerId, thread)));

}
void BitTorrentClient::removePeerFromSwarm(int infoHash, int peerId, int connId,
    bool active) {
    Swarm & swarm = this->getSwarm(infoHash);
    // remove from the connected list
    PeerMap & peerMap = swarm.peerMap;
    peerMap.erase(peerId);
    // Remove the peer from the swarm modules
    // Added by contentManager->addPeerBitField() and choker->addPeer()
    swarm.contentManager->removePeerBitField(peerId);
    swarm.choker->removePeer(peerId);

    this->printDebugMsgConnections("removePeerInfo", infoHash, swarm);

    if (peerMap.empty()) {
        if (swarm.closing) {
            // stop and delete the dynamic modules, and delete the swarm
            swarm.contentManager->callFinish();
            swarm.contentManager->deleteModule();
            swarm.choker->callFinish();
            swarm.choker->deleteModule();
            this->swarmMap.erase(infoHash);

        } else {
            // There are no peers to unchoke
            swarm.choker->stopChokerRounds();
        }
    }
}
bool BitTorrentClient::canConnect(int infoHash, int peerId, bool active) const {
    bool successful = false;

    SwarmMapConstIt it = this->swarmMap.find(infoHash);
    if (it == this->swarmMap.end()) {
#ifdef DEBUG_MSG
        std::string out = "No swarm " + toStr(infoHash) + " found";
        this->printDebugMsg(out);
#endif
    } else if (it->second.closing) {
#ifdef DEBUG_MSG
        std::string out = "Swarm " + toStr(infoHash) + " closing";
        this->printDebugMsg(out);
#endif
    } else {
        Swarm const& swarm = it->second;
        PeerMap const& peerMap = swarm.peerMap;

        // can't connect if already in the PeerMap
        if (!peerMap.count(peerId)) {
            // check if there is a free passive slot for connecting
            // if seeding, also count the active slots
            int availSlots = this->numPassiveConn - swarm.numPassive;
            if (swarm.seeding) {
                availSlots += this->numActiveConn - swarm.numActive;
            }

            // If active, already checked the slots at attemptActiveConnections()
            successful = active || availSlots > 0;
#ifdef DEBUG_MSG
            if (!active && availSlots == 0) {
                this->printDebugMsg("No slots available for connection");
            }
        } else {
            std::string out = "Peer " + toStr(peerId) + " already connected";
            this->printDebugMsg(out);
#endif
        }
    }
    return successful;
}

std::pair<Choker*, ContentManager*> BitTorrentClient::checkSwarm(int infoHash) {
    Choker * choker = NULL;
    ContentManager * contentManager = NULL;
    SwarmMapConstIt it = this->swarmMap.find(infoHash);
    if (it != this->swarmMap.end()) {
        choker = it->second.choker;
        contentManager = it->second.contentManager;
    }
    return std::make_pair(choker, contentManager);
}

void BitTorrentClient::processNextThread() {
    // allThreads will never be empty here
    assert(!this->allThreads.empty());

    std::list<PeerWireThread*>::iterator nextThreadIt =
        this->threadInProcessingIt;

    // don't start processing another thread if the current one is still processing
    if (!(*this->threadInProcessingIt)->busy
        && !this->endOfProcessingTimer.isScheduled()) {
        // increment the nextThreadIt until a thread with messages is found or
        // until a full circle is reached
        bool hasMessages = false;
#ifdef DEBUG_MSG
        std::ostringstream out;
        out << "== Next thread: ";
        out << (*nextThreadIt)->getThreadId();
#endif
        do {
            ++nextThreadIt;
            if (nextThreadIt == this->allThreads.end()) {
                nextThreadIt = this->allThreads.begin();
            }
#ifdef DEBUG_MSG
            out << " > " << (*nextThreadIt)->getThreadId();
#endif
            hasMessages = (*nextThreadIt)->hasMessagesToProcess();
        } while (nextThreadIt != this->threadInProcessingIt && !hasMessages);
#ifdef DEBUG_MSG
        out << " ==";
#endif

        if (hasMessages) {
#ifdef DEBUG_MSG
            this->printDebugMsg(out.str());
#endif
            this->threadInProcessingIt = nextThreadIt;
            simtime_t processingTime =
                (*this->threadInProcessingIt)->startProcessing();
            emit(this->processingTime_Signal, processingTime);
            this->scheduleAt(simTime() + processingTime, //Evitamos el retardo para iniciar el procesamiento de la pieza
            //this->scheduleAt(simTime(),
                &this->endOfProcessingTimer);
#ifdef DEBUG_MSG
        } else {
            std::string out_str = "== Thread " + (*nextThreadIt)->getThreadId()
                + " idle ==";
            this->printDebugMsg(out_str);
#endif
        }
    }
}
void BitTorrentClient::setInterested(bool interested, int infoHash,
    int peerId) {
    this->getPeerStatus(infoHash, peerId).setInterested(interested);
}
void BitTorrentClient::setOldUnchoked(bool oldUnchoke, int infoHash,
    int peerId) {
    this->getPeerStatus(infoHash, peerId).setOldUnchoked(oldUnchoke);
}
void BitTorrentClient::setSnubbed(bool snubbed, int infoHash, int peerId) {
    this->getPeerStatus(infoHash, peerId).setSnubbed(snubbed);
}

// Private Methods
void BitTorrentClient::attemptActiveConnections(Swarm & swarm, int infoHash) {
    const PeerMap & peerMap = swarm.peerMap;
    // only make active connections if not seeding
    //¿Cómo alterar la condición del número de slots activos?
//    std::cerr << "Soy el par ::" << this->localPeerId << "\n";
    if (!(swarm.seeding || swarm.closing)) {
        UnconnectedList & unconnectedList = swarm.unconnectedList;

        int numActiveSlost = this->numActiveConn - swarm.numActive;
        while (numActiveSlost && !unconnectedList.empty()) {
            // Get a random peer to connect with
            UnconnectedListIt it = unconnectedList.begin();
            for (int i = intrand(unconnectedList.size()); i > 0; --i) {
                ++it;
            }
//            if(unconnectedList.size() == 2){
//                std::cerr << "Lista casi vacia :: " << this->localPeerId << "\n";
//            }
            PeerConnInfo const& peer = *it;
            int peerId = peer.get<0>();
            bool notConnected = !peerMap.count(peerId);
            bool notConnecting = this->activeConnectedPeers.count(
                std::make_pair(infoHash, peerId)) == 0;
            // only connect if not already connecting or connected
            if (notConnected && notConnecting) {
                --numActiveSlost; // decrease the number of slots
                ++swarm.numActive; // increase the number of active connections
                // get unconnected peer, connect with it then remove it from the list
                this->connect(infoHash, peer); // establish the tcp connection
                this->activeConnectedPeers.insert(
                    std::make_pair(infoHash, peerId));
            }
//          peers_swap.push_back(peer);
            unconnectedList.erase(it);// Evitamos eliminar la lista de pares que se requieren posteriormente

//            std::cerr<< "Tamaño de la lista :: " << unconnectedList.size() << "\n";
            //Tamaño de la lista
        }

        // Either the active slots are full or the unconnected list is empty
        // If more than half of the active slots is unoccupied, ask for more peers
//        if (swarm.numActive < this->numActiveConn / 2/*unconnectedList.empty()*/) {
//            askMoreUnconnectedPeers(infoHash);
//        }
    }
}
/*!
 * The Peer address is acquired from the Tracker.
 */
void BitTorrentClient::connect(int infoHash, PeerConnInfo const& peer) {
    // initialize variables with the tuple content
    int peerId, port;
    IPvXAddress ip;
    boost::tie(peerId, ip, port) = peer;

    TCPSocket * socket = new TCPSocket();
    //EAM :: Nueva sentencia
    socket->readDataTransferModePar(*this);
    //EAM* :: socket->setDataTransferMode(TCP_TRANSFER_OBJECT);
    //EAM :: socket->setDataTransferMode(TCPDataTransferMode::TCP_TRANSFER_OBJECT);

    this->createThread(socket, infoHash, peerId);
#ifdef DEBUG_MSG
    std::ostringstream out;
    out << "connect with " << ip << ":" << port;
    out << " peerId " << toStr(peerId) << " infoHash " << infoHash;
    this->printDebugMsg(out.str());
#endif

    socket->connect(ip, port);
//    emit(numDownloadComplete_Signal,simTime()); //Prueba
    //Desde que se realiza la primera conexión se inicia a tomar el tiempo de descarga
    if(this->connectPeer){
        emit(this->numDownloadComplete_Signal,simTime());
        this->connectPeer = false;
    }
}
//void BitTorrentClient::closeListeningSocket() {
//    // If the socket isn't closed, close it.
//    if (this->serverSocket.getState() != TCPSocket::CLOSED) {
//        this->serverSocket.close();
//
//        this->printDebugMsg("closed listening socket.");
//    }
//}
void BitTorrentClient::emitReceivedSignal(int messageId) {
#define CASE( X, Y ) case (X): emit(Y, this->localPeerId); break;
    switch (messageId) {
    //EAM :: CASE(PW_DOWNLOADCOMPLETE_MSG,this->numDownloadComplete_Signal)
    CASE(PW_CHOKE_MSG, this->chokeReceived_Signal)
    CASE(PW_UNCHOKE_MSG, this->unchokeReceived_Signal)
    CASE(PW_INTERESTED_MSG, this->interestedReceived_Signal)
    CASE(PW_NOT_INTERESTED_MSG, this->notInterestedReceived_Signal)
    CASE(PW_HAVE_MSG, this->haveReceived_Signal)
    CASE(PW_BITFIELD_MSG, this->bitFieldReceived_Signal)
    CASE(PW_REQUEST_MSG, this->requestReceived_Signal)
    CASE(PW_PIECE_MSG, this->pieceReceived_Signal)
    CASE(PW_CANCEL_MSG, this->cancelReceived_Signal)
    CASE(PW_KEEP_ALIVE_MSG, this->keepAliveReceived_Signal)
    CASE(PW_HANDSHAKE_MSG, this->handshakeReceived_Signal)
    }
#undef CASE
}
void BitTorrentClient::emitSentSignal(int messageId) {
#define CASE( X, Y ) case (X): emit(Y, this->localPeerId); break;
    switch (messageId) {
    CASE(PW_CHOKE_MSG, this->chokeSent_Signal)
    CASE(PW_UNCHOKE_MSG, this->unchokeSent_Signal)
    CASE(PW_INTERESTED_MSG, this->interestedSent_Signal)
    CASE(PW_NOT_INTERESTED_MSG, this->notInterestedSent_Signal)
    CASE(PW_HAVE_MSG, this->haveSent_Signal)
    CASE(PW_BITFIELD_MSG, this->bitFieldSent_Signal)
    CASE(PW_REQUEST_MSG, this->requestSent_Signal)
    CASE(PW_PIECE_MSG, this->pieceSent_Signal)
    CASE(PW_CANCEL_MSG, this->cancelSent_Signal)
    CASE(PW_KEEP_ALIVE_MSG, this->keepAliveSent_Signal)
    CASE(PW_HANDSHAKE_MSG, this->handshakeSent_Signal)
    }
#undef CASE
}
Swarm const& BitTorrentClient::getSwarm(int infoHash) const {
    assert(this->swarmMap.count(infoHash));
//EAM ::    std::cerr << "Problema [1] :: infoHash = " << infoHash << "\n";
    return this->swarmMap.at(infoHash);
}
Swarm & BitTorrentClient::getSwarm(int infoHash) {
    assert(this->swarmMap.count(infoHash));
//EAM :: std::cerr << "Problema [2] :: infoHash = " << infoHash  << "\n";
    return this->swarmMap.at(infoHash);
}
PeerStatus & BitTorrentClient::getPeerStatus(int infoHash, int peerId) {
    Swarm & swarm = this->getSwarm(infoHash);
    assert(swarm.peerMap.count(peerId));

    return swarm.peerMap.at(peerId);
}
PeerStatus const& BitTorrentClient::getPeerStatus(int infoHash,
    int peerId) const {
    Swarm const& swarm = this->getSwarm(infoHash);
    assert(swarm.peerMap.count(peerId));
    return swarm.peerMap.at(peerId);
}
//void BitTorrentClient::openListeningSocket() {
//    // only open the socket if it was already closed.
//    if (this->serverSocket.getState() == TCPSocket::CLOSED) {
//        this->serverSocket.renewSocket();
//
//        // bind the socket to the same address and port.
//        this->serverSocket.bind(this->localIp, this->localPort);
//        this->serverSocket.listen();
//
//        this->printDebugMsg("opened listening socket.");
//    }
//}

void BitTorrentClient::peerWireStatistics(cMessage const*msg, bool sending =
    false) {
    if (dynamic_cast<PeerWireMsg const*>(msg)) {
        PeerWireMsg const* peerWireMsg = static_cast<PeerWireMsg const*>(msg);
        int messageId = peerWireMsg->getMessageId();

        // bytes of useful content
        int content = 0;
        // bytes used by the PeerWire protocol
        int size = peerWireMsg->getByteLength();

        if (messageId == PW_PIECE_MSG) {
            content = static_cast<PieceMsg const*>(peerWireMsg)->getBlockSize();
            size -= content;
        }

        if (sending) {
            this->emitSentSignal(messageId);
            emit(this->contentBytesSent_Signal, content);
            emit(this->peerWireBytesSent_Signal, size);
        } else {
            this->emitReceivedSignal(messageId);
            emit(this->contentBytesReceived_Signal, content);
            emit(this->peerWireBytesReceived_Signal, size);
        }
    } else if (dynamic_cast<PeerWireMsgBundle const*>(msg)) {
        cQueue const* bundle =
            static_cast<PeerWireMsgBundle const*>(msg)->getBundle();
        for (int i = 0; i < bundle->length(); ++i) {
            PeerWireMsg const* peerWireMsg =
                static_cast<PeerWireMsg const*>(bundle->get(i));
            int messageId = peerWireMsg->getMessageId();
            int size = peerWireMsg->getByteLength();

            // PieceMsg are not bundled ever
            if (sending) {
                this->emitSentSignal(messageId);
                emit(this->peerWireBytesSent_Signal, size);
            } else {
                this->emitReceivedSignal(messageId);
                emit(this->peerWireBytesReceived_Signal, size);
            }
        }
    }
}
void BitTorrentClient::printDebugMsg(std::string s) const {
#ifdef DEBUG_MSG
    if (this->debugFlag) {
        // debug "header"
        std::cerr << simulation.getEventNumber();
        std::cerr << ";" << simulation.getSimTime();
        std::cerr << ";(btclient);Peer " << this->localPeerId << ";";
        std::cerr << s << "\n";
    }
#endif
}

void BitTorrentClient::printDebugMsgConnections(std::string methodName,
    int infoHash, Swarm const&swarm) const {
#ifdef DEBUG_MSG
    std::ostringstream out;
    out << methodName << "(" << infoHash << "): ";
    out << " Num Active=" << swarm.numActive;
    out << " Num Passive=" << swarm.numPassive;
    out << " PeerMap size=" << swarm.peerMap.size();
    this->printDebugMsg(out.str());
#endif
}

void BitTorrentClient::createThread(TCPSocket * socket, int infoHash,
    int peerId) {
    socket->setOutputGate(gate("tcpOut"));
    socket->readDataTransferModePar(*this);
    //EAM* :: socket->setDataTransferMode(TCP_TRANSFER_OBJECT);
    assert((infoHash > 0 && peerId > 0) || (infoHash == -1 && peerId == -1));
    PeerWireThread *proc = new PeerWireThread(infoHash, peerId);
    // store proc here so it can be deleted at the destructor.
    this->allThreads.push_back(proc);
    if (this->allThreads.size() == 1) {
        // there was no thread before this insertion, so no processing was being
        // made. Set the iterator to point to the beginning of the set
        this->threadInProcessingIt = this->allThreads.begin();
    }

    // attaches the callback obj to the socket
    socket->setCallbackObject(proc);
    proc->init(this, socket);

    this->socketMap.addSocket(socket);

    //update display string
    this->updateDisplay();

}
void BitTorrentClient::removeThread(PeerWireThread *thread) {
#ifdef DEBUG_MSG
    thread->printDebugMsg("Thread removed");
#endif
    if (*this->threadInProcessingIt == thread) {
        // Delete the current thread and set the iterator so that when
        // incremented, the thread after this one will be executed.
        // Steps        (1)    =>    (2)   =>   (3)    =>   (4)
        // Iterator  i         =>  i       =>        i =>      i
        // Set      |1|2|3|4|e => |2|3|4|e => |2|3|4|e => |2|3|4|e

        // (1) Iterator pointing to the current element
        this->allThreads.erase(this->threadInProcessingIt++);
        // (2) Erase current and keep the iterator valid by post-incrementing

        if (!this->allThreads.empty()) {
            // (3) If the iterator is begin(), send it to end()
            if (this->threadInProcessingIt == this->allThreads.begin()) {
                this->threadInProcessingIt = this->allThreads.end();
            }
            // (4) Decrement so that when incremented by processNextThread(),
            // the next thread is the one after the current one
            --this->threadInProcessingIt;
        }
    } else {
        this->allThreads.remove(thread);
    }

    int infoHash = thread->infoHash;
    int peerId = thread->remotePeerId;

    SwarmMap::iterator it = this->swarmMap.find(infoHash);
    Swarm & swarm = it->second;

    // The connection was established, so decrement the connection counter
    if (it != this->swarmMap.end()) {
        Swarm & swarm = it->second;
        // If the peer was actively connected, one active slot became available for
        // connection.
        if (this->activeConnectedPeers.erase(
            std::make_pair(infoHash, peerId))) {
            --swarm.numActive;
            this->attemptActiveConnections(swarm, infoHash);
        } else {
            --swarm.numPassive;
        }
    }


    // remove socket from the map and delete the thread.
    TCPSrvHostApp::removeThread(thread);
}
void BitTorrentClient::registerEmittedSignals() {
    // signal that this Client entered a swarm
#define SIGNAL(X, Y) this->X = registerSignal("BitTorrentClient_" #Y)

    //Tiempo de descarga exitosa del par
    SIGNAL(numDownloadComplete_Signal,DownloadComplete_Test);

    SIGNAL(numUnconnected_Signal, NumUnconnected);
    SIGNAL(numConnected_Signal, NumConnected);
    SIGNAL(processingTime_Signal, ProcessingTime);

    // TODO use these signals
    SIGNAL(peerWireBytesSent_Signal, PeerWireBytesSent);
    SIGNAL(peerWireBytesReceived_Signal, PeerWireBytesReceived);
    SIGNAL(contentBytesSent_Signal, ContentBytesSent);
    SIGNAL(contentBytesReceived_Signal, ContentBytesReceived);

    SIGNAL(bitFieldSent_Signal, BitFieldSent);
    SIGNAL(bitFieldReceived_Signal, BitFieldReceived);
    SIGNAL(cancelSent_Signal, CancelSent);
    SIGNAL(cancelReceived_Signal, CancelReceived);
    SIGNAL(chokeSent_Signal, ChokeSent);
    SIGNAL(chokeReceived_Signal, ChokeReceived);
    SIGNAL(handshakeSent_Signal, HandshakeSent);
    SIGNAL(handshakeReceived_Signal, HandshakeReceived);
    SIGNAL(haveSent_Signal, HaveSent);
    SIGNAL(haveReceived_Signal, HaveReceived);
    SIGNAL(interestedSent_Signal, InterestedSent);
    SIGNAL(interestedReceived_Signal, InterestedReceived);
    SIGNAL(keepAliveSent_Signal, KeepAliveSent);
    SIGNAL(keepAliveReceived_Signal, KeepAliveReceived);
    SIGNAL(notInterestedSent_Signal, NotInterestedSent);
    SIGNAL(notInterestedReceived_Signal, NotInterestedReceived);
    SIGNAL(pieceSent_Signal, PieceSent);
    SIGNAL(pieceReceived_Signal, PieceReceived);
    SIGNAL(requestSent_Signal, RequestSent);
    SIGNAL(requestReceived_Signal, RequestReceived);
    SIGNAL(unchokeSent_Signal, UnchokeSent);
    SIGNAL(unchokeReceived_Signal, UnchokeReceived);

#undef SIGNAL
}
// Protected methods
int BitTorrentClient::numInitStages() const {
    return 4;
}
void BitTorrentClient::finish() {
        this->doubleProcessingTimeHist.record();
}
void BitTorrentClient::initialize(int stage) {
    if (stage == 0) {
        this->registerEmittedSignals();
    } else if (stage == 3) {
        // create a listening connection
        TCPSrvHostApp::initialize();

        this->swarmManager = check_and_cast<SwarmManager*>(getParentModule()->getSubmodule("swarmManager"));
        this->localPeerId = this->getParentModule()->getParentModule()->getId();

        // get parameters from the modules
        this->snubbedInterval = par("snubbedInterval");
        this->timeoutInterval = par("timeoutInterval");
        this->keepAliveInterval = par("keepAliveInterval");
        //        this->oldUnchokeInterval = par("oldUnchokeInterval");
        this->downloadRateInterval = par("downloadRateInterval");
        this->uploadRateInterval = par("uploadRateInterval");
        this->globalNumberOfPeers = par("globalNumberOfPeers"); //FIXME use this!
        this->numActiveConn = par("numberOfActivePeers");
        this->numPassiveConn = par("numberOfPassivePeers");
        this->numWant = par("numWant");
        this->sizeX = par("sizeX");
        this->sizeY = par("sizeY");

        IPvXAddressResolver resolver;
        this->localIp = resolver.addressOf(getParentModule()->getParentModule(),
                IPvXAddressResolver::ADDR_PREFER_IPv4);
        //EAM :: port
        this->localPort = par("localPort");

        this->subModulesDebugFlag = par("subModulesDebugFlag").boolValue();
        this->debugFlag = par("debugFlag").boolValue();

        char const* histogramFileName = par("processingTimeHistogram");
        FILE * histogramFile = fopen(histogramFileName, "r");
        if (histogramFile == NULL) {
            //throw std::invalid_argument
            printf("\nHistogram file not found.");
        } else {
            this->doubleProcessingTimeHist.loadFromFile(histogramFile);
            fclose(histogramFile);
        }

        //Adaptación de la construcción del archivo torrent
        // read all contents from the xml file.
        cXMLElementList contentsList = par("contents").xmlValue()->getChildrenByTagName("content");

        if (contentsList.empty()) {
            //EAMthrow std::invalid_argument
            printf("List of contents is empty. Check the xml file");
        }
        cXMLElementList::iterator it = contentsList.begin();

        for (; it != contentsList.end(); ++it) {
            TorrentMetadataBTM torrentMetadata;
            cXMLElement * child;
            // create the torrent metadata
            child = (*it)->getFirstChildWithTag("numOfPieces");
            torrentMetadata.numOfPieces = atoi(child->getNodeValue());
            child = (*it)->getFirstChildWithTag("numOfSubPieces");
            torrentMetadata.numOfSubPieces = atoi(child->getNodeValue());
            child = (*it)->getFirstChildWithTag("subPieceSize");
            torrentMetadata.subPieceSize = atoi(child->getNodeValue());
            torrentMetadata.infoHash = simulation.getUniqueNumber(); //Número único para identifacar al archivo: *.torrent
            this->swarms[torrentMetadata.infoHash];
            // save a list of torrents for each video content
            std::string contentName((*it)->getAttribute("name"));
            this->contents[contentName] = torrentMetadata;
        }

        //Arreglo de pares presentes en la simulación
        // Test if Tracker has this content -> especificamos el hash del contenido digital a compartir
        this->numberOfPeers = par("numberOfPeers");
        this->communicationRange = par("communicationRange");
//        std::string opt;
//        std::ostringstream numNode;
//        int peerId;
//
//
//
//        // get all the pointers to the PeerInfo objects, except for self
//        for(int i=0; i< this->numberOfPeers; i++){
//            opt = std::string("peer[");
//            numNode << i;
//            opt.append(numNode.str());
//            opt.append("]");
//            peerId = simulation.getModuleByPath(opt.c_str())->getId();
//
//            if (this->localPeerId != peerId){
//                PeerConnInfo peer = boost::make_tuple(peerId, IPvXAddressResolver().resolve(opt.c_str(),IPvXAddressResolver::ADDR_IPv4),this->localPort);
//                // the size of the peerList minus self
//                this->peers_aux.push_back(peer);
//            }
//            opt.clear();
//            numNode.str("");
//        }

        //Lista con pares aleatorios
//        selectListPeersRandom();

//        if(peerIdActual == 6){
//            std::cerr << "Lista de pares a contactar :: \t";
//            presentElementsList(peers);
//            std::cerr << "Lista de pares en reserva :: \t";
//            presentElementsList(peers_extra);
//        }

        //Destruimos el vector auxiliar <- Evitar hacer el trabajo del recolector de basura (error al cerrar la simulación)
//        peers_aux.~vector();

    }
}
void BitTorrentClient::finishDownload()
{
    cTopology topo;
    topo.extractByProperty("controller");

    ClientController * clientController = check_and_cast<ClientController *>(topo.getNode(0)->getModule());
    //topo.getNode(0)->getModule()->getSubmodule("clientController"));
    cMessage *data = new cMessage("end");
    data->setKind(333);
    sendDirect(data, clientController, "userController");

    //Cambiando icono para marcar a las semillas
    //Variables locales para el cambio de icono


    //Contrucción de la cadena que identifica al par en la interfaz gráfica
//    this->opt = std::string("peer[");
//    this->numNode << this->idDisplay;
//    this->opt.append(numNode.str());
//    this->opt.append("]");

    std::string pos = simulation.getModuleByPath(this->strCurrentNode.c_str())->getDisplayString().str();
    //              idModule = topo.getNode(i)->getModuleId();

    /*Utilizamos la misma posición del nodo para realizar el cambio de representación gráfica
    boost::char_separator<char> sep(";");
    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

    tokenizer mytokenizer(pos,sep);
    for(auto& token: mytokenizer){
    //                  std::cerr << "\t" << token << "\n";
        newArg = token;
        if(count == 3) //Terminamos de recorrer la cadena al obtener la referencia a la posición actual
            break;
        count++;
    }*/
    //Cambio de representación gráfica para identificar a las semillas en la simulación
    pos.append(";i=old/x_green");
    simulation.getModuleByPath(this->strCurrentNode.c_str())->getDisplayString().parse(pos.c_str());
}

void BitTorrentClient::getPeerUnconnected(std::vector<PeerConnInfo> vec, std::list<PeerConnInfo> peers)
{
    //EAM :: int i = 0;
    //Es necesario utilizar una asignación al tipo de dato que utilizan los prototipos de función de EbitSim
    BOOST_FOREACH(PeerConnInfo peer, vec){
        //EAM :: std::cerr<< i << " :: " << peer.head << "\n";
        peers.push_back(peer);
        //EAM :: i++;
    }
}

void BitTorrentClient::presentElementsList(std::list<PeerConnInfo> peers)
{
    BOOST_FOREACH(PeerConnInfo peer, peers){
        std::cerr << boost::get<0>(peer) << " |\t";
    }
    std::cerr << "\n";
}

void BitTorrentClient::askMoreUnconnectedPeers(int infoHash)
{
    Enter_Method("askMoreUnconnectedPeers(%d)", infoHash);
    #ifdef DEBUG_MSG
        this->printDebugMsg("Ask for more peers for swarm " + toStr(infoHash));
    #endif

      Swarm & swarm = this->getSwarm(infoHash);

      swarm.unconnectedList.clear(); //Evitamos cerrar conexion

      selectListPeersRandom(); //Selección por cuadrantes, no al azar!!!

      if(this->peers.size()){
        //          std::cerr << "[Renovación] Nueva lista de pares :: " << this->peers.size()<< "\n";
//          peers.unique();
          std::cerr << "***[Agregando] Lista de pares extra, nodo " << this->localIdDisplay << ", agregando :: "<< peers.size()<<"\n";
          addUnconnectedPeers(infoHash, this->peers);
      }else{
          std::cerr << "***[Agregando] Lista de pares extra, nodo " << this->localIdDisplay << " aislado :(\n";
      }

//    //Es posible actualizar la lista de pares disponibles con los pares que se convierten en semillas*
//    if(opt_peers){
//        std::cerr << "*** Lista extra :: " << peers_extra.size() << "\n";
//        //Lista con los pares designados al inicio de la simulación
//        std::copy(peers.begin(),peers.end(),
//                std::back_insert_iterator<std::list<PeerConnInfo>>(peers_swap));
//        //Agregamos pares no seleccionados al inicio de la simulación
//        if (peers_extra.size()){
//            peers_swap.clear();
//            this->addUnconnectedPeers(infoHash, peers_extra);
//        }
//
//    }else{
//
//        std::cerr << "*** Lista peers :: " << peers.size() << "\n";
//        std::copy(peers_extra.begin(),peers_extra.end(),
//                            std::back_insert_iterator<std::list<PeerConnInfo>>(peers_swap));
//        if (peers.size()){
//            peers_swap.clear();
//            this->addUnconnectedPeers(infoHash, peers);
//            //Lista con los pares designados al inicio de la simulación
//        }
//
//    }
//    opt_peers = !opt_peers;

}

void BitTorrentClient::currentPosition(const char* peer, int *x, int *y) {
    //Localización del nodo actual en el escenario de simulación
    //Leyenda del nodo actual
    std::string pos = simulation.getModuleByPath(peer)->getDisplayString().str();
//    std::cerr << "Nodo :: " << pos <<"\n";
    //Separador de las propiedades de la leyenda del nodo
    boost::char_separator<char> sep(";");
    this->count = 0;
    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    //Recorrido entre los parámetros de la leyenda del nodo
    tokenizer mytokenizer(pos,sep);
    for(auto& token: mytokenizer){
        if(this->count == 3){
            this->strArgNode = token; //Primera posición
            break;
        }
        this->count++;

    }

    //    std::cerr << "Nodo :: " << this->idDisplay << " | Posición :: " <<  this->newArg <<"\n";
    //Separando coordenadas (x,y) del nodo actual de acuerdo a su leyenda en el escenario de simulación
    boost::char_separator<char> sepA("=");
    tokenizer mytokenizerA(this->strArgNode,sepA);
    //Posicion del par
    for(auto& token: mytokenizerA){
        pos = token; //Última posición
    }
    //Posicion en X , Y
    boost::char_separator<char> sepB(",");
    tokenizer mytokenizerB(pos,sepB);
    //Contador a cero (para reutilizarlo)
    this->count = 0;
    for(auto& token: mytokenizerB){
        if(this->count == 0){
            this->strArgNode = token;
            *x = std::atoi(this->strArgNode.c_str());
        }else{
            this->strArgNode = token;
            *y = std::atoi(this->strArgNode.c_str());
        }
        this->count++; //Solo dos posiciones son consideradas
    }
    //Variables reutilizables
    this->count = 0;
    this->strArgNode.clear();
    //    std::cerr << "Nodo :: " << this->localIdDisplay << " | Posición :: " <<  this->peerX << ", " << this->peerY <<"\n";

}

void BitTorrentClient::selectListPeersRandom()
{

    //1.- Definir en que cuadrante se encuentra el par
    //Contrucción de la cadena que identifica al par en la interfaz gráfica
    currentPosition(this->strCurrentNode.c_str(),&(this->peerX),&(this->peerY)); //Coordenada (x,y) del nodo actual
    //std::cerr << "Nodo :: " << this->localIdDisplay << " | Posición :: " <<  this->peerX << ", " << this->peerY <<"\n";
    //2.- Iterar sobre todos los nodos y determinar a que cuadrante corresponde (es posible ser más eficiente -> clientController)
    int x2;
    int y2;
    int peerIdNode;
    int d = -1; //Distancia entre los pares (redondeo de entero)
    int x_ = -1;
    int y_ = -1;
    //Reutilizando variables
    this->strArgNode.clear();
    this->optNumtoStr.str("");
    std::string strNode;

    int listaPares = 0;
    for(int i=0; i<this->numberOfPeers; i++){
            this->strArgNode = std::string("peer[");
            this->optNumtoStr << i;
            this->strArgNode.append(this->optNumtoStr.str());
            this->strArgNode.append("]");
            peerIdNode = simulation.getModuleByPath(this->strArgNode.c_str())->getId();

            if(this->localPeerId != peerIdNode){ //Cuidamos que no se trate del par actual
                //Obtenemos coordenadas del par
                strNode = this->strArgNode.c_str();
    //            std::cerr << "Nodo ::: " << this->strArgNode.c_str();
                currentPosition(this->strArgNode.c_str(),&x2,&y2);
                //Incluir validanciones
                x_ = std::pow((double)(x2-this->peerX),2);
                y_ = std::pow((double)(y2-this->peerY),2);
                d = std::sqrt((x_+y_));
                //std::cerr << this->strCurrentNode.c_str() <<" -> Distancia :: " << d << "***\n";
                //De acuerdo al umbral es como se permite el envio
                if(d <= (this->communicationRange*2)){ //Se permite la igualdad (considerar el error de redondeo*). Dos saltos desde el nodo actual
                    if(this->numWant > listaPares ){
                        PeerConnInfo peer = boost::make_tuple(peerIdNode, IPvXAddressResolver().resolve(strNode.c_str(),IPvXAddressResolver::ADDR_IPv4),this->localPort); //Todos comparten el mismo puerto
                        this->peers_swap.push_back(peer);
                    }//Podriamos almacenar los pares extra ***
                    listaPares++;
                }
                strNode.clear();
    //          std::cerr << " | Posición :: " <<  x2 << ", " << y2 <<"\n";
                //Distancia entre el par actual y el par que estamos visitando
            }

            this->strArgNode.clear();
            this->optNumtoStr.str("");
    }

    //Validamos la inclusión de pares en la lista
    if(this->peers_swap.size()){
        //    //Cambio en el orden anterior
        std::random_shuffle(peers_swap.begin(), peers_swap.end(), intrand);

        this->peers.clear();
        BOOST_FOREACH(PeerConnInfo peer, peers_swap){
            //EAM :: std::cerr<< i << " :: " << peer.head << "\n";
            this->peers.push_back(peer);
            //       //EAM :: i++;
        }
    }
}

void BitTorrentClient::handleMessage(cMessage* msg) {
    if (!msg->isSelfMessage()) {
        // message arrived after the processing time, process as it normally would
        TCPSocket *socket = socketMap.findSocketFor(msg);

        if (socket == NULL) {
            if (msg->getKind() == TCP_I_ESTABLISHED) {
                // new connection, create socket and process message
                socket = new TCPSocket(msg);
                socket->readDataTransferModePar(*this);
                //EAM* :: socket->setDataTransferMode(TCP_TRANSFER_OBJECT);
                this->createThread(socket, -1, -1);
            } else {
                // ignore message
//                std::ostringstream out;
//                out << "Message " << msg->getName()
//                    << " don't belong to any socket";

                delete msg;
                msg = NULL;
                std::cerr << "\nError lógico"; //<- Acción no contemplada
                //Solicitamos más pares para contactar
                //throw std::logic_error(out.str());
            }
        }

        // statistics about the PeerWireMsgs
//        this->peerWireStatistics(msg);
//        socket->processMessage(msg);
//        socket->getState();
//        std::cerr << msg <<" <-- ***\n";
        if(msg != NULL){
            if(!(msg->getKind() == TCP_I_TIMED_OUT || msg->getKind() == TCP_I_CONNECTION_REFUSED || msg->getKind() == TCP_I_CONNECTION_RESET)){
//                std::cerr << "Error TCP detectado ! Peer :: [ " << this->localPeerId << "]\n";
                //            //EAM :: delete msg;
//            }else{
//                // statistics about the PeerWireMsgs
                //            //EAM :: this->peerWireStatistics(msg,false);
                this->peerWireStatistics(msg);
                socket->processMessage(msg);
            }
        }
    } else {
        if (msg == &this->endOfProcessingTimer) {
            (*this->threadInProcessingIt)->finishProcessing();
            if (!this->allThreads.empty()) {
                this->processNextThread();
            }
        } else if (msg->isName("Delete thread")) {
            PeerWireThread * thread =
                static_cast<PeerWireThread *>(msg->getContextPointer());
            this->removeThread(thread);

            delete msg;

        } else {
            // PeerWireThread self-messages
            PeerWireThread *thread =
                static_cast<PeerWireThread *>(msg->getContextPointer());

            assert(thread != NULL);
            // pass the message along to the thread
            thread->timerExpired(msg);
        }
    }
}


