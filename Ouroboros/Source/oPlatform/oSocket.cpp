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
#include <oBasis/oInitOnce.h>
#include <oBasis/oLockThis.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oScopedPartialTimeout.h>
#include <oBase/concurrent_queue.h>
#include <oPlatform/oSocket.h>
#include <oCore/windows/win_winsock.h>
#include "oIOCP.h"
#include "SoftLink/oOpenSSL.h"

#include <oCore/windows/win_error.h>

using namespace ouro;
using namespace ouro::windows;

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

namespace ouro {

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oNetHost& _Host)
{
	const oNetHost_Internal* pHost = reinterpret_cast<const oNetHost_Internal*>(&_Host);
	unsigned long addr = ntohl(pHost->IP);
	return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%u.%u.%u.%u", (addr&0xFF000000)>>24, (addr&0xFF0000)>>16, (addr&0xFF00)>>8, addr&0xFF) ? _StrDestination : nullptr;
}

bool from_string(oNetHost* _pHost, const char* _StrSource)
{
	oNetHost_Internal* pHost = reinterpret_cast<oNetHost_Internal*>(_pHost);

	ADDRINFO* pAddrInfo = nullptr;
	ADDRINFO Hints;
	memset(&Hints, 0, sizeof(Hints));
	Hints.ai_family = AF_INET;
	getaddrinfo(_StrSource, nullptr, &Hints, &pAddrInfo);

	if (!pAddrInfo)
		return false;

	pHost->IP = ((SOCKADDR_IN*)pAddrInfo->ai_addr)->sin_addr.s_addr;
	freeaddrinfo(pAddrInfo);
	return true;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oNetAddr& _Address)
{
	if (to_string(_StrDestination, _SizeofStrDestination, _Address.Host))
	{
		const oNetAddr_Internal* pAddress = reinterpret_cast<const oNetAddr_Internal*>(&_Address);
		size_t len = strlen(_StrDestination);
		return -1 == snprintf(_StrDestination + len, _SizeofStrDestination - len, ":%u", ntohs(pAddress->Port)) ? _StrDestination : nullptr;
	}

	return nullptr;
}

bool from_string(oNetAddr* _pAddress, const char* _StrSource)
{
	char tempStr[512];
	oASSERT(strlen(_StrSource) < oCOUNTOF(tempStr)+1, "");
	strlcpy(tempStr, _StrSource);

	char* seperator = strstr(tempStr, ":");

	if (!seperator)
		return false;

	*seperator = 0;

	ADDRINFO* pAddrInfo = nullptr;
	ADDRINFO Hints;
	memset(&Hints, 0, sizeof(Hints));
	Hints.ai_family = AF_INET;
	getaddrinfo(tempStr, seperator+1, &Hints, &pAddrInfo);

	if (!pAddrInfo)
		return false;

	oSockAddrToNetAddr(*((SOCKADDR_IN*)pAddrInfo->ai_addr), _pAddress);
	freeaddrinfo(pAddrInfo);
	return true;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oSocket::PROTOCOL& _Protocol)
{
	switch (_Protocol)
	{
	case oSocket::TCP:
		snprintf(_StrDestination, _SizeofStrDestination, "tcp");
		break;
	case oSocket::UDP:
		snprintf(_StrDestination, _SizeofStrDestination, "udp");
		break;
	default:
		return nullptr;
		break;
	}

	return _StrDestination;
}

bool from_string(oSocket::PROTOCOL* _Protocol, const char* _StrSource)
{
	if (strncmp(_StrSource, "tcp", 3) == 0)
	{
		*_Protocol = oSocket::TCP;
	}
	else if (strncmp(_StrSource, "udp", 3) == 0)
	{
		*_Protocol = oSocket::UDP;
	}
	else
	{
		return false;
	}

	return true;
}

} // namespace ouro

oAPI void oSocketPortGet(const oNetAddr& _Addr, unsigned short* _pPort)
{
	const oNetAddr_Internal* pAddr = reinterpret_cast<const oNetAddr_Internal*>(&_Addr);
	*_pPort = ntohs(pAddr->Port);
}

