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
#pragma once
#ifndef oWinsock_h
#define oWinsock_h

#include <oooii/oWindows.h>
#include <oooii/oSingleton.h>
#include <oooii/oStdio.h>
#include <Winsock.h>
#include <winsock2.h>
#include <Mstcpip.h>
#include <Mswsock.h>
#include <nspapi.h>
#include <Rpc.h>
#include <Ws2tcpip.h>

// Convenient assert for always printing out winsock-specific errors.
#define oWINSOCK_ASSERT(x, msg, ...) oASSERT(x, msg "\n%s(%d): %s", ## __VA_ARGS__ oWinsock::AsString(oWinsock::Singleton()->WSAGetLastError()), oWinsock::Singleton()->WSAGetLastError(), oWinsock::GetErrorDesc(oWinsock::Singleton()->WSAGetLastError()))

// Call oSetLastError() with errno_t equivalent, but also include WSA error and extended desc in description
#define oWINSOCK_SETLASTERROR(strFnName) oSetLastError(oWinsock::GetErrno(oWinsock::Singleton()->WSAGetLastError()), "oWinsock::" strFnName " failed %s(%d): %s", oWinsock::AsString(oWinsock::Singleton()->WSAGetLastError()), oWinsock::Singleton()->WSAGetLastError(), oWinsock::GetErrorDesc(oWinsock::Singleton()->WSAGetLastError()))

struct oWinsock : public oSingleton<oWinsock>
{
	// NOTE: gai_strerror is an inline function, so does not need to be linked
	// against or called from this oWinSock interface.

	oWinsock();
	~oWinsock();
	
	// Enum as string (similar to oAsString())
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
	BOOL (__stdcall *AcceptEx)(SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped);
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

protected:
	oHDLL hMswsock;
	oHDLL hWs2_32;
	oHDLL hFwpucint;
};

#endif
