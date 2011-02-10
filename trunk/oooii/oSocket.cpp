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
#include "pch.h"
#include <oooii/oSocket.h>
#include <oooii/oByte.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSocket.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oThreading.h>
#include <oooii/oThread.h>
#include <oooii/oAllocator.h>
#include <oooii/oLockFreeQueue.h>
#include "oWinsock.h"

oSocket::size_t oWinsockRecvBlocking( SOCKET hSocket, void* _pData, oSocket::size_t _szReceive, unsigned int _Timeout)
{
	oWinsock* ws = oWinsock::Singleton();

	oSocket::size_t TotalReceived = 0;
	while( _Timeout && TotalReceived < _szReceive )
	{
		oScopedPartialTimeout PartialTimeout(&_Timeout);
		if (SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&_Timeout, sizeof(unsigned int)) ) 
			goto error;

		oSocket::size_t received = ws->recv( hSocket, (char*)_pData + TotalReceived, _szReceive - TotalReceived, 0 );
		if( SOCKET_ERROR == received ) 
			goto error;

		TotalReceived += received;
	}
	
	return TotalReceived;
error:

	oSetLastError(oWinsock::GetErrno(ws->WSAGetLastError()));
	return 0;
}

bool oWinsockSendBlocking( SOCKET hSocket, const void* _pData, oSocket::size_t _szSend, unsigned int _Timeout)
{
	oWinsock* ws = oWinsock::Singleton();

	if (SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO,(char *)&_Timeout, sizeof(unsigned int)) ) goto error;

	if( _szSend != (oSocket::size_t) ws->send( hSocket, (const char*)_pData, _szSend, 0 ) ) goto error;

	return true;
error:

	oSetLastError(oWinsock::GetErrno(ws->WSAGetLastError()));
	return false;
}

void oSocket::ForceInitWinSock()
{
	oWinsock::Singleton();
}

struct CLIENT_INIT_HEADER
{
	oSocket::size_t MaxSendSize;
	oSocket::size_t MaxReceiveSize; 
	oSocket::size_t MaxSimultaneousMessages;
};

struct CLIENT_PACKET_HEADER
{
	int MagicNumber; // 'oNET'
	oSocket::size_t MessageLength;
};

struct SocketBlocking_Impl : public oSocketBlocking
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);

	SocketBlocking_Impl(const char* _DebugName, const DESC& _Desc, SOCKET _SystemSocket, bool* _pSuccess);
	~SocketBlocking_Impl();

	const char* GetDebugName() const threadsafe override;
	bool IsConnected() const threadsafe override;

	bool GetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe override;
	bool GetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe override;

	bool Send(const void* _pSource, oSocket::size_t _SizeofSource, unsigned int _TimeoutMS /*= oINFINITE_WAIT */) threadsafe override;
	oSocket::size_t Receive(void* _pDestination, oSocket::size_t _SizeofDestination, unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe override;

	DESC Desc;
	SOCKET hSocket;
	oRefCount RefCount;
	oMutex SendMutex;
	oMutex ReceiveMutex;
	char DebugName[64];
	char Peername[_MAX_PATH];
	int bCanReceive; // atomic bool, if a receive fails due to disconnect, don't allow another call to wait/block indefinitely
	oSocket::size_t LeftOverBytesToFlush;
	errno_t LeftOverError;
};

bool oSocketBlocking::Create(const char* _DebugName, const DESC& _Desc, threadsafe oSocketBlocking** _ppSocketClient)
{
	if (!_DebugName || !_ppSocketClient)
	{
		oSetLastError(EINVAL);
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppSocketClient, SocketBlocking_Impl(_DebugName, _Desc, INVALID_SOCKET, &success));
	return !!*_ppSocketClient;
}

// @oooii-tony: temporary glue code to keep the new unified oSocketServer 
// implementation relatively centralized. I haven't thought about how to unify
// oSocketClient and oSocketClientAsync yet... that's next.
void* CreateSocketClient(const char* _DebugName, SOCKET _hTarget, const char* _FullPeername, unsigned int _TimeoutMS, bool* _pSuccess)
{
	oSocketBlocking::DESC desc;
	desc.Peername = _FullPeername;
	desc.ConnectionTimeoutMS = _TimeoutMS;

	// Place the socket into non-blocking mode by first clearing the event then disabling FIONBIO
	{
		oWinsock* ws = oWinsock::Singleton();

		if( SOCKET_ERROR == ws->WSAEventSelect(_hTarget, NULL, NULL ) ) return NULL;

		u_long nonBlocking = 0;
		if (SOCKET_ERROR == ws->ioctlsocket(_hTarget, FIONBIO, &nonBlocking) ) return NULL;
	}

	return new SocketBlocking_Impl(_DebugName, desc, _hTarget, _pSuccess);
}

SocketBlocking_Impl::SocketBlocking_Impl(const char* _DebugName, const DESC& _Desc, SOCKET _SystemSocket, bool* _pSuccess)
: Desc(_Desc)
, hSocket(_SystemSocket)
, bCanReceive(true)
, LeftOverBytesToFlush(0)
, LeftOverError(0)
{
	*_pSuccess = false;

	*DebugName = 0;
	if (_DebugName)
		strcpy_s(DebugName, _DebugName);

	*Peername = 0;
	strcpy_s(Peername, _Desc.Peername);
	Desc.Peername = Peername;

	if (INVALID_SOCKET == hSocket) // create a socket from scratch (client-side)
	{
		// Client side socket attempting to connect to a server
		hSocket = oWinsockCreate(Desc.Peername, oWINSOCK_RELIABLE|oWINSOCK_REUSE_ADDRESS|oWINSOCK_BLOCKING, 0, Desc.MaxSendSize, Desc.MaxReceiveSize);
		if (hSocket == INVALID_SOCKET)
			return; // respect last-error settings from oWinsockCreate()

		// Since we created a socket this socket is connecting to a server we need 
		// to tell the server what size it should make its buffers which is a mirror 
		// of our sizes.
		CLIENT_INIT_HEADER header;
		header.MaxReceiveSize = Desc.MaxSendSize;
		header.MaxSendSize = Desc.MaxReceiveSize;
		header.MaxSimultaneousMessages = Desc.MaxSimultaneousMessages;

		if( !oWinsockSendBlocking( hSocket, &header, sizeof( CLIENT_INIT_HEADER), Desc.ConnectionTimeoutMS ) )
			return;
	}

	else // create this interface from the information in the valid specified socket
	{
		CLIENT_INIT_HEADER header;
		if( !oWinsockRecvBlocking( hSocket, &header, sizeof( CLIENT_INIT_HEADER ), Desc.ConnectionTimeoutMS ) )
			return;

		Desc.MaxSendSize = header.MaxSendSize;
		Desc.MaxReceiveSize = header.MaxReceiveSize;
		Desc.MaxSimultaneousMessages = header.MaxSimultaneousMessages;

		if (SOCKET_ERROR == oWinsock::Singleton()->setsockopt(hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&Desc.MaxSendSize, sizeof(Desc.MaxSendSize)))
		{
			oWINSOCK_SETLASTERROR("setsockopt SO_SNDBUF");
			return;
		}

		if (SOCKET_ERROR == oWinsock::Singleton()->setsockopt(hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&Desc.MaxReceiveSize, sizeof(Desc.MaxReceiveSize)))
		{
			oWINSOCK_SETLASTERROR("setsockopt SO_SNDBUF");
			return;
		}
	}

	*_pSuccess = true;
}

