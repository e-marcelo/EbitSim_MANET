//
// ex: set ro:
// DO NOT EDIT.
// generated by smc (http://smc.sourceforge.net/)
// from file : ConnectionSM.sm
//

#ifndef CONNECTIONSM_SM_H
#define CONNECTIONSM_SM_H


#define SMC_USES_IOSTREAMS

#include <statemap.h>

// Forward declarations.
class ConnectionMap;
class ConnectionMap_Unconnected;
class ConnectionMap_HandshakeSent;
class ConnectionMap_WaitHandshake;
class ConnectionMap_Connected;
class ConnectionMap_ClosingConnection;
class ConnectionMap_LocalClosed;
class ConnectionMap_RemoteClosed;
class ConnectionMap_Closed;
class ConnectionMap_Default;
class ConnectionSMState;
class ConnectionSMContext;
class PeerWireThread;
class BitFieldMsg;
class cPacket;
class Handshake;
class PeerWireMsg;

class ConnectionSMState :
    public statemap::State
{
public:

    ConnectionSMState(const char * const name, const int stateId)
    : statemap::State(name, stateId)
    {};

    virtual void Entry(ConnectionSMContext&) {};
    virtual void Exit(ConnectionSMContext&) {};

    virtual void applicationClose(ConnectionSMContext& context);
    virtual void handshakeMsg(ConnectionSMContext& context, Handshake const& hs);
    virtual void incomingPeerWireMsg(ConnectionSMContext& context);
    virtual void keepAliveTimer(ConnectionSMContext& context);
    virtual void localClose(ConnectionSMContext& context);
    virtual void outgoingPeerWireMsg(ConnectionSMContext& context, cPacket * msg);
    virtual void remoteClose(ConnectionSMContext& context);
    virtual void tcpActiveConnection(ConnectionSMContext& context);
    virtual void tcpPassiveConnection(ConnectionSMContext& context);
    virtual void timeout(ConnectionSMContext& context);

protected:

    virtual void Default(ConnectionSMContext& context);
};

class ConnectionMap
{
public:

    static ConnectionMap_Unconnected Unconnected;
    static ConnectionMap_HandshakeSent HandshakeSent;
    static ConnectionMap_WaitHandshake WaitHandshake;
    static ConnectionMap_Connected Connected;
    static ConnectionMap_ClosingConnection ClosingConnection;
    static ConnectionMap_LocalClosed LocalClosed;
    static ConnectionMap_RemoteClosed RemoteClosed;
    static ConnectionMap_Closed Closed;
};

class ConnectionMap_Default :
    public ConnectionSMState
{
public:

    ConnectionMap_Default(const char * const name, const int stateId)
    : ConnectionSMState(name, stateId)
    {};

    virtual void incomingPeerWireMsg(ConnectionSMContext& context);
    virtual void keepAliveTimer(ConnectionSMContext& context);
    virtual void remoteClose(ConnectionSMContext& context);
    virtual void timeout(ConnectionSMContext& context);
};

class ConnectionMap_Unconnected :
    public ConnectionMap_Default
{
public:
    ConnectionMap_Unconnected(const char * const name, const int stateId)
    : ConnectionMap_Default(name, stateId)
    {};

    virtual void Exit(ConnectionSMContext&);
    virtual void tcpActiveConnection(ConnectionSMContext& context);
    virtual void tcpPassiveConnection(ConnectionSMContext& context);
};

class ConnectionMap_HandshakeSent :
    public ConnectionMap_Default
{
public:
    ConnectionMap_HandshakeSent(const char * const name, const int stateId)
    : ConnectionMap_Default(name, stateId)
    {};

    virtual void Entry(ConnectionSMContext&);
    virtual void handshakeMsg(ConnectionSMContext& context, Handshake const& hs);
<<<<<<< HEAD
    virtual void timeout(ConnectionSMContext& context);
=======
>>>>>>> bf6c1c16f9b827bce19ea9ae57ca1a9d7e51e990
};

class ConnectionMap_WaitHandshake :
    public ConnectionMap_Default
{
public:
    ConnectionMap_WaitHandshake(const char * const name, const int stateId)
    : ConnectionMap_Default(name, stateId)
    {};

    virtual void Entry(ConnectionSMContext&);
    virtual void handshakeMsg(ConnectionSMContext& context, Handshake const& hs);
};

