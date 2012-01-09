// $(header)
#include <oPlatform/oSocket.h>
#include <oBasis/oBlockAllocatorGrowable.h>
#include <oBasis/oMutex.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oString.h>
#include <oPlatform/oSocket.h>
#include "oIOCP.h"
#include "oWinsock.h"
#include "oOpenSSL.h"

// The Internal versions of these structs simply have the private
// classification removed. In the event that we need to address multiple
// families of address types in the same program we can convert this to
// a traditional blob pattern.
struct oNetHost_Internal
{
	unsigned long IP;
};

struct oNetAddr_Internal
{
	oNetHost Host;
	unsigned short Port;
};

inline void oNetAddrToSockAddr(const oNetAddr& _NetAddr, SOCKADDR_IN* _pSockAddr)
{
	const oNetAddr_Internal* pAddr = reinterpret_cast<const oNetAddr_Internal*>(&_NetAddr);
	const oNetHost_Internal* pHost = reinterpret_cast<const oNetHost_Internal*>(&pAddr->Host);

	_pSockAddr->sin_addr.s_addr = pHost->IP;
	_pSockAddr->sin_port = pAddr->Port;
	_pSockAddr->sin_family = AF_INET;
}

inline void oSockAddrToNetAddr(const SOCKADDR_IN& _SockAddr, oNetAddr* _pNetAddr)
{
	oNetAddr_Internal* pAddr = reinterpret_cast<oNetAddr_Internal*>(_pNetAddr);
	oNetHost_Internal* pHost = reinterpret_cast<oNetHost_Internal*>(&pAddr->Host);

	pHost->IP = _SockAddr.sin_addr.s_addr;
	pAddr->Port = _SockAddr.sin_port;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oNetHost& _Host)
{
	const oNetHost_Internal* pHost = reinterpret_cast<const oNetHost_Internal*>(&_Host);
	oWinsock* ws = oWinsock::Singleton();
	unsigned long addr = ws->ntohl(pHost->IP);
	return -1 != sprintf_s(_StrDestination, _SizeofStrDestination, "%u.%u.%u.%u", (addr&0xFF000000)>>24, (addr&0xFF0000)>>16, (addr&0xFF00)>>8, addr&0xFF) ? _StrDestination : nullptr;
}

bool oFromString(oNetHost* _pHost, const char* _StrSource)
{
	oNetHost_Internal* pHost = reinterpret_cast<oNetHost_Internal*>(_pHost);
	oWinsock* ws = oWinsock::Singleton();

	ADDRINFO* pAddrInfo = nullptr;
	ADDRINFO Hints;
	memset(&Hints, 0, sizeof(Hints));
	Hints.ai_family = AF_INET;
	ws->getaddrinfo(_StrSource, nullptr, &Hints, &pAddrInfo);

	if (!pAddrInfo)
		return false;

	pHost->IP = ((SOCKADDR_IN*)pAddrInfo->ai_addr)->sin_addr.s_addr;
	ws->freeaddrinfo(pAddrInfo);
	return true;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oNetAddr& _Address)
{
	if (oToString(_StrDestination, _SizeofStrDestination, _Address.Host))
	{
		const oNetAddr_Internal* pAddress = reinterpret_cast<const oNetAddr_Internal*>(&_Address);
		oWinsock* ws = oWinsock::Singleton();
		size_t len = strlen(_StrDestination);
		return -1 == sprintf_s(_StrDestination + len, _SizeofStrDestination - len, ":%u", ws->ntohs(pAddress->Port)) ? _StrDestination : nullptr;
	}

	return nullptr;
}

bool oFromString(oNetAddr* _pAddress, const char* _StrSource)
{
	char tempStr[512];
	oASSERT(strlen(_StrSource) < oCOUNTOF(tempStr)+1, "");
	strcpy_s(tempStr, _StrSource);

	char* seperator = strstr(tempStr, ":");

	if (!seperator)
		return false;

	*seperator = 0;

	oWinsock* ws = oWinsock::Singleton();
	ADDRINFO* pAddrInfo = nullptr;
	ADDRINFO Hints;
	memset(&Hints, 0, sizeof(Hints));
	Hints.ai_family = AF_INET;
	ws->getaddrinfo(tempStr, seperator+1, &Hints, &pAddrInfo);

	if (!pAddrInfo)
		return false;

	oSockAddrToNetAddr(*((SOCKADDR_IN*)pAddrInfo->ai_addr), _pAddress);
	ws->freeaddrinfo(pAddrInfo);
	return true;
}


oAPI void oSocketPortGet(const oNetAddr& _Addr, unsigned short* _pPort)
{
	oWinsock* ws = oWinsock::Singleton();
	const oNetAddr_Internal* pAddr = reinterpret_cast<const oNetAddr_Internal*>(&_Addr);
	*_pPort = ws->ntohs(pAddr->Port);
}

oAPI void oSocketPortSet(const unsigned short _Port, oNetAddr* _pAddr)
{
	oWinsock* ws = oWinsock::Singleton();
	oNetAddr_Internal* pAddr = reinterpret_cast<oNetAddr_Internal*>(_pAddr);
	pAddr->Port = ws->htons(_Port);
}


oSocket::size_t oWinsockRecvBlocking(SOCKET hSocket, void* _pData, oSocket::size_t _szReceive, unsigned int _Timeout, unsigned int _Flags = 0)
{
	oWinsock* ws = oWinsock::Singleton();

	oSocket::size_t TotalReceived = 0;
	oScopedPartialTimeout PartialTimeout(&_Timeout); // @oooii-tony: moving this to oStd::chrono is a bit tricky because we need to pass #ms to setsockopt and decrement it each iteration
	do
	{
		if (SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&_Timeout, sizeof(unsigned int))) 
			goto error;

		oSocket::size_t received = ws->recv(hSocket, (char*)_pData + TotalReceived, _szReceive - TotalReceived, _Flags);
		if(SOCKET_ERROR == received) 
			goto error;

		TotalReceived += received;

		PartialTimeout.UpdateTimeout();
	} while(_Timeout && TotalReceived < _szReceive);
	
	return TotalReceived;
error:

	oErrorSetLast(oERROR_IO, "%s", oWinsock::AsString(oWinsock::Singleton()->WSAGetLastError()));
	return TotalReceived;
}

bool oWinsockSendBlocking(SOCKET hSocket, const void* _pData, oSocket::size_t _szSend, unsigned int _Timeout)
{
	oWinsock* ws = oWinsock::Singleton();

	if (SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO,(char *)&_Timeout, sizeof(unsigned int))) goto error;

	if(_szSend != (oSocket::size_t) ws->send(hSocket, (const char*)_pData, _szSend, 0)) goto error;

	return true;
error:

	oErrorSetLast(oERROR_IO, "%s", oWinsock::AsString(oWinsock::Singleton()->WSAGetLastError()));
	return false;
}

//void oSocket::ForceInitWinSock()
//{
//	oWinsock::Singleton();
//}

// {768192D8-BECE-4981-9351-03790001A8C3}
static const oGUID oMSGGUID = { 0x768192d8, 0xbece, 0x4981, { 0x93, 0x51, 0x3, 0x79, 0x0, 0x1, 0xa8, 0xc3 } };

struct CLIENT_PACKET_HEADER
{
	int MagicNumber; // 'oNET'
	oSocket::size_t MessageLength;
};

#define oDEFINE_SOCKET_SHARED_IMPL() \
	const char* GetDebugName() const threadsafe override { return thread_cast<const char*>(DebugName); /* safe because DebugName never change */ } \
	bool IsConnected() const threadsafe override { return oWinsockIsConnected(hSocket); } \
	bool GetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe override { return oWinsockGetHostname(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, hSocket); } \
	bool GetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe override { return oWinsockGetPeername(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, hSocket); }

struct SocketBlocking_Impl : public oSocketEncrypted
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oSocket);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);
	oDEFINE_SOCKET_SHARED_IMPL();

	SocketBlocking_Impl(const char* _DebugName, const DESC& _Desc, SOCKET _SystemSocket, bool* _pSuccess);
	~SocketBlocking_Impl();

	bool Send(const void* _pSource, oSocket::size_t _SizeofSource) threadsafe override; //, unsigned int _TimeoutMS /*= oINFINITE_WAIT */) threadsafe override;
	bool SendTo(const void* _pSource, oSocket::size_t _SizeofSource, const oNetAddr& _DestinationAddr) threadsafe override;//, unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe override;
	oSocket::size_t Recv(void* _pDestination, oSocket::size_t _SizeofDestination) threadsafe override;
	
	bool SendEncrypted(const void* _pData, oSocket::size_t _Size) threadsafe override;
	oSocket::size_t RecvEncrypted(void* _pBuffer, oSocket::size_t _Size) threadsafe override;

	DESC Desc;
	SOCKET hSocket;
	oRefCount RefCount;
	oMutex SendMutex;
	oMutex ReceiveMutex;
	char DebugName[64];
	int bCanReceive; // atomic bool, if a receive fails due to disconnect, don't allow another call to wait/block indefinitely
	oSocket::size_t LeftOverBytesToFlush;
	errno_t LeftOverError;
	oRef<oSocketEncryptor> Encryptor;
};

