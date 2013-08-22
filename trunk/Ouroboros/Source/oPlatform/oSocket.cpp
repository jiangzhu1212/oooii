/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oPlatform/oSocket.h>
#include <oBasis/oLockThis.h>
#include <oConcurrency/mutex.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oString.h>
#include <oConcurrency/concurrent_queue.h>
#include <oPlatform/oSocket.h>
#include "oIOCP.h"
#include "SoftLink/oWinsock.h"
#include "SoftLink/oOpenSSL.h"

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

namespace oStd {

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oNetHost& _Host)
{
	const oNetHost_Internal* pHost = reinterpret_cast<const oNetHost_Internal*>(&_Host);
	oWinsock* ws = oWinsock::Singleton();
	unsigned long addr = ws->ntohl(pHost->IP);
	return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%u.%u.%u.%u", (addr&0xFF000000)>>24, (addr&0xFF0000)>>16, (addr&0xFF00)>>8, addr&0xFF) ? _StrDestination : nullptr;
}

bool from_string(oNetHost* _pHost, const char* _StrSource)
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

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oNetAddr& _Address)
{
	if (to_string(_StrDestination, _SizeofStrDestination, _Address.Host))
	{
		const oNetAddr_Internal* pAddress = reinterpret_cast<const oNetAddr_Internal*>(&_Address);
		oWinsock* ws = oWinsock::Singleton();
		size_t len = oStrlen(_StrDestination);
		return -1 == oPrintf(_StrDestination + len, _SizeofStrDestination - len, ":%u", ws->ntohs(pAddress->Port)) ? _StrDestination : nullptr;
	}

	return nullptr;
}

bool from_string(oNetAddr* _pAddress, const char* _StrSource)
{
	char tempStr[512];
	oASSERT(oStrlen(_StrSource) < oCOUNTOF(tempStr)+1, "");
	oStrcpy(tempStr, _StrSource);

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

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oSocket::PROTOCOL& _Protocol)
{
	switch (_Protocol)
	{
	case oSocket::TCP:
		oPrintf(_StrDestination, _SizeofStrDestination, "tcp");
		break;
	case oSocket::UDP:
		oPrintf(_StrDestination, _SizeofStrDestination, "udp");
		break;
	default:
		return nullptr;
		break;
	}

	return _StrDestination;
}

bool from_string(oSocket::PROTOCOL* _Protocol, const char* _StrSource)
{
	if(oStrncmp(_StrSource, "tcp", 3) == 0)
	{
		*_Protocol = oSocket::TCP;
	}
	else if(oStrncmp(_StrSource, "udp", 3) == 0)
	{
		*_Protocol = oSocket::UDP;
	}
	else
	{
		return false;
	}

	return true;
}

} // namespace oStd

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


oAPI bool oSocketHostIsLocal( oNetHost _Host )
{
	oNetHost_Internal* pHost = reinterpret_cast<oNetHost_Internal*>(&_Host);
	return 16777343 == pHost->IP;
}

oAPI void oSocketEnumerateAllAddress( oFUNCTION<void(oNetAddr _Addr)> _Enumerator )
{
	oWinsockEnumerateAllAddress(
		[&](sockaddr_in _SockAddr)
	{
		oNetAddr NetAddr;
		oSockAddrToNetAddr(_SockAddr, &NetAddr);
		_Enumerator(NetAddr);
	});
}

const int SaneMaxTimout = 60000;
oSocket::size_t oWinsockRecvFromBlocking(SOCKET hSocket, void* _pData, oSocket::size_t _szReceive, unsigned int _Timeout, const SOCKADDR_IN& _RecvAddr, unsigned int _Flags = 0)
{
	oWinsock* ws = oWinsock::Singleton();

	oSocket::size_t TotalReceived = 0;
	
	if(oInfiniteWait == _Timeout)
		_Timeout = SaneMaxTimout; // oInfiniteWait doesn't seem to work with setsockopt, so use a sane max

	if (SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&_Timeout, sizeof(unsigned int))) 
		goto error;

	int AddrSize = sizeof(_RecvAddr);
	TotalReceived = ws->recvfrom(hSocket, (char*)_pData, _szReceive, _Flags, (sockaddr*)const_cast<SOCKADDR_IN*>(&_RecvAddr)/*const_cast for bad windows API*/, &AddrSize);
	if(SOCKET_ERROR == TotalReceived) 
		goto error;
	
	return TotalReceived;

error:

	oErrorSetLast(std::errc::io_error, "%s", oWinsock::AsString(oWinsock::Singleton()->WSAGetLastError()));
	return 0;
}

struct oSocketImpl
{
	//This class handles the ref counting but its the proxy that needs to be deleted. this class may be deleted in response to that,
	//	or it could be returned to a pool.
	typedef oFUNCTION<void ()> ProxyDeleterFn;

	oSocketImpl(const char* _DebugName, SOCKET _hTarget, bool* _pSuccess);
	~oSocketImpl();

	//Note that this class never deletes itself, and neither does iocp. This instance will be held by a proxy
	//	and the proxy (proxy could be a socket pool) will delete us. but we will handle the refcounting for the proxy.
	int Reference() threadsafe
	{ 
		oConcurrency::shared_lock<oConcurrency::shared_mutex> Lock(DescMutex);
		if( pIOCP ) 
			return pIOCP->Reference(); 
		else
			return (Refcount).Reference() - 1;
	}  

	void Release() threadsafe 
	{ 
		DescMutex.lock_shared();
		auto lockedThis = thread_cast<oSocketImpl*>(this); // Safe because of Mutex

		if( pIOCP ) 
		{
			// Unlock prior to releasing via IOCP in case it causes the destruction task
			// to fire.
			DescMutex.unlock_shared();
			pIOCP->Release(); //may inderectly trigger the proxyDeleter
			return;
		}
		else if( Refcount.Release())
		{
			// Unlock prior to deleting
			DescMutex.unlock_shared();
			lockedThis->ProxyDeleter(); //This will either cause this object to be deleted as well, or this object may be returned to a pool instead
			return;
		}

		DescMutex.unlock_shared(); 
	} 

	bool Initialize(const oSocket::DESC& _Desc, oSocket* _Proxy, ProxyDeleterFn) threadsafe;
	bool GoAsynchronous(const oSocket::ASYNC_SETTINGS& _Settings) threadsafe;

	bool Send(const void* _pData, oSocket::size_t _Size, const void* _pBody, oSocket::size_t _SizeBody) threadsafe;
	bool SendTo(const void* _pData, oSocket::size_t _Size, const oNetAddr& _Destination, const void* _pBody, oSocket::size_t _SizeBody) threadsafe;
	oSocket::size_t Recv(void* _pBuffer, oSocket::size_t _Size) threadsafe;

