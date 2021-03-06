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

#ifndef __CONTENTTABLE_H__
#define __CONTENTTABLE_H__

#include <omnetpp.h>
#include <signal.h>
#include <boost/dynamic_bitset.hpp>

#include "BitField.h"
#include "RarestPieceCounter.h"

class BitFieldMsg;
class BitTorrentClient;
class PeerWireMsgBundle;
class PieceMsg;
class RequestMsg;
//class Statistics;
/**
 * Controls the download and upload of blocks from a Swarm. Also stores the
 * BitField of all connected Peers.
 *
 * The content is divided into pieces, and these pieces are divided into smaller
 * parts, called blocks (more at http://wiki.theory.org/BitTorrentSpecification).
 * Peers actually exchange blocks, not pieces. In the real implementations, the
 * content size cannot be ensured to be a multiple of the piece size, making the
 * last piece not necessarily the same size as the others. This simulation don't
 * take that into consideration, that is, the content size is always a multiple
 * of the piece size.
 * <pre>
 * ┌───────────────────────┐
 * │      Full Content     │
 * ├───────────┬───────────┤
 * │     p1    │     p2    │
 * ├──┬──┬──┬──┼──┬──┬──┬──┤
 * │b1│b2│b3│b4│b1│b2│b3│b4│
 * └──┴──┴──┴──┴──┴──┴──┴──┘
 * </pre>
 * Pieces are always downloaded concurrently from a set of Peers, as this is the
 * main idea around BitTorrent, but blocks can be downloaded in two ways:
 * <ol>
 * <li> All blocks of a piece are acquired from the same Peer.
 * <li> Blocks are concurrently downloaded from a set of Peers.
 * </ol>
 *
 * The usual way is to download all blocks from the same piece, and this is the
 * way the ContentManager is implemented.
 */

class ContentManager: public cSimpleModule {
public:
    ContentManager();
    virtual ~ContentManager();

    //!@name Methods used by the PeerWireThread
    //@{
    /*!
     * Set an empty BitField for the Peer identified by the connection Id.
     */
    void addEmptyBitField(int peerId);
    /*!
     * Set the BitField of the Peer identified by the connection Id.
     * Return true if the BitField is valid, where validity means that the
     * BitField has the expected size and, if the Client is a seeder, the
     * BitField is not of a seeder(in which case the connection bears no purpose).
     */
    void addPeerBitField(const BitField & bitField, int peerId);
    //! Empty the pending request queue.
    void cancelDownloadRequests(int peerId);
    //! Cancel the pieces requested by the Peer.
    void cancelUploadRequests(int peerId);
    //! Schedule to send the BitField of the Client.
    BitFieldMsg* getClientBitFieldMsg() const;
    /*!
     * Schedule to send a bundle of request messages if the pending request
     * queue is empty, or NULL otherwise.
     */
    PeerWireMsgBundle* getNextRequestBundle(int peerId);
    /*!
     * Schedule to send a PieceMsg if the piece exists in the client BitField,
     * or NULL otherwise.
     *
     * @param peerId[in] The id of the Peer.
     * @param index[in] The index of the piece message.
     * @param begin[in] The beginning of the sub piece.
     * @param reqLength[in] The size of the request.
     */
    void requestPieceMsg(int peerId, int index, int begin, int reqLength);
    /*!
     * Get the PieceMsg created for the Peer's request.
     *
     * @param infoHash[in] The infoHash that identifies the swarm.
     * @param peerId[in] The id of the Peer.
     */
    PieceMsg* getPieceMsg(int peerId);
    //! Return the total downloaded from the Peer with the passed peerId, in bytes.
    unsigned long getTotalDownloaded(int peerId) const;
    //! Return the total uploaded to the Peer with the passed peerId, in bytes.
    unsigned long getTotalUploaded(int peerId) const;
    bool isBitFieldEmpty() const;
    //! Update the client BitField.
    void processBlock(int peerId, int pieceIndex, int begin, int blockSize);
    /*!
     * Add the piece to the Peer BitField, and then verify if the Peer became
     * interesting.
     */
    void processHaveMsg(int index, int peerId);
    //! Remove this Peer from the content manager.
    void removePeerBitField(int peerId);
    //@}
private:
    //!@name Pimpl classes
    //@{
    class PieceBlocks;
    class TokenBucket;
    friend class TokenBucket;
    //! Token Bucket object
    TokenBucket * tokenBucket;
    //@}

    //!@name Pointers to other modules
    //@{
    BitTorrentClient *bitTorrentClient;
    //@}

    //!@name Parameters
    //@{
    //! The size of a sub-piece, in KB.
    int subPieceSize;
    //! Set to true to print debug messages
    bool debugFlag;
    //! The number of have messages  that can be sent together.
    unsigned int haveBundleSize;
    //! The number of sub-pieces in each piece.
    int numberOfSubPieces;
    //! The number of pieces the content is divided.
    int numberOfPieces;
    //! The number of request messages that can be sent together.
    unsigned int requestBundleSize;
    //! Total downloaded from all Peers
    int totalBytesDownloaded;
    //! Total uploaded from all Peers
    int totalBytesUploaded;
    //! The infoHash of the swarm this ContentManager is serving.
    int infoHash;
    //! The peerId of this Client.
    int localPeerId;
    //@}

