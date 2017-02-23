

#ifndef __CLIENTBEHAVIOR_H__
#define __CLIENTBEHAVIOR_H__

#include <omnetpp.h>
#include <clistener.h>
#include <list>

//#include "TrackerApp.h"

//class SwarmManager;

/**
 * This module is responsible for controlling the Client's behavior: what content
 * to download, when to start downloading, how long should the seeding time be.
 */
class Controller: public cSimpleModule, cListener {
public:
    //!cListener overloaded method
    void receiveSignal(cComponent *source, simsignal_t signalID, long l);
public:
    Controller();
    virtual ~Controller();
    void emitEnterTime(simtime_t enterTime);
private:
    //! Set to true to print debug messages
    bool debugFlag;
    int endPeerDownload;
    int numNodesTotal;

    //!@name Signals
    //@{
    simsignal_t enterTime_Signal; //! Emitted the instant a peer enters the swarm
    //@}
private:
    void endUserDownload(cMessage * msg);
    /*! Return the torrent metadata for the passed content. Must be called after
     * init stage 0.
     */
    TorrentMetadata getTorrentMetadata(const char* content);

    //! Print a debug message to clog.
    void printDebugMsg(std::string s);
    void updateStatusString();
    //!@name Signal registration and subscription methods
    //@{
    //! Subscribe to signals.
    void subscribeToSignals();
    //@}
protected:
    int numInitStages() const;
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif
