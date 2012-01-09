// $(header)
// Socket abstractions for TCP and UDP. Note: Server/Client should be used 
// together and Sender/Receiver should be used together only, any other 
// match-up will not work.
#pragma once
#ifndef oSocket_h
#define oSocket_h

#include <oBasis/oRef.h>
#include <oPlatform/oEvent.h>
#include <oBasis/oInterface.h>

// A Host is a simple identifier for a net device. This is an IP address
// for Internet connections. Use oFromString to create a new oNetHost.
struct oNetHost
{
	oNetHost()
		: IP(0)
	{
	}
	bool operator==(const oNetHost& rhs) const { return (IP == rhs.IP); }
	bool operator!=(const oNetHost& rhs) const { return !(IP == rhs.IP); }
	bool operator<(const oNetHost& _Other) const { return IP < _Other.IP; }
	bool Valid() const { return 0 != IP; }

private:
	unsigned long IP;
};


// An Address is the combination of a Host and any data used to distinguish
// it from other connections from the same Host. In Internet terms this is an
// IP address and a port together. Use oFromString to create a new oNetAddress
// in the form "<host>:<port>" ex: "google.com:http" or "127.0.0.1:11000".
struct oNetAddr
{
	oNetAddr()
		: Port(0)
	{}
	bool operator==(const oNetAddr& rhs) const { return (Host == rhs.Host && Port == rhs.Port); }
	bool operator<(const oNetAddr& _Other) const { return Host < _Other.Host || (Host == _Other.Host && Port < _Other.Port); }
	bool Valid() const { return Host.Valid(); }

	oNetHost Host;
private:
	unsigned short Port;
};

// In addition to oFromString/oToString these helper functions aid in working with oNetAddr
oAPI void oSocketPortGet(const oNetAddr& _Addr, unsigned short* _pPort);
oAPI void oSocketPortSet(const unsigned short _Port, oNetAddr* _pAddr);

interface oSocketAsyncCallback;
interface oSocket : oInterface
{
	enum PROTOCOL
	{
		UDP,
		TCP,
		oMSG, // oMSG is a reliable protocol built on TCP that ensures an entire send is received with one call to receive
	};

	enum STYLE
	{
		BLOCKING, // When calls to Send/SendTo/Recv return the operation has completed and the memory can be re-used
		ASYNC,    // When calls to Send/SendTo/Recv return the operation is queued up and the memory must not be touched until the supplied call back has fired
	};

	// For efficiency oSocket uses a size type that matches the underlying implementation
	typedef unsigned int size_t;

	struct BLOCKING_SETTINGS
	{
		BLOCKING_SETTINGS()
			: SendTimeout(oINFINITE_WAIT)
			, RecvTimeout(oINFINITE_WAIT)
		{}
		unsigned int SendTimeout;
		unsigned int RecvTimeout;
	};

	struct ASYNC_SETTINGS
	{
		ASYNC_SETTINGS()
			: MaxSimultaneousMessages(16)
		{}

		oRef<threadsafe oSocketAsyncCallback> Callback;

		// The maximum number of messages that will be in flight in either direction,
		// this only has implications for Asynchronous sockets in that if more messages
		// than Max are in flight a stall will occur.  Increasing this increases memory usage
		// but allows for more messages to be in flight.
		oSocket::size_t MaxSimultaneousMessages;
	};

	struct DESC
	{
		DESC()
			: ConnectionTimeoutMS(500)
			, Protocol(oMSG)
			, Style(BLOCKING)
		{}

		PROTOCOL Protocol;

		STYLE Style;

		// IP and port (i.e. "computername:1234") of a host computer. Calling 
		// GetDesc() will retain whatever is passed in here. GetHostname() and 
		// GetPeername() can be used to get the specifics of a socket at any given
		// time.
		oNetAddr Addr;

		// Settings specific to blocking sockets
		BLOCKING_SETTINGS BlockingSettings;

		// Settings specific to ASYNC sockets
		ASYNC_SETTINGS AsyncSettings;

		// How long the Create() function should wait before giving up on creation 
		// of this connection object
		oSocket::size_t ConnectionTimeoutMS;
	};

	// Failure cases: 
	// oERROR_INVALID_PARAMETER: This can occur when a call to Send is made on a connectionless based protocol (i.e. UDP)
	// oERROR_AT_CAPACITY: This can occur when the MaxSimultaneousMessages has been exhausted
	virtual bool Send(const void* _pData, oSocket::size_t _Size) threadsafe = 0;