    //!@name Download related
    //@{
    /*!
     * Map the peer id to the pieces requested to it. May have more than one
     * piece requested per Peer, but never the same piece requested by two
     * different peers.
     */
    std::multimap<int, int> requestedPieces;
    /*!
     * Count of pending requests per Peer. Maximum of requestBundleSize
     */
    std::map<int, int> numPendingRequests;
    /*!
     * Store the blocks of incomplete pieces. When the piece becomes complete,
     * it is removed from the missingBlocks and added to the Client's BitField.
     */
    std::map<int, PieceBlocks> missingBlocks;
    //@}

    //!@name Bitfield related
    //@{
    //! BitField with all pieces fully acquired.
    BitField clientBitField;
    /*!
     * The BitField of each connected Peer, acquired during the Handshake and
     * updated with the reception of Have Messages.
     */
    std::map<int, BitField> peerBitFields;
    /*!
     * Contain all Peers in which the Client shows interest (those that have a
     * piece not owned by the Client). This set changes with the arrival of
     * Have Messages (when the peer becomes interesting) and with the completion
     * of a piece (when one or more peers stop being interesting).
     */
    std::set<int> interestingPeers;
    //! Object that counts the pieces and return the rarest ones.
    RarestPieceCounter rarestPieceCounter;
    //@}

    //!@name Statistics
    //@{
    //! Map the start times of all requested pieces.
    std::map<int, simtime_t> pieceRequestTime;
    //! Map the total downloaded from a Peer to its peerId.
    std::map<int, unsigned long> totalDownloadedByPeer;
    //! Map the total uploaded from a Peer to its peerId.
    std::map<int, unsigned long> totalUploadedByPeer;
    //@}

    //!@name Signals
    //@{
    simsignal_t becameSeeder_Signal; // t seeder
    simsignal_t pieceDownloadTime_Signal; // dt last piece download
    simsignal_t totalBytesDownloaded_Signal;
    simsignal_t totalBytesUploaded_Signal; // inc bytes

    //    simsignal_t startDownloadMarkTime_Signal;
    simsignal_t _25_percentDownloadTime_Signal;
    simsignal_t _50_percentDownloadTime_Signal;
    simsignal_t _75_percentDownloadTime_Signal;
    simsignal_t _100_percentDownloadTime_Signal;

    // t this peerId emitted a signal
    simsignal_t emittedPeerId_Signal;

    // t this piece was downloaded
    simsignal_t pieceDownloaded_Signal;

    // FIXME use these completion marks instead of the bools below
    // enum {_0, _25, _50, _75} mark;

    bool firstMarkEmitted;
    bool secondMarkEmitted;
    bool thirdMarkEmitted;

    simtime_t downloadStartTime;
    //@}
private:
    //!@name Token bucket related methods
    //@{
    /*!
     * Send the message if there are enough tokens in the bucket.
     * Return true if the message was sent.
     */
    void tryToSend();
    //@}

    //!@name
    //@{
    /*!
     * Create a RequestMsg and insert it in the bundle.
     * Requirements: The bundle's length is smaller than this->requestBundleSize.
     *
     * @param bundle[inout] The bundle where to insert the requests.
     * @param pieceIndex[in] The piece index.
     * @param blockIndex[in] The block index.
     */
    void insertRequestInBundle(cPacketQueue *const bundle,
        std::ostringstream & bundleMsgName, int pieceIndex, int blockIndex);
    /*!
     * Insert block requests from pieces being downloaded by this peer.
     *
     * @param bundle [inout] The bundle where to insert the requests.
     * @param bundleMsgName [inout] A stringstream used to compose the name of
     * the bundle.
     * @param peerId [in] The id of the Peer.
     */
    void getRemainingPieces(cPacketQueue *const bundle,
        std::ostringstream & bundleMsgName, int peerId, int requestBundleSize);
    /*!
     *
     * @param bundle [inout] The bundle where to insert the requests.
     * @param peerId [in] The bundle where to insert the requests.
     * @param interestingBitField [in] The bundle where to insert the requests.
     */
    void getInterestingPieces(cPacketQueue *const bundle,
        std::ostringstream & bundleMsgName, int peerId, int requestBundleSize);
    //! Return true if the peer's BitField is interesting
    bool isPeerInteresting(int peerId) const;
    //! Generate download statistics.
    void generateDownloadStatistics(int pieceIndex);
    //! Return a list of available blocks from the passed Peer.
//    std::list<std::pair<int, int> > requestAvailableBlocks(int peerId);
    //! Verify which Peers are not interesting for the Client and send
    //! NotInterestedMsg to them.
    void verifyInterestOnAllPeers();
    //@}

    //! Find if this piece was requested, then remove it from the set.
//    void eraseRequestedPiece(int pieceIndex);

    //!@name Signal registration and subscription methods
    //@{
    //! Register all signals this module is going to emit.
    void registerEmittedSignals();
    //@}

    //!@name module methods
    //@{
    //! Print a debug message to clog.
    void printDebugMsg(std::string s);
    void updateStatusString();
    //@}
protected:
    virtual void handleMessage(cMessage *msg);
    virtual void initialize();
};

#endif
