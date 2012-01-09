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
#include "oWinsock.h"
#include <oBasis/oAssert.h>
#include <oBasis/oError.h>
#include <oPlatform/oReporting.h>
#include <cerrno>

namespace detail {

static const char* ws2_32_dll_Functions[] = 
{
	"accept",
	"bind",
	"closesocket",
	"connect",
	"freeaddrinfo",
	"FreeAddrInfoEx",
	"FreeAddrInfoW",
	"getaddrinfo",
	"GetAddrInfoW",
	"gethostbyaddr",
	"gethostbyname",
	"gethostname",
	"getnameinfo",
	"GetNameInfoW",
	"getpeername",
	"getprotobyname",
	"getprotobynumber",
	"getservbyname",
	"getservbyport",
	"getsockname",
	"getsockopt",
	"htonl",
	"htons",
	"inet_addr",
	"inet_ntoa",
	"inet_pton",
	"ioctlsocket",
	"listen",
	"ntohl",
	"ntohs",
	"recv",
	"recvfrom",
	"select",
	"send",
	"sendto",
	"setsockopt",
	"shutdown",
	"socket",
	"WSAAccept",
	"WSAAddressToStringA",
	"WSAAddressToStringW",
	"WSAAsyncGetHostByAddr",
	"WSAAsyncGetHostByName",
	"WSAAsyncGetProtoByName",
	"WSAAsyncGetProtoByNumber",
	"WSAAsyncGetServByName",
	"WSAAsyncGetServByPort",
	"WSAAsyncSelect",
	"WSACancelAsyncRequest",
	"WSACleanup",
	"WSACloseEvent",
	"WSAConnect",
	"WSAConnectByList",
	"WSAConnectByNameA",
	"WSAConnectByNameW",
	"WSACreateEvent",
	"WSADuplicateSocketA",
	"WSADuplicateSocketW",
	"WSAEnumNameSpaceProvidersA",
	"WSAEnumNameSpaceProvidersW",
	"WSAEnumNameSpaceProvidersExA",
	"WSAEnumNameSpaceProvidersExW",
	"WSAEnumNetworkEvents",
	"WSAEnumProtocolsA",
	"WSAEnumProtocolsW",
	"WSAEventSelect",
	"__WSAFDIsSet",
	"WSAGetLastError",
	"WSAGetOverlappedResult",
	"WSAGetQOSByName",
	"WSAGetServiceClassInfoA",
	"WSAGetServiceClassInfoW",
	"WSAGetServiceClassNameByClassIdA",
	"WSAGetServiceClassNameByClassIdW",
	"WSAHtonl",
	"WSAHtons",
	"WSAInstallServiceClassA",
	"WSAInstallServiceClassW",
	"WSAIoctl",
	"WSAJoinLeaf",
	"WSALookupServiceBeginA",
	"WSALookupServiceBeginW",
	"WSALookupServiceEnd",
	"WSALookupServiceNextA",
	"WSALookupServiceNextW",
	"WSANSPIoctl",
	"WSANtohl",
	"WSANtohs",
	"WSAPoll",
	"WSAProviderConfigChange",
	"WSARecv",
	"WSARecvDisconnect",
	"WSARecvFrom",
	"WSARemoveServiceClass",
	"WSAResetEvent",
	"WSASend",
	"WSASendDisconnect",
	"WSASendMsg",
	"WSASendTo",
	"WSASetEvent",
	"WSASetLastError",
	"WSASetServiceA",
	"WSASetServiceW",
	"WSAStartup",
	"WSAStringToAddressA",
	"WSAStringToAddressW",
	"WSAWaitForMultipleEvents",
	"WSASocketA",
	"WSASocketW",
	"GetAddrInfoExA",
	"GetAddrInfoExW",
	"SetAddrInfoExA",
	"SetAddrInfoExW",
	"inet_ntop", // InetNtopA as a macro
	"InetNtopW",
};

static const char* mswsock_dll_Functions[] = 
{
	"AcceptEx",
	"GetNameByTypeA",
	"GetNameByTypeW",
	"GetTypeByNameA",
	"GetTypeByNameW",
	"WSARecvEx",
	"TransmitFile",
};

static const char* fwpucint_dll_Functions[] = 
{
	"WSADeleteSocketPeerTargetName",
	"WSAImpersonateSocketPeer",
	"WSAQuerySocketSecurity",
	"WSARevertImpersonation",
	"WSASetSocketSecurity",
	"WSASetSocketPeerTargetName",
};

} // namespace detail