SocketBlocking_Impl::~SocketBlocking_Impl()
{
	if (INVALID_SOCKET != hSocket)
		oVERIFY(oWinsockClose(hSocket));
}

const char* SocketBlocking_Impl::GetDebugName() const threadsafe
{
	return thread_cast<const char*>(DebugName); // safe because DebugName never changes
}

bool SocketBlocking_Impl::IsConnected() const threadsafe
{
	return oWinsockIsConnected(hSocket);
}

bool SocketBlocking_Impl::GetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe
{
	return oWinsockGetHostname(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, hSocket);
}

bool SocketBlocking_Impl::GetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe
{
	return oWinsockGetPeername(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, hSocket);
}

bool SocketBlocking_Impl::Send(const void* _pSource, oSocket::size_t _SizeofSource, unsigned int _TimeoutMS ) threadsafe
{
	oMutex::ScopedLock lock(SendMutex);
	CLIENT_PACKET_HEADER header;
	header.MagicNumber = 'oNET';
	header.MessageLength = _SizeofSource;
	oASSERT(header.MessageLength == _SizeofSource, "");

	oWinsock* ws = oWinsock::Singleton();
	oScopedPartialTimeout Timeout( &_TimeoutMS );
	if (SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO,(char *)&_TimeoutMS, sizeof(unsigned int)) )
		return false;

	if( !oWinsockSend(hSocket, &header, sizeof(header), 0) )
		return false;

	if (SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO,(char *)&_TimeoutMS, sizeof(unsigned int)) )
		return false;

	Timeout = oScopedPartialTimeout( &_TimeoutMS );

	if( !oWinsockSend(hSocket, _pSource, _SizeofSource, 0) )
		return false;

	return true;
}

oSocket::size_t SocketBlocking_Impl::Receive(void* _pDestination, oSocket::size_t _SizeofDestination, unsigned int _TimeoutMS) threadsafe
{
	oMutex::ScopedLock lock(ReceiveMutex);

	oScopedPartialTimeout Timeout( &_TimeoutMS );

	CLIENT_PACKET_HEADER header;
	if( !oWinsockRecvBlocking( hSocket, &header, sizeof( CLIENT_PACKET_HEADER ), _TimeoutMS ) )
		return 0;

	if( _SizeofDestination < header.MessageLength )
		return 0;

	Timeout = oScopedPartialTimeout( &_TimeoutMS );

	if( !oWinsockRecvBlocking( hSocket, _pDestination, header.MessageLength, _TimeoutMS ) )
		return 0;

	return header.MessageLength;
}	

class SocketAsync_Impl : public oSocketAsync
{
public:
	struct INIT_HEADER
	{
		oSocket::size_t MaxSendSize;
		oSocket::size_t MaxReceiveSize; 
	};

	struct PACKET_HEADER
	{
		int MagicNumber; // 'oNET'
		oSocket::size_t MessageLength;
	};

	struct OverlappedSend
	{
	public:
		OverlappedSend();
		~OverlappedSend();

		void Send( SOCKET hSocket, void* pData, oSocket::size_t szData);

		bool Finished();

	private:
		WSAOVERLAPPED Overlap;

		// This is the header that gets sent across with every send
		PACKET_HEADER Header;
		WSABUF Messages[2];
		oEvent Event; // Event that is set when the group and memory it's pointing to can be re-used.
	};

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);

	SocketAsync_Impl(SOCKET _hSocket, const char* _DebugName, const char* _Peername, unsigned int _Timeout, bool* _pSuccess);
	SocketAsync_Impl(const char* _DebugName, const DESC& _Desc, bool* _pSuccess);
	~SocketAsync_Impl();

	const char* GetDebugName() const threadsafe override;
	bool IsConnected() const threadsafe override;
	bool GetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe override;
	bool GetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe override;

	void SendInternal(OverlappedSend* _pMessageGroup) threadsafe;

	virtual void MapSend(oSocket::size_t _MaxSize, void** _ppData) threadsafe override;
	virtual void UnmapSend(oSocket::size_t _ActualSize, void* _pData) threadsafe override;
	void GarbageCollectSend();

	bool SetOwnership( threadsafe oSocketAsyncReceiver* Receiver, oFUNCTION<void(void*,oSocket::size_t)> _Callback) threadsafe;
	
	void BeginSending(oSocket::size_t _MaxOutstandingSends);
	
	void BeginReceiving(SocketAsync_Impl** _ppContextStorage);
	void EndReceiving();

	void TerminateConnection(unsigned int _DisconnectReason);

private:
	oRefCount RefCount;
	oRWMutex SendMutex;

	SOCKET hSocket;
	unsigned int DisconnectReason;

	// Objects necessary to handle sends
	struct SEND_MANAGEMENT
	{
		SEND_MANAGEMENT()
			: OutstandingSends(MAX_OUTSTANDING_SENDS)
		{}
		~SEND_MANAGEMENT()
		{

		}

		static const size_t MAX_OUTSTANDING_SENDS = 1024;
		typedef std::vector<char> send_arena_t;
		send_arena_t Arena;
		oRef<oAllocator> Allocator;
		oLockFreeQueue<OverlappedSend*> OutstandingSends;
	} SendManagement;

	// Objects necessary to process receives
	threadsafe oRef<oSocketAsyncReceiver> OwnedReceiver;
	struct RECEIVE_MANAGEMENT
	{
		WSAOVERLAPPED Overlap;
		oFUNCTION<void(void*,oSocket::size_t)> CallBack;
		WSABUF Messages[2];
		int LastMessageCount;

		oSocket::size_t BytesUnprocessed;
		oSocket::size_t NextMessageSize;

		typedef std::vector<char> receive_buffer_t;
		receive_buffer_t Buffer;
		receive_buffer_t::iterator ReadHead;
		receive_buffer_t::iterator WriteHead;
	
		receive_buffer_t::iterator SecondBankStart;
		receive_buffer_t::iterator ThirdBankStart;

	} ReceiveManagement;

	DESC Desc;
	char DebugName[64];
	char Peername[_MAX_PATH];

	static void CALLBACK ReceiveCompletionRoutine(
		IN DWORD dwError, 
		IN DWORD cbTransferred, 
		IN LPWSAOVERLAPPED lpOverlapped, 
		IN DWORD dwFlags
		);

	void ProcessReceive(oSocket::size_t _NumBytesTransferred); 
	
	// These functions wrap the particulars of overlapped IO receives and
	// need to be called in very specific order
	void IssueRead(oSocket::size_t _NumBytesTransferred, oSocket::size_t _NumAdditionalBytesNeeded);
};

