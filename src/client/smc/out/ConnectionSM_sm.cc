//
// ex: set ro:
// DO NOT EDIT.
// generated by smc (http://smc.sourceforge.net/)
// from file : ConnectionSM.sm
//


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


#include "PeerWireThread.h"
#include "out/ConnectionSM_sm.h"

using namespace statemap;

// Static class declarations.
ConnectionMap_Unconnected ConnectionMap::Unconnected("ConnectionMap::Unconnected", 0);
ConnectionMap_HandshakeSent ConnectionMap::HandshakeSent("ConnectionMap::HandshakeSent", 1);
ConnectionMap_WaitHandshake ConnectionMap::WaitHandshake("ConnectionMap::WaitHandshake", 2);
ConnectionMap_Connected ConnectionMap::Connected("ConnectionMap::Connected", 3);
ConnectionMap_ClosingConnection ConnectionMap::ClosingConnection("ConnectionMap::ClosingConnection", 4);
ConnectionMap_LocalClosed ConnectionMap::LocalClosed("ConnectionMap::LocalClosed", 5);
ConnectionMap_RemoteClosed ConnectionMap::RemoteClosed("ConnectionMap::RemoteClosed", 6);
ConnectionMap_Closed ConnectionMap::Closed("ConnectionMap::Closed", 7);

void ConnectionSMState::applicationClose(ConnectionSMContext& context)
{
    Default(context);
}

void ConnectionSMState::handshakeMsg(ConnectionSMContext& context, Handshake const& hs)
{
    Default(context);
}

void ConnectionSMState::incomingPeerWireMsg(ConnectionSMContext& context)
{
    Default(context);
}

void ConnectionSMState::keepAliveTimer(ConnectionSMContext& context)
{
    Default(context);
}

void ConnectionSMState::localClose(ConnectionSMContext& context)
{
    Default(context);
}

void ConnectionSMState::outgoingPeerWireMsg(ConnectionSMContext& context, cPacket * msg)
{
    Default(context);
}

void ConnectionSMState::remoteClose(ConnectionSMContext& context)
{
    Default(context);
}

void ConnectionSMState::tcpActiveConnection(ConnectionSMContext& context)
{
    Default(context);
}

void ConnectionSMState::tcpPassiveConnection(ConnectionSMContext& context)
{
    Default(context);
}

void ConnectionSMState::timeout(ConnectionSMContext& context)
{
    Default(context);
}

void ConnectionSMState::Default(ConnectionSMContext& context)
{
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "TRANSITION   : Default"
            << std::endl;
    }

    throw (
        TransitionUndefinedException(
            context.getState().getName(),
            context.getTransition()));

}

void ConnectionMap_Default::incomingPeerWireMsg(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::Default"
                << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::Default::incomingPeerWireMsg()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::Default::incomingPeerWireMsg()"
            << std::endl;
    }


}

void ConnectionMap_Default::keepAliveTimer(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::Default"
                << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::Default::keepAliveTimer()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::Default::keepAliveTimer()"
            << std::endl;
    }


}

void ConnectionMap_Default::remoteClose(ConnectionSMContext& context)
{
    PeerWireThread& ctxt = context.getOwner();

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::Default"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::Default::remoteClose()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.stopHandshakeTimers();
        ctxt.closeLocalConnection();
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : ConnectionMap::Default::remoteClose()"
                << std::endl;
        }

        context.setState(ConnectionMap::RemoteClosed);
    }
    catch (...)
    {
        context.setState(ConnectionMap::RemoteClosed);
        throw;
    }
    context.getState().Entry(context);

}

void ConnectionMap_Unconnected::Exit(ConnectionSMContext& context)

{
    PeerWireThread& ctxt = context.getOwner();

    ctxt.startHandshakeTimers();
}

void ConnectionMap_Unconnected::localClose(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::Unconnected"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::Unconnected::localClose()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::Unconnected::localClose()"
            << std::endl;
    }

    context.setState(ConnectionMap::LocalClosed);
    context.getState().Entry(context);

}