// WSAIoctl functions with the SIO_GET_EXTENSION_FUNCTION_POINTER
// "ConnectEx",
// "DisconnectEx",
// "GetAcceptExSockaddrs",
// TransmitPackets
// "WSARecvMsg",

const static unsigned int kWinsockMajorVersion = 2;
const static unsigned int kWinsockMinorVersion = 2;

#define oTRACE_WINSOCK_LIFETIME(strState) oTRACE("oWinsock v%u.%u " strState, kWinsockMajorVersion, kWinsockMinorVersion)

oWinsock::oWinsock()
{
	oTRACE_WINSOCK_LIFETIME("initializing...");

	oReportingReference();

	hWs2_32 = oModuleLink("ws2_32.dll", detail::ws2_32_dll_Functions, (void**)&accept, oCOUNTOF(detail::ws2_32_dll_Functions));
	oASSERT(hWs2_32, "Failed to load and link ws2_32.dll");

	hMswsock = oModuleLink("mswsock.dll", detail::mswsock_dll_Functions, (void**)&AcceptEx, oCOUNTOF(detail::mswsock_dll_Functions));
	oASSERT(hMswsock, "Failed to load and link mswsock.dll");

	//hFwpucint = oModule::Link("fwpucint.dll", detail::fwpucint_dll_Functions, (void**)&WSADeleteSocketPeerTargetName, oCOUNTOF(detail::fwpucint_dll_Functions));
	//oASSERT(hFwpucint, "Failed to load and link fwpucint.dll");

	WORD wVersion = MAKEWORD(kWinsockMajorVersion, kWinsockMinorVersion);
	WSADATA wsaData;

#ifdef oENABLE_ASSERTS
	int err = 
#endif
	WSAStartup(wVersion, &wsaData);

	oWINSOCK_ASSERT(!err, "oWinsock 2.2 initialization failed");
	oTRACE_WINSOCK_LIFETIME("initialized.");
}

oWinsock::~oWinsock()
{
	oTRACE_WINSOCK_LIFETIME("deinitializing...");

	WSACleanup();

	if (hMswsock)
		oModuleUnlink(hMswsock);

	if (hWs2_32)
		oModuleUnlink(hWs2_32);

	oReportingRelease();
}

struct WSA_ERR
{
	HRESULT WSAError;
	const char* WSAErrorStr;
	errno_t Errno;
	const char* WSADesc;
};

#ifndef ESOCKTNOSUPPORT
#define ESOCKTNOSUPPORT -37
#endif
#ifndef ESHUTDOWN
#define ESHUTDOWN -36
#endif

#define WSAERR(x) x, #x
#define WSAERRNO(x) WSA##x, "WSA" #x, x
#define WSAERRNOX(x) WSA##x, "WSA" #x, EINVAL
static const WSA_ERR sErrors[] =
{
	{ WSAERR(WSANOTINITIALISED), /*ENOTRECOVERABLE*/EINVAL, "Either the application has not called WSAStartup or WSAStartup failed. The application may be accessing a socket that the current active task does not own (that is, trying to share a socket between tasks), or WSACleanup has been called too many times." },
	{ WSAERRNOX(ENETDOWN), 0 },
	{ WSAERR(WSAVERNOTSUPPORTED), EINVAL, 0 },
	{ WSAERRNOX(EAFNOSUPPORT), 0 },
	{ WSAERRNOX(EINPROGRESS), 0 },
	{ WSAERRNO(EMFILE), 0 },
	{ WSAERRNO(EINVAL), 0 },
	{ WSAERRNOX(ENOBUFS), 0 },
	{ WSAERRNOX(EPROTONOSUPPORT), 0 },
	{ WSAERRNOX(EPROTOTYPE), 0 },
	{ WSAERRNO(ESOCKTNOSUPPORT), 0 },
	{ WSAERR(WSASYSNOTREADY), EINVAL, 0 },
	{ WSAERR(WSAEPROCLIM), EMLINK, 0 },
	{ WSAERRNO(EFAULT), 0 },
	{ WSAERRNOX(ENOTSOCK), 0 },
	{ WSAERRNOX(ENETRESET), 0 },
	{ WSAERRNOX(ENOTCONN), 0 },
	{ WSAERRNOX(EADDRNOTAVAIL), 0 },
	{ WSAERRNOX(ECONNREFUSED), 0 },
	{ WSAERRNOX(ECONNABORTED), 0 },
	{ WSAERRNOX(ECONNRESET), 0 },
	{ WSAERRNOX(ENETUNREACH), 0 },
	{ WSAERRNOX(EHOSTUNREACH), 0 },
	{ WSAERRNOX(ETIMEDOUT), 0 },
	{ WSAERRNOX(EWOULDBLOCK), 0 },
	{ WSAERR(WSAEACCES), EPERM, 0 },
	{ WSAERRNOX(EADDRINUSE), 0 },
	{ WSAERRNOX(EOPNOTSUPP), 0 },
	{ WSAERRNO(ESHUTDOWN), 0 },
	{ WSAERRNOX(EMSGSIZE), 0 },
};