bool oSocketAsync::Create( const char* _DebugName, const DESC& _Desc, threadsafe oSocketAsync** _ppSocket )
{
	bool success = false;
	oCONSTRUCT( _ppSocket, SocketAsync_Impl( _DebugName, _Desc, &success ) );
	return success;
}

class oSocketAsyncReceiverThread : public oThread::Proc
{
public:
	static const int MAX_SERVICEABLE_SOCKETS = 64;
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oSocketAsyncReceiverThread( const char* _pName, bool* _pSuccess);
	~oSocketAsyncReceiverThread();

	void AddConnection(SocketAsync_Impl* _pConnection);
	void DropConnection(SocketAsync_Impl* _pConnection);

	void WakeupThreadAndWait();

private:
	oRefCount RefCount;
	oRef<threadsafe oThread> Thread;
	WSAEVENT ConnectionEvent;
	oEvent ConnectionProcessedEvent;
	SocketAsync_Impl* AddingConnection;
	SocketAsync_Impl* DroppingConnection;
	std::vector<SocketAsync_Impl*> ValidConnection;

#ifdef _DEBUG
	struct DEBUG_ConnectionStats
	{
		threadsafe SocketAsync_Impl* pSocket;
		double ConnectedTimestamp;
		double DisconnectedTimestamp;
	};
	std::vector<DEBUG_ConnectionStats> ConnectedSockets;
#endif

	virtual void RunIteration();

	virtual bool OnBegin()
	{
		return true;
	}

	virtual void OnEnd()
	{
	}


};

class SocketClientAsyncReceiver_Impl : public oSocketAsyncReceiver
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	SocketClientAsyncReceiver_Impl(const char* _pDebugName, bool* _pSuccess);

	virtual bool AddSocket(threadsafe oSocketAsync* _pSocket, oFUNCTION<void(void*,oSocket::size_t)> _Callback) threadsafe override;
	void DropSocket(threadsafe oSocketAsync* _pSocket) threadsafe ;
private:
	oRefCount RefCount;
	oRWMutex Mutex;
	oRef<oSocketAsyncReceiverThread> ReceiverThread;
};


bool oSocketAsyncReceiver::Create( const char* _pDebugName, threadsafe oSocketAsyncReceiver** _ppReceiver)
{
	bool success = false;
	oCONSTRUCT( _ppReceiver, SocketClientAsyncReceiver_Impl(_pDebugName, &success ) );
	return success;
}

SocketAsync_Impl::SocketAsync_Impl(SOCKET _hSocket, const char* _DebugName, const char* _Peername, unsigned int _Timeout, bool* _pSuccess)
	: hSocket(_hSocket)
	, OwnedReceiver(NULL)
{
	*DebugName = 0;
	if (_DebugName)
		strcpy_s(DebugName, _DebugName);

	*Peername = 0;
	if (_Peername)
		strcpy_s(Peername, _Peername);

	oWinsock* ws = oWinsock::Singleton();

	WSAOVERLAPPED OverlappedRead;
	memset( &OverlappedRead, NULL, sizeof(WSAOVERLAPPED));
	OverlappedRead.hEvent = ws->WSACreateEvent();

	INIT_HEADER initHeader;
	WSABUF HandShake;
	HandShake.buf = (char*)&initHeader;
	HandShake.len = sizeof(initHeader);
	DWORD Flags = 0;
	if(SOCKET_ERROR == ws->WSARecv(hSocket, &HandShake, 1, NULL, &Flags, &OverlappedRead, NULL ) )
	{
		if( WSA_IO_PENDING != ws->WSAGetLastError() )
		{
			oSetLastError(oWinsock::GetErrno(ws->WSAGetLastError()));
			goto error;
		}
		if( !oWinsockWaitMultiple( &OverlappedRead.hEvent, 1, true, true, _Timeout ) )
		{
			oSetLastError(ETIMEDOUT);
			goto error;
		}
	}

	// Configure the Desc based on what our partner socket tells us
	
	Desc.MaxReceiveSize = initHeader.MaxReceiveSize;
	Desc.MaxSendSize = initHeader.MaxSendSize;
	Desc.ConnectionTimeoutMS = _Timeout;
	Desc.Peername = Peername;

	// Successful handshake
	if( OverlappedRead.hEvent )
		ws->WSACloseEvent( OverlappedRead.hEvent );

	DisconnectReason = 0;
	*_pSuccess = true;
	return;

error:
	if( OverlappedRead.hEvent )
		ws->WSACloseEvent( OverlappedRead.hEvent );

	*_pSuccess = false;
	return;
}

SocketAsync_Impl::SocketAsync_Impl( const char* _pDebugName, const DESC& _Desc, bool* _pSuccess )
	: Desc(_Desc)
	, OwnedReceiver(NULL)
{
	*DebugName = 0;
	if (_pDebugName)
		strcpy_s(DebugName, _pDebugName);

	oWinsock* ws = oWinsock::Singleton();

	WSAEVENT hConnectEvent = ws->WSACreateEvent();
	hSocket = ws->WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED );
	if (SOCKET_ERROR == hSocket ) goto error;

	// Turn on non-blocking mode
	u_long enabled = 1;
	if (SOCKET_ERROR == ws->ioctlsocket(hSocket, FIONBIO, &enabled)) goto error;

	// Create an event to verify server connection
	if (SOCKET_ERROR == ws->WSAEventSelect(hSocket, hConnectEvent, FD_CONNECT ) ) goto error;

	sockaddr_in saddr;
	oWinsockCreateAddr(&saddr, Desc.Peername);
	if (SOCKET_ERROR == ws->connect(hSocket, (const sockaddr*)&saddr, sizeof(saddr) ) && WSAEWOULDBLOCK != ws->WSAGetLastError() ) goto error;

	// Wait to make sure the server ack's the connection
	if( !oWinsockWaitMultiple( &hConnectEvent, 1, true, false, Desc.ConnectionTimeoutMS ) ) goto error;

	// Since this socket is initializing the connection we send across our requested size
	// so our partner socket knows how to allocate memory (this is a mirror of our sizes)
	INIT_HEADER InitHeader;
	InitHeader.MaxSendSize = Desc.MaxReceiveSize;
	InitHeader.MaxReceiveSize = Desc.MaxSendSize;

	WSABUF InitBuffer;
	InitBuffer.buf = (char*)&InitHeader;
	InitBuffer.len = sizeof( INIT_HEADER );

	// Before sending set the event to write so we know when it is sent
	WSAOVERLAPPED OverlappedSend;
	memset( &OverlappedSend, NULL, sizeof(WSAOVERLAPPED));
	OverlappedSend.hEvent = hConnectEvent;
	if(SOCKET_ERROR == ws->WSASend(hSocket, &InitBuffer, 1, NULL, 0, &OverlappedSend, NULL ) )
	{
		if( WSA_IO_PENDING != ws->WSAGetLastError() ) goto error;
		if( !oWinsockWaitMultiple( &OverlappedSend.hEvent, 1, true, true, Desc.ConnectionTimeoutMS ) ) goto error;
	}


	// Successful handshake
	if( hConnectEvent )
		ws->WSACloseEvent( hConnectEvent );

	DisconnectReason = 0;
	*_pSuccess = true;
	return;

