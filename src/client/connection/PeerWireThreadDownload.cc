
#include "PeerWireThread.h"

#include <boost/lexical_cast.hpp>

#include "BitTorrentClient.h"
#include "ContentManager.h"
#include "PeerWire_m.h"
#include "PeerWireMsgBundle_m.h"

// Download State Machine methods
void PeerWireThread::addBitField(BitFieldMsg const& msg) {
    assert(this->contentManager);
    this->contentManager->addPeerBitField(msg.getBitField(),
        this->remotePeerId);
}
void PeerWireThread::calculateDownloadRate() {
    assert(this->contentManager);
    unsigned long totalDownloaded = this->contentManager->getTotalDownloaded(
        this->remotePeerId);
    double downRate = this->btClient->updateDownloadRate(this->infoHash,
        this->remotePeerId, totalDownloaded);
#ifdef DEBUG_MSG
    std::string out = "Download rate: "
        + boost::lexical_cast<std::string>(downRate);
    this->printDebugMsg(out);
#endif
}
void PeerWireThread::cancelDownloadRequests() {
    assert(this->contentManager);
    this->contentManager->cancelDownloadRequests(this->remotePeerId);
}
InterestedMsg * PeerWireThread::getInterestedMsg() {
    return new InterestedMsg("InterestedMsg");
}
NotInterestedMsg * PeerWireThread::getNotInterestedMsg() {
    return new NotInterestedMsg("NotInterestedMsg");

}
PeerWireMsgBundle * PeerWireThread::getRequestMsgBundle() {
    assert(this->contentManager);
    PeerWireMsgBundle * bundle = this->contentManager->getNextRequestBundle(
        this->remotePeerId);
#ifdef DEBUG_MSG
    if (bundle == NULL) {
        this->printDebugMsg("Didn't return a Bundle");
    }
#endif
    return bundle;
}
void PeerWireThread::processBlock(PieceMsg const& msg) {
    assert(this->contentManager);
    this->contentManager->processBlock(this->remotePeerId, msg.getIndex(),
        msg.getBegin(), msg.getBlockSize());
}
void PeerWireThread::processHaveMsg(HaveMsg const& msg) {
    assert(this->contentManager);
    this->contentManager->processHaveMsg(msg.getIndex(), this->remotePeerId);
}
void PeerWireThread::renewDownloadRateTimer() {
    cancelEvent(&this->downloadRateTimer);
    scheduleAt(simTime() + this->btClient->downloadRateInterval,
        &this->downloadRateTimer);
}
void PeerWireThread::renewSnubbedTimer() {
    cancelEvent(&this->snubbedTimer);
    scheduleAt(simTime() + this->btClient->snubbedInterval,
        &this->snubbedTimer);
}
void PeerWireThread::setSnubbed(bool snubbed) {
#ifdef DEBUG_MSG
    if (snubbed) {
        this->printDebugMsg("Peer snubbed");
    } else {
        this->printDebugMsg("Peer not snubbed");
    }
#endif
    // snubbed starts as false and is set to true if the timeout occurs
    this->btClient->setSnubbed(snubbed, this->infoHash, this->remotePeerId);
}
void PeerWireThread::startDownloadTimers() {
    this->stopDownloadTimers();

    scheduleAt(simTime() + this->btClient->snubbedInterval,
        &this->snubbedTimer);
    scheduleAt(simTime() + this->btClient->downloadRateInterval,
        &this->downloadRateTimer);
}
void PeerWireThread::stopDownloadTimers() {
    cancelEvent(&this->snubbedTimer);
    cancelEvent(&this->downloadRateTimer);
}
