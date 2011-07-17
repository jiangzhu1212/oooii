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

// This should be included instead of, or at least before <windows.h> to avoid
// much of the conflict-causing overhead as well as include some useful 
#pragma once
#ifndef oWindows_h
#define oWindows_h
#if defined(_WIN32) || defined(_WIN64)

#ifdef _WINDOWS_
	#pragma message("BAD WINDOWS INCLUDE! Applications should #include <oWindows.h> to prevent extra and sometimes conflicting cruft from being included.")
#endif

#include <oooii/oStddef.h>
#include <oooii/oSurface.h>

// _____________________________________________________________________________
// Simplify the contents of Windows.h

#ifndef NOMINMAX
	#define NOMINMAX
#endif

#define NOATOM
#define NOCOMM
#define NOCRYPT
#define NODEFERWINDOWPOS
//#define NODRAWTEXT // used in oWindow.h
#define NOGDICAPMASKS
#define NOHELP
#define NOIMAGE
#define NOKANJI
#define NOKERNEL
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOOPENFILE
#define NOPROFILER
#define NOPROXYSTUB
//#define NORASTEROPS // used in oWindow.h
#define NORPC
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTAPE
#define WIN32_LEAN_AND_MEAN

#ifdef interface
	#undef interface
	#undef INTERFACE_DEFINED
#endif
#undef _M_CEE_PURE

// Including intrin is necessary for interlocked functions, but it causes
// a warning to spew from math. So squelch that warning since it seems the
// declarations are indeed the same.
#include <intrin.h>
#pragma warning(disable:4985) // 'ceil': attributes not present on previous declaration.
#include <math.h>
#pragma warning (default:4985)

// @oooii-tony: How can I define this in such a way that MSVC is just aware of 
// what OS it's running on and sets this appropriately in the build system?
// I want to maintain compatibility with Vista/XP because we're still 
// occasionally use HW that has drivers only for XP/Vista.
#if (_MSC_VER < 1600)
	#define oFORCE_WIN7_VERSION
#endif

#ifdef oFORCE_WIN7_VERSION
	#define WINVER 0x0601
	#define NTDDI_WIN7 WINVER
	#define _WIN32_WINNT_WIN7 WINVER
#endif

#include <windows.h>
#include <winsock2.h>

#define oDXVER_9a 0x0900
#define oDXVER_9b 0x0901
#define oDXVER_9c 0x0902
#define oDXVER_10 0x1000
#define oDXVER_10_1 0x1001
#define oDXVER_11 0x1100
#define oDXVER_11_1 0x1101
#define oDXVER_12 0x1200
#define oDXVER_12_1 0x1201
#define oDXVER_13 0x1300
#define oDXVER_13_1 0x1301

// Define something like WINVER, but for DX functionality
#if (defined(NTDDI_WIN7) && (NTDDI_VERSION >= NTDDI_WIN7))
	#define oDXVER oDXVER_11
#elif (defined(NTDDI_LONGHORN) && (NTDDI_VERSION >= NTDDI_LONGHORN)) || (defined(NTDDI_VISTA) && (NTDDI_VERSION >= NTDDI_VISTA))
	#define oDXVER oDXVER_10_1
#else
	#define oDXVER oDXVER_9c
#endif

// mostly for debugging...
#ifdef oMESSAGE_WINDOWS_VERSION
	#if (defined(NTDDI_WIN7) && (NTDDI_VERSION >= NTDDI_WIN7))
		#pragma message("PLATFORM: Windows7")
	#elif (defined(NTDDI_VISTA) && (NTDDI_VERSION >= NTDDI_VISTA))
		#pragma message("PLATFORM: Windows Vista")
	#elif (defined(NTDDI_LONGHORN) && (NTDDI_VERSION >= NTDDI_LONGHORN))
		#pragma message("PLATFORM: Longhorn")
	#elif (defined(NTDDI_WINXP) && (NTDDI_VERSION >= NTDDI_WINXP))
		#pragma message("PLATFORM: XP")
	#endif

	#if (_MSC_VER >= 1600)
		#pragma message("COMPILER: with Visual Studio 2010")
	#elif (_MSC_VER >= 1500)
		#pragma message("COMPILER: Visual Studio 2008")
	#endif
