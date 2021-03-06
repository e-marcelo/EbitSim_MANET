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

package br.larc.usp.client;

import inet.applications.ITCPApp;
//EAM :: TCPApp --> ITCPApp
simple SwarmManager like ITCPApp
{
    parameters:
        @signal[SwarmManager_EnterSwarm];
        @signal[SwarmManager_LeaveSwarm];
        @signal[SwarmManager_EmittedPeerId];
        @statistic[SwarmManager_EnteredSwarm](
            title="Enter Swarm Instant";
            record=last
        );
        
        // X axis
        @statistic[ContentManager_DownloadMarkPeerId](
            title="Download Time Abscissa";
            unit=s;
            record=vector
        );
//        // 25%
       @statistic[ContentManager_25_percentDownloadMarkTime](
            title="25% Download Time Ordinate";
            unit=s;
            record=histogram
        );
//        // 50%
        @statistic[ContentManager_50_percentDownloadMarkTime](
            title="50% Download Time Ordinate";
            unit=s;
            record=histogram
        );
//        // 75%
        @statistic[ContentManager_75_percentDownloadMarkTime](
            title="75% Download Time Ordinate";
            unit=s;
            record=histogram
        );
//        // 100%
        @statistic[ContentManager_100_percentDownloadMarkTime](
            title="100% Download Time Ordinate";
            unit=s;
            record=histogram
        );       
        // statistics from the ContentManager
        @statistic[ContentManager_TotalBytesDownloaded](
            title="Swarm Total of Bytes Downloaded";
            record=vector
        );
        @statistc[ContentManager_TotalDownloadTime](
            title="Content Download Time";
            record=vector
        );
        @statistic[ContentManager_PieceDownloadTime](
            title="Swarm Piece Download Time";
            record=histogram
        );
        @statistic[SwarmManagerDownloadRate](
            title="Swarm Downloaded Rate";
            record=vector
        );
        @statistic[SwarmManagerUploadRate](
            title="Swarm Upload Rate";
            record=vector
        );

        @display("i=misc/globe,red");
        // own parameters
        bool debugFlag = default(false);
        // The number of Peers the Client requests to the Tracker
        int numWant = default(50);
        double normalRefreshInterval @unit(s) = default(180s);
        double minimumRefreshInterval @unit(s) = default(20s);
        // probability the Peer will stay in the swarm after completion
        double remainingSeeders;
    gates:
        input userCommand @directIn;
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}