const char* oWinsock::AsString(int _WSAWinSockError)
{
	for (size_t i = 0; i < oCOUNTOF(sErrors); i++)
		if (_WSAWinSockError == sErrors[i].WSAError)
			return sErrors[i].WSAErrorStr;
	return "Unknown WSA error";
}

const char* oWinsock::GetErrorDesc(int _WSAWinSockError)
{
	for (size_t i = 0; i < oCOUNTOF(sErrors); i++)
		if (_WSAWinSockError == sErrors[i].WSAError && sErrors[i].WSADesc)
			return sErrors[i].WSADesc;
	
	return "See http://msdn.microsoft.com/en-us/library/ms740668(v=vs.85).aspx for more information.";
}

errno_t oWinsock::GetErrno(int _WSAWinSockError)
{
	for (size_t i = 0; i < oCOUNTOF(sErrors); i++)
		if (_WSAWinSockError == sErrors[i].WSAError)
			return sErrors[i].Errno;
	
	return EINVAL;
}

bool oWinsock::GetFunctionPointer_ConnectEx(SOCKET s, LPFN_CONNECTEX* ppConnectEx)
{
	DWORD dwBytesReturned = 0;
	GUID guid = WSAID_CONNECTEX;
	return SOCKET_ERROR != oWinsock::Singleton()->WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID), ppConnectEx, sizeof(LPFN_CONNECTEX), &dwBytesReturned, 0, 0);
}

bool oWinsock::GetFunctionPointer_DisconnectEx(SOCKET s, LPFN_DISCONNECTEX* ppDisconnectEx)
{
	DWORD dwBytesReturned = 0;
	GUID guid = WSAID_DISCONNECTEX;
	return SOCKET_ERROR != oWinsock::Singleton()->WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID), ppDisconnectEx, sizeof(LPFN_DISCONNECTEX), &dwBytesReturned, 0, 0);
}

bool oWinsock::GetFunctionPointer_GetAcceptExSockaddrs(SOCKET s, LPFN_GETACCEPTEXSOCKADDRS* ppGetAcceptExSockaddrs)
{
	DWORD dwBytesReturned = 0;
	GUID guid = WSAID_GETACCEPTEXSOCKADDRS;
	return SOCKET_ERROR != oWinsock::Singleton()->WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID), ppGetAcceptExSockaddrs, sizeof(LPFN_GETACCEPTEXSOCKADDRS), &dwBytesReturned, 0, 0);
}

bool oWinsock::GetFunctionPointer_TransmitPackets(SOCKET s, LPFN_TRANSMITPACKETS* ppTransmitPackets)
{
	DWORD dwBytesReturned = 0;
	GUID guid = WSAID_TRANSMITPACKETS;
	return SOCKET_ERROR != oWinsock::Singleton()->WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID), ppTransmitPackets, sizeof(LPFN_TRANSMITPACKETS), &dwBytesReturned, 0, 0);
}

bool oWinsock::GetFunctionPointer_WSARecvMsg(SOCKET s, LPFN_WSARECVMSG* ppWSARecvMsg)
{
	DWORD dwBytesReturned = 0;
	GUID guid = WSAID_WSARECVMSG;
	return SOCKET_ERROR != oWinsock::Singleton()->WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID), ppWSARecvMsg, sizeof(LPFN_WSARECVMSG), &dwBytesReturned, 0, 0);
}