#endif

#include <shlobj.h>

#if oDXVER >= oDXVER_10
	#include <dxerr.h>
	#include <d3d11.h>
	#include <d3dx11.h>
	#include <d3dcompiler.h>
	#include <dxgi.h>
	#include <d2d1.h>
	#include <dwrite.h>
#endif

#ifdef interface
	#define INTERFACE_DEFINED
#endif

// _____________________________________________________________________________
// Error check wrappers. These interpret HRESULTS, so should not be used on 
// anything but WIN32 API. oVB is for the return FALSE then GetLastError() 
// pattern, and oV is for direct HRESULT return values.

#ifdef _DEBUG
	#define oVB(fn) do { if (!(fn)) { oWinSetLastError(::GetLastError()); oASSERT_PRINT_MESSAGE(TYPE_ASSERT, oAssert::IGNORE_ONCE, fn, "%s", oGetLastErrorDesc()); } } while(0)
	#define oV(fn) do { HRESULT HR__ = fn; if (FAILED(HR__)) { oWinSetLastError(HR__); oASSERT_PRINT_MESSAGE(TYPE_ASSERT, oAssert::IGNORE_ONCE, fn, "%s", oGetLastErrorDesc()); } } while(0)
#else
	#define oVB(fn) fn
	#define oV(fn) fn
#endif

// _____________________________________________________________________________
// Wrappers for the Windows-specific crtdbg API. Prefer oASSERT macros found
// in oAssert.h, but for a few systems that are lower-level than oAssert, these
// may be necessary.

#ifdef _DEBUG
	#include <crtdbg.h>

	#define oCRTASSERT(expr, msg, ...) if (!(expr)) { _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, "OOOii Debug Library", #expr "\n\n%s", msg, ## __VA_ARGS__); }
	#define oCRTWARNING(msg, ...) do { _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, "OOOii Debug Library", "WARNING: " msg, ## __VA_ARGS__); } while(0)
	#define oCRTTRACE(msg, ...) do { _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, "OOOii Debug Library", msg, ## __VA_ARGS__); } while(0)

	// Convenience wrapper for quick scoped leak checking
	class oLeakCheck
	{
		const char* Name;
		_CrtMemState StartState;
	public:
		oLeakCheck(const char* _ConstantName = "") : Name(_ConstantName ? _ConstantName : "(unnamed)") { _CrtMemCheckpoint(&StartState); }
		~oLeakCheck()
		{
			_CrtMemState endState, stateDiff;
			_CrtMemCheckpoint(&endState);
			_CrtMemDifference(&stateDiff, &StartState, &endState);
			oCRTTRACE("---- Mem diff for %s ----", Name);
			_CrtMemDumpStatistics(&stateDiff);
		}
	};
#else
	#define oCRTASSERT(expr, msg, ...) __noop
	#define oCRTWARNING(msg, ...) __noop
	#define oCRTTRACE(msg, ...) __noop
#endif

// _____________________________________________________________________________
// Declare functions that are in Windows DLLs, but not exposed in headers

// Secret function that is not normally exposed in headers.
// Typically pass 0 for wLanguageId, and specify a timeout
// for the dialog in milliseconds, returns MB_TIMEDOUT if
// the timeout is reached.
int MessageBoxTimeoutA(IN HWND hWnd, IN LPCSTR lpText, IN LPCSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);
int MessageBoxTimeoutW(IN HWND hWnd, IN LPCWSTR lpText, IN LPCWSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);

#ifdef UNICODE
	#define MessageBoxTimeout MessageBoxTimeoutW
#else
	#define MessageBoxTimeout MessageBoxTimeoutA
#endif 

#define MB_TIMEDOUT 32000

#define oWINDOWS_DEFAULT 0x80000000

// Given the specified HRESULT, set both the closest errno value and the 
// platform-specific description associated with the error code.
// if oWINDOWS_DEFAULT is specified, ::GetLastError() is used
bool oWinSetLastError(HRESULT _hResult = oWINDOWS_DEFAULT, const char* _ErrorDescPrefix = 0);