oAPI void oSocketPortSet(const unsigned short _Port, oNetAddr* _pAddr)
{
	oNetAddr_Internal* pAddr = reinterpret_cast<oNetAddr_Internal*>(_pAddr);
	pAddr->Port = htons(_Port);
}


oAPI bool oSocketHostIsLocal( oNetHost _Host )
{
	oNetHost_Internal* pHost = reinterpret_cast<oNetHost_Internal*>(&_Host);
	return 16777343 == pHost->IP;
}

oAPI void oSocketEnumerateAllAddress( std::function<void(oNetAddr _Addr)> _Enumerator )
{
	winsock::enumerate_addresses(
		[&](sockaddr_in _SockAddr)
	{
		oNetAddr NetAddr;
		oSockAddrToNetAddr(_SockAddr, &NetAddr);
		_Enumerator(NetAddr);
	});
}

struct oSocketImpl
{
	//This class handles the ref counting but its the proxy that needs to be deleted. this class may be deleted in response to that,
	//	or it could be returned to a pool.
	typedef std::function<void ()> ProxyDeleterFn;

	oSocketImpl(const char* _DebugName, SOCKET _hTarget, bool* _pSuccess);
	~oSocketImpl();

	//Note that this class never deletes itself, and neither does iocp. This instance will be held by a proxy
	//	and the proxy (proxy could be a socket pool) will delete us. but we will handle the refcounting for the proxy.
	int Reference() threadsafe
	{ 
		ouro::shared_lock<ouro::shared_mutex> Lock(thread_cast<ouro::shared_mutex&>(DescMutex));
		if ( pIOCP ) 
			return pIOCP->Reference(); 
		else
			return (Refcount).Reference() - 1;
	}  

	void Release() threadsafe 
	{ 
		thread_cast<ouro::shared_mutex&>(DescMutex).lock_shared();
		auto lockedThis = thread_cast<oSocketImpl*>(this); // Safe because of Mutex

		if ( pIOCP ) 
		{
			// Unlock prior to releasing via IOCP in case it causes the destruction task
			// to fire.
			thread_cast<ouro::shared_mutex&>(DescMutex).unlock_shared();
			pIOCP->Release(); //may inderectly trigger the proxyDeleter
			return;
		}
		else if ( Refcount.Release())
		{
			// Unlock prior to deleting
			thread_cast<ouro::shared_mutex&>(DescMutex).unlock_shared();
			lockedThis->ProxyDeleter(); //This will either cause this object to be deleted as well, or this object may be returned to a pool instead
			return;
		}

		thread_cast<ouro::shared_mutex&>(DescMutex).unlock_shared(); 
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

	ouro::shared_mutex DescMutex;
	oRefCount Refcount;
	oSocket::DESC Desc;
	oSocket* Proxy;

	oInitOnce<sstring>	DebugName;
	SOCKADDR_IN DefaultAndRecvAddr;

	oIOCP*		pIOCP;
	SOCKET		hSocket;
	intrusive_ptr<threadsafe oSocketAsyncCallback> InternalCallback;

	intrusive_ptr<oSocketEncryptor> Encryptor;

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
	winsock::close(hSocket);
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

		if (INVALID_SOCKET == hSocket)
		{
			unsigned int Options = winsock::reuse_address | (Desc.Style == oSocket::BLOCKING ? winsock::blocking : 0);
			if (oSocket::UDP == Desc.Protocol)
			{
				// For un-connected receives (UDP) it is necessary that we bind to a local address and port
				// so bind to INADDR_ANY and keep the port
				SOCKADDR_IN LocalAddr;
				oNetAddrToSockAddr(lockedThis->Desc.Addr, &LocalAddr);
				LocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
				hSocket = winsock::make(LocalAddr, Options | winsock::allow_broadcast, lockedThis->Desc.ConnectionTimeoutMS);
			}
			else
			{
				hSocket = winsock::make(Saddr, Options | winsock::reliable, lockedThis->Desc.ConnectionTimeoutMS);
			}
		}
		if (hSocket == INVALID_SOCKET)
			throw ouro::windows::error();
	}

	{
		auto lockedThis = oLockThis(DescMutex);
		Disabled = false;
	}