void ConnectionMap_Unconnected::tcpActiveConnection(ConnectionSMContext& context)
{
    PeerWireThread& ctxt = context.getOwner();

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::Unconnected"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::Unconnected::tcpActiveConnection()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.sendPeerWireMsg(ctxt.getHandshake());
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : ConnectionMap::Unconnected::tcpActiveConnection()"
                << std::endl;
        }

        context.setState(ConnectionMap::HandshakeSent);
    }
    catch (...)
    {
        context.setState(ConnectionMap::HandshakeSent);
        throw;
    }
    context.getState().Entry(context);

}

void ConnectionMap_Unconnected::tcpPassiveConnection(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::Unconnected"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::Unconnected::tcpPassiveConnection()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::Unconnected::tcpPassiveConnection()"
            << std::endl;
    }

    context.setState(ConnectionMap::WaitHandshake);
    context.getState().Entry(context);

}

void ConnectionMap_HandshakeSent::Entry(ConnectionSMContext& context)

{
    PeerWireThread& ctxt = context.getOwner();

    ctxt.printDebugMsg("connSM - HandshakeSent");
}

void ConnectionMap_HandshakeSent::handshakeMsg(ConnectionSMContext& context, Handshake const& hs)
{
    PeerWireThread& ctxt = context.getOwner();

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::HandshakeSent"
                << std::endl;
    }

    if (!ctxt.checkHandshake(hs))
    {
        context.getState().Exit(context);
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "ENTER TRANSITION: ConnectionMap::HandshakeSent::handshakeMsg(Handshake const& hs)"
                << std::endl;
        }

        // No actions.
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : ConnectionMap::HandshakeSent::handshakeMsg(Handshake const& hs)"
                << std::endl;
        }

        context.setState(ConnectionMap::ClosingConnection);
        context.getState().Entry(context);
    }
    else if (ctxt.isBitFieldEmpty())

    {
        context.getState().Exit(context);
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "ENTER TRANSITION: ConnectionMap::HandshakeSent::handshakeMsg(Handshake const& hs)"
                << std::endl;
        }

        // No actions.
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : ConnectionMap::HandshakeSent::handshakeMsg(Handshake const& hs)"
                << std::endl;
        }

        context.setState(ConnectionMap::Connected);
        context.getState().Entry(context);
    }
    else
    {
        context.getState().Exit(context);
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "ENTER TRANSITION: ConnectionMap::HandshakeSent::handshakeMsg(Handshake const& hs)"
                << std::endl;
        }

        context.clearState();
        try
        {
            ctxt.sendPeerWireMsg(ctxt.getBitFieldMsg());
            if (context.getDebugFlag())
            {
                std::ostream& str = context.getDebugStream();

                str << "EXIT TRANSITION : ConnectionMap::HandshakeSent::handshakeMsg(Handshake const& hs)"
                    << std::endl;
            }

            context.setState(ConnectionMap::Connected);
        }
        catch (...)
        {
            context.setState(ConnectionMap::Connected);
            throw;
        }
        context.getState().Entry(context);
    }

}

void ConnectionMap_HandshakeSent::timeout(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::HandshakeSent"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::HandshakeSent::timeout()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::HandshakeSent::timeout()"
            << std::endl;
    }

    context.setState(ConnectionMap::Unconnected);
    context.getState().Entry(context);

}

void ConnectionMap_WaitHandshake::Entry(ConnectionSMContext& context)

{
    PeerWireThread& ctxt = context.getOwner();

    ctxt.printDebugMsg("connSM - WaitHandshake");
}