error:
	oWINSOCK_SETLASTERROR("SocketClientAsync_Impl::SocketClientAsync_Impl");
	// Something went wrong
	if( hConnectEvent )
		ws->WSACloseEvent( hConnectEvent);

	*_pSuccess = false;
	return;
}

SocketAsync_Impl::~SocketAsync_Impl()
{
	// If we have an owned receiver we need to notify 
	// the receiver
	if( OwnedReceiver )
	{
		static_cast<threadsafe SocketClientAsyncReceiver_Impl*>( OwnedReceiver.c_ptr())->DropSocket( this );
	}
	else
		EndReceiving();

	if( SendManagement.Allocator )
	{
		OverlappedSend* pMessageGroup;
		while( SendManagement.OutstandingSends.TryPop(pMessageGroup) )
		{
			SendManagement.Allocator->Destroy(pMessageGroup);
		}
	}
	SendManagement.Allocator = NULL;
}

void SocketAsync_Impl::BeginSending( oSocket::size_t _MaxOutstandingSends )
{
	oASSERT(!SendManagement.Allocator, "Socket already configured for sending");

	size_t ArenaSize = __max( _MaxOutstandingSends * Desc.MaxSendSize, 1024 * 512 );
	SendManagement.Arena.resize( ArenaSize );

	oAllocator::DESC AllocDesc;
	AllocDesc.ArenaSize = ArenaSize;
	AllocDesc.pArena = &(SendManagement.Arena[0]);
	oVERIFY( oAllocator::Create( "oSocketAsync Allocator", &AllocDesc, &SendManagement.Allocator ) );
}


void SocketAsync_Impl::MapSend(oSocket::size_t _MaxSize, void** _ppData) threadsafe
{
	oRWMutex::ScopedLock LockSend( SendMutex );
	*_ppData = NULL;

	SocketAsync_Impl* pThis = thread_cast<SocketAsync_Impl*>(this);

	SEND_MANAGEMENT* pSendManagement = thread_cast<SEND_MANAGEMENT*>(&SendManagement);
	if( !pSendManagement->Allocator )
	{
		pThis->BeginSending( Desc.MaxSimultaneousMessages );
	}

	size_t TotalSize =_MaxSize + sizeof( OverlappedSend );
	void* pBlob = pSendManagement->Allocator->Allocate( TotalSize );
	if( !pBlob )
	{
		pThis->GarbageCollectSend();
		pBlob = pSendManagement->Allocator->Allocate( TotalSize );
		if( !pBlob )
			return;
	}

	new(pBlob) OverlappedSend ();
	*_ppData = oByteAdd( (char*)pBlob, sizeof( OverlappedSend ) ); 
}

void SocketAsync_Impl::UnmapSend(oSocket::size_t _ActualSize, void* _pData) threadsafe
{
	oRWMutex::ScopedLock LockSend( SendMutex );
	OverlappedSend* pMessageGroup = (OverlappedSend*)((unsigned char*)_pData - sizeof( OverlappedSend ));
	oVB( SendManagement.OutstandingSends.TryPush(pMessageGroup) );
	pMessageGroup->Send( hSocket, _pData, _ActualSize );
}

void SocketAsync_Impl::GarbageCollectSend()
{
	oSocket::size_t OutstandingSends = (oSocket::size_t)SendManagement.OutstandingSends.Size();
	for( oSocket::size_t i =0; i < OutstandingSends; ++i )
	{
		OverlappedSend* pMessageGroup;
		if( SendManagement.OutstandingSends.TryPop(pMessageGroup) )
		{
			if( pMessageGroup->Finished() )
			{
				SendManagement.Allocator->Deallocate( pMessageGroup );
			}
			else
				SendManagement.OutstandingSends.TryPush(pMessageGroup);
		}
	}
}


const char* SocketAsync_Impl::GetDebugName() const threadsafe
{
	return thread_cast<const char*>(DebugName); // threadsafe because name never changes
}

bool SocketAsync_Impl::IsConnected() const threadsafe
{
	return oWinsockIsConnected(hSocket);
}

bool SocketAsync_Impl::GetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe
{
	return oWinsockGetHostname(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, hSocket);
}

bool SocketAsync_Impl::GetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe
{
	return oWinsockGetPeername(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, hSocket);
}

void CALLBACK SocketAsync_Impl::ReceiveCompletionRoutine( IN DWORD dwError, IN DWORD cbTransferred, IN LPWSAOVERLAPPED lpOverlapped, IN DWORD dwFlags )
{
	SocketAsync_Impl* pImpl = *(SocketAsync_Impl**)lpOverlapped->hEvent;
	if( !pImpl )
		return;

	// Catch any errors here
	switch( dwError )
	{
	// These errors indicate the socket has been destroyed
	// meaning pImpl is invalid
	case WSA_OPERATION_ABORTED:
		return;

	// These errors indicate network problems and require us to 
	// terminate the connection 
	case WSAECONNRESET:
	case WSAECONNABORTED:
		pImpl->TerminateConnection(dwError);
		return;
	}

	oASSERT( !dwError, "Unhandled error. Investigate what this means");

	if( !cbTransferred )
		return;

	pImpl->ProcessReceive( cbTransferred );
}

bool SocketAsync_Impl::SetOwnership( threadsafe oSocketAsyncReceiver* Receiver, oFUNCTION<void(void*,oSocket::size_t)> _Callback ) threadsafe
{
	oRWMutex::ScopedLock LockSend( SendMutex );
	if( OwnedReceiver && Receiver ) // A socket can only be owned by one receiver
		return false;

	// thread_cast safe because of mutex
	SocketAsync_Impl* pUnsafeThis = thread_cast<SocketAsync_Impl*>(this);

	pUnsafeThis->OwnedReceiver = Receiver;
	pUnsafeThis->ReceiveManagement.CallBack = _Callback;
	return true;
}