// oV_RETURN executes a block of code that returns an HRESULT
// if the HRESULT is not S_OK it returns the HRESULT
#define oV_RETURN(fn) do { HRESULT HR__ = fn; if (FAILED(HR__)) return HR__; } while(0)

// oVB_RETURN executes a block of Windows API code that returns bool and populates
// oGetLastError() with ::GetLastError() and returns false.
#define oVB_RETURN(fn) do { if (!(fn)) { oWinSetLastError(oWINDOWS_DEFAULT, #fn " failed: "); return false; } } while(0)

// _____________________________________________________________________________
// Smart pointer support

inline ULONG oGetRefCount(IUnknown* unk) { ULONG r = unk->AddRef()-1; unk->Release(); return r; }
inline void intrusive_ptr_add_ref(IUnknown* unk) { unk->AddRef(); }
inline void intrusive_ptr_release(IUnknown* unk) { unk->Release(); }

// _____________________________________________________________________________
// Time conversion

static const FILETIME oFTPreserve = { oINVALID, oINVALID };
void oUnixTimeToFileTime(time_t _Time, FILETIME* _pFileTime);
time_t oFileTimeToUnixTime(const FILETIME* _pFileTime);

void oUnixTimeToSystemTime(time_t _Time, SYSTEMTIME* _pSystemTime);
time_t oSystemTimeToUnixTime(const SYSTEMTIME* _pSystemTime);

// Uses a waitable timer to call the specified function at the specified time.
// If _Alertable is true, this can wake up a sleeping system.
bool oScheduleFunction(const char* _DebugName, time_t _AbsoluteTime, bool _Alertable, oFUNCTION<void()> _Function);

// _____________________________________________________________________________
// Concurrency

// So many problems would be solved if this were threadsafe! Be careful, using 
// this in concurrent situations can cause undesired synchronization because of
// the mutex used to protect messages from being stomped on. NOTE: The mutex is
// process-wide, no matter what DLLs or how this code was linked.
void oThreadsafeOutputDebugStringA(const char* _OutputString);

// returns true if wait finished successfully, or false if
// timed out or otherwise errored out.
inline bool oWaitSingle(HANDLE _Handle, unsigned int _TimeoutMS = oINFINITE_WAIT) { return WAIT_OBJECT_0 == ::WaitForSingleObject(_Handle, _TimeoutMS == oINVALID ? INFINITE : _TimeoutMS); }
inline bool oWaitMultiple(HANDLE* _pHandles, size_t _NumberOfHandles, bool _WaitAll, unsigned int _TimeoutMS = oINFINITE_WAIT) { return WAIT_ABANDONED_0 > ::WaitForMultipleObjects(static_cast<DWORD>(_NumberOfHandles), _pHandles, _WaitAll, _TimeoutMS == ~0u ? INFINITE : _TimeoutMS); }

bool oWaitSingle(DWORD _ThreadID, unsigned int _Timeout = oINFINITE_WAIT);
bool oWaitMultiple(DWORD* _pThreadIDs, size_t _NumberOfThreadIDs, bool _WaitAll, unsigned int _TimeoutMS = oINFINITE_WAIT);

// _____________________________________________________________________________
// Winsock

// Hostname is an address or name with a port (i.e. localhost:123 or 127.0.0.1:123)
// For INADDR_ANY, use 0.0.0.0 as the IP and any appropriate port.
void oWinsockCreateAddr(sockaddr_in* _pOutSockAddr, const char* _Hostname);

void oWinsockAddrToHostname(sockaddr_in* _pSockAddr, char* _OutHostname, size_t _SizeOfHostname);

// Looks at a WSANETWORKEVENTS struct and returns a summary error. Also traces out
// detailed information in debug. This is mostly a convenience function.
int oWinsockTraceEvents(const char* _TracePrefix, const char* _TraceName, const WSANETWORKEVENTS* _pNetworkEvents);

// Create a WinSock socket that can support overlapped operations. MSDN docs 
// seem to indicate it's a good idea for "most sockets" to enable overlapped ops
// by default, so that's what we'll do.
// _Hostname: IP/DNS name colon the port number (127.0.0.1:123 or myhostname:321)
// WINSOCK_RELIABLE: if true this uses TCP, else UDP
// WINSOCK_ALLOW_BROADCAST: Enable broadcast for UDP (ignored if _Reliable is true)
// WINSOCK_REUSE_ADDRESS: if true allows the hostname to be reused
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
	oWINSOCK_BLOCKING = 0x8,
};

