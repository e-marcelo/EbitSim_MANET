
#include "client/connection/PeerWireThread.h"

#include <cassert>
#include <cpacketqueue.h>
#include <omnetpp.h>
#include <vector>
#include <boost/lexical_cast.hpp>

#include "BitTorrentClient.h"
#include "ContentManager.h"
#include "Application_m.h"
#include "PeerWireMsgBundle_m.h"

//#define DEBUG_CONN

//Register_Class(PeerWireThread);
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::ostr

namespace {
std::string toStr(int i) {
    return boost::lexical_cast<std::string>(i);
}
}

// Own methods
PeerWireThread::PeerWireThread(int infoHash, int remotePeerId) :
    // initialize the state machines
    connectionSm(*this), downloadSm(*this), uploadSm(*this),
    // initialize the pointers to other objects
    btClient(NULL), choker(NULL), contentManager(NULL), //
    // initialize the connection parameters
    activeConnection(true), infoHash(infoHash), remotePeerId(remotePeerId), //
    terminating(false), lastEvent(0), busy(false),
    // initialize the cObjects with names
    messageQueue("Awaiting messages"), //
    postProcessingAppMsg("Post-processing messages"), //
    downloadRateTimer("Download Rate Timer", APP_DOWNLOAD_RATE_TIMER), //
    keepAliveTimer("KeepAlive Timer", APP_KEEP_ALIVE_TIMER), //
    snubbedTimer("Snubbed Timer", APP_SNUBBED_TIMER), //
    timeoutTimer("Timeout Timer", APP_TIMEOUT_TIMER), //
    uploadRateTimer("Upload Rate Timer", APP_UPLOAD_RATE_TIMER) {
    // Active connection if both the infoHash and the remotePeerId are undefined
    this->activeConnection = !(infoHash == -1 && remotePeerId == -1);
}
PeerWireThread::~PeerWireThread() {
    delete this->sock;
    this->cancelEvent(&this->downloadRateTimer);
    this->cancelEvent(&this->keepAliveTimer);
    this->cancelEvent(&this->snubbedTimer);
    this->cancelEvent(&this->timeoutTimer);
    this->cancelEvent(&this->uploadRateTimer);
}