// @oooii-tony: temporary glue code to keep the new unified oSocketServer 
// implementation relatively centralized. I haven't thought about how to unify
// oSocketClient and oSocketClientAsync yet... that's next.
oSocket* CreateSocketBlocking(const char* _DebugName, SOCKET _hTarget, oSocket::DESC SocketDesc, bool* _pSuccess)
{
	// Place the socket into non-blocking mode by first clearing the event then disabling FIONBIO
	{
		oWinsock* ws = oWinsock::Singleton();

		if(SOCKET_ERROR == ws->WSAEventSelect(_hTarget, NULL, NULL)) return nullptr;

		u_long nonBlocking = 0;
		if (SOCKET_ERROR == ws->ioctlsocket(_hTarget, FIONBIO, &nonBlocking)) return nullptr;
	}

	return new SocketBlocking_Impl(_DebugName, SocketDesc, _hTarget, _pSuccess);
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

	if (INVALID_SOCKET == hSocket) // create a socket from scratch (client-side)
	{
		SOCKADDR_IN saddr;
		oNetAddrToSockAddr(Desc.Addr, &saddr);

		int flags = oWINSOCK_REUSE_ADDRESS|oWINSOCK_BLOCKING;
		if(Desc.Protocol != UDP)
			flags |= oWINSOCK_RELIABLE;

		// Client side socket attempting to connect to a server
		hSocket = oWinsockCreate(saddr, flags, Desc.ConnectionTimeoutMS);
	
		if (hSocket == INVALID_SOCKET)
			return; // respect last-error settings from oWinsockCreate()

		if(oMSG == Desc.Protocol)
		{
			// Inform the server that we're using oMSG
			if(!oWinsockSendBlocking(hSocket, &oMSGGUID, sizeof(oMSGGUID), Desc.ConnectionTimeoutMS))
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

bool SocketBlocking_Impl::SendTo(const void* _pSource, oSocket::size_t _SizeofSource, const oNetAddr& _DestinationAddr) threadsafe
{
	unsigned int _TimeoutMS = Desc.BlockingSettings.SendTimeout;

	sockaddr_in sendToAddr;
	oNetAddrToSockAddr(thread_cast<oNetAddr &>(_DestinationAddr), &sendToAddr);

	oLockGuard<oMutex> lock(SendMutex);
	oWinsock* ws = oWinsock::Singleton();

	oScopedPartialTimeout Timeout(&_TimeoutMS);
	if (SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO,(char *)&_TimeoutMS, sizeof(unsigned int)))
		return false;

	if(Desc.Protocol == oMSG)
	{
		CLIENT_PACKET_HEADER header;
		header.MagicNumber = 'oNET';
		header.MessageLength = _SizeofSource;
		oASSERT(header.MessageLength == _SizeofSource, "");

		if(!oWinsockSend(hSocket, &header, sizeof(header), 0))
		if(SOCKET_ERROR == ws->sendto(hSocket, (const char*)&header, _SizeofSource, 0, (const sockaddr*)&sendToAddr, sizeof(sockaddr_in)))
			return false;

		if (SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO,(char *)&_TimeoutMS, sizeof(unsigned int)))
			return false;

		Timeout = oScopedPartialTimeout(&_TimeoutMS);
	}

	if(SOCKET_ERROR == ws->sendto(hSocket, (const char*)_pSource, _SizeofSource, 0, (const sockaddr*)&sendToAddr, sizeof(sockaddr_in)))
		return false;

	return true;
}

bool SocketBlocking_Impl::SendEncrypted(const void* _pData, oSocket::size_t _Size) threadsafe
{
	int ret = 0;
	
	oLockGuard<oMutex> lock(SendMutex);

	// Lazy init and Open SSL Connection because google's TLS requires that STARTTLS be sent and response received
	// before an TLS connecion can be established.  This may be different for other servers.
	if (!Encryptor.c_ptr())
	{
		oSocketEncryptor::Create(&Encryptor);
		// Open SSLconnection
		if (Encryptor.c_ptr())
			Encryptor->OpenSSLConnection(hSocket, Desc.BlockingSettings.SendTimeout);
	}

	if (Encryptor.c_ptr())
	{
		ret = Encryptor->Send(hSocket, _pData, _Size, Desc.BlockingSettings.SendTimeout);
	}
	return (_Size == (size_t)ret);
}

oSocket::size_t SocketBlocking_Impl::RecvEncrypted(void* _pBuffer, oSocket::size_t _Size) threadsafe
{
	oLockGuard<oMutex> lock(ReceiveMutex);
	if (Encryptor.c_ptr())
	{
		return Encryptor->Receive(hSocket, (char *)_pBuffer, _Size, Desc.BlockingSettings.RecvTimeout);
	}
	return 0;
}

// @oooii-will: Now using SendTo for all SocketBlocking send commands
bool SocketBlocking_Impl::Send(const void* _pSource, oSocket::size_t _SizeofSource) threadsafe
{
	return SendTo(_pSource, _SizeofSource, thread_cast<oNetAddr &>(Desc.Addr));
}

oSocket::size_t SocketBlocking_Impl::Recv(void* _pDestination, oSocket::size_t _SizeofDestination) threadsafe
{
	unsigned int _TimeoutMS = Desc.BlockingSettings.RecvTimeout;

	oLockGuard<oMutex> lock(ReceiveMutex);

	if(oMSG != Desc.Protocol)
	{
		oLocalTimeout to(_TimeoutMS);

		size_t recvSize = 0;
		do
		{
			WSAEVENT hEvent = 0;
			::size_t bytesRecvd = 0;
			if(!oWinsockReceiveNonBlocking(hSocket, hEvent, (char*)_pDestination + recvSize, _SizeofDestination - recvSize, nullptr, &bytesRecvd))
				return 0;

			recvSize += (size_t)bytesRecvd;
		} while(!recvSize && !to.TimedOut());

		return recvSize;
	}

	oScopedPartialTimeout Timeout(&_TimeoutMS); // @oooii-tony: Why is a scoped partial timeout used here?

	CLIENT_PACKET_HEADER header;
	if(!oWinsockRecvBlocking(hSocket, &header, sizeof(CLIENT_PACKET_HEADER), _TimeoutMS))
		return 0;

	if(_SizeofDestination < header.MessageLength)
		return 0;

	Timeout = oScopedPartialTimeout(&_TimeoutMS); // @oooii-tony: Why is a scoped partial timeout used here?

	if(!oWinsockRecvBlocking(hSocket, _pDestination, header.MessageLength, _TimeoutMS))
		return 0;

	return header.MessageLength;
}

const oGUID& oGetGUID(threadsafe const oSocketServer* threadsafe const *)
{
	// {EE38455C-A057-4b72-83D2-4E809FF1C059}
	static const oGUID oIIDSocketServer = { 0xee38455c, 0xa057, 0x4b72, { 0x83, 0xd2, 0x4e, 0x80, 0x9f, 0xf1, 0xc0, 0x59 } };
	return oIIDSocketServer;
}

struct SocketServer_Impl : public oSocketServer
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oSocketServer);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);

	SocketServer_Impl(const char* _DebugName, const DESC& _Desc, bool* _pSuccess);
	~SocketServer_Impl();
	const char* GetDebugName() const threadsafe override;
	bool WaitForConnection(const oSocket::BLOCKING_SETTINGS& _BlockingSettings, threadsafe oSocket** _ppNewlyConnectedClient, unsigned int _TimeoutMS) threadsafe override;
	bool WaitForConnection(const oSocket::ASYNC_SETTINGS& _AsyncSettings, threadsafe oSocket** _ppNewlyConnectedClient, unsigned int _TimeoutMS) threadsafe override;
	bool GetHostname(char* _pString, size_t _strLen)  const threadsafe override;

private:
	oSharedMutex Mutex;
	oRefCount RefCount;
	SOCKET hSocket;
	WSAEVENT hConnectEvent;
	char DebugName[64];
	DESC Desc;
	oMutex AcceptedSocketsMutex;
	std::vector<oRef<oSocket>> AcceptedSockets;
};

bool oSocketServerCreate(const char* _DebugName, const oSocketServer::DESC& _Desc, threadsafe oSocketServer** _ppSocketServer)
{
	if (!_DebugName || !_ppSocketServer)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
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
	sprintf_s(hostname, "0.0.0.0:%i", Desc.ListenPort);

	hSocket = DEPRECATED_oWinsockCreate(hostname, oWINSOCK_RELIABLE|oWINSOCK_REUSE_ADDRESS, Desc.MaxNumConnections, 0, 0);
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

bool SocketServer_Impl::GetHostname(char* _pString, size_t _strLen) const threadsafe 
{
	return oWinsockGetHostname(_pString, _strLen, nullptr, NULL, nullptr, NULL, hSocket);
}

static bool UNIFIED_WaitForConnection(
	const char* _ServerDebugName
	, threadsafe oSharedMutex& _Mutex
	, threadsafe WSAEVENT _hConnectEvent
	, unsigned int _TimeoutMS
	, SOCKET _hServerSocket
	, oSocket::DESC _Desc
	, oFUNCTION<oSocket*(const char* _DebugName, SOCKET _hTarget, oSocket::DESC SocketDesc, bool* _pSuccess)> _CreateClientSocket
	, threadsafe oMutex& _AcceptedSocketsMutex
	, threadsafe std::vector<oRef<oSocket>>& _AcceptedSockets)
{
	oLockGuard<oSharedMutex> lock(_Mutex);
	oWinsock* ws = oWinsock::Singleton();
	bool success = false;

	oScopedPartialTimeout timeout = oScopedPartialTimeout(&_TimeoutMS);
	if (oWinsockWaitMultiple(thread_cast<WSAEVENT*>(&_hConnectEvent), 1, true, false, _TimeoutMS)) // thread_cast safe because of mutex
	{
		sockaddr_in saddr;
		int size = sizeof(saddr);
		ws->WSAResetEvent(_hConnectEvent); // be sure to reset otherwise the wait will always return immediately. however we could have more than 1 waiting to be accepted.
		SOCKET hTarget;
		do 
		{
			hTarget = ws->accept(_hServerSocket, (sockaddr*)&saddr, &size);
			if (hTarget != INVALID_SOCKET)
			{
				char hostname[_MAX_PATH];
				char ip[_MAX_PATH];
				char port[16];
				oVERIFY(oWinsockGetHostname(hostname, ip, port, hTarget));
				oTRACE("oSocketServer %s accepting connection from %s:%s (%s:%s)", _ServerDebugName, hostname, port, ip, port);	

				
				// Determine protocol
				{
					_Desc.Protocol = oSocket::TCP; // Default to TCP

					// Check to see if this is a oMSG connection by peeking at the GUID
					timeout = oScopedPartialTimeout(&_TimeoutMS);
					oGUID ProtocolGUID;
					if(oWinsockRecvBlocking(hTarget, &ProtocolGUID, sizeof(ProtocolGUID), _TimeoutMS, MSG_PEEK))
					{
						if(ProtocolGUID == oMSGGUID)
						{
							_Desc.Protocol = oSocket::oMSG;

							// Drain the GUID
							oWinsockRecvBlocking(hTarget, &ProtocolGUID, sizeof(ProtocolGUID), _TimeoutMS);
						}
					}
				}
				// Fill in the remaining portions of the desc
				char fullHostname[_MAX_PATH];
				sprintf_s(fullHostname, "%s:%s", hostname, port);
				oFromString(&_Desc.Addr, fullHostname);
				_Desc.ConnectionTimeoutMS = _TimeoutMS;

				char debugName[_MAX_PATH];
				sprintf_s(debugName, "connection to %s", fullHostname);

				oRef<oSocket> newSocket(_CreateClientSocket(debugName, hTarget, _Desc, &success), false);
				{
					oLockGuard<oMutex> lock(_AcceptedSocketsMutex);
					thread_cast<std::vector<oRef<oSocket>>&>(_AcceptedSockets).push_back(newSocket); // safe because of lock above
				}

				success = true;
			}
		} while (hTarget != INVALID_SOCKET);
	}

	else
	{
		oErrorSetLast(oERROR_NONE); // It's ok if we don't find a connection
	}

	return success;
}

template<typename T> static inline bool FindTypedSocket(threadsafe oMutex& _AcceptedSocketsMutex, threadsafe std::vector<oRef<oSocket>>& _AcceptedSockets, T** _ppNewlyConnectedClient)
{
	oLockGuard<oMutex> lock(_AcceptedSocketsMutex);
	std::vector<oRef<oSocket>>& SafeSockets = thread_cast<std::vector<oRef<oSocket>>&>(_AcceptedSockets);

	if (!SafeSockets.empty())
	{
		for (std::vector<oRef<oSocket>>::iterator it = SafeSockets.begin(); it != SafeSockets.end(); ++it)
		{
			oSocket* s = *it;
			if (s->QueryInterface(oGetGUID<T>(), (void**)_ppNewlyConnectedClient))
			{
				SafeSockets.erase(it);
				return true;
			}
		}
	}

	return false;
}

bool SocketServer_Impl::WaitForConnection(const oSocket::BLOCKING_SETTINGS& _BlockingSettings, threadsafe oSocket** _ppNewlyConnectedClient, unsigned int _TimeoutMS) threadsafe
{
	if (FindTypedSocket(AcceptedSocketsMutex, AcceptedSockets, _ppNewlyConnectedClient))
		return true;

	oSocket::DESC Desc;
	Desc.Style = oSocket::BLOCKING;
	Desc.BlockingSettings = _BlockingSettings;
	Desc.ConnectionTimeoutMS = _TimeoutMS;
	Desc.BlockingSettings.RecvTimeout = _TimeoutMS;
	Desc.BlockingSettings.SendTimeout = _TimeoutMS;

	bool result = UNIFIED_WaitForConnection(GetDebugName(), Mutex, hConnectEvent, _TimeoutMS, hSocket, Desc, CreateSocketBlocking, AcceptedSocketsMutex, AcceptedSockets);
	
	if (result)
		result = FindTypedSocket(AcceptedSocketsMutex, AcceptedSockets, _ppNewlyConnectedClient);

	return result;
}

oSocket* CreateSocketAsync(const char* _DebugName, SOCKET _hTarget, oSocket::DESC _SocketDesc, bool* _pSuccess);
bool SocketServer_Impl::WaitForConnection(const oSocket::ASYNC_SETTINGS& _AsyncSettings, threadsafe oSocket** _ppNewlyConnectedClient, unsigned int _TimeoutMS) threadsafe
{
	if (FindTypedSocket(AcceptedSocketsMutex, AcceptedSockets, _ppNewlyConnectedClient))
		return true;

	oSocket::DESC Desc;
	Desc.Style = oSocket::ASYNC;
	Desc.AsyncSettings = _AsyncSettings;
	Desc.ConnectionTimeoutMS = _TimeoutMS;

	bool result = UNIFIED_WaitForConnection(GetDebugName(), Mutex, hConnectEvent, _TimeoutMS, hSocket, Desc, CreateSocketAsync, AcceptedSocketsMutex, AcceptedSockets);

	if (result)
		result = FindTypedSocket(AcceptedSocketsMutex, AcceptedSockets, _ppNewlyConnectedClient);

	return result; // NYI
}

const oGUID& oGetGUID(threadsafe const oSocket* threadsafe const *)
{
	// {68F693CE-B01B-4235-A401-787691707365}
	static const oGUID oIIDSocketAsyncUDP = { 0x68f693ce, 0xb01b, 0x4235, { 0xa4, 0x1, 0x78, 0x76, 0x91, 0x70, 0x73, 0x65 } };
	return oIIDSocketAsyncUDP;
}

struct oSocketAsyncoMSG : public oSocketAsyncCallback
{
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_NOOP_QUERYINTERFACE();
	oSocketAsyncoMSG(threadsafe oSocketAsyncCallback* _pChildCallback, unsigned int _MaxOutstandingMessages);
	~oSocketAsyncoMSG();

	virtual void ProcessSocketReceive(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe override;
	virtual void ProcessSocketSend(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe override;

	bool PrepareHeader(const void* _pData, oSocket::size_t _Size, WSABUF* pHeader) threadsafe;
	void ReleaseHeader(const WSABUF& _Header) threadsafe;

	oSocket::size_t ProtocolCanRecv(void* _pData, oSocket::size_t _Size) threadsafe;
private:
	void ProcessSocketReceiveInternal(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket);

	oBlockAllocatorGrowableT<CLIENT_PACKET_HEADER> HeaderPool;
	
	oRefCount Refcount;
	oRef<threadsafe oSocketAsyncCallback> Callback;
	oSocket::size_t ExpectedMessageSize;
	oSocket::size_t ReceivedMessageSize;
	oStd::atomic<oSocket::size_t> MaximumReceiveSize;
	static thread_local bool ProtocolRecvOverride;
};

thread_local bool oSocketAsyncoMSG::ProtocolRecvOverride = false;

oSocketAsyncoMSG::oSocketAsyncoMSG(threadsafe oSocketAsyncCallback* _pChildCallback, unsigned int _MaxOutstandingMessages)
	: Callback(_pChildCallback)
	, ExpectedMessageSize(oInvalid)
	, ReceivedMessageSize(0)
	, MaximumReceiveSize(0)
{}

oSocketAsyncoMSG::~oSocketAsyncoMSG()
{
	HeaderPool.Reset(); // Reset as there may be messages outstanding that will never complete because they would have been canceled by this time
}

void oSocketAsyncoMSG::ProcessSocketReceive(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe
{
	thread_cast<oSocketAsyncoMSG*>(this)->ProcessSocketReceiveInternal(_pData, _SizeData, _Addr, _pSocket);
};

void oSocketAsyncoMSG::ProcessSocketSend(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe
{
	Callback->ProcessSocketSend(_pData, _SizeData, _Addr, _pSocket);
};

oSocket::size_t oSocketAsyncoMSG::ProtocolCanRecv(void* _pData, oSocket::size_t _Size) threadsafe
{
	if(MaximumReceiveSize.compare_exchange(0, _Size) || ProtocolRecvOverride)
	{
		oSocket::size_t szToRecieve = ProtocolRecvOverride ? _Size : sizeof(CLIENT_PACKET_HEADER);
		ProtocolRecvOverride = false;
		return szToRecieve;
	}

	return 0;
}

bool oSocketAsyncoMSG::PrepareHeader(const void* _pData, oSocket::size_t _Size, WSABUF* pHeader) threadsafe
{
	CLIENT_PACKET_HEADER* pPacketHeader = HeaderPool.Allocate();
	oASSERT(pPacketHeader, "Failed to allocate header");

	pPacketHeader->MagicNumber = 'oNET';
	pPacketHeader->MessageLength = _Size;
	pHeader->buf = (char*)pPacketHeader;
	pHeader->len = sizeof(CLIENT_PACKET_HEADER);
	return true;
}

void oSocketAsyncoMSG::ReleaseHeader(const WSABUF& _Header)threadsafe
{
	oASSERT('oNET' == ((CLIENT_PACKET_HEADER*)(_Header.buf))->MagicNumber, "Invalid header");
	HeaderPool.Deallocate((CLIENT_PACKET_HEADER*)_Header.buf);
};

void oSocketAsyncoMSG::ProcessSocketReceiveInternal(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket)
{
	ReceivedMessageSize += _SizeData;
	unsigned char* pTail = (unsigned char*)oByteAdd(_pData, _SizeData);

	if(oInvalid == ExpectedMessageSize)
	{
		// Should be receiving a header
		if(ReceivedMessageSize < sizeof(CLIENT_PACKET_HEADER))
		{
			// No complete header so re-issue
			ProtocolRecvOverride = true;
			_pSocket->Recv(oByteAdd(_pData, _SizeData),  sizeof(CLIENT_PACKET_HEADER) - ReceivedMessageSize);
			return;
		}

		// Got the header
		CLIENT_PACKET_HEADER* pHeader = (CLIENT_PACKET_HEADER*)(pTail - ReceivedMessageSize);
		if('oNET' != pHeader->MagicNumber)
		{
			// Error
			oErrorSetLast(oERROR_IO, "oSocket using oMSG got invalid packet");
			goto error;
		}
		ExpectedMessageSize = pHeader->MessageLength;
		
		if(ExpectedMessageSize > MaximumReceiveSize)
		{
			oErrorSetLast(oERROR_IO, "Message too large for buffer");
			goto error;
		}

		// Overwrite the header
		{
			unsigned char* pCharHeader = (unsigned char*)pHeader;

			while(pCharHeader + sizeof(CLIENT_PACKET_HEADER) < pTail)
			{
				pCharHeader[0] = *(pCharHeader + sizeof(CLIENT_PACKET_HEADER));
				++pCharHeader;
			}
		}
		
		// Remove the header from the received bytes and the tail
		ReceivedMessageSize -= sizeof(CLIENT_PACKET_HEADER);
		pTail -= sizeof(CLIENT_PACKET_HEADER);
	}

	if(ReceivedMessageSize < ExpectedMessageSize)
	{
		// Haven't received entire message so re-issue
		ProtocolRecvOverride = true;
		oVERIFY(_pSocket->Recv(pTail, ExpectedMessageSize - ReceivedMessageSize));
		return;
	}

	// Message has arrived so copy results
	oSocket::size_t MessageSize = ReceivedMessageSize;
	void* pMessage = pTail - MessageSize;

	// Reset prior to calling back in case this gets called again
	ExpectedMessageSize = oInvalid;
	ReceivedMessageSize = 0;
	MaximumReceiveSize.exchange(0);

	Callback->ProcessSocketReceive(pMessage, MessageSize, _Addr, _pSocket);
	return;

error:
	ExpectedMessageSize = oInvalid;
	ReceivedMessageSize = 0;
	MaximumReceiveSize.exchange(0);

	Callback->ProcessSocketReceive(NULL, 0, _Addr, _pSocket);
};


struct oSocketAsync_Impl : public oSocket
{
	oDEFINE_REFCOUNT_IOCP_INTERFACE(Refcount, IOCP);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oSocket);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);
	oDEFINE_SOCKET_SHARED_IMPL();

	struct Operation
	{
		enum TYPE
		{
			Op_Recv,
			Op_Send,
		};

		WSABUF		Header;
		WSABUF		Payload;
		SOCKADDR_IN	SockAddr;
		TYPE		Type;
	};

	char		DebugName[64];
	DESC		Desc;
	oRefCount	Refcount;
	SOCKADDR_IN Saddr;

	oSocketAsync_Impl(const char* _DebugName, const DESC& _Desc, SOCKET _hTarget, bool* _pSuccess);
	~oSocketAsync_Impl();
	virtual bool Send(const void* _pData, oSocket::size_t _Size) threadsafe override;
	virtual bool SendTo(const void* _pData, oSocket::size_t _Size, const oNetAddr& _Destination) threadsafe override;
	virtual oSocket::size_t Recv(void* _pBuffer, oSocket::size_t _Size) threadsafe override;

	bool SendToInternal(const void* _pData, oSocket::size_t _Size, const SOCKADDR_IN& _Destination) threadsafe;
	
	static bool					NOPPrepareHeader(const void* _pData, oSocket::size_t _Size, WSABUF* pHeader);
	static void					NOPReleaseHeader(const WSABUF& _Header);
	static oSocket::size_t		NOPProtocolCanRecv(void* _pData, oSocket::size_t _Size);

	void			IOCPCallback(oIOCPOp* _pSocketOp);

	oIOCP*		IOCP;
	SOCKET		hSocket;
	oRef<threadsafe oSocketAsyncCallback> InternalCallback;

	oFUNCTION<bool(const void* _pData, oSocket::size_t _Size, WSABUF* pHeader)> PrepareSend;
	oFUNCTION<void(const WSABUF& _Header)> ReleaseHeader;
	oFUNCTION<oSocket::size_t(void* _pData, oSocket::size_t _Size)> ProtocolCanRecv;
};

oSocketAsync_Impl::oSocketAsync_Impl(const char* _DebugName, const DESC& _Desc, SOCKET _hTarget, bool* _pSuccess)
	: Desc(_Desc)
	, hSocket(_hTarget)
	, IOCP(nullptr)
{
	*_pSuccess = false;
	*DebugName = 0;
	if (_DebugName)
		strcpy_s(DebugName, _DebugName);

	oNetAddrToSockAddr(Desc.Addr, &Saddr);

	if(nullptr == Desc.AsyncSettings.Callback)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "Sockets of style ASYNC need a callback specified.");
		return;
	}

	// Initialize the internal call back with the user specified one
	InternalCallback = Desc.AsyncSettings.Callback;

	oIOCP::DESC IOCPDesc;

	if(INVALID_SOCKET == hSocket)
	{
		if(UDP == Desc.Protocol)
			hSocket = oWinsockCreate(Saddr, oWINSOCK_REUSE_ADDRESS | oWINSOCK_ALLOW_BROADCAST, Desc.ConnectionTimeoutMS);
		else
		{
			hSocket = oWinsockCreate(Saddr, oWINSOCK_REUSE_ADDRESS | oWINSOCK_RELIABLE,  Desc.ConnectionTimeoutMS);

			if(oMSG == Desc.Protocol)
			{
				// Inform the server that we're using oMSG
				if(!oWinsockSendBlocking(hSocket, &oMSGGUID, sizeof(oMSGGUID), Desc.ConnectionTimeoutMS)) goto error;
			}
		}

		if (hSocket == INVALID_SOCKET)  goto error;
	}

	IOCPDesc.Handle = reinterpret_cast<oHandle>(hSocket);
	IOCPDesc.IOCompletionRoutine = oBIND(&oSocketAsync_Impl::IOCPCallback, this, oBIND1);
	IOCPDesc.MaxOperations = Desc.AsyncSettings.MaxSimultaneousMessages;
	IOCPDesc.PrivateDataSize = sizeof(Operation);

	if(!oIOCPCreate(IOCPDesc, this, &IOCP))
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "Could not create IOCP.");
		goto error;
	}

	// Handle protocol specific callbacks
	if(oMSG == Desc.Protocol)
	{
		oRef<oSocketAsyncoMSG> oMSGCallback(new oSocketAsyncoMSG(InternalCallback, Desc.AsyncSettings.MaxSimultaneousMessages), false);
		InternalCallback = oMSGCallback;
		PrepareSend = oBIND(&oSocketAsyncoMSG::PrepareHeader, oMSGCallback.c_ptr(), oBIND1, oBIND2, oBIND3);
		ReleaseHeader = oBIND(&oSocketAsyncoMSG::ReleaseHeader, oMSGCallback.c_ptr(), oBIND1);
		ProtocolCanRecv = oBIND(&oSocketAsyncoMSG::ProtocolCanRecv, oMSGCallback.c_ptr(), oBIND1, oBIND2);
	}
	else // Setup the raw protocol callbacks
	{
		PrepareSend = oBIND(&oSocketAsync_Impl::NOPPrepareHeader, oBIND1, oBIND2, oBIND3);
		ReleaseHeader = oBIND(&oSocketAsync_Impl::NOPReleaseHeader, oBIND1);
		ProtocolCanRecv = oBIND(&oSocketAsync_Impl::NOPProtocolCanRecv, oBIND1, oBIND2);
	}

	*_pSuccess = true;
	return;