SOCKET oWinsockCreate(const char* _Hostname, int _ORedWinsockOptions, unsigned int _MaxNumConnections, size_t _SendBufferSize, size_t _ReceiveBufferSize);

// Thoroughly closes a socket according to best-practices described on the web.
// If this returns false, check oGetLastError() for more information.
bool oWinsockClose(SOCKET _hSocket);

// This wrapper on WinSocks specialized event/wait system to make it look like
// the above oWait*'s
bool oWinsockWaitMultiple(WSAEVENT* _pHandles, size_t _NumberOfHandles, bool _WaitAll, bool _Alertable, unsigned int _TimeoutMS = oINFINITE_WAIT);

// If the socket was created using oSocketCreate (WSAEventSelect()), this function can 
// be used to wait on that event and receive any events breaking the wait. This 
// function handles "spurious waits", so if using WSANETWORKEVENTS structs, use
// this wrapper always.
bool oWinsockWait(SOCKET _hSocket, WSAEVENT _hEvent, WSANETWORKEVENTS* _pNetEvents, unsigned int _TimeoutMS = oINFINITE_WAIT);

// Returns true if all data was sent. If false, use oGetLastError() for more details.
bool oWinsockSend(SOCKET _hSocket, const void* _pSource, size_t _SizeofSource, const sockaddr_in* _pDestination);

// Returns number of bytes read. This may include more than one Send()'s worth
// of data due to Nagel's Algorithm. If size is 0, use oGetLastError() for more 
// details. It can be that this function returns 0 and the error status is 
// ESHUTDOWN, meaning a valid and error-free closing of the peer socket has 
// occurred and no further steps should occur.
// _pInOutCanReceive is the address of an integer used as a boolean (atomics 
// used to change its state) that is 0 if Receive should not wait because of a 
// bad socket state, or non-zero if the receive should block on FD_READ events.
size_t oWinsockReceive(SOCKET _hSocket, WSAEVENT _hEvent, void* _pDestination, size_t _SizeofDestination, unsigned int _TimeoutMS, int* _pInOutCanReceive, sockaddr_in* _pSource);

// Returns true on success whether data was received or not. If return is false,
// use oGetLastError() for more details. It can be that this function returns
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

// _____________________________________________________________________________
// Cursor

HRESULT oClipCursorToClient(HWND _hWnd, bool _Clip);
bool oIsCursorVisible();
bool oIsCursorClipped(HWND _hWnd);
bool oGetCursorClientPos(HWND _hWnd, int* _pX, int* _pY);
bool oSetCursorClientPos(HWND _hWnd, int _X, int _Y);

// _____________________________________________________________________________
// Window utilties

enum oWINDOWS_VERSION
{
	oWINDOWS_UNKNOWN,
	oWINDOWS_2000,
	oWINDOWS_XP,
	oWINDOWS_XP_PRO_64BIT,
	oWINDOWS_SERVER_2003,
	oWINDOWS_HOME_SERVER,
	oWINDOWS_SERVER_2003R2,
	oWINDOWS_VISTA,
	oWINDOWS_SERVER_2008,
	oWINDOWS_SERVER_2008R2,
	oWINDOWS_7,
};

oWINDOWS_VERSION oGetWindowsVersion();
bool oIsWindows64Bit();

bool oIsAeroEnabled();

// Some Windows API take string blocks, that is one buffer of 
// nul-separated strings that end in a nul terminator. This is
// often not convenient to construct, so allow construction in
// another way and use this to convert.
bool oConvertEnvStringToEnvBlock(char* _EnvBlock, size_t _SizeofEnvBlock, const char* _EnvString, char _Delimiter);
template<size_t size> inline bool oConvertEnvStringToEnvBlock(char (&_EnvBlock)[size], const char* _EnvString, char _Delimiter) { return oConvertEnvStringToEnvBlock(_EnvBlock, size, _EnvString, _Delimiter); }

