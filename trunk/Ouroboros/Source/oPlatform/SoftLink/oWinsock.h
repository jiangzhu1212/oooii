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
// A singleton providing soft-link to Window's winsock API as 
// well as some wrappers that address some of the odd and 
// boilerplate work required when dealing with networking on
// Windows.
#pragma once
#ifndef oWinsock_h
#define oWinsock_h

#include <oPlatform/oModule.h>
#include <oPlatform/Windows/oWindows.h>
#include <oPlatform/oSingleton.h>
#include <Winsock.h>
#include <winsock2.h>
#include <Mstcpip.h>
#include <Mswsock.h>
#include <nspapi.h>
#include <Rpc.h>
#include <Ws2tcpip.h>

// Convenient assert for always printing out winsock-specific errors.
#define oWINSOCK_ASSERT(x, msg, ...) oASSERT(x, msg "\n%s(%d): %s", ## __VA_ARGS__ oWinsock::AsString(oWinsock::Singleton()->WSAGetLastError()), oWinsock::Singleton()->WSAGetLastError(), oWinsock::GetErrorDesc(oWinsock::Singleton()->WSAGetLastError()))

// Call oErrorSetLast() with errno_t equivalent, but also include WSA error and extended desc in description
#define oWINSOCK_SETLASTERROR(strFnName) oErrorSetLast(std::errc::io_error, "oWinsock::" strFnName " failed %s(%d): %s", oWinsock::AsString(oWinsock::Singleton()->WSAGetLastError()), oWinsock::Singleton()->WSAGetLastError(), oWinsock::GetErrorDesc(oWinsock::Singleton()->WSAGetLastError()))

class oWinsock : public oProcessSingleton<oWinsock>
{
	// NOTE: gai_strerror is an inline function, so does not need to be linked
	// against or called from this oWinSock interface.
	oHMODULE hMswsock;
	oHMODULE hWs2_32;
	oHMODULE hFwpucint;

public:
	static const oGUID GUID;
	oWinsock();
	~oWinsock();

	// Enum as string (similar to oStd::as_string())
	static const char* AsString(int _WSAWinSockError);

	// Returns the extended MSDN error description of the error
	static const char* GetErrorDesc(int _WSAWinSockError);
	static errno_t GetErrno(int _WSAWinSockError);