void SocketAsync_Impl::BeginReceiving(SocketAsync_Impl** _ppContextStorage)
{
	// @oooii-kevin: three bank buffering system
	// The receive buffer needs to three times the size of the maximum
	// bytes sent in one message.  This allows the system room to deliver
	// message groups without ever doing an additional memcpy. We fill
	// the first two banks of memory until full, and if a message is too 
	// big and needs to write into the third bank we let it complete
	// then wrap back to the beginning of the first bank
	ReceiveManagement.Buffer.resize( Desc.MaxReceiveSize * 3 );
	ReceiveManagement.WriteHead = ReceiveManagement.Buffer.begin();
	ReceiveManagement.ReadHead = ReceiveManagement.WriteHead;
	ReceiveManagement.SecondBankStart = ReceiveManagement.WriteHead + Desc.MaxReceiveSize;
	ReceiveManagement.ThirdBankStart = ReceiveManagement.SecondBankStart + Desc.MaxReceiveSize;

	// Setup the Overlap
	memset( &ReceiveManagement.Overlap, NULL, sizeof( WSAOVERLAPPED ) );

	// Store the context storage which will allow the completion
	// routine to deliver messages to the correct socket.  The receiver
	// that called this will have already setup the context storage to point to this
	oASSERT(*_ppContextStorage == this, "Context storage was not setup correctly");
	ReceiveManagement.Overlap.hEvent = _ppContextStorage;
	ReceiveManagement.NextMessageSize = (oSocket::size_t)-1;  // This indicates we're searching for a header
	ReceiveManagement.BytesUnprocessed = 0;

	IssueRead(0, 0);
}

void SocketAsync_Impl::EndReceiving()
{
	// The first step is to always shutdown the socket
	// this will stop anymore IO completion routines
	// from queueing up on the receiver
	if (INVALID_SOCKET != hSocket)
		oVERIFY(oWinsockClose(hSocket));

	// Set the context storage to NULL which will allow any late
	// firing completion routines to be safely ignored
	if( OwnedReceiver )
		*(SocketAsync_Impl**)ReceiveManagement.Overlap.hEvent = NULL;

}

void SocketAsync_Impl::TerminateConnection(unsigned int _DisconnectReason)
{
	// Lock out the sender while we terminate the connection
	// this should only ever be called by the receiver thread
	// when it realizes a connection is dead
	oASSERT( DisconnectReason == 0 && _DisconnectReason != 0, "Expected connection to still be alive.  We should only be closing the connection once");
	oRWMutex::ScopedLock LockSend( SendMutex );
	oVERIFY(oWinsockClose(hSocket));
	hSocket = NULL;
	DisconnectReason = _DisconnectReason; 
}

void SocketAsync_Impl::ProcessReceive(unsigned int _NumBytesTransfered)
{
	oASSERT(_NumBytesTransfered, "No bytes were received");

	ReceiveManagement.BytesUnprocessed += _NumBytesTransfered;

	bool ReadIssued = false;
	do
	{
		// If NextMessageSize is (oSocket::socket_size_t)-1 this means we're looking for the next header
		if( (oSocket::size_t)-1 == ReceiveManagement.NextMessageSize  )
		{
			if( ReceiveManagement.BytesUnprocessed < sizeof( PACKET_HEADER ) )  
			{
				oASSERT( ReadIssued, "Handle uber-small message");
				//PACKET_HEADER hdr = *((PACKET_HEADER*)&ReceiveManagement.ReadHead[0]);
				//ReceiveManagement.ReadHead = ReceiveManagement.WriteHead = ReceiveManagement.Buffer.begin();
				//*((PACKET_HEADER*)&ReceiveManagement.WriteHead[0]) = hdr;
				//IssueRead(_NumBytesTransfered, 0 );
				return;
			}

			oSocket::size_t MessageLength;
			// Determine the message length
			{
				PACKET_HEADER* pHdr = (PACKET_HEADER*)&ReceiveManagement.ReadHead[0];
				oASSERT( pHdr->MagicNumber == 'oNET', "Unknown packet!");
				MessageLength = pHdr->MessageLength;

				// If the length turned up -1 this indicates the buffer has wrapped and we need to 
				// go back to the first bank
				if( (oSocket::size_t)-1 == MessageLength )
				{
					ReceiveManagement.ReadHead = ReceiveManagement.Buffer.begin();
					PACKET_HEADER* pHdr = (PACKET_HEADER*)&ReceiveManagement.ReadHead[0];
					oASSERT( pHdr->MagicNumber == 'oNET', "Unknown packet!");
					MessageLength = pHdr->MessageLength;
				}
			}

			oASSERT( MessageLength != 0 && MessageLength != (oSocket::size_t)-1, "Invalid message length");

			// Advance the bookeeping as we have found a header and now need to get 
			// enough bytes to complete the message
			ReceiveManagement.ReadHead += sizeof( PACKET_HEADER ); 
			ReceiveManagement.BytesUnprocessed -= sizeof( PACKET_HEADER );
			ReceiveManagement.NextMessageSize = MessageLength; 
		}

		// Determine how many (if any) bytes are needed to deliver another complete message
		oSocket::size_t BytesNeeded = ReceiveManagement.BytesUnprocessed > ReceiveManagement.NextMessageSize ? 0 : ReceiveManagement.NextMessageSize - ReceiveManagement.BytesUnprocessed;
		
		if( !ReadIssued )
		{
			// The read is issued here (after the first header check) to ensure that 
			// we know how large subsequent messages are but before any heavy processing
			// occurs so that we minimize internal buffering
			IssueRead(_NumBytesTransfered, BytesNeeded );
			ReadIssued = true;
		}
		
		// There aren't enough bytes to complete a message so early out and wait for more
		if( BytesNeeded )
			return;

		ReceiveManagement.CallBack( &ReceiveManagement.ReadHead[0], ReceiveManagement.NextMessageSize );
		
		// Advance the bookeeping as we have consumed the NextMessageSize and need another header
		ReceiveManagement.ReadHead += ReceiveManagement.NextMessageSize;
		ReceiveManagement.BytesUnprocessed -= ReceiveManagement.NextMessageSize;
		ReceiveManagement.NextMessageSize = (oSocket::size_t)-1;

	} while ( ReceiveManagement.BytesUnprocessed );
}