void oWinsockCreateAddr(sockaddr_in* _pOutSockAddr, const char* _Hostname)
{
	oWinsock* ws = oWinsock::Singleton();

	char host[1024];
	strcpy_s(host, _Hostname);
	unsigned short port = 0;
	char* strPort = strstr(host, ":");
	if (strPort)
	{
		port = static_cast<unsigned short>(atoi(strPort+1));
		*strPort = 0;
	}

	memset(_pOutSockAddr, 0, sizeof(sockaddr_in));
	hostent* pHost = ws->gethostbyname(host);
	_pOutSockAddr->sin_family = AF_INET;
	_pOutSockAddr->sin_port = ws->htons(port);
	_pOutSockAddr->sin_addr.s_addr = ws->inet_addr(ws->inet_ntoa(*(in_addr*)*pHost->h_addr_list));
}

void oWinsockAddrToHostname(sockaddr_in* _pSockAddr, char* _OutHostname, size_t _SizeOfHostname)
{
	oWinsock* ws = oWinsock::Singleton();

	unsigned long addr = ws->ntohl(_pSockAddr->sin_addr.s_addr);
	unsigned short port = ws->ntohs(_pSockAddr->sin_port);

	sprintf_s(_OutHostname, _SizeOfHostname, "%u.%u.%u.%u:%u", (addr&0xFF000000)>>24, (addr&0xFF0000)>>16, (addr&0xFF00)>>8, addr&0xFF, port);
}

//#define FD_TRACE(TracePrefix, TraceName, FDEvent) oTRACE("%s%s%s: %s (%s)", oSAFESTR(TracePrefix), _TracePrefix ? " " : "", oSAFESTR(TraceName), #FDEvent, oWinsock::GetErrorString(_pNetworkEvents->iErrorCode[FDEvent##_BIT]))
#define FD_TRACE(TracePrefix, TraceName, FDEvent)

// Assumes WSANETWORKEVENTS ne; int err;
#define FD_CHECK(FDEvent) \
	if (_pNetworkEvents->lNetworkEvents & FDEvent) \
	{	FD_TRACE(_TracePrefix, _TraceName, ##FDEvent); \
		err = _pNetworkEvents->iErrorCode[FDEvent##_BIT]; \
	}

// Checks all values from a network event and return an error based on what happened.
bool oWinsockTraceEvents(const char* _TracePrefix, const char* _TraceName, const WSANETWORKEVENTS* _pNetworkEvents)
{
	int err = 0;

	if (!_pNetworkEvents->lNetworkEvents)
	{
		// http://www.mombu.com/microsoft/alt-winsock-programming/t-wsaenumnetworkevents-returns-no-event-how-is-this-possible-1965867.html
		// Also Google "spurious wakeup" 
		oASSERT(false, "%s%s%s: WSAEVENT, but no lNetworkEvent: \"spurious wakeup\". You should ignore the event as if it never happened by testing for _pNetworkEvents->lNetworkEvents == 0 in calling code.", oSAFESTR(_TracePrefix), _TracePrefix ? " " : "", oSAFESTR(_TraceName));
		return oErrorSetLast(oERROR_GENERIC, "%s%s%s: WSAEVENT, but no lNetworkEvent: \"spurious wakeup\". You should ignore the event as if it never happened by testing for _pNetworkEvents->lNetworkEvents == 0 in calling code.", oSAFESTR(_TracePrefix), _TracePrefix ? " " : "", oSAFESTR(_TraceName));
	}
	
	FD_CHECK(FD_READ); FD_CHECK(FD_WRITE); FD_CHECK(FD_OOB); FD_CHECK(FD_CONNECT); 
	FD_CHECK(FD_ACCEPT); FD_CHECK(FD_CLOSE); FD_CHECK(FD_QOS); FD_CHECK(FD_GROUP_QOS); 
	FD_CHECK(FD_ROUTING_INTERFACE_CHANGE); FD_CHECK(FD_ADDRESS_LIST_CHANGE);

	return true;
}

template<typename SOCKADDR_T>
SOCKET oWinsockCreateImpl( const SOCKADDR_T _Addr, int _ORedWinsockOptions, unsigned int _TimeoutMS /*= 0*/, unsigned int _MaxNumConnections /*= 0*/ )
{
	oWinsock* ws = oWinsock::Singleton();
	const bool kReliable = !!(_ORedWinsockOptions & oWINSOCK_RELIABLE);

	WSAEVENT hConnectEvent = 0;
	SOCKET hSocket = ws->WSASocket(AF_INET, kReliable ? SOCK_STREAM : SOCK_DGRAM, kReliable ? IPPROTO_TCP : IPPROTO_UDP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET) goto error;

	u_long enabled = !( _ORedWinsockOptions & oWINSOCK_BLOCKING );
	if (SOCKET_ERROR == ws->ioctlsocket(hSocket, FIONBIO, &enabled)) goto error;
	if ((_ORedWinsockOptions & oWINSOCK_REUSE_ADDRESS) && SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enabled, sizeof(enabled))) goto error;
	if (!kReliable && (_ORedWinsockOptions & oWINSOCK_ALLOW_BROADCAST) && SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&enabled, sizeof(enabled))) goto error;
	
	if (kReliable)
	{
		if (_MaxNumConnections)
		{
			if (SOCKET_ERROR == ws->bind(hSocket, (const sockaddr*)&_Addr, sizeof(_Addr))) goto error;
			if (kReliable && SOCKET_ERROR == ws->listen(hSocket, _MaxNumConnections)) goto error;
		}
		else
		{
			if( _TimeoutMS > 0 )
			{
				hConnectEvent = ws->WSACreateEvent();
				if (SOCKET_ERROR == ws->WSAEventSelect(hSocket, hConnectEvent, FD_CONNECT ) ) goto error;
			}

			if (SOCKET_ERROR == ws->connect(hSocket, (const sockaddr*)&_Addr, sizeof(_Addr)) && ws->WSAGetLastError() != WSAEWOULDBLOCK) goto error;

			if( hConnectEvent )
				if( !oWinsockWaitMultiple( &hConnectEvent, 1, true, false, _TimeoutMS ) ) 
					goto error;
				else
				{
					if (SOCKET_ERROR == ws->WSAEventSelect(hSocket, nullptr, 0 ) ) goto error;
					unsigned long cmd = 0;
					if(SOCKET_ERROR == ws->ioctlsocket(hSocket, FIONBIO, &cmd) ) goto error;
					ws->WSACloseEvent(hConnectEvent);
				}
		}
	}
	else
	{
		if (SOCKET_ERROR == ws->bind(hSocket, (const sockaddr*)&_Addr, sizeof(_Addr))) goto error;
	}

	return hSocket;
error:
	oWINSOCK_SETLASTERROR("oWinsockCreate");
	if (hSocket != INVALID_SOCKET)
		ws->closesocket(hSocket);
	if( hConnectEvent )
		ws->WSACloseEvent(hConnectEvent);

	return INVALID_SOCKET;
}

