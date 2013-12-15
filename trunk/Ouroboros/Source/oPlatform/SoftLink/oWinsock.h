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
#ifndef oCore_win_socket_h
#define oCore_win_socket_h

#include <oPlatform/oSingleton.h>
#include <oCore/module.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <Mstcpip.h>
#include <Mswsock.h>
#include <nspapi.h>
#include <Rpc.h>
#include <Ws2tcpip.h>

// Convenient assert for always printing out winsock-specific errors.
#define oWINSOCK_ASSERT(x, msg, ...) oASSERT(x, msg "\n%s(%d): %s", ## __VA_ARGS__ oWinsockAsString(WSAGetLastError()), WSAGetLastError(), oWinsockGetErrorDesc(WSAGetLastError()))

// Call oErrorSetLast() with errno_t equivalent, but also include WSA error and extended desc in description
#define oWINSOCK_SETLASTERROR(strFnName) oErrorSetLast(std::errc::io_error, "oWinsock::" strFnName " failed %s(%d): %s", oWinsockAsString(WSAGetLastError()), WSAGetLastError(), oWinsockGetErrorDesc(WSAGetLastError()))

// Enum as string (similar to ouro::as_string())
const char* oWinsockAsString(int _WSAWinSockError);

// Returns the extended MSDN error description of the error
const char* oWinsockGetErrorDesc(int _WSAWinSockError);
errno_t oWinsockGetErrno(int _WSAWinSockError);

bool oWinsockGetFunctionPointer_ConnectEx(SOCKET s, LPFN_CONNECTEX* ppConnectEx);
bool oWinsockGetFunctionPointer_DisconnectEx(SOCKET s, LPFN_DISCONNECTEX* ppDisconnectEx);
bool oWinsockGetFunctionPointer_GetAcceptExSockaddrs(SOCKET s, LPFN_GETACCEPTEXSOCKADDRS* ppGetAcceptExSockaddrs);
bool oWinsockGetFunctionPointer_TransmitPackets(SOCKET s, LPFN_TRANSMITPACKETS* ppTransmitPackets);
bool oWinsockGetFunctionPointer_WSARecvMsg(SOCKET s, LPFN_WSARECVMSG* ppWSARecvMsg);
bool oWinsockGetFunctionPointer_AcceptEx(SOCKET s, LPFN_ACCEPTEX* ppAcceptEx);

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
//                     to fire on any FD_* event. Use oWinsockWait() to wait on the event.

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
oWINSOCK_ASYNC_RESULT oWinsockAsyncAcceptExSockAddrs(SOCKET _ListenSocket, void* _Buffer, LPSOCKADDR* _LocalAddr, LPINT _SzLocalAddr, LPSOCKADDR* _RemoteAddr, LPINT _SzRemoteAddr);

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