error:
	oWinsockClose(hSocket);
}


oSocketAsync_Impl::~oSocketAsync_Impl()
{
	if(INVALID_SOCKET != hSocket)
		oVERIFY(oWinsockClose(hSocket));

	hSocket = INVALID_SOCKET;
}


bool oSocketAsync_Impl::Send(const void* _pData, oSocket::size_t _Size) threadsafe
{
	if(UDP == Desc.Protocol)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Socket is connectionless.  Send is invalid");

	return SendToInternal(_pData, _Size, thread_cast<SOCKADDR_IN&>(Saddr)); // thread_cast safe because Saddr never changes
}

bool oSocketAsync_Impl::SendTo(const void* _pData, oSocket::size_t _Size, const oNetAddr& _Destination) threadsafe
{
	if(UDP != Desc.Protocol)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Socket is connected.  SendTo is invalid");

	SOCKADDR_IN Saddr;
	oNetAddrToSockAddr(_Destination, &Saddr);

	 return SendToInternal(_pData, _Size, Saddr);
}

bool oSocketAsync_Impl::SendToInternal(const void* _pData, oSocket::size_t _Size, const SOCKADDR_IN& _Destination) threadsafe
{
	oWinsock* ws = oWinsock::Singleton();

	oIOCPOp* pIOCPOp = IOCP->AcquireSocketOp();
	if(!pIOCPOp)
		return oErrorSetLast(oERROR_AT_CAPACITY, "IOCPOpPool is empty, you're sending too fast.");

	Operation* pOp;
	pIOCPOp->GetPrivateData(&pOp);
	pOp->Type = Operation::Op_Send;
	pOp->Payload.len = _Size;
	pOp->Payload.buf = (CHAR*)_pData;
	pOp->SockAddr = _Destination;

	static DWORD bytesSent;

	WSABUF* pBuff = &pOp->Payload;
	unsigned int BuffCount = 1;

	if(thread_cast<oSocketAsync_Impl*>(this)->PrepareSend(_pData, _Size, &pOp->Header))
	{
		pBuff = &pOp->Header;
		BuffCount = 2;
	}

	if(0 != ws->WSASendTo(hSocket, pBuff, BuffCount, &bytesSent, 0, (SOCKADDR*)&pOp->SockAddr, sizeof(sockaddr_in), (WSAOVERLAPPED*)pIOCPOp, nullptr))
	{
#ifdef _DEBUG
		int lastError = ws->WSAGetLastError();
		oASSERT(lastError == WSA_IO_PENDING, "WSASendTo reported error: %i", lastError);
#endif
	}

	return true;
}