SOCKET oWinsockCreate( const sockaddr_in _Addr, int _ORedWinsockOptions, unsigned int _TimeoutMS /*= 0*/, unsigned int _MaxNumConnections /*= 0*/ )
{
	return oWinsockCreateImpl(_Addr, _ORedWinsockOptions, _TimeoutMS, _MaxNumConnections);
}

SOCKET oWinsockCreate( const sockaddr_in6 _Addr, int _ORedWinsockOptions, unsigned int _TimeoutMS /*= 0*/, unsigned int _MaxNumConnections /*= 0*/ )
{
	return oWinsockCreateImpl(_Addr, _ORedWinsockOptions, _TimeoutMS, _MaxNumConnections);
}


SOCKET DEPRECATED_oWinsockCreate(const char* _Hostname, int _ORedWinsockOptions, unsigned int _MaxNumConnections, size_t _SendBufferSize, size_t _ReceiveBufferSize)
{
	oWinsock* ws = oWinsock::Singleton();
	const bool kReliable = !!(_ORedWinsockOptions & oWINSOCK_RELIABLE);

	SOCKET hSocket = ws->WSASocket(AF_INET, kReliable ? SOCK_STREAM : SOCK_DGRAM, kReliable ? IPPROTO_TCP : IPPROTO_UDP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET) goto error;

	u_long enabled = !( _ORedWinsockOptions & oWINSOCK_BLOCKING );
	if (SOCKET_ERROR == ws->ioctlsocket(hSocket, FIONBIO, &enabled)) goto error;
	if ((_ORedWinsockOptions & oWINSOCK_REUSE_ADDRESS) && SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enabled, sizeof(enabled))) goto error;
	if (_SendBufferSize && SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&_SendBufferSize, sizeof(_SendBufferSize))) goto error;
	if (_ReceiveBufferSize && SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&_ReceiveBufferSize, sizeof(_ReceiveBufferSize))) goto error;
	if (!kReliable && (_ORedWinsockOptions & oWINSOCK_ALLOW_BROADCAST) && SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&enabled, sizeof(enabled))) goto error;
	
	char host[1024];
	strcpy_s(host, _Hostname);	

	unsigned short port = 0;
	char* strPort = strstr(host, ":");
	if (strPort)
	{
		port = static_cast<unsigned short>(atoi(strPort+1));
		*strPort = 0;
	}

	sockaddr_in saddr;
	oWinsockCreateAddr(&saddr, _Hostname);

	if (_MaxNumConnections)
	{
		if (SOCKET_ERROR == ws->bind(hSocket, (const sockaddr*)&saddr, sizeof(saddr))) goto error;
		if (kReliable && SOCKET_ERROR == ws->listen(hSocket, _MaxNumConnections)) goto error;
	}

	else
	{
		if (kReliable)
		{
			if (SOCKET_ERROR == ws->connect(hSocket, (const sockaddr*)&saddr, sizeof(saddr)) && ws->WSAGetLastError() != WSAEWOULDBLOCK) goto error;
		}

		else
		{
			if (SOCKET_ERROR == ws->bind(hSocket, (const sockaddr*)&saddr, sizeof(saddr))) goto error;
		}
	}

	return hSocket;