	void GetDesc(oSocket::DESC* _pDesc) const threadsafe;
	const char* GetDebugName() const threadsafe; 
	bool IsConnected() const threadsafe; 
	bool GetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe;
	bool GetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe;
	bool SetKeepAlive(unsigned int _TimeoutMS = 0x6DDD00, unsigned int _IntervalMS = 0x3E8) const threadsafe;

	bool SendEncrypted(const void* _pData, oSocket::size_t _Size) threadsafe;
	oSocket::size_t RecvEncrypted(void* _pBuffer, oSocket::size_t _Size) threadsafe;

	void Disable() threadsafe;
	
	SOCKET GetHandle() const threadsafe { return hSocket; }

	struct Operation
	{
		enum TYPE
		{
			Op_Recv,
			Op_Send,
		};

		WSABUF		Header;
		WSABUF		Payload[2];
		SOCKADDR_IN	SockAddr;
		TYPE		Type;
	};

private:
	bool SendToInternal(const void* _pData, oSocket::size_t _Size, const SOCKADDR_IN& _Destination, const void* _pBody, oSocket::size_t _SizeBody) threadsafe;
	void IOCPCallback(oIOCPOp* _pSocketOp);
	void RunProxyDeleter() threadsafe;

	oConcurrency::shared_mutex DescMutex;
	oRefCount Refcount;
	oSocket::DESC Desc;
	oSocket* Proxy;

	oInitOnce<oStd::sstring>	DebugName;
	SOCKADDR_IN DefaultAndRecvAddr;

	oIOCP*		pIOCP;
	SOCKET		hSocket;
	oRef<threadsafe oSocketAsyncCallback> InternalCallback;

	oRef<oSocketEncryptor> Encryptor;

	ProxyDeleterFn ProxyDeleter;

	bool Disabled;
};

oSocketImpl::oSocketImpl(const char* _DebugName, SOCKET _hTarget, bool* _pSuccess)
	: DebugName(_DebugName)
	, hSocket(_hTarget)
	, pIOCP(nullptr)
	, Disabled(true)
	, Proxy(nullptr)
{
	*_pSuccess = true;
}

oSocketImpl::~oSocketImpl()
{
	if (hSocket != INVALID_SOCKET)
		oWinsockClose(hSocket);
}

bool oSocketImpl::Initialize(const oSocket::DESC& _Desc, oSocket* _Proxy, ProxyDeleterFn _ProxyDeleter) threadsafe
{
	{
		auto lockedThis = oLockThis(DescMutex);

		lockedThis->Desc = _Desc;
		lockedThis->ProxyDeleter = _ProxyDeleter;
		Proxy = _Proxy;

		SOCKADDR_IN Saddr;
		oNetAddrToSockAddr(lockedThis->Desc.Addr, &Saddr);
		lockedThis->DefaultAndRecvAddr = Saddr;

		if(INVALID_SOCKET == hSocket)
		{
			unsigned int Options = oWINSOCK_REUSE_ADDRESS | (Desc.Style == oSocket::BLOCKING ? oWINSOCK_BLOCKING : 0);
			if(oSocket::UDP == Desc.Protocol)
			{
				// For un-connected receives (UDP) it is necessary that we bind to a local address and port
				// so bind to INADDR_ANY and keep the port
				SOCKADDR_IN LocalAddr;
				oNetAddrToSockAddr(lockedThis->Desc.Addr, &LocalAddr);
				LocalAddr.sin_addr.s_addr = oWinsock::Singleton()->htonl(INADDR_ANY);
				hSocket = oWinsockCreate(LocalAddr, Options | oWINSOCK_ALLOW_BROADCAST, lockedThis->Desc.ConnectionTimeoutMS);
			}
			else
			{
				hSocket = oWinsockCreate(Saddr, Options | oWINSOCK_RELIABLE,  lockedThis->Desc.ConnectionTimeoutMS);
			}
		}
		if (hSocket == INVALID_SOCKET)
			return oWinSetLastError();
	}

	{
		auto lockedThis = oLockThis(DescMutex);
		Disabled = false;
	}

	auto lockelessThis = thread_cast<oSocketImpl*>(this); //in init, not available yet
	if(oSocket::ASYNC == lockelessThis->Desc.Style)
	{
		// Clear the style so GoAsynchronous works
		Desc.Style = oSocket::BLOCKING;
		if(!GoAsynchronous(lockelessThis->Desc.AsyncSettings))
			return false;
	}
	
	return true;
}

//Used by pool. you can still get some late iocp callbacks even after the object is in the pool. Note that iocp doesn't provide a way
//	to de register a handle. so just disable the socket so such callbacks are ignored.
void oSocketImpl::Disable() threadsafe
{
	auto lockedThis = oLockThis(DescMutex);
	Disabled = true;
	Proxy = nullptr;
	lockedThis->ProxyDeleter = nullptr;
	lockedThis->Desc.Style = oSocket::BLOCKING;
	pIOCP = nullptr;
}

void oSocketImpl::RunProxyDeleter() threadsafe
{ 
	ProxyDeleterFn deleter;
	{
		auto lockedThis = oLockThis(DescMutex);
		deleter = lockedThis->ProxyDeleter; //in case it where to change while deleter is executing. i.e. it may disable this very object.
	}

	if(deleter) 
		deleter(); 
}

bool oSocketImpl::GoAsynchronous(const oSocket::ASYNC_SETTINGS& _Settings) threadsafe
{
	// @oooii-tony: can't replace this lock-and-cast with oLockThis because it
	// causes type problems with calling the member oFUNCTION RunProxyDeleter().
	// Someone with more meta-magic fingers should take another look at this.
	#if 1
		oConcurrency::lock_guard<oConcurrency::shared_mutex> Lock(DescMutex);
		auto lockedThis = thread_cast<oSocketImpl*>(this); // Safe because of Mutex
	#else
		auto lockedThis = oLockThis(DescMutex);
	#endif

	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if(Disabled)
		return false;

	if(oSocket::ASYNC == lockedThis->Desc.Style)
		return oErrorSetLast(std::errc::operation_in_progress, "Socket is already asynchronous");

	if(Encryptor)
		return oErrorSetLast(std::errc::invalid_argument, "Socket is encrypted, cannot go asynchronous");

	if(!_Settings.Callback)
		return oErrorSetLast(std::errc::invalid_argument, "No valid callback specified");

	if(!lockedThis->pIOCP) //if this socket is being reused, it may already be registered with windows iocp, but this should still be null, as it need to be re-registered with our iocp
	{
		oIOCP::DESC IOCPDesc;
		IOCPDesc.Handle = reinterpret_cast<oHandle>(lockedThis->hSocket);
		IOCPDesc.IOCompletionRoutine = oBIND(&oSocketImpl::IOCPCallback, lockedThis, oBIND1);
		IOCPDesc.MaxOperations = _Settings.MaxSimultaneousMessages;
		IOCPDesc.PrivateDataSize = sizeof(Operation);

		if(!oIOCPCreate(IOCPDesc, [lockedThis]()
		{ 
			oASSERT(lockedThis->Proxy, "What happened to the proxy?");
			lockedThis->RunProxyDeleter(); 
		}, &lockedThis->pIOCP))
			return oErrorSetLast(std::errc::invalid_argument, "Could not create IOCP.");
	}
	oASSERT(lockedThis->pIOCP, "IOCP Should exist by now");

	lockedThis->Desc.Style = oSocket::ASYNC;
	lockedThis->Desc.AsyncSettings = _Settings;

	// Initialize the internal call back with the user specified one
	lockedThis->InternalCallback = Desc.AsyncSettings.Callback;

	// Patch the refcount
	{
		int SocketCount = Refcount.Reference() - 1;
		Refcount.Release();
	
		// Will be 1 from the create above
		for(int r = 1; r < SocketCount; ++r)
			pIOCP->Reference();
	}

	return true;
}