HRESULT oCreateSimpleWindow(HWND* _pHwnd, WNDPROC _Wndproc, void* _pInstance = 0, const char* _Title = "", int _X = CW_USEDEFAULT, int _Y = CW_USEDEFAULT, int _Width = CW_USEDEFAULT, int _Height = CW_USEDEFAULT, bool _SupportDoubleClicks = false);

// oGetWindowContext is a utility function for simplifying the association of a 
// 'this' pointer with the HWND. When calling CreateWindow, pass the 'this' 
// pointer as the create data param. In the WndProc passed, call this early to 
// get a pointer, and if the pointer is not null, C-cast it to the type of the 
// this pointer, else exit the WndProc gracefully. What this does is capture the
// WM_CREATE message and packs the this pointer in the USERDATA member of the 
// window, and for any other message this returns that USERDATA value.
// If strDescription is not NULL, it will be filled with a summary of the window
// and its dimensions for debug log purposes.
void* oGetWindowContext(HWND _hWnd, UINT _uMsg, WPARAM _WParam, LPARAM _lParam, char* _StrDescription = 0, size_t _SizeofStrDescription = 0);

HRESULT oGetClientScreenRect(HWND _hWnd, RECT* _pRect);

inline bool oHasFocus(HWND _hWnd) { return _hWnd == ::GetForegroundWindow(); }
inline void oSetFocus(HWND _hWnd) { if (!_hWnd) return; ::SetForegroundWindow(_hWnd); ::SetActiveWindow(_hWnd); ::SetFocus(_hWnd); }

// Restore a minimized window to what it was before it was minimized. In more
// complex cases, like full-screen DirectX apps there needs to be a little 
// more than just ShowWindow() to ensure proper behavior, which is why this
// API exists. NOTE: In such DX cases remember to not try to change the window
// state immediately as various context-switch resources may still be in flight,
// so allow a window to come to rest as minimized fully before calling this to
// restore the window.
void oRestoreWindow(HWND _hWnd);

// Respects user settings as to whether window animated transitions are on
void oRespectfulAnimateWindow(HWND _hWnd, const RECT* _pDestinationRect);

// Return the HWND of the "system tray" or "notification area"
HWND oTrayGetHwnd();

// Get the rectangle of the specified icon. Returns true
// if the rect is valid, or false if the icon doesn't 
// exist, so this can be used to determine if the icon 
// exists at all.
bool oTrayGetIconRect(HWND _hWnd, UINT _ID, RECT* _pRect);

// Icons are identified by the HWND and the ID. If a CallbackMessage is not 
// zero, then this is a message that can be handled in the HWND's WNDPROC. Use 
// WM_USER+n for the custom code. If HICON is 0, the icon from the HWND will be 
// used. If HICON is valid it will be used. All lifetime management is up to the 
// user.
void oTrayShowIcon(HWND _hWnd, UINT _ID, UINT _CallbackMessage, HICON _hIcon, bool _Show);

// Sets the focus on the tray itself, not any icon in it
void oTraySetFocus();

// Once an icon has been created with oTrayShowIcon, use this to display a 
// message on it. If _hIcon is null, then the _hWnd's 
bool oTrayShowMessage(HWND _hWnd, UINT _ID, HICON _hIcon, UINT _TimeoutMS, const char* _Title, const char* _Message);

// Helper function for the WNDPROC that handles the _CallbackMessage specified above
void oTrayDecodeCallbackMessageParams(WPARAM _wParam, LPARAM _lParam, UINT* _pNotificationEvent, UINT* _pID, int* _pX, int* _pY);

// Minimize a window to the tray (animates a window to the tray)
void oTrayMinimize(HWND _hWnd, UINT _CallbackMessage, HICON _hIcon);

// Animates an existing tray icon to a restored window
void oTrayRestore(HWND _hWnd);

inline HICON oGetIcon(HWND _hWnd, bool _BigIcon) { return (HICON)SendMessage(_hWnd, WM_GETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), 0); }
inline void oSetIcon(HWND _hWnd, bool _BigIcon, HICON _hIcon) { SendMessage(_hWnd, WM_SETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), (LPARAM)_hIcon); }
HICON oIconFromBitmap(HBITMAP _hBmp);