oSocket::size_t oSocketAsync_Impl::Recv(void* _pBuffer, oSocket::size_t _Size) threadsafe
{
	oWinsock* ws = oWinsock::Singleton();

	oIOCPOp* pIOCPOp = IOCP->AcquireSocketOp();
	if(!pIOCPOp)
	{
		oErrorSetLast(oERROR_AT_CAPACITY, "IOCPOpPool is empty, you're sending too fast.");
		return 0;
	}

	oSocket::size_t RecvSize = thread_cast<oSocketAsync_Impl*>(this)->ProtocolCanRecv(_pBuffer, _Size);
	if(0 == RecvSize)
	{
		IOCP->ReturnOp(pIOCPOp);
		oErrorSetLast(oERROR_AT_CAPACITY, "Protocol can't issue Recv at this time");
		return RecvSize;
	}

	Operation* pOp;
	pIOCPOp->GetPrivateData(&pOp);

	pOp->Payload.buf = (CHAR*)_pBuffer;
	pOp->Payload.len = RecvSize;
	memset(&pOp->SockAddr, 0, sizeof(SOCKADDR_IN));
	pOp->Type = Operation::Op_Recv;

	static DWORD flags = 0;
	static int sizeOfSockAddr = sizeof(SOCKADDR_IN);

	static DWORD bytesRecvd;

	ws->WSARecvFrom(hSocket, &pOp->Payload, 1, &bytesRecvd, &flags, (SOCKADDR*)&pOp->SockAddr, &sizeOfSockAddr, (WSAOVERLAPPED*)pIOCPOp, nullptr);

	return RecvSize;
}