bool oSocketImpl::Send(const void* _pData, oSocket::size_t _Size, const void* _pBody, oSocket::size_t _SizeBody) threadsafe
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if(Disabled)
		return false;

	if(oSocket::UDP == Desc.Protocol)
		return oErrorSetLast(std::errc::invalid_argument, "Socket is connectionless.  Send is invalid");

	auto locklessThis = thread_cast<oSocketImpl*>(this);

	return SendToInternal(_pData, _Size, locklessThis->DefaultAndRecvAddr, _pBody, _SizeBody); 
}

bool oSocketImpl::SendTo(const void* _pData, oSocket::size_t _Size, const oNetAddr& _Destination, const void* _pBody, oSocket::size_t _SizeBody) threadsafe
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if(Disabled)
		return false;

	if(oSocket::UDP != Desc.Protocol)
		return oErrorSetLast(std::errc::invalid_argument, "Socket is connected.  SendTo is invalid");

	SOCKADDR_IN Saddr;
	oNetAddrToSockAddr(_Destination, &Saddr);

	return SendToInternal(_pData, _Size, Saddr, _pBody, _SizeBody);
}

bool oSocketImpl::SendToInternal(const void* _pHeader, oSocket::size_t _SizeHeader, const SOCKADDR_IN& _Destination, const void* _pBody, oSocket::size_t _SizeBody) threadsafe
{
	auto lockedThis = oLockThis(DescMutex);

	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if(Disabled)
		return false;

	const auto &CurDesc = lockedThis->Desc;

	if(oSocket::BLOCKING == CurDesc.Style)
	{
		unsigned int _TimeoutMS = CurDesc.BlockingSettings.SendTimeout;

		oWinsock* ws = oWinsock::Singleton();

		oScopedPartialTimeout Timeout(&_TimeoutMS);
		if (SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO,(char *)&_TimeoutMS, sizeof(unsigned int)))
			return false;

		if(!oWinsockSend(hSocket, _pHeader, _SizeHeader, &_Destination))
		{
			oWINSOCK_SETLASTERROR("Send");
			return false;
		}

		if(_pBody &&  (!oWinsockSend(hSocket, _pBody, _SizeBody, &_Destination)))
		{
			oWINSOCK_SETLASTERROR("Send");
			return false;
		}
	}
	else
	{
		oWinsock* ws = oWinsock::Singleton();
		oIOCPOp* pIOCPOp = pIOCP->AcquireSocketOp();
		if(!pIOCPOp)
			return oErrorSetLast(std::errc::no_buffer_space, "IOCPOpPool is empty, you're sending too fast.");

		Operation* pOp;
		pIOCPOp->GetPrivateData(&pOp);
		pOp->Type = Operation::Op_Send;
		pOp->Payload[0].len = _SizeHeader;
		pOp->Payload[0].buf = (CHAR*)_pHeader;
		pOp->Payload[1].len = _SizeBody;
		pOp->Payload[1].buf = (CHAR*)_pBody;
		pOp->SockAddr = _Destination;

		static DWORD bytesSent;

		WSABUF* pBuff = pOp->Payload;
		unsigned int BuffCount = _pBody ? 2 : 1;

		if(0 != ws->WSASendTo(hSocket, pBuff, BuffCount, &bytesSent, 0, (SOCKADDR*)&pOp->SockAddr, sizeof(sockaddr_in), (WSAOVERLAPPED*)pIOCPOp, nullptr))
		{
			int lastError = ws->WSAGetLastError();
			if (lastError != WSA_IO_PENDING)
				return false;
		}
	}

	return true;
}

oSocket::size_t oSocketImpl::Recv(void* _pBuffer, oSocket::size_t _Size) threadsafe
{
	auto lockedThis = oLockThis(DescMutex);

	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if(Disabled)
		return false;

	const auto &CurDesc = lockedThis->Desc;

	if(oSocket::BLOCKING == CurDesc.Style)
	{
		return oWinsockRecvFromBlocking(lockedThis->hSocket, _pBuffer, _Size, CurDesc.BlockingSettings.RecvTimeout, lockedThis->DefaultAndRecvAddr);
	}
	else
	{
		oWinsock* ws = oWinsock::Singleton();

		oIOCPOp* pIOCPOp = pIOCP->AcquireSocketOp();
		if(!pIOCPOp)
		{
			oErrorSetLast(std::errc::no_buffer_space, "IOCPOpPool is empty, you're sending too fast.");
			return 0;
		}

		Operation* pOp;
		pIOCPOp->GetPrivateData(&pOp);

		pOp->Payload[0].buf = (CHAR*)_pBuffer;
		pOp->Payload[0].len = _Size;
		pOp->SockAddr = lockedThis->DefaultAndRecvAddr;
		pOp->Type = Operation::Op_Recv;

		static DWORD flags = 0;
		static int sizeOfSockAddr = sizeof(SOCKADDR_IN);

		static DWORD bytesRecvd;

		ws->WSARecvFrom(hSocket, pOp->Payload, 1, &bytesRecvd, &flags, (SOCKADDR*)&pOp->SockAddr, &sizeOfSockAddr, (WSAOVERLAPPED*)pIOCPOp, nullptr);
		return _Size;
	}
}