	// Failure cases: 
	// oERROR_INVALID_PARAMETER: This can occur when a call to SendTo is made on a connection based protocol (i.e. TCP)
	// oERROR_AT_CAPACITY: This can occur when the MaxSimultaneousMessages has been exhausted
	virtual bool SendTo(const void* _pData, oSocket::size_t _Size, const oNetAddr& _Destination) threadsafe = 0;

	// Queue up a pending receive. The RecvCallback will be called once the
	// passed in buffer has been filled with the data from a single packet.
	// This can fail if too many requests are outstanding
	virtual oSocket::size_t Recv(void* _pBuffer, oSocket::size_t _Size) threadsafe = 0;

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	virtual const char* GetDebugName() const threadsafe = 0;

	// If false, this interface should be released. To get a more thorough error
	// message, a new interface should be created to attempt a reconnection, and 
	// if that fails oErrorGetLast() will have more robust reasoning as to why.
	virtual bool IsConnected() const threadsafe = 0;

	// Returns the host's ('this' computer's) full information including name, ip,
	// and port. Specify NULL for undesired fields from this API.
	virtual bool GetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe = 0;

	// Returns the peer's (the other computer's) full information including name, 
	// ip, and port. Specify NULL for undesired fields from this API.
	virtual bool GetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe = 0;

	template<size_t hostnameSize> inline bool GetHostname(char (&_OutHostname)[hostnameSize]) const threadsafe { return GetHostname(_OutHostname, hostnameSize, 0, 0, 0, 0); }
	template<size_t peernameSize> inline bool GetPeername(char (&_OutPeername)[peernameSize]) const threadsafe { return GetPeername(_OutPeername, peernameSize, 0, 0, 0, 0); }
	template<size_t hostnameSize, size_t ipSize, size_t portSize> inline bool GetHostname(char (&_OutHostname)[hostnameSize], char (&_OutIPAddress)[ipSize], char (&_OutPort)[portSize]) const threadsafe { return GetHostname(_OutHostname, hostnameSize, _OutIPAddress, ipSize, _OutPort, portSize); }
	template<size_t peernameSize, size_t ipSize, size_t portSize> inline bool GetPeername(char (&_OutPeername)[peernameSize], char (&_OutIPAddress)[ipSize], char (&_OutPort)[portSize]) const threadsafe { return GetPeername(_OutPeername, peernameSize, _OutIPAddress, ipSize, _OutPort, portSize); }
};

oAPI bool oSocketCreate(const char* _DebugName, const oSocket::DESC& _Desc, threadsafe oSocket** _pSocket);

// Encrypted sockets work only in blocking mode, supporting all the same behavior
// as regular sockets, but with the additional ability to receive and send encrypted data
interface oSocketEncrypted : public oSocket
{
	virtual bool SendEncrypted(const void* _pData, oSocket::size_t _Size) threadsafe = 0;
	virtual oSocket::size_t RecvEncrypted(void* _pBuffer, oSocket::size_t _Size) threadsafe = 0;
};

oAPI bool oSocketEncryptedCreate(const char* _DebugName, const oSocket::DESC& _Desc, threadsafe oSocketEncrypted** _ppSocket);


interface oSocketAsyncCallback : oInterface
{
	// Called when new data arrives over the network.
	virtual void ProcessSocketReceive(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe = 0;

	// Called once a send operation has finished. A buffer passed in for
	// sending must remain available until this callback is sent.
	virtual void ProcessSocketSend(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe = 0;
};

interface oSocketServer : oInterface
{
	// Server-side of a reliable two-way communication pipe (TCP). The server 
	// merely listens for incoming connections. Use the oSocketClientAsync returned 
	// from WaitForConnection() to do communication to the remote client.

	struct DESC
	{
		DESC()
			: ListenPort(0)
			, MaxNumConnections(1)
		{}

		unsigned short ListenPort; // The port that the server listens for incoming connections on
		unsigned int MaxNumConnections; // Maximum number of connections this server will service
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	virtual const char* GetDebugName() const threadsafe = 0;

	virtual bool GetHostname(char* _pString, size_t _strLen)  const threadsafe = 0;
	template<size_t hostnameSize> inline bool GetHostname(char (&_OutHostname)[hostnameSize]) const threadsafe { return GetHostname(_OutHostname, hostnameSize); }

	virtual bool WaitForConnection(const oSocket::BLOCKING_SETTINGS& _BlockingSettings, threadsafe oSocket** _ppNewlyConnectedClient, unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe = 0;
	virtual bool WaitForConnection(const oSocket::ASYNC_SETTINGS& _AsyncSettings, threadsafe oSocket** _ppNewlyConnectedClient, unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe = 0;
};

oAPI bool oSocketServerCreate(const char* _DebugName, const oSocketServer::DESC& _Desc, threadsafe oSocketServer** _ppSocketServer);




#endif
