// EbitSim - Enhanced BitTorrent Simulation
// This program is under the terms of the Creative Commons
// Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
//
// You are free:
//
//    to Share - to copy, distribute and transmit the work
//    to Remix - to adapt the work
//
// Under the following conditions:
//
//    Attribution - You must attribute the work in the manner specified by the
//    author or licensor (but not in any way that suggests that they endorse you
//    or your use of the work).
//
//    Noncommercial - You may not use this work for commercial purposes.
//
//    Share Alike - If you alter, transform, or build upon this work, you may
//    distribute the resulting work only under the same or similar license to
//    this one.
//
// With the understanding that:
//
//    Waiver - Any of the above conditions can be waived if you get permission
//    from the copyright holder.
//
//    Public Domain - Where the work or any of its elements is in the public
//    domain under applicable law, that status is in no way affected by the
//    license.
//
//    Other Rights - In no way are any of the following rights affected by the
//    license:
//        - Your fair dealing or fair use rights, or other applicable copyright
//          exceptions and limitations;
//        - The author's moral rights;
//        - Rights other persons may have either in the work itself or in how
//          the work is used, such as publicity or privacy rights.
//
//    Notice - For any reuse or distribution, you must make clear to others the
//    license terms of this work. The best way to do this is with a link to this
//    web page. <http://creativecommons.org/licenses/by-nc-sa/3.0/>
//
// Author:
//     Pedro Manoel Fabiano Alves Evangelista <pevangelista@larc.usp.br>
//     Supervised by Prof Tereza Cristina M. B. Carvalho <carvalho@larc.usp.br>
//     Graduate Student at Escola Politecnica of University of Sao Paulo, Brazil
//
// Contributors:
//     Marcelo Carneiro do Amaral <mamaral@larc.usp.br>
//     Victor Souza <victor.souza@ericsson.com>
//
// Disclaimer:
//     This work is part of a Master Thesis developed by:
//        Pedro Evangelista, graduate student at
//        Laboratory of Computer Networks and Architecture
//        Escola Politecnica
//        University of Sao Paulo
//        Brazil
//     and supported by:
//        Innovation Center
//        Ericsson Telecomunicacoes S.A., Brazil.
//
// UNLESS OTHERWISE MUTUALLY AGREED TO BY THE PARTIES IN WRITING AND TO THE
// FULLEST EXTENT PERMITTED BY APPLICABLE LAW, LICENSOR OFFERS THE WORK AS-IS
// AND MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND CONCERNING THE WORK,
// EXPRESS, IMPLIED, STATUTORY OR OTHERWISE, INCLUDING, WITHOUT LIMITATION,
// WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
// NONINFRINGEMENT, OR THE ABSENCE OF LATENT OR OTHER DEFECTS, ACCURACY, OR THE
// PRESENCE OF ABSENCE OF ERRORS, WHETHER OR NOT DISCOVERABLE. SOME
// JURISDICTIONS DO NOT ALLOW THE EXCLUSION OF IMPLIED WARRANTIES, SO THIS
// EXCLUSION MAY NOT APPLY TO YOU.

#ifndef __TRACKERAPP_H__
#define __TRACKERAPP_H__

#include <omnetpp.h>
#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>

#include "TCPSrvHostApp.h"
#include "messages/AnnounceRequestMsg_m.h"
#include "messages/AnnounceResponseMsg_m.h"

struct TorrentMetadata {
    int numOfPieces;
    int numOfSubPieces;
    int subPieceSize;
    int infoHash;
};

typedef std::set<PeerInfo> SwarmPeerList;

// TODO if the peer don't respond during an interval, remove it from list
class TrackerApp: public TCPSrvHostApp {
public:
    friend class PeerConnectionThread;

    TrackerApp();
    virtual ~TrackerApp();
    TorrentMetadata const& getTorrentMetaData(std::string contentName);
private:
    bool debugFlag;
    int totalBytesUploaded;
    int totalBytesDownloaded;

    std::map<std::string, TorrentMetadata> contents; //The key is the content name from XML file
    std::map<int, SwarmPeerList> swarms; // The key is the infoHash

    //Signal
    simsignal_t seederSignal;
private:
    //! Print a debug message to the passed ostream, which defaults to clog.
    void printDebugMsg(std::string s);
    std::vector<PeerInfo const*> getListOfPeers(const PeerInfo & peerInfo,
            int infoHash, unsigned int numberOfPeers) const;
    std::vector<PeerInfo const*> getRandomPeers(PeerInfo const& peerInfo,
            int infoHash, unsigned int numberOfPeers) const;
    void updatePeerStatus(PeerInfo const& peerInfo, int infoHash,
            AnnounceType status);
    void startupPeerList();
    //!@name Signal registration and subscription methods
    //@{
    //! Register all signals this module is going to emit.
    void registerEmittedSignals();
    //@}
protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