bool oSocketImpl::SendEncrypted(const void* _pData, oSocket::size_t _Size) threadsafe
{
	auto lockedThis = oLockThis(DescMutex);

	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if(Disabled)
		return false;

	const auto &CurDesc = lockedThis->Desc;
	if(oSocket::ASYNC == CurDesc.Style)
		return oErrorSetLast(std::errc::operation_would_block, "Socket is asynchronous");

	int ret = 0;
	// Lazy init and Open SSL Connection because google's TLS requires that STARTTLS be sent and response received
	// before an TLS connecion can be established.  This may be different for other servers.
	if (!Encryptor.c_ptr())
	{
		oSocketEncryptor::Create(&lockedThis->Encryptor);
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

oSocket::size_t oSocketImpl::RecvEncrypted(void* _pBuffer, oSocket::size_t _Size) threadsafe
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if(Disabled)
		return false;

	if (Encryptor.c_ptr())
	{
		return Encryptor->Receive(hSocket, (char *)_pBuffer, _Size, Desc.BlockingSettings.RecvTimeout);
	}
	return 0;
}

void oSocketImpl::IOCPCallback(oIOCPOp* pIOCPOp)
{
	if(Disabled) //probably sitting in a reuse pool and got a late callback from iocp
		return;

	oNetAddr address;
	oSocket::size_t szData;
	void* pHeader;
	void* pBody;
	Operation::TYPE Type;
	WSABUF Header;
	{
		Operation* pOp;
		pIOCPOp->GetPrivateData(&pOp);
		pHeader = pOp->Payload[0].buf;
		pBody = pOp->Payload[1].buf;
		oSockAddrToNetAddr(pOp->SockAddr, &address);
		Type = pOp->Type;
		Header = pOp->Header;

		DWORD flags;
		oWinsock* ws = oWinsock::Singleton();
		//this can fail if the socket is getting closed.
		BOOL result = ws->WSAGetOverlappedResult(hSocket, (WSAOVERLAPPED*)pIOCPOp, (LPDWORD)&szData, false, &flags);
		oASSERT(result, "WSAGetOverlappedResult failed.");
	}

	// We return the op before calling back the user so the op is available to use again
	pIOCP->ReturnOp(pIOCPOp);

	switch(Type)
	{
	case Operation::Op_Recv:
		if(InternalCallback)
			InternalCallback->ProcessSocketReceive(pHeader, szData, address, Proxy);
		break;
	case Operation::Op_Send:
		if(InternalCallback)
			InternalCallback->ProcessSocketSend(pHeader, pBody, szData, address, Proxy);
		break;
	default:
		oASSERT(false, "Unknown socket operation.");
	}
}

void oSocketImpl::GetDesc(oSocket::DESC* _pDesc) const threadsafe
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	*_pDesc = oLockSharedThis(DescMutex)->Desc;
}

const char* oSocketImpl::GetDebugName() const threadsafe 
{
	return *DebugName;
}

bool oSocketImpl::IsConnected() const threadsafe 
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if(Disabled)
		return false;

	return oWinsockIsConnected(hSocket);
}

bool oSocketImpl::GetHostname( char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort ) const threadsafe 
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if(Disabled)
		return false;

	return oWinsockGetHostname(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, hSocket);
}

bool oSocketImpl::GetPeername( char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort ) const threadsafe 
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if(Disabled)
		return false;

	return oWinsockGetPeername(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, hSocket);
}

bool oSocketImpl::SetKeepAlive(unsigned int _TimeoutMS, unsigned int _IntervalMS) const threadsafe
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if(Disabled)
		return false;

	return oWinsockSetKeepAlive(hSocket, _TimeoutMS, _IntervalMS);
}

struct oSocketImplProxy : public oSocketEncrypted
{
	oDEFINE_TRIVIAL_QUERYINTERFACE(oSocket);
	// _____________________________________________________________________________
	// Refcounting is a little tricky here as the socket can switch from blocking 
	// (no IOCP) to asynchronous (IOCP) at runtime.  When using IOCP we don't 
	// explicitly refcount but instead rely on IOCP calling the destruction event.
	// To be certain we're using the correct method for refcounting, we need to ensure
	// the description is not changing hence the shared lock.
	int Reference() threadsafe override 
	{ 
		auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
		oASSERT(lockelessThis->Socket, "Don't have a socket implementation to use");
		return lockelessThis->Socket->Reference();
	}  
	
	void Release() threadsafe override 
	{ 
		auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
		oASSERT(lockelessThis->Socket, "Don't have a socket implementation to use");
		lockelessThis->Socket->Release();
	} 

	oSocketImplProxy(const char* _DebugName, const DESC& _Desc, SOCKET _hTarget, bool* _pSuccess);
	oSocketImplProxy(const DESC& _Desc, std::shared_ptr<oSocketImpl> _Socket, bool* _pSuccess);
	~oSocketImplProxy();
	

	virtual bool GoAsynchronous(const oSocket::ASYNC_SETTINGS& _Settings) threadsafe override;
	virtual bool Send(const void* _pData, oSocket::size_t _Size, const void* _pBody, oSocket::size_t _SizeBody) threadsafe override;
	virtual bool SendTo(const void* _pData, oSocket::size_t _Size, const oNetAddr& _Destination, const void* _pBody, oSocket::size_t _SizeBody) threadsafe override;
	virtual oSocket::size_t Recv(void* _pBuffer, oSocket::size_t _Size) threadsafe override;

	void GetDesc(DESC* _pDesc) const threadsafe override;
	const char* GetDebugName() const threadsafe override; 
	bool IsConnected() const threadsafe override; 
	bool GetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe override;
	bool GetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe override;
	bool SetKeepAlive(unsigned int _TimeoutMS = 0x6DDD00, unsigned int _IntervalMS = 0x3E8) const threadsafe override;
	
	virtual bool SendEncrypted(const void* _pData, oSocket::size_t _Size) threadsafe override;
	virtual oSocket::size_t RecvEncrypted(void* _pBuffer, oSocket::size_t _Size) threadsafe override;

private:
	//Don't really need the baggage of shared_ptr, but taking advantage of type erasure on the deleter, that it provides. unique_ptr doesn't provide that.
	std::shared_ptr<oSocketImpl> Socket;
};

oSocketImplProxy::oSocketImplProxy(const char* _DebugName, const DESC& _Desc, SOCKET _hTarget, bool* _pSuccess)
{
	Socket = std::make_shared<oSocketImpl>(_DebugName, _hTarget, _pSuccess);
	if(*_pSuccess)
		*_pSuccess = Socket->Initialize(_Desc, this, [this](){delete this;});
}

oSocketImplProxy::oSocketImplProxy(const DESC& _Desc, std::shared_ptr<oSocketImpl> _Socket, bool* _pSuccess)
{
	Socket = _Socket;
	*_pSuccess = Socket->Initialize(_Desc, this, [this](){delete this;});
}

oSocketImplProxy::~oSocketImplProxy()
{
}

bool oSocketImplProxy::GoAsynchronous(const oSocket::ASYNC_SETTINGS& _Settings) threadsafe
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	return lockelessThis->Socket->GoAsynchronous(_Settings);
}

bool oSocketImplProxy::Send(const void* _pData, oSocket::size_t _Size, const void* _pBody, oSocket::size_t _SizeBody) threadsafe
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	return lockelessThis->Socket->Send(_pData, _Size, _pBody, _SizeBody);
}