bool oSocketAsync_Impl::NOPPrepareHeader(const void* _pData, oSocket::size_t _Size, WSABUF* pHeader)
{
	// By default most raw protocols have no user space header
	return false;
};

void oSocketAsync_Impl::NOPReleaseHeader(const WSABUF& _Header)
{
	// By default most raw protocols have no user space header
}

oSocket::size_t oSocketAsync_Impl::NOPProtocolCanRecv(void* _pData, oSocket::size_t _Size)
{
	// By default most raw protocols can always receive
	return _Size;
}

void oSocketAsync_Impl::IOCPCallback(oIOCPOp* pIOCPOp)
{
	oNetAddr address;
	oSocket::size_t szData;
	void* pData;
	Operation::TYPE Type;
	WSABUF Header;
	{
		Operation* pOp;
		pIOCPOp->GetPrivateData(&pOp);
		pData = pOp->Payload.buf;
		oSockAddrToNetAddr(pOp->SockAddr, &address);
		Type = pOp->Type;
		Header = pOp->Header;

		DWORD flags;
		oWinsock* ws = oWinsock::Singleton();
		//this can fail if the socket is getting closed.
		BOOL result = ws->WSAGetOverlappedResult(hSocket, (WSAOVERLAPPED*)pIOCPOp, (LPDWORD)&szData, false, &flags);
		oASSERT(result, "WSAGetOverlappedResult failed.");
	}

	// The header is released prior to releasing the OP so that the underlying protocol can recycle it's headers prior to another send being issued
	if(Operation::Op_Send == Type)
		ReleaseHeader(Header);

	// We return the op before calling back the user so the op is available to use again
	IOCP->ReturnOp(pIOCPOp);

	switch(Type)
	{
	case Operation::Op_Recv:
		if(InternalCallback)
			InternalCallback->ProcessSocketReceive(pData, szData, address, this);
		break;
	case Operation::Op_Send:
		if(InternalCallback)
			InternalCallback->ProcessSocketSend(pData, szData, address, this);
		break;
	default:
		oASSERT(false, "Unknown socket operation.");
	}
	

}