// Implementation of virtual methods from TCPServerThreadBase
void PeerWireThread::closed() {
    this->sendApplicationMessage(APP_TCP_LOCAL_CLOSE);
}
void PeerWireThread::peerClosed() {
    this->sendApplicationMessage(APP_TCP_REMOTE_CLOSE);
}
void PeerWireThread::dataArrived(cMessage *msg, bool urgent) {
#ifdef DEBUG_MSG
    std::string out = std::string(msg->getName()) + "\" arrived";
    this->printDebugMsg(out);
#endif
//    std::cerr << msg->getName() << " | " << msg <<" ***\n"; //<- Dato interesante
    this->sendPeerWireMessage(msg);
}
void PeerWireThread::established() {
    // first transition, issued directly to the state machine
    //EAM
    //EAM* :: this->sock->setDataTransferMode(TCP_TRANSFER_OBJECT);
    IPvXAddress localAddr = this->sock->getLocalAddress();
    IPvXAddress remoteAddr = this->sock->getRemoteAddress();
    int localPort = this->sock->getLocalPort();
    int remotePort = this->sock->getRemotePort();

    if (this->activeConnection) {
        this->connectionSm.tcpActiveConnection();
    } else {
        this->connectionSm.tcpPassiveConnection();
    }
}
//Redefinimos comportamiento para hacer algo ante el error en el comportamiento del socket ***
void PeerWireThread::failure(int code) {
//#ifdef DEBUG_MSG
////    std::ostringstream out;
////    out << "Connection failure - ";
//    switch (code) {
//    case TCP_I_CONNECTION_REFUSED:
//        std::cerr << "[PeerWireThread] TCP connection failure :: refused\n";
////        out << "refused";
//        break;
//    case TCP_I_CONNECTION_RESET:
//        std::cerr << "[PeerWireThread] TCP connection failure :: reset\n";
////        out << "reset";
//        break;
//    case TCP_I_TIMED_OUT:
//        std::cerr << "[PeerWireThread] TCP connection failure :: time out :: "<< this->btClient->localPeerId << " ---> " << this->getSocket()->getRemoteAddress() <<"\n";
//        //out << "Basura -> timed out";
////        finishProcessing();
//        //Puedo solicitar más pares
////        btClient->swarmManager->askMorePeers(20);
//        break;
//    }
//      std::cerr << "[PeerWireThread] TCP connection failure :: "<< code <<" :: "<< this->btClient->localPeerId << " ---> " << this->getSocket()->getRemoteAddress() <<"\n";
      this->btClient->updateBitField();
//    this->printDebugMsg(out.str());
//#endif
    //EAM :: throw std::logic_error
    //Either the active slots are full or the unconnected list is empty
    // If more than half of the active slots is unoccupied, ask for more peers
//    this->error_count++;
//    if (error_count >= 2 ) {
//        this->btClient->askMoreUnconnectedPeers(this->infoHash);
//        this->error_count = 0;
//    }
//    selectListPeersRandom();
//                    if(this->peers.size()){
//    //                        std::cerr << "[Error lógico] Nueva lista de pares :: " << this->peers.size()<< "\n";
//                            addUnconnectedPeers(this->infoHash_, this->peers);
//
//                    }
    //Terminamos la ejecución del hilo de procesamiento
//    this->askMorePeers++;
//    this->terminating = true;
    finishProcessing();
////    //Preguntamos por más pares en el enjambre
//    if(this->askMorePeers > 2/*(this->btClient->numActiveConn / 2) (this->btClient->numWant / 2)*/){
//        //Si mas de la mitad de las conexiones solicitadas fallan solicitamos mas pares
//      this->btClient->askMoreUnconnectedPeers(this->infoHash);
//        this->askMorePeers = 0;
//    }

}
void PeerWireThread::init(TCPSrvHostApp* hostmodule, TCPSocket* socket) {
    // call parent method
    TCPServerThreadBase::init(hostmodule, socket);

    //EAM :: socket->readDataTransferModePar(*this);
    //EAM* :: socket->setDataTransferMode(TCP_TRANSFER_OBJECT);
    this->btClient = dynamic_cast<BitTorrentClient*>(this->hostmod);
}
void PeerWireThread::timerExpired(cMessage *timer) {
    simtime_t nextRound;
    // timers are not deleted, only rescheduled.
    int kind = timer->getKind();
    switch (kind) {
    case APP_DOWNLOAD_RATE_TIMER:
    case APP_KEEP_ALIVE_TIMER:
    case APP_SNUBBED_TIMER:
    case APP_TIMEOUT_TIMER:
    case APP_UPLOAD_RATE_TIMER:
        this->sendApplicationMessage(kind);
        break;
    default:
        //EAM :: throw std::logic_error
        printf("Unknown timer");
        break;
    }
}

// public methods
std::string PeerWireThread::getThreadId() {
    std::string out;
//    out << this->sock->getConnectionId() << "(";
//    out << this->remotePeerId << ")";
    //out << ;
    out.append(""+this->remotePeerId);
    return out;
}
void PeerWireThread::printDebugMsg(std::string s) {
//#ifdef DEBUG_MSG
    std::ostringstream out;
    if (this->remotePeerId != -1) {
        out << "peerId " << this->remotePeerId;
        out << " info " << this->infoHash;
    } else {
        out << "connId " << this->sock->getConnectionId();
    }
    out << ";" << s;

    this->btClient->printDebugMsg(out.str());
//#endif
}