bool oSocketImplProxy::SendTo(const void* _pData, oSocket::size_t _Size, const oNetAddr& _Destination, const void* _pBody, oSocket::size_t _SizeBody) threadsafe
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	return lockelessThis->Socket->SendTo(_pData, _Size, _Destination, _pBody, _SizeBody);
}

oSocket::size_t oSocketImplProxy::Recv(void* _pBuffer, oSocket::size_t _Size) threadsafe
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	return lockelessThis->Socket->Recv(_pBuffer, _Size);
}

bool oSocketImplProxy::SendEncrypted(const void* _pData, oSocket::size_t _Size) threadsafe
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	return lockelessThis->Socket->SendEncrypted(_pData, _Size);
}

oSocket::size_t oSocketImplProxy::RecvEncrypted(void* _pBuffer, oSocket::size_t _Size) threadsafe
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	return lockelessThis->Socket->RecvEncrypted(_pBuffer, _Size);
}

void oSocketImplProxy::GetDesc(DESC* _pDesc) const threadsafe
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	lockelessThis->Socket->GetDesc(_pDesc);
}

const char* oSocketImplProxy::GetDebugName() const threadsafe 
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	return lockelessThis->Socket->GetDebugName();
}

bool oSocketImplProxy::IsConnected() const threadsafe 
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	return lockelessThis->Socket->IsConnected();
}

bool oSocketImplProxy::GetHostname( char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort ) const threadsafe 
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	return lockelessThis->Socket->GetHostname(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort);
}

bool oSocketImplProxy::GetPeername( char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort ) const threadsafe 
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	return lockelessThis->Socket->GetPeername(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort);
}

bool oSocketImplProxy::SetKeepAlive(unsigned int _TimeoutMS, unsigned int _IntervalMS) const threadsafe
{
	auto lockelessThis = thread_cast<const oSocketImplProxy*>(this); 
	oASSERT(lockelessThis->Socket, "Trying to use a socket without an implementation");

	return lockelessThis->Socket->SetKeepAlive(_TimeoutMS, _IntervalMS);
}

oSocket* oSocketCreateFromServer(const char* _DebugName, SOCKET _hTarget, oSocket::DESC _Desc, bool* _pSuccess)
{
	if(oSocket::BLOCKING == _Desc.Style)
	{
		// Place the socket into non-blocking mode by first clearing the event then disabling FIONBIO
		{
			oWinsock* ws = oWinsock::Singleton();

			if(SOCKET_ERROR == ws->WSAEventSelect(_hTarget, NULL, NULL)) return nullptr;

			u_long nonBlocking = 0;
			if (SOCKET_ERROR == ws->ioctlsocket(_hTarget, FIONBIO, &nonBlocking)) return nullptr;
		}
	}

	oSocket* pSocket = nullptr;
	bool success = false;
	oCONSTRUCT(&pSocket, oSocketImplProxy(_DebugName, _Desc, _hTarget, &success));
	return pSocket;
}

oAPI bool oSocketCreate(const char* _DebugName, const oSocket::DESC& _Desc, threadsafe oSocket** _ppSocket)
{
	bool success = false;
	oCONSTRUCT(_ppSocket, oSocketImplProxy(_DebugName, _Desc, INVALID_SOCKET, &success));
	return success;
}

oAPI bool oSocketEncryptedCreate(const char* _DebugName, const oSocket::DESC& _Desc, threadsafe oSocketEncrypted** _ppSocket)
{
	if (_Desc.Style == oSocket::BLOCKING && _Desc.Protocol == oSocket::TCP)
	{
		bool success = false;
		oCONSTRUCT(_ppSocket, oSocketImplProxy(_DebugName, _Desc, INVALID_SOCKET, &success));
		return success;
	}
	else
		return oErrorSetLast(std::errc::invalid_argument, "Encryped Sockets must have style blocking and protocol TCP");
}


// _____________________________________________________________________________
// SocketServer

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
	oConcurrency::shared_mutex Mutex;
	oRefCount RefCount;
	SOCKET hSocket;
	WSAEVENT hConnectEvent;
	char DebugName[64];
	DESC Desc;
	oConcurrency::mutex AcceptedSocketsMutex;
	std::vector<oRef<oSocket>> AcceptedSockets;
};

bool oSocketServerCreate(const char* _DebugName, const oSocketServer::DESC& _Desc, threadsafe oSocketServer** _ppSocketServer)
{
	if (!_DebugName || !_ppSocketServer)
		return oErrorSetLast(std::errc::invalid_argument);
	bool success = false;
	oCONSTRUCT(_ppSocketServer, SocketServer_Impl(_DebugName, _Desc, &success));
	return success;
}

SocketServer_Impl::SocketServer_Impl(const char* _DebugName, const DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, hConnectEvent(nullptr)
{
	*DebugName = 0;
	if (_DebugName)
		oStrcpy(DebugName, _DebugName);

	*_pSuccess = false;

	oNetAddr Addr;
	oSocketPortSet(Desc.ListenPort, &Addr);
	sockaddr_in SAddr;
	oNetAddrToSockAddr(Addr, &SAddr);

	hSocket = oWinsockCreate(SAddr, oWINSOCK_RELIABLE | oWINSOCK_EXCLUSIVE_ADDRESS, oInfiniteWait, Desc.MaxNumConnections);
	if (INVALID_SOCKET == hSocket)
		return; // leave last error from inside oWinsockCreate

	if (!Desc.ListenPort)
	{
		if (!oWinsockGetPort(hSocket, &Desc.ListenPort))
			return; // leave last error from inside oWinsockGetPort
	}

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
	, threadsafe oConcurrency::shared_mutex& _Mutex
	, threadsafe WSAEVENT _hConnectEvent
	, unsigned int _TimeoutMS
	, SOCKET _hServerSocket
	, oSocket::DESC _Desc
	, oFUNCTION<oSocket*(const char* _DebugName, SOCKET _hTarget, oSocket::DESC SocketDesc, bool* _pSuccess)> _CreateClientSocket
	, threadsafe oConcurrency::mutex& _AcceptedSocketsMutex
	, threadsafe std::vector<oRef<oSocket>>& _AcceptedSockets)
{
	oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(_Mutex);
	oWinsock* ws = oWinsock::Singleton();
	bool success = false;

	oScopedPartialTimeout timeout = oScopedPartialTimeout(&_TimeoutMS);
	if (oWinsockWaitMultiple(thread_cast<WSAEVENT*>(&_hConnectEvent), 1, true, false, _TimeoutMS)) // thread_cast safe because of mutex
	{
		sockaddr_in saddr;
		int size = sizeof(saddr);
		ws->WSAResetEvent(_hConnectEvent); // be sure to reset otherwise the wait will always return immediately. however we could have more than 1 waiting to be accepted.
		SOCKET hTarget;

		hTarget = ws->accept(_hServerSocket, (sockaddr*)&saddr, &size);
		if (INVALID_SOCKET == hTarget)
			return oErrorSetLast(std::errc::protocol_error, "Invalid socket");

		u_long enabled = 1;
		if (SOCKET_ERROR == ws->setsockopt(hTarget, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*)&enabled, sizeof(enabled)))
			return oErrorSetLast(std::errc::permission_denied, "Already have a socket on this port");

		// Fill in the remaining portions of the desc
		_Desc.Protocol = oSocket::TCP;
		_Desc.ConnectionTimeoutMS = _TimeoutMS;
		oSockAddrToNetAddr(saddr, &_Desc.Addr);

		oRef<oSocket> newSocket(_CreateClientSocket("", hTarget, _Desc, &success), false);
		{
			oConcurrency::lock_guard<oConcurrency::mutex> lock(_AcceptedSocketsMutex);
			thread_cast<std::vector<oRef<oSocket>>&>(_AcceptedSockets).push_back(newSocket); // safe because of lock above
		}

		success = true;
	}
	else
	{
		oErrorSetLast(0); // It's ok if we don't find a connection
	}

	return success;
}