error:
	oWINSOCK_SETLASTERROR("oWinsockCreate");
	if (hSocket != INVALID_SOCKET)
		ws->closesocket(hSocket);
	return INVALID_SOCKET;
}

bool oWinsockClose(SOCKET _hSocket)
{
	// http://msdn.microsoft.com/en-us/library/ms738547(v=vs.85).aspx
	// Specifically read the community comment that says the main article doesn't 
	// work...

	// @oooii-tony: Ignore WSAENOTCONN since we're closing this socket anyway. It 
	// means the other side is detached already.

	if (_hSocket)
	{
		oWinsock* ws = oWinsock::Singleton();

		if (SOCKET_ERROR == ws->shutdown(_hSocket, SD_BOTH) && ws->WSAGetLastError() != WSAENOTCONN)
		{
			oWINSOCK_SETLASTERROR("shutdown");
			return false;
		}

		LPFN_DISCONNECTEX FNDisconnectEx = 0;
		if (!ws->GetFunctionPointer_DisconnectEx(_hSocket, &FNDisconnectEx) && ws->WSAGetLastError() != WSAENOTCONN)
		{
			oWINSOCK_SETLASTERROR("GetFunctionPointer_DisconnectEx");
			return false;
		}

		if (!FNDisconnectEx(_hSocket, 0, 0, 0) && ws->WSAGetLastError() != WSAENOTCONN)
		{
			oWINSOCK_SETLASTERROR("DisconnectEx");
			return false;
		}

		if (SOCKET_ERROR == ws->closesocket(_hSocket) && ws->WSAGetLastError() != WSAENOTCONN)
		{
			oWINSOCK_SETLASTERROR("closesocket");
			return false;
		}
	}

	return true;
}

bool oWinsockWaitMultiple(WSAEVENT* _pHandles, size_t _NumberOfHandles, bool _WaitAll, bool _Alertable, unsigned int _TimeoutMS)
{
	// @oooii-tony: there is something called "spurious wakeups" (Google it for 
	// more info) that can signal an event though no user-space event has been 
	// triggered. My reading was that it only applied to the mask in 
	// WSANETWORKEVENTS, so if you're not using that then this doesn't matter, but
	// it's the event that gets triggered incorrectly, not the mask that gets 
	// filled out incorrectly from my readings, so it *could* happen any time the
	// event is waited on. If something funky occurs in this wait, start debugging 
	// with more "spurious wakeup" investigation.

	return WSA_WAIT_TIMEOUT != oWinsock::Singleton()->WSAWaitForMultipleEvents(static_cast<DWORD>(_NumberOfHandles), _pHandles, _WaitAll, _TimeoutMS == oINFINITE_WAIT ? WSA_INFINITE : _TimeoutMS, _Alertable);
}