oSocket* CreateSocketAsync(const char* _DebugName, SOCKET _hTarget, oSocket::DESC _SocketDesc, bool* _pSuccess)
{
	bool success = false;
	oSocketAsync_Impl* pSocket = nullptr;
	oCONSTRUCT(&pSocket, oSocketAsync_Impl(_DebugName, _SocketDesc, _hTarget, &success));
	*_pSuccess = success;
	return pSocket;
}

oAPI bool oSocketCreate(const char* _DebugName, const oSocket::DESC& _Desc, threadsafe oSocket** _ppSocket)
{
	bool success = false;

	switch(_Desc.Style)
	{
		case oSocket::ASYNC:
			oCONSTRUCT(_ppSocket, oSocketAsync_Impl(_DebugName, _Desc, INVALID_SOCKET, &success));
		break;

		case oSocket::BLOCKING:
			oCONSTRUCT(_ppSocket, SocketBlocking_Impl(_DebugName, _Desc, INVALID_SOCKET, &success));
		break;

		default:
			oASSERT(false, "Unknown socket operation.");
			return false;
	}
	
	return !!*_ppSocket;
}

oAPI bool oSocketEncryptedCreate(const char* _DebugName, const oSocket::DESC& _Desc, threadsafe oSocketEncrypted** _ppSocket)
{
	if (_Desc.Style == oSocket::BLOCKING && _Desc.Protocol == oSocket::TCP)
	{
		bool success = false;
		oCONSTRUCT(_ppSocket, SocketBlocking_Impl(_DebugName, _Desc, INVALID_SOCKET, &success));
		return !!*_ppSocket;
	}
	else
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Encryped Sockets must have style blocking and protocol TCP");
}