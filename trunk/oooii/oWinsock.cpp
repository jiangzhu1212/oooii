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
#include <oooii/oAssert.h>
#include <oooii/oAtomic.h>
#include <oooii/oErrno.h>
#include <oooii/oStdio.h>
#include <oooii/oThreading.h>

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

	oAssert::Reference();

	hWs2_32 = oModule::Link("ws2_32.dll", detail::ws2_32_dll_Functions, (void**)&accept, oCOUNTOF(detail::ws2_32_dll_Functions));
	oASSERT(hWs2_32, "Failed to load and link ws2_32.dll");

	hMswsock = oModule::Link("mswsock.dll", detail::mswsock_dll_Functions, (void**)&AcceptEx, oCOUNTOF(detail::mswsock_dll_Functions));
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
		oModule::Unlink(hMswsock);

	if (hWs2_32)
		oModule::Unlink(hWs2_32);

	oAssert::Release();
}

struct WSA_ERR
{
	HRESULT WSAError;
	const char* WSAErrorStr;
	errno_t Errno;
	const char* WSADesc;
};

#define WSAERR(x) x, #x
#define WSAERRNO(x) WSA##x, "WSA" #x, x
static const WSA_ERR sErrors[] =
{
	{ WSAERR(WSANOTINITIALISED), ENOTRECOVERABLE, "Either the application has not called WSAStartup or WSAStartup failed. The application may be accessing a socket that the current active task does not own (that is, trying to share a socket between tasks), or WSACleanup has been called too many times." },
	{ WSAERRNO(ENETDOWN), 0 },
	{ WSAERR(WSAVERNOTSUPPORTED), EINVAL, 0 },
	{ WSAERRNO(EAFNOSUPPORT), 0 },
	{ WSAERRNO(EINPROGRESS), 0 },
	{ WSAERRNO(EMFILE), 0 },
	{ WSAERRNO(EINVAL), 0 },
	{ WSAERRNO(ENOBUFS), 0 },
	{ WSAERRNO(EPROTONOSUPPORT), 0 },
	{ WSAERRNO(EPROTOTYPE), 0 },
	{ WSAERRNO(ESOCKTNOSUPPORT), 0 },
	{ WSAERR(WSASYSNOTREADY), EINVAL, 0 },
	{ WSAERR(WSAEPROCLIM), EMLINK, 0 },
	{ WSAERRNO(EFAULT), 0 },
	{ WSAERRNO(ENOTSOCK), 0 },
	{ WSAERRNO(ENETRESET), 0 },
	{ WSAERRNO(ENOTCONN), 0 },
	{ WSAERRNO(EADDRNOTAVAIL), 0 },
	{ WSAERRNO(ECONNREFUSED), 0 },
	{ WSAERRNO(ECONNABORTED), 0 },
	{ WSAERRNO(ECONNRESET), 0 },
	{ WSAERRNO(ENETUNREACH), 0 },
	{ WSAERRNO(EHOSTUNREACH), 0 },
	{ WSAERRNO(ETIMEDOUT), 0 },
	{ WSAERRNO(EWOULDBLOCK), 0 },
	{ WSAERR(WSAEACCES), EPERM, 0 },
	{ WSAERRNO(EADDRINUSE), 0 },
	{ WSAERRNO(EOPNOTSUPP), 0 },
	{ WSAERRNO(ESHUTDOWN), 0 },
	{ WSAERRNO(EMSGSIZE), 0 },
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