// If the socket was created using oWinSockCreate (WSAEventSelect()), this function can 
// be used to wait on that event and receive any events breaking the wait.
bool oWinsockWait(SOCKET _hSocket, WSAEVENT _hEvent, WSANETWORKEVENTS* _pNetEvents, unsigned int _TimeoutMS)
{
	oWinsock* ws = oWinsock::Singleton();
	bool eventFired = true;
	unsigned int timeout = _TimeoutMS;
	_pNetEvents->lNetworkEvents = 0;
	oScopedPartialTimeout spt(&timeout);
	while (!_pNetEvents->lNetworkEvents && eventFired)
	{
		{
			spt.UpdateTimeout();
			eventFired = oWinsockWaitMultiple(&_hEvent, 1, true, false, timeout);
			if (eventFired)
			{
				if (SOCKET_ERROR == ws->WSAEnumNetworkEvents(_hSocket, _hEvent, _pNetEvents))
				{
					oErrorSetLast(oERROR_IO, "%s", oWinsock::AsString(oWinsock::Singleton()->WSAGetLastError()));
					eventFired = false;
					break;
				}

				if (_pNetEvents->lNetworkEvents)
					break;
			}
		}
	}

	return eventFired;
}

bool oWinsockSend(SOCKET _hSocket, const void* _pSource, size_t _SizeofSource, const sockaddr_in* _pDestination)
{
	oASSERT(_SizeofSource < INT_MAX, "Underlying implementation uses 32-bit signed int for buffer size.");
	oWinsock* ws = oWinsock::Singleton();
	int bytesSent = 0;
	
	if (_pDestination)
		bytesSent = ws->sendto(_hSocket, (const char*)_pSource, static_cast<int>(_SizeofSource), 0, (const sockaddr*)_pDestination, sizeof(sockaddr_in));
	else
		bytesSent = ws->send(_hSocket, (const char*)_pSource, static_cast<int>(_SizeofSource), 0);
	if (bytesSent == SOCKET_ERROR)
		oErrorSetLast(oERROR_IO, "%s", oWinsock::AsString(ws->WSAGetLastError()));
	else if ((size_t)bytesSent == _SizeofSource)
		oErrorSetLast(oERROR_NONE);
	return (size_t)bytesSent == _SizeofSource;
}

size_t oWinsockReceive(SOCKET _hSocket, WSAEVENT _hEvent, void* _pDestination, size_t _SizeofDestination, unsigned int _TimeoutMS, int* _pInOutCanReceive, sockaddr_in* _pSource)
{
	oASSERT(_pInOutCanReceive, "_pInOutCanReceive must be specified.");
	oASSERT(_SizeofDestination < INT_MAX, "Underlying implementation uses 32-bit signed int for buffer size.");

	if (!_pDestination)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "Must specify a destination buffer");
		return 0;
	}

	oWinsock* ws = oWinsock::Singleton();

	int err = 0;
	WSANETWORKEVENTS ne;
	memset(&ne, 0, sizeof(ne));
	if (*_pInOutCanReceive)
	{
		err = WSAETIMEDOUT;
		bool eventFired = oWinsockWait(_hSocket, _hEvent, &ne, _TimeoutMS);
		if (eventFired)
				err = oWinsockTraceEvents("oWinSockReceive", 0, &ne) ? 0 : EINVAL;
	}

	int bytesReceived = 0;
	if (!err && (ne.lNetworkEvents & FD_READ))
	{
		if (_pSource)
		{
			int size = sizeof(sockaddr_in);
			bytesReceived = ws->recvfrom(_hSocket, (char*)_pDestination, static_cast<int>(_SizeofDestination), 0, (sockaddr*)_pSource, &size);
		}

		else
			bytesReceived = ws->recv(_hSocket, (char*)_pDestination, static_cast<int>(_SizeofDestination), 0);

		if (bytesReceived == SOCKET_ERROR)
		{
			err = oWinsock::GetErrno(ws->WSAGetLastError());
			bytesReceived = 0;
		}

		else if (!bytesReceived)
		{
			oStd::atomic_exchange(_pInOutCanReceive, false);
			err = ESHUTDOWN;
		}

		else
			err = 0;
	}

	else if ((ne.lNetworkEvents & FD_CLOSE) || ((ne.lNetworkEvents & FD_CONNECT) && err))
	{
		oErrorSetLast(oERROR_IO, "%s", oWinsock::AsString(ne.iErrorCode[FD_CLOSE_BIT]));
		oStd::atomic_exchange(_pInOutCanReceive, false);
	}

	char strerr[256];
	strerror_s(strerr, err);
	oErrorSetLast(oERROR_INVALID_PARAMETER, "%s", strerr);
	return bytesReceived;
}