// Allocates a new BITMAPINFO (if an 8-bit format, a palette is required
// for proper drawing using GDI). If you pass the address of a valid 
// pointer you don't need to specify an allocate function, but the size
// of the destination BMI must be correct. Use oGetBMISize() to ensure
// the correct size is available. The first parameter should be a pointer to
// a pointer to a BITMAPINFO.
void oAllocateBMI(BITMAPINFO** _ppBITMAPINFO, const oSurface::DESC* _pDesc, oFUNCTION<void*(size_t _Size)> _Allocate, bool _FlipVertically = true, unsigned int _ARGBMonochrome8Zero = 0xFF000000, unsigned int _ARGBMonochrome8One = 0xFFFFFFFF);

// 8 bit formats won't render correctly because BITMAPINFO infers 
// paletted textures from 8-bit, so allocate enough room for the palette.
size_t oGetBMISize(oSurface::FORMAT _Format);

inline RECT oBuildRECT(int _Left, int _Top, int _Right, int _Bottom) { RECT r; r.left = _Left; r.top = _Top; r.right = _Right; r.bottom = _Bottom; return r; }
inline RECT oBuildRECTWH(int _Left, int _Top, int _Width, int _Height) { RECT r; r.left = _Left; r.top = _Top; r.right = _Width == oWINDOWS_DEFAULT ? oWINDOWS_DEFAULT : (r.left + _Width); r.bottom = _Height == oWINDOWS_DEFAULT ? oWINDOWS_DEFAULT : (r.top + _Height); return r; }

// Options string:
// 'l' left-aligned, 'c' center-aligned, 'r' right-aligned
// 't' top-aligned, 'm' middle-aligned, 'b' bottom-aligned
// 'f' fit rect to parent rect
// 's' single-line (multi-line is default)

void oResolveRect(const RECT* _pParentRect, const RECT* _pInRect, bool _ClipToParent, RECT* _pOutRect);
void oAlignRect(const RECT* _pParentRect, const RECT* _pInRect, RECT* _pOutAlignedRect, const char* _Options);
void oAdjustRect(const RECT* _pParentRect, const RECT* _pInRect, RECT* _pOutAdjustedRect, int _Scale, const char* _Options);

class oGDIScopedSelect
{
	HDC hDC;
	HGDIOBJ hObj;
	HGDIOBJ hOldObj;
public:
	oGDIScopedSelect(HDC _hDC, HGDIOBJ _hObj) : hDC(_hDC), hObj(_hObj) { hOldObj = SelectObject(hDC, hObj); }
	~oGDIScopedSelect() { SelectObject(hDC, hOldObj); }
};

// Captures image data of the specified window and fills _pImageBuffer.
// pImageBuffer can be NULL, in which case only _pBitmapInfo is filled out.
// For RECT, either specify a client-space rectangle, or NULL to capture the 
// while window including the frame.
bool oGDIScreenCaptureWindow(HWND _hWnd, const RECT* _pRect, void* _pImageBuffer, size_t _SizeofImageBuffer, BITMAPINFO* _pBitmapInfo);

// _dwROP is one of the raster operations from BitBlt()
BOOL oGDIDrawBitmap(HDC _hDC, INT _X, INT _Y, HBITMAP _hBitmap, DWORD _dwROP);
BOOL oGDIStretchBitmap(HDC _hDC, INT _X, INT _Y, INT _Width, INT _Height, HBITMAP _hBitmap, DWORD _dwROP);

// _Options are same as above for oAdjustRect
BOOL oGDIDrawText(HDC _hDC, const RECT* _pRect, unsigned int _ARGBForegroundColor, unsigned int _ARGBBackgroundColor, const char* _Options, const char* _Text);

// If color alpha is true 0, then a null/empty objects is returned. Use 
// DeleteObject on the value returned from these functions when finish with the
// object. (width == 0 means "default")
HPEN oGDICreatePen(unsigned int _ARGBColor, int _Width = 0);
HBRUSH oGDICreateBrush(unsigned int _ARGBColor);
HFONT oGDICreateFont(const char* _FontName, int _PointSize, bool _Bold, bool _Italics, bool _Underline);

