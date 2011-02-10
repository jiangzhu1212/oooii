/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
// Socket abstractions for TCP and UDP. Note: Server/Client should be used 
// together and Sender/Receiver should be used together only, any other 
// match-up will not work.
#pragma once
#ifndef oSocket_h
#define oSocket_h

#include <oooii/oEvent.h>
#include <oooii/oInterface.h>

interface oSocket : public oInterface
{
	// For efficiency oSocket uses a size type that matches the underlying implementation
	typedef unsigned int size_t;

	static const oSocket::size_t DEFAULT_MAX_BUFFER_SIZE = 1024 * 1024;
	static const oSocket::size_t DEFAULT_TIMEOUT_MS = 500;

#if defined(_WIN32) || defined(_WIN64)
	// Force-initializes Winsock library. This is useful to put in a known
	// place so you can immediately attach a packet sniffer for debugging.
	static void ForceInitWinSock();
#endif

	struct DESC
	{
		DESC()
			: Peername(0)
			, MaxSendSize(DEFAULT_MAX_BUFFER_SIZE)
			, MaxReceiveSize(DEFAULT_MAX_BUFFER_SIZE)
			, MaxSimultaneousMessages(16)
			, ConnectionTimeoutMS(DEFAULT_TIMEOUT_MS)
		{}

		// IP and port (i.e. "computername:1234") of a host computer. Calling 
		// GetDesc() will retain whatever is passed in here. GetHostname() and 
		// GetPeername() can be used to get the specifics of a socket at any given
		// time.
		const char* Peername; 

		// The largest size of any one send operation (total size of all messages in 
		// one send)
		oSocket::size_t MaxSendSize;	

		// The largest size of any one receive operation
		oSocket::size_t MaxReceiveSize;

		// The maximum number of messages that will be in flight in either direction,
		// this only has implications for Asynchronous sockets in that if more messages
		// than Max are in flight a stall will occur.  Increasing this increases memory usage
		// but allows for more messages to be in flight.
		oSocket::size_t MaxSimultaneousMessages;


		// How long the Create() function should wait before giving up on creation 
		// of this connection object
		oSocket::size_t ConnectionTimeoutMS;
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	virtual const char* GetDebugName() const threadsafe = 0;

	// If false, this interface should be released. To get a more thorough error
	// message, a new interface should be created to attempt a reconnection, and 
	// if that fails oGetLastError() will have more robust reasoning as to why.
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

interface oSocketBlocking : public oSocket
{
	// Client-side of a reliable 2-way communication pipe (TCP). This 
	// implementation is blocking and uses the full Berkeley sockets approach. 
	// This does not particularly try to take advantage of underlying platform 
	// optimizations, but is quite useful in handshake or minimal communication 
	// situations. For high-usage, larger-buffer transfers look to 
	// oSocketClientAsync for more appropriate API.

	static bool Create(const char* _DebugName, const DESC& _Desc, threadsafe oSocketBlocking** _ppSocketClient);