void ConnectionMap_WaitHandshake::handshakeMsg(ConnectionSMContext& context, Handshake const& hs)
{
    PeerWireThread& ctxt = context.getOwner();

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::WaitHandshake"
                << std::endl;
    }

    if (!ctxt.checkHandshake(hs))
    {
        context.getState().Exit(context);
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "ENTER TRANSITION: ConnectionMap::WaitHandshake::handshakeMsg(Handshake const& hs)"
                << std::endl;
        }

        // No actions.
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : ConnectionMap::WaitHandshake::handshakeMsg(Handshake const& hs)"
                << std::endl;
        }

        context.setState(ConnectionMap::ClosingConnection);
        context.getState().Entry(context);
    }
    else if (ctxt.isBitFieldEmpty())

    {
        context.getState().Exit(context);
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "ENTER TRANSITION: ConnectionMap::WaitHandshake::handshakeMsg(Handshake const& hs)"
                << std::endl;
        }

        context.clearState();
        try
        {
            ctxt.sendPeerWireMsg(ctxt.getHandshake());
            if (context.getDebugFlag())
            {
                std::ostream& str = context.getDebugStream();

                str << "EXIT TRANSITION : ConnectionMap::WaitHandshake::handshakeMsg(Handshake const& hs)"
                    << std::endl;
            }

            context.setState(ConnectionMap::Connected);
        }
        catch (...)
        {
            context.setState(ConnectionMap::Connected);
            throw;
        }
        context.getState().Entry(context);
    }
    else
    {
        context.getState().Exit(context);
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "ENTER TRANSITION: ConnectionMap::WaitHandshake::handshakeMsg(Handshake const& hs)"
                << std::endl;
        }

        context.clearState();
        try
        {
            ctxt.sendPeerWireMsg(ctxt.getHandshake());
            ctxt.sendPeerWireMsg(ctxt.getBitFieldMsg());
            if (context.getDebugFlag())
            {
                std::ostream& str = context.getDebugStream();

                str << "EXIT TRANSITION : ConnectionMap::WaitHandshake::handshakeMsg(Handshake const& hs)"
                    << std::endl;
            }

            context.setState(ConnectionMap::Connected);
        }
        catch (...)
        {
            context.setState(ConnectionMap::Connected);
            throw;
        }
        context.getState().Entry(context);
    }

}

void ConnectionMap_WaitHandshake::timeout(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::WaitHandshake"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::WaitHandshake::timeout()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::WaitHandshake::timeout()"
            << std::endl;
    }

    context.setState(ConnectionMap::ClosingConnection);
    context.getState().Entry(context);

}

void ConnectionMap_Connected::Entry(ConnectionSMContext& context)

{
    PeerWireThread& ctxt = context.getOwner();

    ctxt.printDebugMsg("connSM - Connected");
    ctxt.startMachines();
    ctxt.connected();
    ctxt.startHandshakeTimers();
}

void ConnectionMap_Connected::Exit(ConnectionSMContext& context)

{
    PeerWireThread& ctxt = context.getOwner();

    ctxt.stopMachines();
}

void ConnectionMap_Connected::applicationClose(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::Connected"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::Connected::applicationClose()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::Connected::applicationClose()"
            << std::endl;
    }

    context.setState(ConnectionMap::ClosingConnection);
    context.getState().Entry(context);

}

void ConnectionMap_Connected::incomingPeerWireMsg(ConnectionSMContext& context)
{
    PeerWireThread& ctxt = context.getOwner();

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::Connected"
                << std::endl;
    }

    ConnectionSMState& endState = context.getState();

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::Connected::incomingPeerWireMsg()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.renewTimeoutTimer();
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : ConnectionMap::Connected::incomingPeerWireMsg()"
                << std::endl;
        }

        context.setState(endState);
    }
    catch (...)
    {
        context.setState(endState);
        throw;
    }

}

void ConnectionMap_Connected::keepAliveTimer(ConnectionSMContext& context)
{
    PeerWireThread& ctxt = context.getOwner();

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::Connected"
                << std::endl;
    }

    ConnectionSMState& endState = context.getState();

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::Connected::keepAliveTimer()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.sendPeerWireMsg(ctxt.getKeepAliveMsg());
        ctxt.renewKeepAliveTimer();
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : ConnectionMap::Connected::keepAliveTimer()"
                << std::endl;
        }

        context.setState(endState);
    }
    catch (...)
    {
        context.setState(endState);
        throw;
    }

}

