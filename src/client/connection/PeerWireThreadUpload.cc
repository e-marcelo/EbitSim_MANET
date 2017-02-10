
#include "PeerWireThread.h"

#include <boost/lexical_cast.hpp>

#include "BitTorrentClient.h"
#include "ContentManager.h"
#include "Choker.h"
#include "PeerWire_m.h"

// Upload State Machine methods
void PeerWireThread::cancelPiece(CancelMsg const& msg) {
    //TODO
    throw std::logic_error("Cancel not implemented");
}
void PeerWireThread::cancelUploadRequests() {
    assert(this->contentManager);
    this->contentManager->cancelUploadRequests(this->remotePeerId);
}
void PeerWireThread::calculateUploadRate() {
    assert(this->contentManager);
    unsigned long totalUploaded = this->contentManager->getTotalUploaded(
        this->remotePeerId);
    double upRate = this->btClient->updateUploadRate(this->infoHash,
        this->remotePeerId, totalUploaded);
#ifdef DEBUG_MSG
    std::string out = "Upload rate: "
        + boost::lexical_cast<std::string>(upRate);
    this->printDebugMsg(out);
#endif
}
void PeerWireThread::callChokeAlgorithm() {
    // The choke algorithm is called when the upload machine stops, even when
    // it stopped because the swarm has been deleted
    assert(this->choker);
    this->choker->callChokeAlgorithm();
}
void PeerWireThread::addPeerToChoker() {
    assert(this->choker);
    this->choker->addInterestedPeer(this->remotePeerId);
}
ChokeMsg * PeerWireThread::getChokeMsg() {
    return new ChokeMsg("ChokeMsg");
}
void PeerWireThread::requestPieceMsg(RequestMsg const& msg) {
    assert(this->contentManager);
    // the Peer must ask for pieces that the
    this->contentManager->requestPieceMsg(this->remotePeerId, msg.getIndex(),
        msg.getBegin(), msg.getReqLength());
}
PieceMsg * PeerWireThread::getPieceMsg() {
    assert(this->contentManager);
    // the Peer must ask for pieces that the
    PieceMsg * pieceMsg = this->contentManager->getPieceMsg(this->remotePeerId);
    return pieceMsg;
}
UnchokeMsg * PeerWireThread::getUnchokeMsg() {
    return new UnchokeMsg("UnchokeMsg");
}
void PeerWireThread::renewUploadRateTimer() {
    cancelEvent(&this->uploadRateTimer);

    scheduleAt(simTime() + this->btClient->uploadRateInterval,
        &this->uploadRateTimer);
}
void PeerWireThread::setInterested(bool interested) {
    this->btClient->setInterested(interested, this->infoHash,
        this->remotePeerId);
}
void PeerWireThread::startUploadTimers() {
    this->stopUploadTimers();
    scheduleAt(simTime() + this->btClient->uploadRateInterval,
        &this->uploadRateTimer);
}
void PeerWireThread::stopUploadTimers() {
    cancelEvent(&this->uploadRateTimer);
}