	// returns true if send sent all bytes. If false, the send failed and the socket
	// is invalid (all subsequent sends/receives will fail), use oGetLastError().
	// Once Send returns, the operation is complete and _pSource can be re-used
	virtual bool Send(const void* _pSource, oSocket::size_t _SizeofSource, unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe = 0;

	// Returns number of bytes received. If 0, the receive failed and the socket
	// is invalid (all subsequent sends/receives will fail), use oGetLastError().
	//  A last error of ESHUTDOWN means that the receive failed, but because during its 
	// processing the peer validly shut down without error. This method always 
	// returns exactly one send's worth of data (if _SizeofDestination is big 
	// enough), even if the send was split up, or several sends were compacted 
	// into a single buffer (Nagle's optimization). If there is only a partial 
	// receive, and any portion of the message is lost, this function will report 
	// as if the whole message was lost and the next call to Receive() will 
	// receive the next message, not the remanants of the failed message.
	virtual oSocket::size_t Receive(void* _pDestination, oSocket::size_t _SizeofDestination, unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe = 0;
};

interface oSocketAsync : public oSocket
{
	// Client-side of a reliable 2-way communication pipe (TCP). This 
	// implementation is asynchronous and takes aggressive advantages of 
	// underlying platform-specific optimizations. 
	static bool Create(const char* _DebugName, const DESC& _Desc, threadsafe oSocketAsync** _ppSocket);

	// MapSend allows for a zero-memcpy send by returning the user memory that they can operate
	// directly on.  The user specifies a reasonable maximum for the amount of memory needed
	// and specifies the actual amount to be copied in UnmapSend which queues the send up asynchronously
	virtual void MapSend(oSocket::size_t _MaxSize, void** _ppData) threadsafe = 0;
	virtual void UnmapSend(oSocket::size_t _ActualSize, void* _pData) threadsafe = 0;

	template<typename T>
	inline void MapSend( T** _ppData ) threadsafe
	{
		MapSend( sizeof(T), (void**)_ppData );
	}

	template<typename T>
	inline void UnmapSend( T* _pData ) threadsafe
	{
		UnmapSend( sizeof(T), _pData );
	}
};

interface oSocketAsyncReceiver : public oInterface
{
	typedef oFUNCTION<void(void*,oSocket::size_t)> receiver_callback_t;

	// A socket receiver is responsible for the receive end of a Socket.
	// Since oSocketClientAsync is an asynchronous/overlapped IO socket receives
	// need to be handled by a separate thread (hence the separate interface).  
	// A single oSocketClientAsyncReceiver is able to handle up to 64 sockets.  
	// Sockets are automatically dropped when they go out of scope
	static bool Create(const char* _pDebugName, threadsafe oSocketAsyncReceiver** _ppReceiver);

	// Adds a socket to this receiver.  When a MESSAGE_GROUP is received the 
	// supplied callback will be fired with the MESSAGE_GROUP flattened into
	// a single piece of contiguous memory. Failure can occur when the socket
	// is already owned by another receiver
	virtual bool AddSocket(threadsafe oSocketAsync* _pSocket, receiver_callback_t _Callback) threadsafe = 0;
};

interface oSocketBlocking;
interface oSocketServer : public oInterface
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

		unsigned short ListenPort;
		unsigned int MaxNumConnections;
	};

	static bool Create(const char* _DebugName, const DESC& _Desc, threadsafe oSocketServer** _ppSocketServer);

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	virtual const char* GetDebugName() const threadsafe = 0;

	// @oooii-tony: Unifying oSocketServer and oSocket2Server here... eventually
	// there are two different behaviors for the clients, mainly overlapped or not.
	// For now allow for unified server code between the two, but be able to connect
	// to either type... This will become more unified in the future.

	virtual bool WaitForConnection(threadsafe oSocketAsync** _ppNewlyConnectedClient, unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe = 0;

	virtual bool WaitForConnection(threadsafe oSocketBlocking** _ppNewlyConnectedClient, unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe = 0;
};

interface oSocketSender : public oInterface
{
	// Broadcast-side of an unreliable, 1-way communication pipe (UDP).

	struct DESC
	{
		DESC()
			: ReceiverHostname(0)
			, SendPort(0)
			, SendBufferSize(0)
		{}

		const char* ReceiverHostname;
		unsigned short SendPort;
		size_t SendBufferSize;
	};

	static bool Create(const char* _DebugName, const DESC* _pDesc, threadsafe oSocketSender** _ppSocketSender);

	// returns true if send sent all bytes. If false, use oGetLastError()
	virtual bool Send(const void* _pSource, size_t _SizeofSource) threadsafe = 0;
};

interface oSocketReceiver : public oInterface
{
	// Reception-side of an unreliable, 1-way communication pipe (UDP).

	struct DESC
	{
		DESC()
			: SenderHostname(0)
			, ReceivePort(0)
			, ReceiveBufferSize(0)
		{}

		const char* SenderHostname;
		unsigned short ReceivePort;
		size_t ReceiveBufferSize;
	};

	static bool Create(const char* _DebugName, const DESC* _pDesc, threadsafe oSocketReceiver** _ppSocketReceiver);

	// returns number of bytes received. If 0, use oGetLastError()
	// This does NOT behave the same as oSocketClient, this receives
	// the raw buffering with no respect for sends and Nagel's opt
	// might mean this buffer contains more than one send's worth of
	// data.
	virtual size_t Receive(void* _pDestination, size_t _SizeofDestination, unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe = 0;
};

#endif