// Private methods
void PeerWireThread::cancelMessages() {
    // cancel all messages that were going to be executed
    while (!this->messageQueue.empty()) {
        cObject * msg = this->messageQueue.pop();
#ifdef DEBUG_MSG
        std::string name(msg->getName());
        std::string out = "Canceled message \"" + name + "\"";
        this->printDebugMsg(out);
#endif
        delete msg;
    }
    while (!this->postProcessingAppMsg.empty()) {
        cObject * msg = this->postProcessingAppMsg.pop();
#ifdef DEBUG_MSG
        std::string name(msg->getName());
        std::string out = "Canceled post-message \"" + name + "\"";
        this->printDebugMsg(out);
#endif
        delete msg;
    }
}
bool PeerWireThread::hasMessagesToProcess() {
    return !this->messageQueue.empty();
}
simtime_t PeerWireThread::startProcessing() {
    assert(*(this->btClient->threadInProcessingIt) == this && !this->busy);
    // start processing this thread
    this->busy = true;

    // process all ApplicationMsgs, since they take zero processing time
    processAppMessages();

    // PeerWireMsg is the only type of message that take time to process.
    simtime_t processingTime = 0;
    if (!this->messageQueue.empty()) {
        cObject *o = this->messageQueue.pop();
        cMessage *messageInProcessing = static_cast<cMessage *>(o);
        assert(dynamic_cast<PeerWireMsg *>(messageInProcessing)); // not NULL
        //Valores del histograma
        processingTime = this->btClient->doubleProcessingTimeHist.random();
        //EAM :: processingTime = 0;// <- Procesamiento del histograma
        this->issueTransition(messageInProcessing);
    }
    return processingTime;
}
void PeerWireThread::finishProcessing() {
    // finish processing this thread
    this->busy = false;
#ifdef DEBUG_MSG
    this->printDebugMsg("Post-processing.");
#endif
    // process all application messages that were generated during the
    // processing of the last PeerWire message.
    while (!this->postProcessingAppMsg.empty()) {
        cObject *o = this->postProcessingAppMsg.pop();
        cMessage *msg = static_cast<cMessage *>(o);
        this->issueTransition(msg);
    }
    // Process all ApplicationMsg until a PeerWireMsg is found
    this->processAppMessages();
#ifdef DEBUG_MSG
    this->printDebugMsg("Finished processing.");
#endif

    if (this->terminating) {
        cMessage * deleteMsg = new cMessage("Delete thread");
        deleteMsg->setContextPointer(this);
        this->btClient->scheduleAt(simTime(), deleteMsg);
    }
}
void PeerWireThread::processAppMessages() {
    while (!this->messageQueue.empty()) {
        cObject * o = this->messageQueue.front();
        ApplicationMsg *appMsg = dynamic_cast<ApplicationMsg *>(o);
        if (appMsg != NULL) {
            this->messageQueue.pop();
            this->issueTransition(appMsg);
        } else {
            break; // not ApplicationMsg, so break out of loop
        }
    }
}
void PeerWireThread::sendApplicationMessage(int id) {
    char const* name;
    char const* debugMsg;

    switch (id) {
#define CASE(X) case X:\
        name = #X;\
        debugMsg = #X " issued.";\
        break
    // Content Manager events
    CASE(APP_CLOSE);
    CASE(APP_PEER_INTERESTING);
    CASE(APP_PEER_NOT_INTERESTING);
    CASE(APP_SEND_PIECE_MSG);
        // Choker events
    CASE(APP_CHOKE_PEER);
    CASE(APP_UNCHOKE_PEER);
        // TCP events
    CASE(APP_TCP_ACTIVE_CONNECTION);
    CASE(APP_TCP_PASSIVE_CONNECTION);
    CASE(APP_TCP_LOCAL_CLOSE);
    CASE(APP_TCP_REMOTE_CLOSE);
        // timers
    CASE(APP_DOWNLOAD_RATE_TIMER);
    CASE(APP_KEEP_ALIVE_TIMER);
    CASE(APP_SNUBBED_TIMER);
    CASE(APP_TIMEOUT_TIMER);
    CASE(APP_UPLOAD_RATE_TIMER);
    default:
        //EAM :: throw std::logic_error
        std::cerr << "Trying to call a non-existing application transition";
        break;
    }
#undef CASE
#ifdef DEBUG_MSG
    this->printDebugMsg(std::string(debugMsg));
#endif
    ApplicationMsg* appMessage = new ApplicationMsg(name);
    appMessage->setMessageId(id);
    // If this application message is being issued in the same event as the
    // last one, then it was issued as a result of a transition in the state
    // machines. Transitions cannot be issued from inside another transition,
    // so insert it in the queue for post-processing
    if (this->lastEvent < simulation.getEventNumber()) {
        this->issueTransition(appMessage);
    } else {
        this->postProcessingAppMsg.insert(appMessage);
    }
    // Ensure the processing of the first message to arrive.
    if (!this->busy) this->btClient->processNextThread();
}
void PeerWireThread::issueTransition(cMessage const* msg) { // get message Id
    ApplicationMsg const* appMsg = dynamic_cast<ApplicationMsg const*>(msg);
    PeerWireMsg const* pwMsg = dynamic_cast<PeerWireMsg const*>(msg);
    this->lastEvent = simulation.getEventNumber();
    int msgId  = -1;

    if (appMsg) {
        msgId = appMsg->getMessageId();
    } else if (pwMsg) {
        msgId = pwMsg->getMessageId();
    } else {
        // consume application message
        delete msg;
        msg = NULL;
        //EAM :: throw std::logic_error
        std::cerr << "Wrong type of message\n";
    }
#ifdef DEBUG_MSG
    std::string msgName = msg->getName();
    std::string debugString = "Processing " + msgName;
    this->printDebugMsg(debugString);
#endif
    try {

        switch (msgId) {
#define CONST_CAST(X) static_cast<X const&>(*msg)
#define CASE_CONN( X , Y ) case X:\
            this->connectionSm.Y;\
            break
#define CASE_DOWNLOAD( X , Y ) case X:\
            this->connectionSm.incomingPeerWireMsg();\
            this->downloadSm.Y;\
            break
#define CASE_APP_DOWNLOAD( X , Y ) case X:\
            this->downloadSm.Y;\
            break
#define CASE_UPLOAD( X , Y ) case X:\
            this->connectionSm.incomingPeerWireMsg();\
            this->uploadSm.Y;\
            break
#define CASE_APP_UPLOAD( X , Y ) case X:\
            this->uploadSm.Y;\
            break

        // connectionSm PeerWire transitions
        CASE_CONN(PW_KEEP_ALIVE_MSG, incomingPeerWireMsg());
        CASE_CONN(PW_HANDSHAKE_MSG, handshakeMsg(CONST_CAST(Handshake)));
            // connectionSM Application transitions
        CASE_CONN(APP_CLOSE, applicationClose());
        CASE_CONN(APP_TCP_REMOTE_CLOSE, remoteClose());
        CASE_CONN(APP_TCP_LOCAL_CLOSE, localClose());
        CASE_CONN(APP_TCP_ACTIVE_CONNECTION, tcpActiveConnection());
        CASE_CONN(APP_TCP_PASSIVE_CONNECTION, tcpPassiveConnection());
            // connectionSM timers
        CASE_CONN(APP_KEEP_ALIVE_TIMER, keepAliveTimer());
        //CASE_CONN(APP_TIMEOUT_TIMER, keepAliveTimer());
        CASE_CONN(APP_TIMEOUT_TIMER, timeout());

            // downloadSm PeerWire transitions
        CASE_DOWNLOAD(PW_CHOKE_MSG, chokeMsg());
        CASE_DOWNLOAD(PW_UNCHOKE_MSG, unchokeMsg());
        CASE_DOWNLOAD(PW_HAVE_MSG, haveMsg(CONST_CAST(HaveMsg)));
        CASE_DOWNLOAD(PW_BITFIELD_MSG, bitFieldMsg(CONST_CAST(BitFieldMsg)));
        CASE_DOWNLOAD(PW_PIECE_MSG, pieceMsg(CONST_CAST(PieceMsg)));
            // downloadSm Application transitions
        CASE_APP_DOWNLOAD(APP_PEER_INTERESTING, peerInteresting());
        CASE_APP_DOWNLOAD(APP_PEER_NOT_INTERESTING, peerNotInteresting());
            // downloadSm timers
        CASE_APP_DOWNLOAD(APP_DOWNLOAD_RATE_TIMER, downloadRateTimer());
        CASE_APP_DOWNLOAD(APP_SNUBBED_TIMER, snubbedTimer());

            // uploadSm PeerWire transitions
        CASE_UPLOAD(PW_INTERESTED_MSG, interestedMsg());
        CASE_UPLOAD(PW_NOT_INTERESTED_MSG, notInterestedMsg());
        CASE_UPLOAD(PW_REQUEST_MSG, requestMsg(CONST_CAST(RequestMsg)));
        CASE_UPLOAD(PW_CANCEL_MSG, cancelMsg(CONST_CAST(CancelMsg)));
            // uploadSm Application transitions
        CASE_APP_UPLOAD(APP_CHOKE_PEER, chokePeer());
        CASE_APP_UPLOAD(APP_UNCHOKE_PEER, unchokePeer());
        CASE_APP_UPLOAD(APP_SEND_PIECE_MSG, sendPieceMsg());
            // uploadSm timers
        CASE_APP_UPLOAD(APP_UPLOAD_RATE_TIMER, uploadRateTimer());
        default:

            break;

#undef CONST_CAST
#undef CASE_CONN
#undef CASE_APP_CONN
#undef CASE_DOWNLOAD
#undef CASE_APP_DOWNLOAD
#undef CASE_UPLOAD
#undef CASE_APP_UPLOAD
        }

        delete msg;
        msg = NULL; // consume the message
    } catch (statemap::TransitionUndefinedException & e) {
//        std::cerr << "[A] Wrong type of message :: "<< msgId << "\n";
        delete msg;
        msg = NULL; // consume the message
//
//        std::ostringstream out;
//        out << e.what();
//        out << " - Transition " << e.getTransition();
//        out << " in state " << e.getState();
        std::cerr << "\n * " << msgId << " | "<< e.what() << " | -Transition "<< e.getTransition() <<" in state -> "<< e.getState() <<"\n";
//        printf("\n Error :: - Transition  in state *** ");
        //EAM :: throw cException(out.str().c_str());
    } catch (statemap::StateUndefinedException &e) {
//        std::cerr << "[B] Wrong type of message :: " << msgId << "\n";
//        std::cerr << e.what() << " - Transition " <<  msg->getName() << " called " << "\n";
//        std::ostringstream out;
//        out << e.what();
//        out << " - Transition " << msg->getName();
//        out << " called";

        delete msg;
        msg = NULL;

        std::cerr << "** Wrong type of message :: "<< msgId<< " ??? [ " << e.what() << " ]";
//        std::cerr << "\n[B]" << e.what() << " :: -Transition "<< msg->getName() <<" :: called\n";
//        printf("\n Error :: - Transition  in state *** ");

        //throw cException(out.str().c_str());
    } catch (std::invalid_argument &e) {
        delete msg;
        msg = NULL; // consume the message
        //throw std::logic_error
        std::cerr << "Passed wrong argument to the state machine.";
//        printf("Passed wrong argument to the state machine.");
    }
}
void PeerWireThread::sendPeerWireMessage(cMessage * msg) {
    //EAM :: cPacket * dato = dynamic_cast<cPacket *>(msg);
    PeerWireMsgBundle * bundleMsg = dynamic_cast<PeerWireMsgBundle *>(msg);

    PeerWireMsg* peerWireMsg = dynamic_cast<PeerWireMsg *>(msg);
    assert(bundleMsg || peerWireMsg); // either one or the other

    if (bundleMsg != NULL) {
        // unpacks the bundle and inserts all messages in the messageQueue
        cPacketQueue * bundle = bundleMsg->getBundle();
        while (!bundle->empty()) {
            this->messageQueue.insert(bundle->pop());
        }
        delete bundle;
        delete bundleMsg; // the bundle is no longer needed, so delete it
    } else {
        this->messageQueue.insert(peerWireMsg);
    }

    // Try to ensure the processing of the first message to arrive.
    if (!this->busy) this->btClient->processNextThread();
}
//void PeerWireThread::oldUnchoked() {
//    this->btClient->setOldUnchoked(true, this->infoHash, this->remotePeerId);
//}