template<typename T> static inline bool FindTypedSocket(threadsafe oConcurrency::mutex& _AcceptedSocketsMutex, threadsafe std::vector<oRef<oSocket>>& _AcceptedSockets, T** _ppNewlyConnectedClient)
{
	oConcurrency::lock_guard<oConcurrency::mutex> lock(_AcceptedSocketsMutex);
	std::vector<oRef<oSocket>>& SafeSockets = thread_cast<std::vector<oRef<oSocket>>&>(_AcceptedSockets);

	if (!SafeSockets.empty())
	{
		for (std::vector<oRef<oSocket>>::iterator it = SafeSockets.begin(); it != SafeSockets.end(); ++it)
		{
			oSocket* s = *it;
			if (s->QueryInterface(oGetGUID(_ppNewlyConnectedClient), (void**)_ppNewlyConnectedClient))
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

	bool result = UNIFIED_WaitForConnection(GetDebugName(), Mutex, hConnectEvent, _TimeoutMS, hSocket, Desc, oSocketCreateFromServer, AcceptedSocketsMutex, AcceptedSockets);

	if (result)
		result = FindTypedSocket(AcceptedSocketsMutex, AcceptedSockets, _ppNewlyConnectedClient);

	return result;
}

bool SocketServer_Impl::WaitForConnection(const oSocket::ASYNC_SETTINGS& _AsyncSettings, threadsafe oSocket** _ppNewlyConnectedClient, unsigned int _TimeoutMS) threadsafe
{
	if (FindTypedSocket(AcceptedSocketsMutex, AcceptedSockets, _ppNewlyConnectedClient))
		return true;

	oSocket::DESC Desc;
	Desc.Style = oSocket::ASYNC;
	Desc.AsyncSettings = _AsyncSettings;
	Desc.ConnectionTimeoutMS = _TimeoutMS;

	bool result = UNIFIED_WaitForConnection(GetDebugName(), Mutex, hConnectEvent, _TimeoutMS, hSocket, Desc, oSocketCreateFromServer, AcceptedSocketsMutex, AcceptedSockets);

	if (result)
		result = FindTypedSocket(AcceptedSocketsMutex, AcceptedSockets, _ppNewlyConnectedClient);

	return result; // NYI
}

////////////
// server socket 2
////////////

oSocket* oSocketCreateFromServer2(std::shared_ptr<oSocketImpl> _Target, oSocket::DESC _Desc, bool* _pSuccess)
{
	if(oSocket::BLOCKING == _Desc.Style)
	{
		// Place the socket into non-blocking mode by first clearing the event then disabling FIONBIO
		{
			oWinsock* ws = oWinsock::Singleton();

			if(SOCKET_ERROR == ws->WSAEventSelect(_Target->GetHandle(), NULL, NULL)) return nullptr;

			u_long nonBlocking = 0;
			if (SOCKET_ERROR == ws->ioctlsocket(_Target->GetHandle(), FIONBIO, &nonBlocking)) return nullptr;
		}
	}

	oSocket* pSocket = nullptr;
	bool success = false;
	oCONSTRUCT(&pSocket, oSocketImplProxy(_Desc, _Target, &success));
	return pSocket;
}

struct SocketServerPool : public oInterface
{
	oDEFINE_NOOP_QUERYINTERFACE();
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	
	typedef oFUNCTION<void (oSocketImpl* _Socket)> ServerDisconnect;

	SocketServerPool( ServerDisconnect _ServerDisconnectFn, bool* _pSuccess );
	~SocketServerPool();

	oSocketImpl* GetSocket();
	void ReturnSocket(oSocketImpl* _Socket);
	void RouteDisconnect(oSocketImpl* _Socket);
	
	void ServerClosed() { ServerDisconnectFn = nullptr; }

private:
	static const int SocketPoolGrowthSize = 16;

	void AddNewSocket(int _Num);

	oRefCount RefCount;

	//This pool holds all sockets, they won't get removed until this class is destroyed.
	//	Sockets can get left in the iocp system, and therefore not get returned to the pool
	//	So need a way to keep track of those.
	oConcurrency::concurrent_queue<oSocketImpl*> AllSocketsPool;

	//This is the list of sockets available for use
	oConcurrency::concurrent_queue<oSocketImpl*> AcceptSocketsPool;

	ServerDisconnect ServerDisconnectFn;
};

SocketServerPool::SocketServerPool( ServerDisconnect _ServerDisconnectFn, bool* _pSuccess )
	: ServerDisconnectFn(_ServerDisconnectFn)
{
	*_pSuccess = false;

	int numIOCPThreads = oIOCPThreadCount();
	// filling with some extras to get started, there will be a short delay from the time a socket is closed until it makes it back to the pool
	AddNewSocket(numIOCPThreads*2);

	*_pSuccess = true;
}

SocketServerPool::~SocketServerPool()
{
	oSocketImpl* socket;
	while(AllSocketsPool.try_pop(socket)) 
	{
		delete socket; 
	}
	AllSocketsPool.clear();
	AcceptSocketsPool.clear();
}

void SocketServerPool::AddNewSocket(int _Num)
{
 	for (int i = 0;i < _Num; ++i)
 	{
		SOCKET hSocket = oWinsockCreateForAsyncAccept();
		bool success = true;
		auto pSocket = new oSocketImpl("accept socket", hSocket, &success);
		oASSERT(success, "This should always succeed, oSocketImpl constructor doesn't do anything");

 		AcceptSocketsPool.push(pSocket);
 		AllSocketsPool.push(pSocket);
 	}
}

oSocketImpl* SocketServerPool::GetSocket()
{
	oSocketImpl* socket;
	while(!AcceptSocketsPool.try_pop(socket)) //pool empty, add some more, could loop if someone else steals all the pushes
	{
		AddNewSocket(SocketPoolGrowthSize);
	}
	
	return socket;
}

void SocketServerPool::ReturnSocket(oSocketImpl* _Socket)
{
	AcceptSocketsPool.push(_Socket);
}

void SocketServerPool::RouteDisconnect(oSocketImpl* _Socket)
{
	if(ServerDisconnectFn)
		ServerDisconnectFn(_Socket);
	else //wont be able to be reused, this should only happen when gathering up stragglers after the socket server is closed
		_Socket->Disable();
}

struct SocketServer2_Impl : public oSocketServer2
{
	int Reference() threadsafe override;
	void Release() threadsafe override;
		
	oDEFINE_TRIVIAL_QUERYINTERFACE(oSocketServer2);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);

	SocketServer2_Impl(const char* _DebugName, const DESC& _Desc, bool* _pSuccess);
	~SocketServer2_Impl();

	const char* GetDebugName() const threadsafe override;
	bool GetHostname(char* _pString, size_t _strLen)  const threadsafe override;

private:
	static const int SocketPoolGrowthSize = 64;
	static const float AcceptCountMultiplier;
	//have twice as many socket ops as our accept count. arbitrary. Once these run out, things will start to get delayed.
	//	however the delay doesn't hurt performance, it helps it(based on testing). probably by keeping the total windows socket count reasonable.
	static const int ExtraSocketOpsMultiplier = 2; 

	struct Operation
	{
		enum OperationType
		{
			ACCEPT_REQUEST,
			DISCONNECT_REQUEST,
		};
		OperationType OpType;
		oSocketImpl* Socket;
		char AcceptAddressesBuffer[oWINSOCK_ACCEPT_BUFFER_SIZE]; //From msdn docs, buffer needs 16 bytes padding for each address. buffer will hold 2
	};

	void IOCPCallback(oIOCPOp* _pSocketOp);
	void Accept();
	void Disconnect(oSocketImpl* _Socket);

	oIOCP* pIOCP;

	SOCKET hListenSocket;

	char DebugName[64];
	DESC Desc;
	oRef<SocketServerPool> SocketPool;

	int DesiredAccepts;
	oStd::atomic_int IssuedAcceptCount; //once this exceeds DesiredAccepts, accepts will begin to get starved out.
	//Once we run out of socket ops, start saving disconnects for later.
	//	Note that before this happens, accepts will have been starved out by IssuedAcceptCount.
	//	Disconnects themselves will start the normal accept "loop" back up.
	oConcurrency::concurrent_queue<oSocketImpl*> PendingDisconnects;
};

//Disconnect can take a bit of time to execute. so have a bit more accepts in flight than we have iocp threads. with this number
//	we should be able to have about 50% of our iocp thread count in outstanding disconnects, before accepts start getting delayed
//	to force handling of disconnects.
const float SocketServer2_Impl::AcceptCountMultiplier = 1.5f;

bool oSocketServer2Create(const char* _DebugName, const oSocketServer2::DESC& _Desc, threadsafe oSocketServer2** _ppSocketServer)
{
	if (!_DebugName || !_ppSocketServer)
		return oErrorSetLast(std::errc::invalid_argument);
	bool success = false;
	oCONSTRUCT(_ppSocketServer, SocketServer2_Impl(_DebugName, _Desc, &success));
	return success;
}

SocketServer2_Impl::SocketServer2_Impl(const char* _DebugName, const DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, pIOCP(nullptr)
	, IssuedAcceptCount(0)
{
	*DebugName = 0;
	if (_DebugName)
		oStrcpy(DebugName, _DebugName);

	*_pSuccess = false;

	oNetAddr Addr;
	oSocketPortSet(Desc.ListenPort, &Addr);
	sockaddr_in SAddr;
	oNetAddrToSockAddr(Addr, &SAddr);

	hListenSocket = oWinsockCreate(SAddr, oWINSOCK_RELIABLE | oWINSOCK_EXCLUSIVE_ADDRESS, oInfiniteWait, oInvalid);
	if (INVALID_SOCKET == hListenSocket)
		return; // leave last error from inside oWinsockCreate
	
	SocketPool = oRef<SocketServerPool>(new SocketServerPool(oBIND(&SocketServer2_Impl::Disconnect, this, oBIND1) , _pSuccess), false);
	if(!(*_pSuccess))
	{
		oErrorSetLast(std::errc::invalid_argument, "Failed to create the socket pool.");
		return;
	}
	*_pSuccess = false;

	int numIOCPThreads = oIOCPThreadCount();
	DesiredAccepts = static_cast<int>(numIOCPThreads*AcceptCountMultiplier);

	oIOCP::DESC IOCPDesc;
	IOCPDesc.Handle = reinterpret_cast<oHandle>(hListenSocket);
	IOCPDesc.IOCompletionRoutine = oBIND(&SocketServer2_Impl::IOCPCallback, this, oBIND1);
	IOCPDesc.MaxOperations = DesiredAccepts*ExtraSocketOpsMultiplier;
	IOCPDesc.PrivateDataSize = sizeof(Operation);

	if(!oIOCPCreate(IOCPDesc, [&](){
		delete this;
	}, &pIOCP))
	{
		oErrorSetLast(std::errc::invalid_argument, "Could not create IOCP.");
		return;
	}

	for (int i = 0;i < DesiredAccepts; ++i) //no reason to have more accepts pending than we have iocp threads to handle
	{
		Accept();
	}

	*_pSuccess = true;
}

SocketServer2_Impl::~SocketServer2_Impl()
{
	if(SocketPool)
		SocketPool->ServerClosed();

	if (INVALID_SOCKET != hListenSocket)
		oVERIFY(oWinsockClose(hListenSocket));
}

int SocketServer2_Impl::Reference() threadsafe 
{ 
	return pIOCP->Reference(); 
}  

void SocketServer2_Impl::Release() threadsafe 
{ 
	if(pIOCP)
		pIOCP->Release();
	else
		delete this;
} 

const char* SocketServer2_Impl::GetDebugName() const threadsafe 
{
	return thread_cast<const char*>(DebugName); // threadsafe because name never changes
}

void SocketServer2_Impl::Accept()
{
	if(IssuedAcceptCount > DesiredAccepts)
	{
		return;
	}

	oSocketImpl* socket = SocketPool->GetSocket();
	
	oIOCPOp* iocpOp = pIOCP->AcquireSocketOp();
	if(!iocpOp)
	{
		//just ignore the call in this case. only way this happens if there are a bunch of disconnects in the chain.
		//disconnects will restart the accept chain
		return;
	}

	++IssuedAcceptCount;

	Operation* myop;
	iocpOp->GetPrivateData(&myop);
	myop->OpType = Operation::ACCEPT_REQUEST;
	myop->Socket = socket;

	oWINSOCK_ASYNC_RESULT result = oWinsockAsyncAccept(hListenSocket, socket->GetHandle(), myop->AcceptAddressesBuffer, iocpOp);
	if(result == oWINSOCK_FAILED)
	{
		oASSERT(false, "Not really expecting the asyncex call to fail");

		oErrorSetLast(std::errc::protocol_error, "Failed to initiate an AsyncAccept");

		pIOCP->ReturnOp(iocpOp);

		return;
	}
	else if(result == oWINSOCK_COMPLETED) //handle very rare case where accept is completed synchronously
	{
		pIOCP->DispatchManualCompletion((oHandle)hListenSocket, iocpOp);
	}
	//else it went to iocp
}

void SocketServer2_Impl::Disconnect(oSocketImpl* _Socket)
{
	oIOCPOp* iocpOp = pIOCP->AcquireSocketOp();
	if(!iocpOp)
	{
		PendingDisconnects.push(_Socket); //ran out of socket ops, save for later
		return;
	}
	Operation* myop;
	iocpOp->GetPrivateData(&myop);
	myop->OpType = Operation::DISCONNECT_REQUEST;
	myop->Socket = _Socket;

	oWINSOCK_ASYNC_RESULT result = oWinsockAsyncAcceptPrepForReuse(hListenSocket, _Socket->GetHandle(), iocpOp);
	if(result == oWINSOCK_FAILED)
	{
		oASSERT(false, "not expecting disconectex to fail, if it is, can't recyle these sockets");
		pIOCP->ReturnOp(iocpOp);
	}
	else if(result == oWINSOCK_COMPLETED) //handle very rare case where Disconnect is completed synchronously
	{
		pIOCP->DispatchManualCompletion((oHandle)hListenSocket, iocpOp);
	}
	//else it went to iocp

	// disconnect will issue an accept once its done. prevent accepts from starving disconnects, iocp isn't fair.
	++IssuedAcceptCount;
}

bool SocketServer2_Impl::GetHostname(char* _pString, size_t _strLen) const threadsafe 
{
	return oWinsockGetHostname(_pString, _strLen, nullptr, NULL, nullptr, NULL, hListenSocket);
}

class oSocketAsyncCallbackNOP : public oSocketAsyncCallback
{
	oDEFINE_NOOP_REFCOUNT_INTERFACE();
	oDEFINE_NOOP_QUERYINTERFACE();
	// Called when new data arrives over the network.
	virtual void ProcessSocketReceive(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe override
	{

	}

	// Called once a send operation has finished. A buffer passed in for
	// sending must remain available until this callback is sent.
	virtual void ProcessSocketSend(void* _pHeader, void* _pBody, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe override
	{

	}
};
oSocketAsyncCallbackNOP oSocketAsyncCallbackNOPInstance;

void SocketServer2_Impl::IOCPCallback(oIOCPOp* _pSocketOp)
{
	Operation* pOp;
	_pSocketOp->GetPrivateData(&pOp);

	if(pOp->OpType == Operation::ACCEPT_REQUEST)
	{
		oSocket::DESC desc;
		desc.Style = oSocket::BLOCKING;
		desc.BlockingSettings = Desc.BlockingSettings;
		desc.ConnectionTimeoutMS = oInfiniteWait; //not used for this type of socket anyway

		sockaddr_in* AddrLocal;
		int SzAddrLocal;
		sockaddr_in* AddrRemote;
		int SzAddrRemote;
		oWinSockAsyncAcceptExSockAddrs(pOp->Socket->GetHandle(), pOp->AcceptAddressesBuffer, (SOCKADDR**)&AddrLocal, &SzAddrLocal, (SOCKADDR**)&AddrRemote, &SzAddrRemote);
		oSockAddrToNetAddr(*AddrRemote, &desc.Addr);

		bool success;
		success = oWinsockCompleteAsyncAccept(hListenSocket, pOp->Socket->GetHandle());

		oRef<oSocket> socket = nullptr;
		oRef<SocketServerPool> sPool = SocketPool;
		if(success)
		{
			auto deleter = [sPool](oSocketImpl* _socket) mutable {
				_socket->Disable();
				sPool->RouteDisconnect(_socket);
			};

			std::shared_ptr<oSocketImpl> socketPtr(pOp->Socket, deleter);
			socket = oRef<oSocket>(oSocketCreateFromServer2(socketPtr, desc, &success), false);
		}

		if(success)
			Desc.NewConnectionCallback(socket);

		// FIXME: Hack for socket recycling.  For DisconectEx to work properly the socket has to be asyncrhonous
		// therefore if the user isn't holding onto the socket ensure we've forced it to asyncrhonous before letting go
		if(1 == socket->Reference())
		{
			oSocket::ASYNC_SETTINGS Settings;
			Settings.Callback = &oSocketAsyncCallbackNOPInstance;
			socket->GoAsynchronous(Settings);
		}
		socket->Release();

		pIOCP->ReturnOp(_pSocketOp); //for this case we want to return the op before the accept. keep max socket ops in flight reasonable.

		--IssuedAcceptCount;
		Accept(); //note that this is not guaranteed to actually start an accept. that can happen if accepts are starved by disconnects, or there are just too many issued already.
	}
	else if(pOp->OpType == Operation::DISCONNECT_REQUEST) 
	{
		//socket should be ready for reuse now
		SocketPool->ReturnSocket(pOp->Socket);
		pIOCP->ReturnOp(_pSocketOp);

		--IssuedAcceptCount;
		oSocketImpl* socketToDis;
		if(PendingDisconnects.try_pop(socketToDis))
		{
			Disconnect(socketToDis);
		}
		else
		{
			Accept(); //see comment in the accept request handler
		}
	}
	else
	{
		oASSERT(false, "Not expecting any other types of callbacks");
		pIOCP->ReturnOp(_pSocketOp); //just return the op
	}	
}


bool oSocketRecvWithTimeout(threadsafe oSocket* _pSocket, void* _pData, unsigned int _SizeofData, unsigned int& _TimeoutMS)
{
	oSocket::size_t Received = 0;
	oScopedPartialTimeout ScopedTimeout(&_TimeoutMS);
	while(Received < _SizeofData && _TimeoutMS)
	{
		Received += _pSocket->Recv(oStd::byte_add(_pData, Received), _SizeofData - Received);
		ScopedTimeout.UpdateTimeout();
	}
	if (Received != _SizeofData)
		return oErrorSetLast(std::errc::timed_out, "oSocketRecv timed out");
	return true;
}
