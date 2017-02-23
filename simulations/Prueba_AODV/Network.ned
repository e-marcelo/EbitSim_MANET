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
//Importación de paquetes 
import br.larc.usp.simulations.Prueba_AODV.nodo.PeerEx;
import br.larc.usp.simulations.Prueba_AODV.controlador.Controller;
import inet.world.radio.ChannelControl;
import inet.nodes.inet.AdhocHost;
import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;



//
// TODO documentation
//
network Network
{
    parameters:
    @display("bgb=400,300,white;bgg=100,2,grey80");
    int numPeers = 4; //Número de pares
    
    
    submodules:
        peer[numPeers]: PeerEx {
        	IPForward = default(true);
        	numTcpApps = 1;
        	@display("is=vs;p=296,71;i=old/x_active");
        }
        channelControl: ChannelControl {
            parameters:
                @display("p=5,8");
        }
        controller: Controller {
            @display("p=80,50");
        }
        flatNetworkConfigurator: FlatNetworkConfigurator {

            @display("p=17,134");
        }
        tracker: AdhocHost {
            @display("i=old/x_blank;is=vs;p=2,2");
        }
        
}