void SocketAsync_Impl::IssueRead(oSocket::size_t _NumBytesTransferred, oSocket::size_t _BytesNeededByNextMessage)
{
	oWinsock* ws = oWinsock::Singleton();

	// Start by advancing the writehead
	if( !_BytesNeededByNextMessage && ReceiveManagement.LastMessageCount == 2 )
	{
		// The buffer has wrapped so we need to move the writehead back
		oSocket::size_t BytesToAdvance = _NumBytesTransferred - ReceiveManagement.Messages[0].len;
		ReceiveManagement.WriteHead = ReceiveManagement.Buffer.begin() + BytesToAdvance;
	}
	else
		ReceiveManagement.WriteHead += _NumBytesTransferred;

	// Advance the primary buffer
	WSABUF* pBuff = &ReceiveManagement.Messages[0];
	bool WriteHeadValid = ReceiveManagement.WriteHead + _BytesNeededByNextMessage < ReceiveManagement.ThirdBankStart;
	
	if( WriteHeadValid )
	{
		ReceiveManagement.LastMessageCount = 1;

		// Since the buffering system re-cycles to the head of the memory
		// the length of the request is limited by the location of the readhead
		if( ReceiveManagement.WriteHead < ReceiveManagement.ReadHead )
			pBuff->len = static_cast<oSocket::size_t>( std::distance( ReceiveManagement.WriteHead, ReceiveManagement.ReadHead ) );
		else
			pBuff->len = static_cast<oSocket::size_t>( std::distance( ReceiveManagement.WriteHead, ReceiveManagement.ThirdBankStart ) );
			

		pBuff->buf = &ReceiveManagement.WriteHead[0];
	}
	else
	{
		// The write head has become "invalid" meaning that to complete
		// the next transfer without moving stuff around we'll need to
		// write into the overrun third bank so we issue a primary request 
		// that will finish the message and nothing more, and a secondary
		// request that will start writing to the beginning of the buffer again
		oSocket::size_t BytesToSpill = _BytesNeededByNextMessage;
		if( !BytesToSpill )
		{
			// If no more bytes are needed to complete the current message and yet 
			// we are still running into the overrun third bank that means
			// a subsequent message may need to spill, so find that message
			oSocket::size_t extraBytes = ReceiveManagement.BytesUnprocessed - ReceiveManagement.NextMessageSize;
			if( !extraBytes )
			{
				oASSERT( false, "Implement single read to cleanup");
			}
			else
			{
				RECEIVE_MANAGEMENT::receive_buffer_t::iterator NextMessage = ReceiveManagement.ReadHead + ReceiveManagement.NextMessageSize;
				do
				{
					PACKET_HEADER* pHdr = (PACKET_HEADER*)(&NextMessage[0]);
					oASSERT( pHdr->MagicNumber == 'oNET', "Unknown packet!");
					oSocket::size_t NextMessageSize = pHdr->MessageLength + sizeof( PACKET_HEADER );
					NextMessage += NextMessageSize;
				}while( NextMessage < ReceiveManagement.ThirdBankStart );

				BytesToSpill = static_cast<oSocket::size_t>( std::distance( ReceiveManagement.ThirdBankStart, NextMessage ) );
			}
		}
		oASSERT( BytesToSpill, "Issuing 2 read requests so there should be bytes to spill");

		pBuff->buf = &ReceiveManagement.WriteHead[0];
		pBuff->len = BytesToSpill;
		
		PACKET_HEADER* pTerminatingHeader = (PACKET_HEADER*)(&ReceiveManagement.WriteHead[0] + BytesToSpill);
		
		oASSERT( pTerminatingHeader, "Terminating header is NULL");
		// Store a cookie at the end
		pTerminatingHeader->MagicNumber = 'oNET';
		pTerminatingHeader->MessageLength = (oSocket::size_t)-1;

		
		// Issue the secondary request that will rout new messages back to the first bank
		WSABUF* pBuffSecondary = pBuff + 1;
		pBuffSecondary->buf = &ReceiveManagement.Buffer[0];
		pBuffSecondary->len = static_cast<oSocket::size_t>( std::distance( ReceiveManagement.Buffer.begin(), ReceiveManagement.ReadHead ) );
		ReceiveManagement.LastMessageCount = 2;
	}

	DWORD RecieveFlags = 0;
	ws->WSARecv(hSocket, pBuff, ReceiveManagement.LastMessageCount, NULL, &RecieveFlags, &ReceiveManagement.Overlap, &SocketAsync_Impl::ReceiveCompletionRoutine ); // Issue the receive
}

SocketAsync_Impl::OverlappedSend::OverlappedSend()
{
	memset( &Overlap, NULL, sizeof( WSAOVERLAPPED ) );
	Overlap.hEvent = Event.GetNativeHandle();

	Messages[0].buf = (char*)&Header;
	Messages[0].len = sizeof( PACKET_HEADER );
	Header.MagicNumber = 'oNET';
}

SocketAsync_Impl::OverlappedSend::~OverlappedSend()
{
	Event.Wait();
}

void SocketAsync_Impl::OverlappedSend::Send( SOCKET hSocket, void* pData, oSocket::size_t szData )
{
	oWinsock* ws = oWinsock::Singleton();

	// Fill the header
	Header.MessageLength = szData;
	Messages[1].buf = (char*)pData;
	Messages[1].len = szData;

	ws->WSASend(hSocket, Messages, 2, NULL, 0, &Overlap, NULL );
}



bool SocketAsync_Impl::OverlappedSend::Finished()
{
	return Event.Wait(0);
}

//////////////////////////////////////////////////////////////////////////
// oSocketClientAsyncReceiverThread
//////////////////////////////////////////////////////////////////////////
oSocketAsyncReceiverThread::oSocketAsyncReceiverThread( const char* _pName, bool* _pSuccess )
	: AddingConnection(NULL)
	, DroppingConnection(NULL)
{
	*_pSuccess = false;
	oWinsock* ws = oWinsock::Singleton();

	// All events must be created before the thread exists
	ConnectionEvent = ws->WSACreateEvent();

	if (!oThread::Create(_pName, 64*1024, false, this, &Thread) )
		return;

	Release(); // prevent circular ref
		
	ValidConnection.reserve(MAX_SERVICEABLE_SOCKETS);

#ifdef _DEBUG
	ConnectedSockets.reserve(MAX_SERVICEABLE_SOCKETS);
#endif

	*_pSuccess = true;
	
}

oSocketAsyncReceiverThread::~oSocketAsyncReceiverThread()
{
	oWinsock* ws = oWinsock::Singleton();

	if (Thread)
	{
		Thread->Exit();

		// Fire the event one last time which will wake the thread up allowing it to 
		// shutdown cleanly
		ws->WSASetEvent( ConnectionEvent );

		oVERIFY( Thread->Wait( 2000 ) );
	}

	ws->WSACloseEvent( ConnectionEvent );
}

void oSocketAsyncReceiverThread::AddConnection(SocketAsync_Impl* _pConnection)
{
	// Set the connection as the connection to be added and wake the threading waiting for it to be processed
	AddingConnection = _pConnection;
	WakeupThreadAndWait();

#ifdef _DEBUG
	DEBUG_ConnectionStats stats;
	stats.ConnectedTimestamp = oTimer();
	stats.DisconnectedTimestamp = 0;
	stats.pSocket = _pConnection;

	ConnectedSockets.push_back(stats);
#endif
}


void oSocketAsyncReceiverThread::DropConnection(SocketAsync_Impl* _pConnection)
{
	// Set the connection as the connection to be dropped and wake the threading waiting for it to be processed
	DroppingConnection = _pConnection;
	WakeupThreadAndWait();

#ifdef _DEBUG
	for( std::vector<DEBUG_ConnectionStats>::iterator iter = ConnectedSockets.begin(); iter != ConnectedSockets.end(); ++iter )
	{
		if( iter->pSocket == _pConnection )
		{
			iter->DisconnectedTimestamp = oTimer();
			return;
		}
	}
	oASSERT( false, "DropConnection is dropping and unknown socket");
#endif
}