const char* oGDIGetFontFamily(BYTE _tmPitchAndFamily);
const char* oGDIGetCharSet(BYTE _tmCharSet);

bool oSetTitle(HWND _hWnd, const char* title);
bool oGetTitle(HWND _hWnd, char* title, size_t numberOfElements);
template<size_t size> inline bool oGetTitle(HWND _hWnd, char (&_Title)[size]) { return oGetTitle(_hWnd, _Title, size); }

// @oooii-tony: A wrapper for SetWindowsHookEx that includes a user-specified context.
// (Is there a way to do this?! Please someone let me know!)
typedef LRESULT (CALLBACK* oHOOKPROC)(int _nCode, WPARAM _wParam, LPARAM _lParam, void* _pUserData);
bool oUnhookWindowsHook(HHOOK _hHook);
HHOOK oSetWindowsHook(int _idHook, oHOOKPROC _pHookProc, void* _pUserData, HINSTANCE _hModule, DWORD _dwThreadId);
// Use this form to hook a window in the current process.
inline HHOOK oSetWindowsHook(int _idHook, oHOOKPROC _pHookProc, void* _pUserData, HWND _hWnd) { return oSetWindowsHook(_idHook, _pHookProc, _pUserData, 0, GetWindowThreadProcessId(_hWnd, 0)); }

// Pump the specified window's message pump for the specified time, by default
// infinitely/until it runs out of messages.
void oPumpMessages(HWND _hWnd, unsigned int _TimeoutMS = oINFINITE_WAIT);