class ConnectionMap_Connected :
    public ConnectionMap_Default
{
public:
    ConnectionMap_Connected(const char * const name, const int stateId)
    : ConnectionMap_Default(name, stateId)
    {};

    virtual void Entry(ConnectionSMContext&);
    virtual void Exit(ConnectionSMContext&);
    virtual void applicationClose(ConnectionSMContext& context);
    virtual void incomingPeerWireMsg(ConnectionSMContext& context);
    virtual void keepAliveTimer(ConnectionSMContext& context);
    virtual void outgoingPeerWireMsg(ConnectionSMContext& context, cPacket * msg);
    virtual void timeout(ConnectionSMContext& context);
};

class ConnectionMap_ClosingConnection :
    public ConnectionMap_Default
{
public:
    ConnectionMap_ClosingConnection(const char * const name, const int stateId)
    : ConnectionMap_Default(name, stateId)
    {};

    virtual void Entry(ConnectionSMContext&);
    virtual void applicationClose(ConnectionSMContext& context);
    virtual void localClose(ConnectionSMContext& context);
    virtual void remoteClose(ConnectionSMContext& context);
};

class ConnectionMap_LocalClosed :
    public ConnectionMap_Default
{
public:
    ConnectionMap_LocalClosed(const char * const name, const int stateId)
    : ConnectionMap_Default(name, stateId)
    {};

    virtual void Entry(ConnectionSMContext&);
    virtual void applicationClose(ConnectionSMContext& context);
    virtual void remoteClose(ConnectionSMContext& context);
};

class ConnectionMap_RemoteClosed :
    public ConnectionMap_Default
{
public:
    ConnectionMap_RemoteClosed(const char * const name, const int stateId)
    : ConnectionMap_Default(name, stateId)
    {};

    virtual void Entry(ConnectionSMContext&);
    virtual void applicationClose(ConnectionSMContext& context);
    virtual void localClose(ConnectionSMContext& context);
};

class ConnectionMap_Closed :
    public ConnectionMap_Default
{
public:
    ConnectionMap_Closed(const char * const name, const int stateId)
    : ConnectionMap_Default(name, stateId)
    {};

    virtual void Entry(ConnectionSMContext&);
};

class ConnectionSMContext :
    public statemap::FSMContext
{
public:

    explicit ConnectionSMContext(PeerWireThread& owner)
    : FSMContext(ConnectionMap::Unconnected),
      _owner(owner)
    {};

    ConnectionSMContext(PeerWireThread& owner, const statemap::State& state)
    : FSMContext(state),
      _owner(owner)
    {};

    virtual void enterStartState()
    {
        getState().Entry(*this);
        return;
    }

    inline PeerWireThread& getOwner()
    {
        return (_owner);
    };

    inline ConnectionSMState& getState()
    {
        if (_state == NULL)
        {
            throw statemap::StateUndefinedException();
        }

        return dynamic_cast<ConnectionSMState&>(*_state);
    };

    inline void applicationClose()
    {
        setTransition("applicationClose");
        getState().applicationClose(*this);
        setTransition(NULL);
    };

    inline void handshakeMsg(Handshake const& hs)
    {
        setTransition("handshakeMsg");
        getState().handshakeMsg(*this, hs);
        setTransition(NULL);
    };

    inline void incomingPeerWireMsg()
    {
        setTransition("incomingPeerWireMsg");
        getState().incomingPeerWireMsg(*this);
        setTransition(NULL);
    };

    inline void keepAliveTimer()
    {
        setTransition("keepAliveTimer");
        getState().keepAliveTimer(*this);
        setTransition(NULL);
    };

    inline void localClose()
    {
        setTransition("localClose");
        getState().localClose(*this);
        setTransition(NULL);
    };

    inline void outgoingPeerWireMsg(cPacket * msg)
    {
        setTransition("outgoingPeerWireMsg");
        getState().outgoingPeerWireMsg(*this, msg);
        setTransition(NULL);
    };

    inline void remoteClose()
    {
        setTransition("remoteClose");
        getState().remoteClose(*this);
        setTransition(NULL);
    };

    inline void tcpActiveConnection()
    {
        setTransition("tcpActiveConnection");
        getState().tcpActiveConnection(*this);
        setTransition(NULL);
    };

    inline void tcpPassiveConnection()
    {
        setTransition("tcpPassiveConnection");
        getState().tcpPassiveConnection(*this);
        setTransition(NULL);
    };

    inline void timeout()
    {
        setTransition("timeout");
        getState().timeout(*this);
        setTransition(NULL);
    };

private:
    PeerWireThread& _owner;
};


#endif // CONNECTIONSM_SM_H

//
// Local variables:
//  buffer-read-only: t
// End:
//