	// Ws2_32 API
	SOCKET (__stdcall *accept)(SOCKET s, struct sockaddr *addr, int *addrlen);
	int (__stdcall *bind)(SOCKET s, const struct sockaddr *name, int namelen);
	int (__stdcall *closesocket)(SOCKET s);
	int (__stdcall *connect)(SOCKET s, const struct sockaddr *name, int namelen);
	void (__stdcall *freeaddrinfo)(struct addrinfo *ai);
	void (__stdcall *FreeAddrInfoEx)(PADDRINFOEX pAddrInfo);
	void (__stdcall *FreeAddrInfoW)(PADDRINFOW pAddrInfo);
	int (__stdcall *getaddrinfo)(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA *pHints, PADDRINFOA *ppResult);
	int (__stdcall *GetAddrInfoW)(PCWSTR pNodeName, PCWSTR pServiceName, const ADDRINFOW *pHints, PADDRINFOW *ppResult);
	struct hostent* (__stdcall *gethostbyaddr)(const char *addr, int len, int type);
	struct hostent* (__stdcall *gethostbyname)(const char *name);
	int (__stdcall *gethostname)(char *name, int namelen);
	int (__stdcall *getnameinfo)(const struct sockaddr *sa, socklen_t salen, char *host, DWORD hostlen, char *serv, DWORD servlen, int flags);
	int (__stdcall *GetNameInfoW)(const SOCKADDR *pSockaddr, socklen_t SockaddrLength, PWCHAR pNodeBuffer, DWORD NodeBufferSize, PWCHAR pServiceBuffer, DWORD ServiceBufferSize, INT Flags);
	int (__stdcall *getpeername)(SOCKET s, struct sockaddr *name, int *namelen);
	struct PROTOENT* (__stdcall *getprotobyname)(const char *name);
	struct PROTOENT* (__stdcall *getprotobynumber)(int number);
	struct servent* (__stdcall *getservbyname)(const char *name, const char *proto);
	struct servent* (__stdcall *getservbyport)(int port, const char *proto);
	int (__stdcall *getsockname)(SOCKET s, struct sockaddr *name, int *namelen);
	int (__stdcall *getsockopt)(SOCKET s, int level, int optname, char *optval, int *optlen);
	u_long (__stdcall *htonl)(u_long hostlong);
	u_short (__stdcall *htons)(u_short hostshort);
	unsigned long (__stdcall *inet_addr)(const char *cp);
	char* (__stdcall *inet_ntoa)(struct in_addr in);
	int (__stdcall *inet_pton)(INT Family, PCTSTR pszAddrString, PVOID pAddrBuf);
	int (__stdcall *ioctlsocket)(SOCKET s, long cmd, u_long *argp);
	int (__stdcall *listen)(SOCKET s, int backlog);
	u_long (__stdcall *ntohl)(u_long netlong);
	u_short (__stdcall *ntohs)(u_short netshort);
	int (__stdcall *recv)(SOCKET s, char *buf, int len, int flags);
	int (__stdcall *recvfrom)(SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen);
	int (__stdcall *select)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout);
	int (__stdcall *send)(SOCKET s, const char *buf, int len, int flags);
	int (__stdcall *sendto)(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen);
	int (__stdcall *setsockopt)(SOCKET s, int level, int optname, const char *optval, int optlen);
	int (__stdcall *shutdown)(SOCKET s, int how);
	SOCKET (__stdcall *socket)(int af, int type, int protocol);
	SOCKET (__stdcall *WSAAccept)(SOCKET s, struct sockaddr *addr, LPINT addrlen, LPCONDITIONPROC lpfnCondition, DWORD dwCallbackData);
	int (__stdcall *WSAAddressToStringA)(LPSOCKADDR lpsaAddress, DWORD dwAddressLength, LPWSAPROTOCOL_INFOA lpProtocolInfo, LPSTR lpszAddressString, LPDWORD lpdwAddressStringLength);
	int (__stdcall *WSAAddressToStringW)(LPSOCKADDR lpsaAddress, DWORD dwAddressLength, LPWSAPROTOCOL_INFOW lpProtocolInfo, LPWSTR lpszAddressString, LPDWORD lpdwAddressStringLength);
	HANDLE (__stdcall *WSAAsyncGetHostByAddr)(HWND hWnd, unsigned int wMsg, const char *addr, int len, int type, char *buf, int buflen);
	HANDLE (__stdcall *WSAAsyncGetHostByName)(HWND hWnd, unsigned int wMsg, const char *name, char *buf, int buflen);
	HANDLE (__stdcall *WSAAsyncGetProtoByName)(HWND hWnd, unsigned int wMsg, const char *name, char *buf, int buflen);
	HANDLE (__stdcall *WSAAsyncGetProtoByNumber)(HWND hWnd, unsigned int wMsg, int number, char *buf, int buflen);
	HANDLE (__stdcall *WSAAsyncGetServByName)(HWND hWnd, unsigned int wMsg, const char *name, const char *proto, char *buf, int buflen);
	HANDLE (__stdcall *WSAAsyncGetServByPort)(HWND hWnd, unsigned int wMsg, int port, const char *proto, char *buf, int buflen);
	int (__stdcall *WSAAsyncSelect)(SOCKET s, HWND hWnd, unsigned int wMsg, long lEvent);
	int (__stdcall *WSACancelAsyncRequest)(HANDLE hAsyncTaskHandle);
	int (__stdcall *WSACleanup)(void);
	BOOL (__stdcall *WSACloseEvent)(WSAEVENT hEvent);
	int (__stdcall *WSAConnect)(SOCKET s, const struct sockaddr *name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS);
	BOOL (__stdcall *WSAConnectByList)(SOCKET s, PSOCKET_ADDRESS_LIST SocketAddressList, LPDWORD LocalAddressLength, LPSOCKADDR LocalAddress, LPDWORD RemoteAddressLength, LPSOCKADDR RemoteAddress, const struct timeval *timeout, LPWSAOVERLAPPED Reserved);
	BOOL (__stdcall *WSAConnectByNameA)(SOCKET s, LPSTR nodename, LPSTR servicename, LPDWORD LocalAddressLength, LPSOCKADDR LocalAddress, LPDWORD RemoteAddressLength, LPSOCKADDR RemoteAddress, const struct timeval *timeout, LPWSAOVERLAPPED Reserved);
	BOOL (__stdcall *WSAConnectByNameW)(SOCKET s, LPWSTR nodename, LPWSTR servicename, LPDWORD LocalAddressLength, LPSOCKADDR LocalAddress, LPDWORD RemoteAddressLength, LPSOCKADDR RemoteAddress, const struct timeval *timeout, LPWSAOVERLAPPED Reserved);
	WSAEVENT (__stdcall *WSACreateEvent)(void);
	int (__stdcall *WSADuplicateSocketA)(SOCKET s, DWORD dwProcessId, LPWSAPROTOCOL_INFOA lpProtocolInfo);
	int (__stdcall *WSADuplicateSocketW)(SOCKET s, DWORD dwProcessId, LPWSAPROTOCOL_INFOW lpProtocolInfo);
	int (__stdcall *WSAEnumNameSpaceProvidersA)(LPDWORD lpdwBufferLength, LPWSANAMESPACE_INFOA lpnspBuffer);
	int (__stdcall *WSAEnumNameSpaceProvidersW)(LPDWORD lpdwBufferLength, LPWSANAMESPACE_INFOW lpnspBuffer);
	int (__stdcall *WSAEnumNameSpaceProvidersExA)(LPDWORD lpdwBufferLength, LPWSANAMESPACE_INFOEXA lpnspBuffer);
	int (__stdcall *WSAEnumNameSpaceProvidersExW)(LPDWORD lpdwBufferLength, LPWSANAMESPACE_INFOEXW lpnspBuffer);
	int (__stdcall *WSAEnumNetworkEvents)(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents);
	int (__stdcall *WSAEnumProtocolsA)(LPINT lpiProtocols, LPWSAPROTOCOL_INFOA lpProtocolBuffer, LPDWORD lpdwBufferLength);
	int (__stdcall *WSAEnumProtocolsW)(LPINT lpiProtocols, LPWSAPROTOCOL_INFOW lpProtocolBuffer, LPDWORD lpdwBufferLength);
	int (__stdcall *WSAEventSelect)(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents);
	int (__stdcall *__WSAFDIsSet)(SOCKET fd, fd_set *set);
	int (__stdcall *WSAGetLastError)(void);
	BOOL (__stdcall *WSAGetOverlappedResult)(SOCKET s, LPWSAOVERLAPPED lpOverlapped, LPDWORD lpcbTransfer, BOOL fWait, LPDWORD lpdwFlags);
	BOOL (__stdcall *WSAGetQOSByName)(SOCKET s, LPWSABUF lpQOSName, LPQOS lpQOS);
	int (__stdcall *WSAGetServiceClassInfoA)(LPGUID lpProviderId, LPGUID lpServiceClassId, LPDWORD lpdwBufferLength, LPWSASERVICECLASSINFOA lpServiceClassInfo);
	int (__stdcall *WSAGetServiceClassInfoW)(LPGUID lpProviderId, LPGUID lpServiceClassId, LPDWORD lpdwBufferLength, LPWSASERVICECLASSINFOW lpServiceClassInfo);
	int (__stdcall *WSAGetServiceClassNameByClassIdA)(LPGUID lpServiceClassId, LPSTR lpszServiceClassName, LPDWORD lpdwBufferLength);
	int (__stdcall *WSAGetServiceClassNameByClassIdW)(LPGUID lpServiceClassId, LPWSTR lpszServiceClassName, LPDWORD lpdwBufferLength);
	int (__stdcall *WSAHtonl)(SOCKET s, u_long hostlong, u_long *lpnetlong);
	int (__stdcall *WSAHtons)(SOCKET s, u_short hostshort, u_short *lpnetshort);
	int (__stdcall *WSAInstallServiceClassA)(LPWSASERVICECLASSINFOA lpServiceClassInfo);
	int (__stdcall *WSAInstallServiceClassW)(LPWSASERVICECLASSINFOW lpServiceClassInfo);
	int (__stdcall *WSAIoctl)(SOCKET s, DWORD dwIoControlCode, LPVOID lpvInBuffer, DWORD cbInBuffer, LPVOID lpvOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	SOCKET (__stdcall *WSAJoinLeaf)(SOCKET s, const struct sockaddr *name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, DWORD dwFlags);
	int (__stdcall *WSALookupServiceBeginA)(LPWSAQUERYSETA lpqsRestrictions, DWORD dwControlFlags, LPHANDLE lphLookup);
	int (__stdcall *WSALookupServiceBeginW)(LPWSAQUERYSETW lpqsRestrictions, DWORD dwControlFlags, LPHANDLE lphLookup);
	int (__stdcall *WSALookupServiceEnd)(HANDLE hLookup);
	int (__stdcall *WSALookupServiceNextA)(HANDLE hLookup, DWORD dwControlFlags, LPDWORD lpdwBufferLength, LPWSAQUERYSETA lpqsResults);
	int (__stdcall *WSALookupServiceNextW)(HANDLE hLookup, DWORD dwControlFlags, LPDWORD lpdwBufferLength, LPWSAQUERYSETW lpqsResults);
	int (__stdcall *WSANSPIoctl)(HANDLE hLookup, DWORD dwControlCode, LPVOID lpvInBuffer, DWORD cbInBuffer, LPVOID lpvOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbBytesReturned, LPWSACOMPLETION lpCompletion);
	int (__stdcall *WSANtohl)(SOCKET s, u_long netlong, u_long *lphostlong);
	int (__stdcall *WSANtohs)(SOCKET s, u_short netshort, u_short *lphostshort);
	int (__stdcall *WSAPoll)(WSAPOLLFD fdarray[], ULONG nfds, INT timeout);
	int (__stdcall *WSAProviderConfigChange)(LPHANDLE lpNotificationHandle, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	int (__stdcall *WSARecv)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	int (__stdcall *WSARecvDisconnect)(SOCKET s, LPWSABUF lpInboundDisconnectData);
	int (__stdcall *WSARecvFrom)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, struct sockaddr *lpFrom, LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	int (__stdcall *WSARemoveServiceClass)(LPGUID lpServiceClassId);
	BOOL (__stdcall *WSAResetEvent)(WSAEVENT hEvent);
	int (__stdcall *WSASend)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	int (__stdcall *WSASendDisconnect)(SOCKET s, LPWSABUF lpOutboundDisconnectData);
	int (__stdcall *WSASendMsg)(SOCKET s, LPWSAMSG lpMsg, DWORD dwFlags, LPDWORD lpNumberOfBytesSent, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	int (__stdcall *WSASendTo)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const struct sockaddr *lpTo, int iToLen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	BOOL (__stdcall *WSASetEvent)(WSAEVENT hEvent);
	void (__stdcall *WSASetLastError)(int iError);
	int (__stdcall *WSASetServiceA)(LPWSAQUERYSETA lpqsRegInfo, WSAESETSERVICEOP essOperation, DWORD dwControlFlags);
	int (__stdcall *WSASetServiceW)(LPWSAQUERYSETW lpqsRegInfo, WSAESETSERVICEOP essOperation, DWORD dwControlFlags);
	int (__stdcall *WSAStartup)(WORD wVersionRequested, LPWSADATA lpWSAData);
	int (__stdcall *WSAStringToAddressA)(LPSTR AddressString, INT AddressFamily, LPWSAPROTOCOL_INFOA lpProtocolInfo, LPSOCKADDR lpAddress, LPINT lpAddressLength);
	int (__stdcall *WSAStringToAddressW)(LPWSTR AddressString, INT AddressFamily, LPWSAPROTOCOL_INFOW lpProtocolInfo, LPSOCKADDR lpAddress, LPINT lpAddressLength);
	DWORD (__stdcall *WSAWaitForMultipleEvents)(DWORD cEvents, const WSAEVENT *lphEvents, BOOL fWaitAll, DWORD dwTimeout, BOOL fAlertable);
	SOCKET (__stdcall *WSASocketA)(int af, int type, int protocol, LPWSAPROTOCOL_INFOA lpProtocolInfo, GROUP g, DWORD dwFlags);
	SOCKET (__stdcall *WSASocketW)(int af, int type, int protocol, LPWSAPROTOCOL_INFOW lpProtocolInfo, GROUP g, DWORD dwFlags);
	int (__stdcall *GetAddrInfoExA)(PCSTR pName, PCSTR pServiceName, DWORD dwNameSpace, LPGUID lpNspId, const ADDRINFOEXA *pHints, PADDRINFOEXA *ppResult, struct timeval *timeout, LPOVERLAPPED lpOverlapped, LPLOOKUPSERVICE_COMPLETION_ROUTINE lpCompletionRoutine, LPHANDLE lpNameHandle);
	int (__stdcall *GetAddrInfoExW)(PCWSTR pName, PCWSTR pServiceName, DWORD dwNameSpace, LPGUID lpNspId, const ADDRINFOEXW *pHints, PADDRINFOEXW *ppResult, struct timeval *timeout, LPOVERLAPPED lpOverlapped, LPLOOKUPSERVICE_COMPLETION_ROUTINE lpCompletionRoutine, LPHANDLE lpNameHandle);
	int (__stdcall *SetAddrInfoExA)(PCSTR pName, PCSTR pServiceName, SOCKET_ADDRESS *pAddresses, DWORD dwAddressCount, LPBLOB lpBlob, DWORD dwFlags, DWORD dwNameSpace, LPGUID lpNspId, struct timeval *timeout, LPOVERLAPPED lpOverlapped, LPLOOKUPSERVICE_COMPLETION_ROUTINE lpCompletionRoutine, LPHANDLE lpNameHandle);
	int (__stdcall *SetAddrInfoExW)(PCWSTR pName, PCWSTR pServiceName, SOCKET_ADDRESS *pAddresses, DWORD dwAddressCount, LPBLOB lpBlob, DWORD dwFlags, DWORD dwNameSpace, LPGUID lpNspId, struct timeval *timeout, LPOVERLAPPED lpOverlapped, LPLOOKUPSERVICE_COMPLETION_ROUTINE lpCompletionRoutine, LPHANDLE lpNameHandle);
	PCSTR (__stdcall *InetNtopA)(INT Family, PVOID pAddr, PSTR pStringBuf, size_t StringBufSize);
	PCWSTR (__stdcall *InetNtopW)(INT Family, PVOID pAddr, PWSTR pStringBuf, size_t StringBufSize);

	// Mswsock API
	int (__stdcall *GetAddressByName)(DWORD dwNameSpace, LPGUID lpServiceType, LPTSTR lpServiceName, LPINT lpiProtocols, DWORD dwResolution, LPSERVICE_ASYNC_INFO lpServiceAsyncInfo, LPVOID lpCsaddrBuffer, LPDWORD lpdwBufferLength, LPTSTR lpAliasBuffer, LPDWORD lpdwAliasBufferLength);
	int (__stdcall *GetNameByTypeA)(LPGUID lpServiceType, LPSTR lpServiceName, DWORD dwNameLength);
	int (__stdcall *GetNameByTypeW)(LPGUID lpServiceType, LPWSTR lpServiceName, DWORD dwNameLength);
	int (__stdcall *GetTypeByNameA)(LPSTR lpServiceName, LPGUID lpServiceType);
	int (__stdcall *GetTypeByNameW)(LPWSTR lpServiceName, LPGUID lpServiceType);
	int (__stdcall *WSARecvEx)(SOCKET s, char *buf, int len, int *flags);
	BOOL (__stdcall *TransmitFile)(SOCKET hSocket, HANDLE hFile, DWORD nNumberOfBytesToWrite, DWORD nNumberOfBytesPerSend, LPOVERLAPPED lpOverlapped, LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers, DWORD dwFlags);

	// Fwpucint API (not yet implemented)
	//int (__stdcall *WSADeleteSocketPeerTargetName)(SOCKET Socket, const struct sockaddr *PeerAddr, ULONG PeerAddrLen, LPWSAOVERLAPPED Overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine);
	//int (__stdcall *WSAImpersonateSocketPeer)(SOCKET Socket, const sockaddr PeerAddress, ULONG peerAddressLen);
	//int (__stdcall *WSARevertImpersonation)(void);
	//int (__stdcall *WSAQuerySocketSecurity)(SOCKET Socket, const SOCKET_SECURITY_QUERY_TEMPLATE *SecurityQueryTemplate, ULONG SecurityQueryTemplateLen,  SOCKET_SECURITY_QUERY_INFO *SecurityQueryInfo, ULONG *SecurityQueryInfoLen, LPWSAOVERLAPPED Overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine);
	//int (__stdcall *WSASetSocketPeerTargetName)(SOCKET Socket, const SOCKET_PEER_TARGET_NAME *PeerTargetName, ULONG PeerTargetNameLen, LPWSAOVERLAPPED Overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine);
	//int (__stdcall *WSASetSocketSecurity)(SOCKET Socket, const SOCKET_SECURITY_SETTINGS *SecuritySettings, ULONG SecuritySettingsLen, LPWSAOVERLAPPED Overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine);

	// WSAIoctl API
	// @oooii-tony: What a bizzarre API - why do I need a socket to get a function
	// pointer. I guess that means it's only safe to call on the socket specified?
	// If these fail, use WSAGetLastError() to determine error.

	bool GetFunctionPointer_ConnectEx(SOCKET s, LPFN_CONNECTEX* ppConnectEx);
	bool GetFunctionPointer_DisconnectEx(SOCKET s, LPFN_DISCONNECTEX* ppDisconnectEx);
	bool GetFunctionPointer_GetAcceptExSockaddrs(SOCKET s, LPFN_GETACCEPTEXSOCKADDRS* ppGetAcceptExSockaddrs);
	bool GetFunctionPointer_TransmitPackets(SOCKET s, LPFN_TRANSMITPACKETS* ppTransmitPackets);
	bool GetFunctionPointer_WSARecvMsg(SOCKET s, LPFN_WSARECVMSG* ppWSARecvMsg);
	bool GetFunctionPointer_AcceptEx(SOCKET s, LPFN_ACCEPTEX* ppAcceptEx);
};

// Hostname is an address or name with a port (i.e. localhost:123 or 127.0.0.1:123)
// For INADDR_ANY, use 0.0.0.0 as the IP and any appropriate port.
void oWinsockCreateAddr(sockaddr_in* _pOutSockAddr, const char* _Hostname);

void oWinsockAddrToHostname(sockaddr_in* _pSockAddr, char* _OutHostname, size_t _SizeOfHostname);

// Looks at a WSANETWORKEVENTS struct and if this returns false a summary error
// is in oErrorGetLast(). Also traces out detailed information in debug. This is 
// mostly a convenience function.
bool oWinsockTraceEvents(const char* _TracePrefix, const char* _TraceName, const WSANETWORKEVENTS* _pNetworkEvents);

// Create a WinSock socket that can support overlapped operations. MSDN docs 
// seem to indicate it's a good idea for "most sockets" to enable overlapped ops
// by default, so that's what we'll do.
// _Hostname: IP/DNS name colon the port number (127.0.0.1:123 or myhostname:321)
// WINSOCK_RELIABLE: if true this uses TCP, else UDP
// WINSOCK_ALLOW_BROADCAST: Enable broadcast for UDP (ignored if _Reliable is true)
// WINSOCK_REUSE_ADDRESS: allows other processes to share the address 
// oWINSOCK_EXCLUSIVE_ADDRESS: ensures this socket has exclusive control of the specified address
// _MaxNumConnections: if 0, the socket will be created and connect will be called. 
//                     If non-zero the socket will be bind()'ed and put into a listen 
//                     state as a server
// _SendBufferSize: socket is configured with this buffer for sending. If 0, the 
//                  default is used.
// _ReceiveBufferSize: socket is configured with this buffer for receiving. If 0,
//                     the default is used.
// _hNonBlockingEvent: If non-zero and a valid WSAEVENT, the event will be registered
//                     to fire on any FD_* event. Use oWinSockWait() to wait on the event.

enum oWINSOCK_OPTIONS
{
	oWINSOCK_RELIABLE = 0x1,
	oWINSOCK_ALLOW_BROADCAST = 0x2,
	oWINSOCK_REUSE_ADDRESS = 0x4,
	oWINSOCK_EXCLUSIVE_ADDRESS = 0x8,
	oWINSOCK_BLOCKING = 0x10,
};

enum oWINSOCK_ASYNC_RESULT //some async functions have the option (decided by windows) of completing synchronously. There is no iocp callback when this happens.
{
	oWINSOCK_COMPLETED, //means the call has completed synchronously. there will be no iocp callback, handle appropriatly.
	oWINSOCK_COMPLETED_ASYNC,
	oWINSOCK_FAILED,
};

//specify oInvalid for MaxNumConnections to create a listen socket, but let the driver decide on the max number of connections.
SOCKET oWinsockCreate(const sockaddr_in _Addr, int _ORedWinsockOptions, unsigned int _TimeoutMS = 0, unsigned int _MaxNumConnections = 0);
SOCKET oWinsockCreate(const sockaddr_in6 _Addr, int _ORedWinsockOptions, unsigned int _TimeoutMS = 0, unsigned int _MaxNumConnections = 0);

// For when creating a Winsock with automatic port selection, get the selected port
bool oWinsockGetPort(SOCKET _hSocket, unsigned short* _pPort);

//For use with AcceptEx (IOCP). oWinsockCreateForAsyncAccept creates an unconnected socket, ready to be sent to AcceptEx. It is not usable at this point.
//	Once iocp accepts a connection with this socket, you must call oWinsockCompleteAsyncAccept to finish prepping the socket for use.
SOCKET oWinsockCreateForAsyncAccept();
static const int oWINSOCK_ACCEPT_BUFFER_SIZE = 2*sizeof(SOCKADDR_IN)+32; //From msdn docs, buffer needs 16 bytes padding for each address. buffer will hold 2
// _OutputBuffer must be at least oWINSOCK_ACCEPT_BUFFER_SIZE in size.
oWINSOCK_ASYNC_RESULT oWinsockAsyncAccept(SOCKET _ListenSocket, SOCKET _AcceptSocket, void* _OutputBuffer, WSAOVERLAPPED* _Overlapped);
bool oWinsockCompleteAsyncAccept(SOCKET _hListenSocket, SOCKET _hAcceptSocket);
//This will disconnect the socket but not destroy it. It will be in the same state as a socket returned from oWinsockCreateForAsyncAccept. use to avoid time
//	to delete and recreate a socket. May or may not complete asynchronously. you won't get an iocp callback if this completes synchronously.
oWINSOCK_ASYNC_RESULT oWinsockAsyncAcceptPrepForReuse(SOCKET _hListenSocket, SOCKET _hAcceptSocket, WSAOVERLAPPED* _Overlapped);
// This function parses the _OutputBuffer obtained from a call to the oWinsockAsyncAccept function and passes the local and remote addresses to a sockaddr structure.
oWINSOCK_ASYNC_RESULT oWinSockAsyncAcceptExSockAddrs(SOCKET _ListenSocket, void* _Buffer, LPSOCKADDR* _LocalAddr, LPINT _SzLocalAddr, LPSOCKADDR* _RemoteAddr, LPINT _SzRemoteAddr);

// Thoroughly closes a socket according to best-practices described on the web.
// If this returns false, check oErrorGetLast() for more information.
bool oWinsockClose(SOCKET _hSocket);

// Set the keep alive time and interval (in milliseconds) default time is 2 hours and 1 second interval
bool oWinsockSetKeepAlive(SOCKET _hSocket, unsigned int _TimeoutMS = 0x6DDD00, unsigned int _IntervalMS = 0x3E8);

// This wrapper on WinSocks specialized event/wait system to make it look like
// the above oWait*'s
bool oWinsockWaitMultiple(WSAEVENT* _pHandles, size_t _NumberOfHandles, bool _WaitAll, bool _Alertable, unsigned int _TimeoutMS = oInfiniteWait);

// If the socket was created using oSocketCreate (WSAEventSelect()), this function can 
// be used to wait on that event and receive any events breaking the wait. This 
// function handles "spurious waits", so if using WSANETWORKEVENTS structs, use
// this wrapper always.
bool oWinsockWait(SOCKET _hSocket, WSAEVENT _hEvent, WSANETWORKEVENTS* _pNetEvents, unsigned int _TimeoutMS = oInfiniteWait);

// Returns true if all data was sent. If false, use oErrorGetLast() for more details.
bool oWinsockSend(SOCKET _hSocket, const void* _pSource, size_t _SizeofSource, const sockaddr_in* _pDestination);

// Returns number of bytes read. This may include more than one Send()'s worth
// of data due to Nagel's Algorithm. If size is 0, use oErrorGetLast() for more 
// details. It can be that this function returns 0 and the error status is 
// ESHUTDOWN, meaning a valid and error-free closing of the peer socket has 
// occurred and no further steps should occur.
// _pInOutCanReceive is the address of an integer used as a boolean (atomics 
// used to change its state) that is 0 if Receive should not wait because of a 
// bad socket state, or non-zero if the receive should block on FD_READ events.
size_t oWinsockReceive(SOCKET _hSocket, WSAEVENT _hEvent, void* _pDestination, size_t _SizeofDestination, unsigned int _TimeoutMS, int* _pInOutCanReceive, sockaddr_in* _pSource);

// Returns true on success whether data was received or not. If return is false,
// use oErrorGetLast() for more details. It can be that this function returns
// false and the error status is ESHUTDOWN, meaning a valid and error-free
// closing of the peer socket has occurred and no further steps should occur.
bool oWinsockReceiveNonBlocking(SOCKET _hSocket, WSAEVENT _hEvent, void* _pDestination, size_t _SizeofDestination, sockaddr_in* _pSource, size_t* _pBytesReceived);

// Fills the specified buffers with data from the specified socket. Null can be 
// specified for any one of these to opt out of getting a particular part of the
// data. Hostname is THIS part of the socket, the local port of connection. Peer
// name is the FOREIGN PART of the connection, meaning the ip/hostname and 
// foreign port of the connection.
bool oWinsockGetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort, SOCKET _hSocket);
template<size_t hostnameSize, size_t ipSize, size_t portSize> inline bool oWinsockGetHostname(char (&_OutHostname)[hostnameSize], char (&_OutIPAddress)[ipSize], char (&_OutPort)[portSize], SOCKET _hSocket) { return oWinsockGetHostname(_OutHostname, hostnameSize, _OutIPAddress, ipSize, _OutPort, portSize, _hSocket); }

bool oWinsockGetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort, SOCKET _hSocket);
template<size_t hostnameSize, size_t ipSize, size_t portSize> inline bool oWinsockGetPeername(char (&_OutHostname)[hostnameSize], char (&_OutIPAddress)[ipSize], char (&_OutPort)[portSize], SOCKET _hSocket) { return oWinsockGetPeername(_OutHostname, hostnameSize, _OutIPAddress, ipSize, _OutPort, portSize, _hSocket); }

// Use recommended practices from the web to determine if a socket is still 
// connected to a peer.
bool oWinsockIsConnected(SOCKET _hSocket);

// Enumerates the addresses of all attached interfaces
void oWinsockEnumerateAllAddress(oFUNCTION<void(sockaddr_in _Addr)> _Enumerator);

#endif