void ConnectionMap_Connected::outgoingPeerWireMsg(ConnectionSMContext& context, cPacket * msg)
{
    PeerWireThread& ctxt = context.getOwner();

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::Connected"
                << std::endl;
    }

    ConnectionSMState& endState = context.getState();

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::Connected::outgoingPeerWireMsg(cPacket * msg)"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.sendPeerWireMsg(msg);
        ctxt.renewKeepAliveTimer();
        if (context.getDebugFlag())
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : ConnectionMap::Connected::outgoingPeerWireMsg(cPacket * msg)"
                << std::endl;
        }

        context.setState(endState);
    }
    catch (...)
    {
        context.setState(endState);
        throw;
    }

}

void ConnectionMap_Connected::timeout(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::Connected"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::Connected::timeout()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::Connected::timeout()"
            << std::endl;
    }

    context.setState(ConnectionMap::ClosingConnection);
    context.getState().Entry(context);

}

void ConnectionMap_ClosingConnection::Entry(ConnectionSMContext& context)

{
    PeerWireThread& ctxt = context.getOwner();

    ctxt.printDebugMsg("connSM - ClosingConnection");
    ctxt.stopHandshakeTimers();
    ctxt.closeLocalConnection();
}

void ConnectionMap_ClosingConnection::applicationClose(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::ClosingConnection"
                << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::ClosingConnection::applicationClose()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::ClosingConnection::applicationClose()"
            << std::endl;
    }


}

void ConnectionMap_ClosingConnection::localClose(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::ClosingConnection"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::ClosingConnection::localClose()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::ClosingConnection::localClose()"
            << std::endl;
    }

    context.setState(ConnectionMap::LocalClosed);
    context.getState().Entry(context);

}

void ConnectionMap_ClosingConnection::remoteClose(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::ClosingConnection"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::ClosingConnection::remoteClose()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::ClosingConnection::remoteClose()"
            << std::endl;
    }

    context.setState(ConnectionMap::RemoteClosed);
    context.getState().Entry(context);

}

void ConnectionMap_LocalClosed::Entry(ConnectionSMContext& context)

{
    PeerWireThread& ctxt = context.getOwner();

    ctxt.printDebugMsg("connSM - LocalClosed");
}

void ConnectionMap_LocalClosed::applicationClose(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::LocalClosed"
                << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::LocalClosed::applicationClose()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::LocalClosed::applicationClose()"
            << std::endl;
    }


}

void ConnectionMap_LocalClosed::remoteClose(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::LocalClosed"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::LocalClosed::remoteClose()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::LocalClosed::remoteClose()"
            << std::endl;
    }

    context.setState(ConnectionMap::Closed);
    context.getState().Entry(context);

}

void ConnectionMap_LocalClosed::timeout(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::LocalClosed"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::LocalClosed::timeout()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::LocalClosed::timeout()"
            << std::endl;
    }

    context.setState(ConnectionMap::Closed);
    context.getState().Entry(context);

}

void ConnectionMap_RemoteClosed::Entry(ConnectionSMContext& context)

{
    PeerWireThread& ctxt = context.getOwner();

    ctxt.printDebugMsg("connSM - RemoteClosed");
}

void ConnectionMap_RemoteClosed::applicationClose(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::RemoteClosed"
                << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::RemoteClosed::applicationClose()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::RemoteClosed::applicationClose()"
            << std::endl;
    }


}

void ConnectionMap_RemoteClosed::localClose(ConnectionSMContext& context)
{

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : ConnectionMap::RemoteClosed"
                << std::endl;
    }

    context.getState().Exit(context);
    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: ConnectionMap::RemoteClosed::localClose()"
            << std::endl;
    }

    if (context.getDebugFlag())
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : ConnectionMap::RemoteClosed::localClose()"
            << std::endl;
    }

    context.setState(ConnectionMap::Closed);
    context.getState().Entry(context);

}

void ConnectionMap_Closed::Entry(ConnectionSMContext& context)

{
    PeerWireThread& ctxt = context.getOwner();

    ctxt.printDebugMsg("connSM - Closed");
    ctxt.removeFromSwarm();
}

//
// Local variables:
//  buffer-read-only: t
// End:
//