	auto lockelessThis = thread_cast<oSocketImpl*>(this); //in init, not available yet
	if (oSocket::ASYNC == lockelessThis->Desc.Style)
	{
		// Clear the style so GoAsynchronous works
		Desc.Style = oSocket::BLOCKING;
		if (!GoAsynchronous(lockelessThis->Desc.AsyncSettings))
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

	if (deleter) 
		deleter(); 
}

bool oSocketImpl::GoAsynchronous(const oSocket::ASYNC_SETTINGS& _Settings) threadsafe
{
	// @tony: can't replace this lock-and-cast with oLockThis because it
	// causes type problems with calling the member std::function RunProxyDeleter().
	// Someone with more meta-magic fingers should take another look at this.
	#if 1
		std::lock_guard<ouro::shared_mutex> Lock(thread_cast<ouro::shared_mutex&>(DescMutex));
		auto lockedThis = thread_cast<oSocketImpl*>(this); // Safe because of Mutex
	#else
		auto lockedThis = oLockThis(thread_cast<ouro::shared_mutex&>(DescMutex));
	#endif

	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if (Disabled)
		return false;

	if (oSocket::ASYNC == lockedThis->Desc.Style)
		return oErrorSetLast(std::errc::operation_in_progress, "Socket is already asynchronous");

	if (Encryptor)
		return oErrorSetLast(std::errc::invalid_argument, "Socket is encrypted, cannot go asynchronous");

	if (!_Settings.Callback)
		return oErrorSetLast(std::errc::invalid_argument, "No valid callback specified");

	if (!lockedThis->pIOCP) //if this socket is being reused, it may already be registered with windows iocp, but this should still be null, as it need to be re-registered with our iocp
	{
		oIOCP::DESC IOCPDesc;
		IOCPDesc.Handle = reinterpret_cast<oHandle>(lockedThis->hSocket);
		IOCPDesc.IOCompletionRoutine = std::bind(&oSocketImpl::IOCPCallback, lockedThis, std::placeholders::_1);
		IOCPDesc.MaxOperations = _Settings.MaxSimultaneousMessages;
		IOCPDesc.PrivateDataSize = sizeof(Operation);

		if (!oIOCPCreate(IOCPDesc, [lockedThis]()
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
	if (Disabled)
		return false;

	if (oSocket::UDP == Desc.Protocol)
		return oErrorSetLast(std::errc::invalid_argument, "Socket is connectionless.  Send is invalid");

	auto locklessThis = thread_cast<oSocketImpl*>(this);

	return SendToInternal(_pData, _Size, locklessThis->DefaultAndRecvAddr, _pBody, _SizeBody); 
}

bool oSocketImpl::SendTo(const void* _pData, oSocket::size_t _Size, const oNetAddr& _Destination, const void* _pBody, oSocket::size_t _SizeBody) threadsafe
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if (Disabled)
		return false;

	if (oSocket::UDP != Desc.Protocol)
		return oErrorSetLast(std::errc::invalid_argument, "Socket is connected.  SendTo is invalid");

	SOCKADDR_IN Saddr;
	oNetAddrToSockAddr(_Destination, &Saddr);

	return SendToInternal(_pData, _Size, Saddr, _pBody, _SizeBody);
}

bool oSocketImpl::SendToInternal(const void* _pHeader, oSocket::size_t _SizeHeader, const SOCKADDR_IN& _Destination, const void* _pBody, oSocket::size_t _SizeBody) threadsafe
{
	auto lockedThis = oLockThis(DescMutex);

	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if (Disabled)
		return false;

	const auto &CurDesc = lockedThis->Desc;

	if (oSocket::BLOCKING == CurDesc.Style)
	{
		unsigned int _TimeoutMS = CurDesc.BlockingSettings.SendTimeout;

		oScopedPartialTimeout Timeout(&_TimeoutMS);
		if (SOCKET_ERROR == setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO,(char *)&_TimeoutMS, sizeof(unsigned int)))
			return false;

		try { winsock::send(hSocket, _pHeader, _SizeHeader, &_Destination); }
		catch (std::exception& e) { return oErrorSetLast(e); }

		if (_pBody)
		{
			try { winsock::send(hSocket, _pBody, _SizeBody, &_Destination); }
			catch (std::exception& e) { return oErrorSetLast(e); }
		}
	}
	else
	{
		oIOCPOp* pIOCPOp = pIOCP->AcquireSocketOp();
		if (!pIOCPOp)
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

		if (0 != WSASendTo(hSocket, pBuff, BuffCount, &bytesSent, 0, (SOCKADDR*)&pOp->SockAddr, sizeof(sockaddr_in), (WSAOVERLAPPED*)pIOCPOp, nullptr))
		{
			int lastError = WSAGetLastError();
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
	if (Disabled)
		return false;

	const auto &CurDesc = lockedThis->Desc;

	if (oSocket::BLOCKING == CurDesc.Style)
	{
		return (oSocket::size_t)winsock::recvfrom_blocking(lockedThis->hSocket, _pBuffer, _Size, CurDesc.BlockingSettings.RecvTimeout, lockedThis->DefaultAndRecvAddr);
	}
	else
	{
		oIOCPOp* pIOCPOp = pIOCP->AcquireSocketOp();
		if (!pIOCPOp)
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

		WSARecvFrom(hSocket, pOp->Payload, 1, &bytesRecvd, &flags, (SOCKADDR*)&pOp->SockAddr, &sizeOfSockAddr, (WSAOVERLAPPED*)pIOCPOp, nullptr);
		return _Size;
	}
}

bool oSocketImpl::SendEncrypted(const void* _pData, oSocket::size_t _Size) threadsafe
{
	auto lockedThis = oLockThis(DescMutex);

	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if (Disabled)
		return false;

	const auto &CurDesc = lockedThis->Desc;
	if (oSocket::ASYNC == CurDesc.Style)
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
	if (Disabled)
		return false;

	if (Encryptor.c_ptr())
	{
		return Encryptor->Receive(hSocket, (char *)_pBuffer, _Size, Desc.BlockingSettings.RecvTimeout);
	}
	return 0;
}

void oSocketImpl::IOCPCallback(oIOCPOp* pIOCPOp)
{
	if (Disabled) //probably sitting in a reuse pool and got a late callback from iocp
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
		//this can fail if the socket is getting closed.
		BOOL result = WSAGetOverlappedResult(hSocket, (WSAOVERLAPPED*)pIOCPOp, (LPDWORD)&szData, false, &flags);
		oASSERT(result, "WSAGetOverlappedResult failed.");
	}

	// We return the op before calling back the user so the op is available to use again
	pIOCP->ReturnOp(pIOCPOp);

	switch(Type)
	{
	case Operation::Op_Recv:
		if (InternalCallback)
			InternalCallback->ProcessSocketReceive(pHeader, szData, address, Proxy);
		break;
	case Operation::Op_Send:
		if (InternalCallback)
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
	if (Disabled)
		return false;

	return winsock::connected(hSocket);
}

bool oSocketImpl::GetHostname( char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort ) const threadsafe 
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if (Disabled)
		return false;

	winsock::get_hostname(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, hSocket);
	return true;
}

bool oSocketImpl::GetPeername( char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort ) const threadsafe 
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if (Disabled)
		return false;

	winsock::get_peername(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, hSocket);
	return true;
}

bool oSocketImpl::SetKeepAlive(unsigned int _TimeoutMS, unsigned int _IntervalMS) const threadsafe
{
	oASSERT(!Disabled, "Should never call functions on disabled sockets");
	if (Disabled)
		return false;

	winsock::set_keepalive(hSocket, _TimeoutMS, _IntervalMS);
	return true;
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
	if (*_pSuccess)
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
	if (oSocket::BLOCKING == _Desc.Style)
	{
		// Place the socket into non-blocking mode by first clearing the event then disabling FIONBIO
		{
			if (SOCKET_ERROR == WSAEventSelect(_hTarget, NULL, NULL)) return nullptr;

			u_long nonBlocking = 0;
			if (SOCKET_ERROR == ioctlsocket(_hTarget, FIONBIO, &nonBlocking)) return nullptr;
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

////////////
// server socket 2
////////////

oSocket* oSocketCreateFromServer2(std::shared_ptr<oSocketImpl> _Target, oSocket::DESC _Desc, bool* _pSuccess)
{
	if (oSocket::BLOCKING == _Desc.Style)
	{
		// Place the socket into non-blocking mode by first clearing the event then disabling FIONBIO
		{
			if (SOCKET_ERROR == WSAEventSelect(_Target->GetHandle(), NULL, NULL)) return nullptr;

			u_long nonBlocking = 0;
			if (SOCKET_ERROR == ioctlsocket(_Target->GetHandle(), FIONBIO, &nonBlocking)) return nullptr;
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
	
	typedef std::function<void (oSocketImpl* _Socket)> ServerDisconnect;

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
	concurrent_queue<oSocketImpl*> AllSocketsPool;

	//This is the list of sockets available for use
	concurrent_queue<oSocketImpl*> AcceptSocketsPool;

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
		SOCKET hSocket = winsock::make_async_accept();
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
	if (ServerDisconnectFn)
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
		char AcceptAddressesBuffer[winsock::accept_buffer_size]; //From msdn docs, buffer needs 16 bytes padding for each address. buffer will hold 2
	};

	void IOCPCallback(oIOCPOp* _pSocketOp);
	void Accept();
	void Disconnect(oSocketImpl* _Socket);

	oIOCP* pIOCP;

	SOCKET hListenSocket;

	char DebugName[64];
	DESC Desc;
	intrusive_ptr<SocketServerPool> SocketPool;

	int DesiredAccepts;
	std::atomic<int> IssuedAcceptCount; //once this exceeds DesiredAccepts, accepts will begin to get starved out.
	//Once we run out of socket ops, start saving disconnects for later.
	//	Note that before this happens, accepts will have been starved out by IssuedAcceptCount.
	//	Disconnects themselves will start the normal accept "loop" back up.
	concurrent_queue<oSocketImpl*> PendingDisconnects;
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
		strlcpy(DebugName, _DebugName);

	*_pSuccess = false;

	oNetAddr Addr;
	oSocketPortSet(Desc.ListenPort, &Addr);
	sockaddr_in SAddr;
	oNetAddrToSockAddr(Addr, &SAddr);

	hListenSocket = winsock::make(SAddr, winsock::reliable | winsock::exclusive_address, ouro::infinite, ouro::invalid);
	if (INVALID_SOCKET == hListenSocket)
		return; // leave last error from inside oWinsockCreate
	
	SocketPool = intrusive_ptr<SocketServerPool>(new SocketServerPool(std::bind(&SocketServer2_Impl::Disconnect, this, std::placeholders::_1) , _pSuccess), false);
	if (!(*_pSuccess))
	{
		oErrorSetLast(std::errc::invalid_argument, "Failed to create the socket pool.");
		return;
	}
	*_pSuccess = false;

	int numIOCPThreads = oIOCPThreadCount();
	DesiredAccepts = static_cast<int>(numIOCPThreads*AcceptCountMultiplier);

	oIOCP::DESC IOCPDesc;
	IOCPDesc.Handle = reinterpret_cast<oHandle>(hListenSocket);
	IOCPDesc.IOCompletionRoutine = std::bind(&SocketServer2_Impl::IOCPCallback, this, std::placeholders::_1);
	IOCPDesc.MaxOperations = DesiredAccepts*ExtraSocketOpsMultiplier;
	IOCPDesc.PrivateDataSize = sizeof(Operation);

	if (!oIOCPCreate(IOCPDesc, [&](){
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
	if (SocketPool)
		SocketPool->ServerClosed();

	winsock::close(hListenSocket);
}

int SocketServer2_Impl::Reference() threadsafe 
{ 
	return pIOCP->Reference(); 
}  

void SocketServer2_Impl::Release() threadsafe 
{ 
	if (pIOCP)
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
	if (IssuedAcceptCount > DesiredAccepts)
	{
		return;
	}

	oSocketImpl* socket = SocketPool->GetSocket();
	
	oIOCPOp* iocpOp = pIOCP->AcquireSocketOp();
	if (!iocpOp)
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

	winsock::async_result result = winsock::accept_async(hListenSocket, socket->GetHandle(), myop->AcceptAddressesBuffer, iocpOp);
	if (result == winsock::failed)
	{
		oASSERT(false, "Not really expecting the asyncex call to fail");

		oErrorSetLast(std::errc::protocol_error, "Failed to initiate an AsyncAccept");

		pIOCP->ReturnOp(iocpOp);

		return;
	}
	else if (result == winsock::completed) //handle very rare case where accept is completed synchronously
	{
		pIOCP->DispatchManualCompletion((oHandle)hListenSocket, iocpOp);
	}
	//else it went to iocp
}

void SocketServer2_Impl::Disconnect(oSocketImpl* _Socket)
{
	oIOCPOp* iocpOp = pIOCP->AcquireSocketOp();
	if (!iocpOp)
	{
		PendingDisconnects.push(_Socket); //ran out of socket ops, save for later
		return;
	}
	Operation* myop;
	iocpOp->GetPrivateData(&myop);
	myop->OpType = Operation::DISCONNECT_REQUEST;
	myop->Socket = _Socket;

	winsock::async_result result = winsock::failed;
	
	try { result = winsock::recycle_async_accept(hListenSocket, _Socket->GetHandle(), iocpOp); }
	catch (std::exception&) {}

	if (result == winsock::failed)
	{
		oASSERT(false, "not expecting disconectex to fail, if it is, can't recyle these sockets");
		pIOCP->ReturnOp(iocpOp);
	}
	else if (result == winsock::completed) //handle very rare case where Disconnect is completed synchronously
	{
		pIOCP->DispatchManualCompletion((oHandle)hListenSocket, iocpOp);
	}
	//else it went to iocp

	// disconnect will issue an accept once its done. prevent accepts from starving disconnects, iocp isn't fair.
	++IssuedAcceptCount;
}

bool SocketServer2_Impl::GetHostname(char* _pString, size_t _strLen) const threadsafe 
{
	winsock::get_hostname(_pString, _strLen, nullptr, NULL, nullptr, NULL, hListenSocket);
	return true;
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

	if (pOp->OpType == Operation::ACCEPT_REQUEST)
	{
		oSocket::DESC desc;
		desc.Style = oSocket::BLOCKING;
		desc.BlockingSettings = Desc.BlockingSettings;
		desc.ConnectionTimeoutMS = ouro::infinite; //not used for this type of socket anyway

		sockaddr_in* AddrLocal;
		int SzAddrLocal;
		sockaddr_in* AddrRemote;
		int SzAddrRemote;
		winsock::acceptexsockaddrs_async(pOp->Socket->GetHandle(), pOp->AcceptAddressesBuffer, (SOCKADDR**)&AddrLocal, &SzAddrLocal, (SOCKADDR**)&AddrRemote, &SzAddrRemote);
		oSockAddrToNetAddr(*AddrRemote, &desc.Addr);

		bool success = true;
		try { winsock::finish_async_accept(hListenSocket, pOp->Socket->GetHandle()); }
		catch (std::exception&) { success = false; }

		intrusive_ptr<oSocket> socket = nullptr;
		intrusive_ptr<SocketServerPool> sPool = SocketPool;
		if (success)
		{
			auto deleter = [sPool](oSocketImpl* _socket) mutable {
				_socket->Disable();
				sPool->RouteDisconnect(_socket);
			};

			std::shared_ptr<oSocketImpl> socketPtr(pOp->Socket, deleter);
			socket = intrusive_ptr<oSocket>(oSocketCreateFromServer2(socketPtr, desc, &success), false);
		}

		if (success)
			Desc.NewConnectionCallback(socket);

		// FIXME: Hack for socket recycling.  For DisconectEx to work properly the socket has to be asyncrhonous
		// therefore if the user isn't holding onto the socket ensure we've forced it to asyncrhonous before letting go
		if (1 == socket->Reference())
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
	else if (pOp->OpType == Operation::DISCONNECT_REQUEST) 
	{
		//socket should be ready for reuse now
		SocketPool->ReturnSocket(pOp->Socket);
		pIOCP->ReturnOp(_pSocketOp);

		--IssuedAcceptCount;
		oSocketImpl* socketToDis;
		if (PendingDisconnects.try_pop(socketToDis))
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