bool oWinsockReceiveNonBlocking(SOCKET _hSocket, WSAEVENT _hEvent, void* _pDestination, size_t _SizeofDestination, sockaddr_in* _pSource, size_t* _pBytesReceived)
{
	oASSERT(_SizeofDestination < INT_MAX, "Underlying implementation uses 32-bit signed int for buffer size.");

	if (!_pDestination)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "Must specify a destination buffer");
		return false;
	}

	oWinsock* ws = oWinsock::Singleton();

	timeval waitTime;
	waitTime.tv_sec = 0;
	waitTime.tv_usec = 0;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(_hSocket, &set);

	if(!ws->select(1, &set, nullptr, nullptr, &waitTime))
	{
		_pBytesReceived = 0;
		return true;
	}

	int err = 0;
	int bytesReceived = 0;
	if (_pSource)
	{
		int size = sizeof(sockaddr_in);
		bytesReceived = ws->recvfrom(_hSocket, (char*)_pDestination, static_cast<int>(_SizeofDestination), 0, (sockaddr*)_pSource, &size);
	}

	else
		bytesReceived = ws->recv(_hSocket, (char*)_pDestination, static_cast<int>(_SizeofDestination), 0);

	if (bytesReceived == SOCKET_ERROR)
	{
		err = oWinsock::GetErrno(ws->WSAGetLastError());
		bytesReceived = 0;
	}

	else if (!bytesReceived)
	{
		err = ESHUTDOWN;
	}

	//else if ((ne.lNetworkEvents & FD_CLOSE) || ((ne.lNetworkEvents & FD_CONNECT) && err))
	//{
	//	oErrorSetLast(oWinsock::GetErrno(ne.iErrorCode[FD_CLOSE_BIT]));
	//	oStd::atomic_exchange(_pInOutCanReceive, false);
	//}

	*_pBytesReceived = bytesReceived;

	char strerr[256];
	strerror_s(strerr, err);
	oErrorSetLast(oERROR_INVALID_PARAMETER, "%s", strerr);
	return(0 == err);
}

bool oWinsockGetNameBase(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort, SOCKET _hSocket, oFUNCTION<int(SOCKET _hSocket, sockaddr* _Name, int* _pNameLen)> _GetSockAddr)
{
	oWinsock* ws = oWinsock::Singleton();

	sockaddr_in saddr;
	int size = sizeof(saddr);
	if (SOCKET_ERROR == _GetSockAddr(_hSocket, (sockaddr*)&saddr, &size))
	{
		errno_t err = oWinsock::GetErrno(ws->WSAGetLastError());
		oErrorSetLast(oERROR_IO, "%s: %s", oWinsock::AsString(ws->WSAGetLastError()), oWinsock::GetErrorDesc(ws->WSAGetLastError()));
		return false;
	}

	// Allow for the user to specify null for the parts they don't want.
	char localHostname[_MAX_PATH];
	char localService[16];

	char* pHostname = _OutHostname ? _OutHostname : localHostname;
	size_t sizeofHostname = _OutHostname ? _SizeofOutHostname : oCOUNTOF(localHostname);
	
	char* pService = _OutPort ? _OutPort : localService;
	size_t sizeofService = _OutPort ? _SizeofOutPort : oCOUNTOF(localService);

	ws->getnameinfo((sockaddr*)&saddr, size, pHostname, static_cast<DWORD>(sizeofHostname), pService, static_cast<DWORD>(sizeofService), 0);

	if (_OutIPAddress)
	{
		const char* ip = ws->inet_ntoa(saddr.sin_addr);
		errno_t err = strcpy_s(_OutIPAddress, _SizeofOutIPAddress, ip);
		if (err)
		{
			char strerr[256];
			strerror_s(strerr, err);
			oErrorSetLast(oERROR_IO, "%s", strerr);
			return false;
		}
	}

	return true;
}

bool oWinsockGetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort, SOCKET _hSocket)
{
	return oWinsockGetNameBase(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, _hSocket, oWinsock::Singleton()->getsockname);
}

bool oWinsockGetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort, SOCKET _hSocket)
{
	return oWinsockGetNameBase(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, _hSocket, oWinsock::Singleton()->getpeername);
}

bool oWinsockIsConnected(SOCKET _hSocket)
{
	// According to Google's think-tank, getting a peer name will touch the 
	// connection, so let's do that. (Another option would be to get the # secs
	// of connection time through SO_CONNECT_TIME, but I worry about checking 
	// immediately after connect before there get's a up for 1 sec count).

	char tmp[_MAX_PATH];
	return oWinsockGetPeername(tmp, oCOUNTOF(tmp), 0, 0, 0, 0, _hSocket);
}
