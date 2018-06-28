//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package br.larc.usp.simulations;

import ned.DatarateChannel;

channel ethernetline extends DatarateChannel
{
    parameters:
        delay = 0.1us;
}

network BasicNetwork
{
    parameters:
        int numPeers;
        int staticPeerA; //1
        int staticPeerB; //2
        int staticPeerC; //3
        int staticPeerD; //4

//        int staticPeerE; //5
//        int staticPeerF; //6
//        int staticPeerG; //7
//        int staticPeerH; //8


//        int staticPeerI = -1; //9
//        int staticPeerJ = -1; //10
//        int staticPeerK = -1; //11
//        int staticPeerL = -1; //12
//        int staticPeerM = -1; //13
//        int staticPeerN = -1; //14
        
}