void oSocketAsyncReceiverThread::WakeupThreadAndWait()
{
	oWinsock* ws = oWinsock::Singleton();
	ws->WSASetEvent( ConnectionEvent );
	ConnectionProcessedEvent.Wait();
	ConnectionProcessedEvent.Reset();
}


void oSocketAsyncReceiverThread::RunIteration()
{
	oWinsock* ws = oWinsock::Singleton();
	if( WSA_WAIT_EVENT_0 == ws->WSAWaitForMultipleEvents(1, &ConnectionEvent, true, WSA_INFINITE, true ) )
	{
		// There are only three times the event can be fired.
		// 1) A new connection has arrived
		// 2) A connection is being dropped
		// 3) The thread is being torn down.  
		// By not resetting the event
		// during teardown, we allow RunIteration to quickly
		// complete allowing the thread to be torn down.
		if( AddingConnection )
		{
			ValidConnection.push_back( AddingConnection );
			AddingConnection->BeginReceiving( &ValidConnection.back() );
			AddingConnection = NULL;
			goto resetevents;
		}

		if( DroppingConnection )
		{
			DroppingConnection->EndReceiving();
			DroppingConnection = NULL;
			goto resetevents;
		}

		return;

resetevents:
		ConnectionProcessedEvent.Set();
		ws->WSAResetEvent(ConnectionEvent); 
	}
}



//////////////////////////////////////////////////////////////////////////
// oSocketClientAsyncReceiver
//////////////////////////////////////////////////////////////////////////
SocketClientAsyncReceiver_Impl::SocketClientAsyncReceiver_Impl( const char* _pDebugName, bool* _pSuccess )
{	
	bool success = false;
	ReceiverThread /= new oSocketAsyncReceiverThread(_pDebugName, &success );
	if( !success ) goto error;

	*_pSuccess = true;
	return;

error:
	*_pSuccess = false;
	return;
}


bool SocketClientAsyncReceiver_Impl::AddSocket(threadsafe oSocketAsync* _pSocket, oFUNCTION<void(void*,oSocket::size_t)> _Callback) threadsafe
{
	threadsafe SocketAsync_Impl* pSocketImpl = static_cast<threadsafe SocketAsync_Impl*>( _pSocket );
	if( !pSocketImpl->SetOwnership( this, _Callback ) )
		return false;

	oRWMutex::ScopedLock lock(Mutex);

	// thread_cast is safe because we now have ownership of the connection and this via the mutex
	thread_cast<oSocketAsyncReceiverThread*>(ReceiverThread.c_ptr() )->AddConnection( thread_cast<SocketAsync_Impl*>( pSocketImpl ) );
	return true;
}

void SocketClientAsyncReceiver_Impl::DropSocket(threadsafe oSocketAsync* _pSocket) threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);

	// thread_cast is safe because we now have ownership of the connection and this via the mutex
	thread_cast<oSocketAsyncReceiverThread*>(ReceiverThread.c_ptr() )->DropConnection(static_cast<SocketAsync_Impl*>( thread_cast<oSocketAsync*>( _pSocket ) ));
}

struct SocketServer_Impl : public oSocketServer
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);

	SocketServer_Impl(const char* _DebugName, const DESC& _Desc, bool* _pSuccess);
	~SocketServer_Impl();
	const char* GetDebugName() const threadsafe override;
	bool WaitForConnection(threadsafe oSocketAsync** _ppNewlyConnectedClient, unsigned int _TimeoutMS) threadsafe override;
	bool WaitForConnection(threadsafe oSocketBlocking** _ppNewlyConnectedClient, unsigned int _TimeoutMS) threadsafe override;

private:
	oRWMutex Mutex;
	oRefCount RefCount;
	SOCKET hSocket;
	WSAEVENT hConnectEvent;
	char DebugName[64];
	DESC Desc;
};

bool oSocketServer::Create(const char* _DebugName, const DESC& _Desc, threadsafe oSocketServer** _ppSocketServer)
{
	if (!_DebugName || !_ppSocketServer)
	{
		oSetLastError(EINVAL);
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppSocketServer, SocketServer_Impl(_DebugName, _Desc, &success));
	return success;
}

SocketServer_Impl::SocketServer_Impl(const char* _DebugName, const DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
{
	*DebugName = 0;
	if (_DebugName)
		strcpy_s(DebugName, _DebugName);

	*_pSuccess = false;

	char hostname[64];
	sprintf_s(hostname, "0.0.0.0:%u", Desc.ListenPort);

	hSocket = oWinsockCreate(hostname, oWINSOCK_RELIABLE|oWINSOCK_REUSE_ADDRESS, Desc.MaxNumConnections, 0, 0);
	if (INVALID_SOCKET == hSocket)
		return; // leave last error from inside oWinsockCreate

	hConnectEvent = oWinsock::Singleton()->WSACreateEvent();
	if (SOCKET_ERROR == oWinsock::Singleton()->WSAEventSelect(hSocket, hConnectEvent, FD_ACCEPT))
	{
		oWINSOCK_SETLASTERROR("WSAEventSelect");
		return;
	}

	*_pSuccess = true;
}

SocketServer_Impl::~SocketServer_Impl()
{
	if (INVALID_SOCKET != hSocket)
		oVERIFY(oWinsockClose(hSocket));

	if (hConnectEvent)
		oWinsock::Singleton()->WSACloseEvent(hConnectEvent);
}

const char* SocketServer_Impl::GetDebugName() const threadsafe 
{
	return thread_cast<const char*>(DebugName); // threadsafe because name never changes
}

static bool UNIFIED_WaitForConnection(
	const char* _ServerDebugName
	, threadsafe oRWMutex& _Mutex
	, threadsafe WSAEVENT _hConnectEvent
	, unsigned int _TimeoutMS
	, SOCKET _hServerSocket
	, oFUNCTION<void*(const char* _DebugName, SOCKET _hTarget, const char* _FullHostname, unsigned int _TimeoutMS, bool* _pSuccess)> _CreateClientSocket
	, void** _ppNewlyConnectedClient)
{
	oRWMutex::ScopedLock lock(_Mutex);
	oWinsock* ws = oWinsock::Singleton();
	bool success = false;

	if (oWinsockWaitMultiple(thread_cast<WSAEVENT*>(&_hConnectEvent), 1, true, false, _TimeoutMS)) // thread_cast safe because of mutex
	{
		sockaddr_in saddr;
		int size = sizeof(saddr);
		SOCKET hTarget = ws->accept(_hServerSocket, (sockaddr*)&saddr, &size);
		if (hTarget != INVALID_SOCKET)
		{
			char hostname[_MAX_PATH];
			char ip[_MAX_PATH];
			char port[16];
			oVERIFY(oWinsockGetHostname(hostname, ip, port, hTarget));
			oTRACE("oSocketServer %s accepting connection from %s:%s (%s:%s)", _ServerDebugName, hostname, port, ip, port);

			char fullHostname[_MAX_PATH];
			sprintf_s(fullHostname, "%s:%s", hostname, port);

			char debugName[_MAX_PATH];
			sprintf_s(debugName, "connection to %s", fullHostname);

			*_ppNewlyConnectedClient = _CreateClientSocket(debugName, hTarget, fullHostname, _TimeoutMS, &success);
		}
	}

	else
	{
		*_ppNewlyConnectedClient = 0;
		oSetLastError(0); // It's ok if we don't find a connection
	}

	return success;
}

void* CreateSocket2(const char* _DebugName, SOCKET _hTarget, const char* _FullHostname, unsigned int _TimeoutMS, bool* _pSuccess)
{
	bool success = false;
	SocketAsync_Impl* pSocket;
	oCONSTRUCT( &pSocket, SocketAsync_Impl(_hTarget, _DebugName, _FullHostname, _TimeoutMS, &success) );
	*_pSuccess = success;
	return pSocket;
}

bool SocketServer_Impl::WaitForConnection(threadsafe oSocketAsync** _ppNewlyConnectedClient, unsigned int _TimeoutMS) threadsafe
{
	return UNIFIED_WaitForConnection(GetDebugName(), Mutex, hConnectEvent, _TimeoutMS, hSocket, CreateSocket2, (void**)_ppNewlyConnectedClient);
}

extern void* CreateSocketClient(const char* _DebugName, SOCKET _hTarget, const char* _FullHostname, unsigned int _TimeoutMS, bool* _pSuccess);
bool SocketServer_Impl::WaitForConnection(threadsafe oSocketBlocking** _ppNewlyConnectedClient, unsigned int _TimeoutMS) threadsafe
{
	return UNIFIED_WaitForConnection(GetDebugName(), Mutex, hConnectEvent, _TimeoutMS, hSocket, CreateSocketClient, (void**)_ppNewlyConnectedClient);
}

struct SocketSender_Impl : public oSocketSender
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);

	SocketSender_Impl(const char* _DebugName, const DESC* _pDesc, bool* _pSuccess);
	~SocketSender_Impl();

	static bool Create(const char* _DebugName, const DESC* _pDesc, threadsafe oSocketSender** _ppSocketSender);
	bool Send(const void* _pSource, size_t _SizeofSource) threadsafe override 
	{ 
		//@oooii-Andrew: since we own the socket, it's safe to do a thread_cast
		oMutex::ScopedLock lock(Mutex); 
		return oWinsockSend(hSocket, _pSource, _SizeofSource, thread_cast<sockaddr_in*>(&Destination)); 
	}

	DESC Desc;
	SOCKET hSocket;
	oRefCount RefCount;
	oMutex Mutex;
	sockaddr_in Destination;
	char DebugName[64];
};

