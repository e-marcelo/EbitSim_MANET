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

package br.larc.usp.simulations.Prueba_AODV;

import inet.nodes.inet.AdhocHost;
import inet.world.radio.ChannelControl;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;


network AODV_test
{
    parameters:
        int numHosts;
        int numFixHosts;
    submodules:

        fixhost[numFixHosts]: AdhocHost {
            parameters:
                //@display("i=device/pocketpc_s;r=50,,#707070");
                @display("is=vs");
        }
        host[numHosts]: AdhocHost {
            parameters:
                //@display("i=device/pocketpc_s;r=50,,#707070");
                @display("is=vs");
        }
        channelControl: ChannelControl {
            parameters:
                @display("p=60,50;i=misc/sun");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=140,50;i=block/cogwheel_s");
        }
}