// Fills _StrDestination with a string of the WM_* message and details 
// about the parameters, useful for printing out details for debugging.
char* oGetWMDesc(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
template<size_t size> inline char* oGetWMDesc(char (&_StrDestination)[size], HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) { return oGetWMDesc(_StrDestination, size, _hWnd, _uMsg, _wParam, _lParam); }
inline char* oGetWMDesc(char* _StrDestination, size_t _SizeofStrDestination, const CWPSTRUCT* _pCWPStruct) { return oGetWMDesc(_StrDestination, _SizeofStrDestination, _pCWPStruct->hwnd, _pCWPStruct->message, _pCWPStruct->wParam, _pCWPStruct->lParam); }
template<size_t size> inline char* oGetWMDesc(char (&_StrDestination)[size], const CWPSTRUCT* _pCWPStruct) { return oGetWMDesc(_StrDestination, size, _pCWPStruct); }

// _____________________________________________________________________________
// Identification/ID Conversion API

DWORD oGetThreadID(HANDLE _hThread);

DWORD oWinGetMainThreadID();

DWORD oGetParentProcessID();

// Uses EnumWindows to find the top-level HWND associated with the specified
// _ThreadID. _ThreadID should be the main thread of the process, so either use
// a thread ID given to you by an API closely associated with process work or
// use oWinGetMainThreadID() for the current application.
HWND oGetWindowFromThreadID(DWORD _ThreadID);

// Returns the current module. Unlike GetModuleHandle(NULL), this returns the
// module handle containing the specified pointer. A typical use case would be
// to pass the address of the function calling oGetModule() to return the 
// current module at that point. If NULL is specified, this returns the handle
// of the main process module.
HMODULE oGetModule(void* _ModuleFunctionPointer);

// Returns handle of the active named process
HANDLE oWinGetProcessHandle(const char* _Name);

// Fills the specified array with the IDs of all threads in this process.
unsigned int oGetProcessThreads(DWORD* _pThreadIDs, size_t _SizeofThreadIDs);
template<size_t size> inline unsigned int oGetProcessThreads(DWORD (&_pThreadIDs)[size]) { return oGetProcessThreads(_pThreadIDs, size); }

void oSetThreadNameInDebugger(DWORD _ThreadID, const char* _Name);

inline void oSetThreadNameInDebugger(HANDLE _hThread, const char* _Name) { oSetThreadNameInDebugger(oGetThreadID(_hThread), _Name); }

// Fill the specified buffer with a robust description string of the specified
// HRESULT error code.
int oGetWindowsErrorDescription(char* _StrDestination, size_t _SizeofStrDestination, HRESULT _hResult);
template<size_t size> inline int oGetWindowsErrorDescription(char (&_StrDestination)[size], HRESULT _hResult) { return oGetWindowsErrorDescription(_StrDestination, size, _hResult); }

// _____________________________________________________________________________
// Misc

// Converts a unicode 16-bit string to its 8-bit equivalent. If 
// _SizeofMultiByteString is 0, this returns the required buffer size to hold 
// the conversion (includes room for the nul terminator). Otherwise, this 
// returns the length of the string written (not including the nul terminator).
size_t oStrConvert(char* _MultiByteString, size_t _SizeofMultiByteString, const wchar_t* _StrUnicodeSource);

// Converts a multi-byte 8-bit string to its 16-bit unicode equivalent. If 
// _NumberOfCharactersInUnicodeString is 0, this returns the NUMBER OF CHARACTERS,
// (not the number of bytes) required for the destination bufferm including the 
// nul terminator. Otherwise, this returns the length of the string written.
size_t oStrConvert(wchar_t* _UnicodeString, size_t _NumberOfCharactersInUnicodeString, const char* _StrMultibyteSource);

inline float oPointToDIP(float _Point) { return 96.0f * _Point / 72.0f; }
inline float oDIPToPoint(float _DIP) { return 72.0f * _DIP / 96.0f; }

inline int oPointToLogicalHeight(HDC _hDC, int _Point) { return -MulDiv(_Point, GetDeviceCaps(_hDC, LOGPIXELSY), 72); }
inline int oPointToLogicalHeight(HDC _hDC, float _Point) { return oPointToLogicalHeight(_hDC, static_cast<int>(_Point)); }

void oGetScreenDPIScale(float* _pScaleX, float* _pScaleY);

#if oDXVER >= oDXVER_10
	// Why do we ever need more than one of these? So wrap details of 
	// construction here.
	IDWriteFactory* oGetDWriteFactorySingleton();

	// IDXGIFactory is special as it loads DLLs so it can not be statically held
	// as it can not be released from DLLmain, so always create it.
	bool oCreateDXGIFactory(IDXGIFactory1** _ppFactory);

	HRESULT oDXGICreateSwapchain(IUnknown* _pDevice, unsigned int Width, unsigned int Height, unsigned int RefreshRateN, unsigned int RefreshRateD, DXGI_FORMAT _Fmt, HWND _Hwnd, IDXGISwapChain** _ppSwapChain);

	// Returns the adapter that is responsible for the largest screen portion of the supplied _Rect
	bool oDXGIGetAdapter(const RECT& _Rect, IDXGIAdapter1** _ppAdapter);
	bool oD2D1CreateFactory(ID2D1Factory** _ppFactory);

	float oDXGIGetD3DVersion(IDXGIAdapter* _pAdapter);

	// Find an output with the specified monitor handle
	bool oDXGIFindOutput(IDXGIFactory* _pFactory, HMONITOR _hMonitor, IDXGIOutput** _ppOutput);

	// Returns true if the specified format can be bound as a depth-stencil format
	bool oDXGIIsDepthFormat(DXGI_FORMAT _Format);

	// When creating a texture that can be used with a depth-stencil view and also
	// as a shader resource view, the texture should be created with a typeless 
	// flavor of the desired format. This returns the true depth version of that
	// format.
	DXGI_FORMAT oDXGIGetDepthCompatibleFormat(DXGI_FORMAT _TypelessDepthFormat);

	// Return the format necessary to use in a shader resource view when reading
	// from a format used for a depth-stencil view.
	DXGI_FORMAT oDXGIGetColorCompatibleFormat(DXGI_FORMAT _DepthFormat);
#endif

#if oDXVER >= oDXVER_11
	float oGetD3DVersion(D3D_FEATURE_LEVEL _Level);
#endif

unsigned int oGetWindowDisplayIndex(HWND _hWnd);
unsigned int oGetDisplayDevice(HMONITOR _hMonitor, DISPLAY_DEVICE* _pDevice);

void oGetVirtualDisplayRect(RECT* _pRect);

#else
	#error Unsupported platform
#endif

#include <oooii/oGUID.h>
inline const oGUID& oGetGUID(const GUID& _WinGUID)
{
	return *reinterpret_cast<const oGUID*>(&_WinGUID);
}
#endif