SocketSender_Impl::SocketSender_Impl(const char* _DebugName, const DESC* _pDesc, bool* _pSuccess)
{
	*DebugName = 0; 
	if (_DebugName)
		strcpy_s(DebugName, _DebugName);

	char hostname[64];
	sprintf_s(hostname, "0.0.0.0:%u", _pDesc->SendPort);

	*_pSuccess = true;
	hSocket = oWinsockCreate(hostname, oWINSOCK_REUSE_ADDRESS, 0, _pDesc->SendBufferSize, 0);
	if (hSocket == INVALID_SOCKET)
		*_pSuccess = false;

	oWinsockCreateAddr(&Destination, _pDesc->ReceiverHostname);
}

SocketSender_Impl::~SocketSender_Impl()
{
	if (hSocket)
		oWinsock::Singleton()->shutdown(hSocket, SD_BOTH);
}

struct SocketReceiver_Impl : public oSocketReceiver
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);

	SocketReceiver_Impl(const char* _DebugName, const DESC* _pDesc, bool* _pSuccess);
	~SocketReceiver_Impl();

	static bool Create(const char* _DebugName, const DESC* _pDesc, threadsafe oSocketSender** _ppSocketSender);

	size_t Receive(void* _pDestination, size_t _SizeofDestination, unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe override 
	{ 
		//@oooii-Andrew: since the receiver owns the socket, it's safe to do a thread_cast
		oMutex::ScopedLock lock(Mutex); 
		return oWinsockReceive(hSocket, hEvent, _pDestination, _SizeofDestination, _TimeoutMS, thread_cast<int*>(&bCanReceive), thread_cast<sockaddr_in*>(&Source));
	}

	DESC Desc;
	SOCKET hSocket;
	WSAEVENT hEvent;
	oRefCount RefCount;
	oMutex Mutex;
	sockaddr_in Source;
	int bCanReceive; // atomic bool, if a receive fails due to disconnect, don't allow another call to wait/block indefinitely
	char DebugName[64];
};

SocketReceiver_Impl::SocketReceiver_Impl(const char* _DebugName, const DESC* _pDesc, bool* _pSuccess)
: bCanReceive(true)
{
	*_pSuccess = false;
	*DebugName = 0;
	if (_DebugName)
		strcpy_s(DebugName, _DebugName);

	hEvent = oWinsock::Singleton()->WSACreateEvent();

	char hostname[64];
	sprintf_s(hostname, "0.0.0.0:%u", _pDesc->ReceivePort);
	hSocket = oWinsockCreate(hostname, oWINSOCK_REUSE_ADDRESS, 0, 0, _pDesc->ReceiveBufferSize);
	if (hSocket == INVALID_SOCKET)
		return; // pass thru errors set in oWinsockCreate

	if (SOCKET_ERROR == oWinsock::Singleton()->WSAEventSelect(hSocket, hEvent, FD_ALL_EVENTS))
	{
		oWINSOCK_SETLASTERROR("WSAEventSelect");
		return;
	}

	oWinsockCreateAddr(&Source, _pDesc->SenderHostname);
	*_pSuccess = true;
}

SocketReceiver_Impl::~SocketReceiver_Impl()
{
	if (hSocket)
		oWinsock::Singleton()->shutdown(hSocket, SD_BOTH);

	if (hEvent)
		oVB(oWinsock::Singleton()->WSACloseEvent(hEvent));
}

bool oSocketSender::Create(const char* _DebugName, const DESC* _pDesc, threadsafe oSocketSender** _ppSocketSender)
{
	if (!_DebugName || !_pDesc || !_ppSocketSender)
	{
		oSetLastError(EINVAL);
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppSocketSender, SocketSender_Impl(_DebugName, _pDesc, &success));
	return !!*_ppSocketSender;
}

bool oSocketReceiver::Create(const char* _DebugName, const DESC* _pDesc, threadsafe oSocketReceiver** _ppSocketReceiver)
{
	if (!_DebugName || !_pDesc || !_ppSocketReceiver)
	{
		oSetLastError(EINVAL);
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppSocketReceiver, SocketReceiver_Impl(_DebugName, _pDesc, &success));
	return !!*_ppSocketReceiver;
}