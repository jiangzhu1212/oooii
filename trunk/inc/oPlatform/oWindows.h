// $(header)

// This should be included instead of, or at least before <windows.h> to avoid
// much of the conflict-causing overhead as well as include some useful 
#pragma once
#ifndef oWindows_h
#define oWindows_h
#if defined(_WIN32) || defined(_WIN64)

#ifdef _WINDOWS_
	#pragma message("BAD WINDOWS INCLUDE! Applications should #include <oWindows.h> to prevent extra and sometimes conflicting cruft from being included.")
#endif

#include <oBasis/oGUID.h>
#include <oBasis/oMathTypes.h>
#include <oBasis/oSurface.h>
#include <oPlatform/oStddef.h>

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
//#define NOSERVICE // used below by services queries
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

// Some GPU drivers have bugs in newer features that we use, so ensure we're at
// least on this version and hope there aren't regressions.

#define oNVVER_MAJOR 285
#define oNVVER_MINOR 62

// @oooii-eric: TODO: Set to ATI catalyst 11.9 which is what I have currently. Some unit tests don't pass with this driver version however. (bcencodedecode, condenseobj)
//	Note that ATI driver version does not follow from the catalyst version naturally
#define oATIVER_MAJOR 8
#define oATIVER_MINOR 892

// Define something like WINVER, but for DX functionality
#if (defined(NTDDI_WIN7) && (NTDDI_VERSION >= NTDDI_WIN7))
	#define oDXVER oDXVER_11
#elif (defined(NTDDI_LONGHORN) && (NTDDI_VERSION >= NTDDI_LONGHORN)) || (defined(NTDDI_VISTA) && (NTDDI_VERSION >= NTDDI_VISTA))
	#define oDXVER oDXVER_11 //oDXVER_10_1
#else
	#define oDXVER oDXVER_9c
#endif

// mostly for debugging...
#if (defined(NTDDI_WIN7) && (NTDDI_VERSION >= NTDDI_WIN7))
	oBUILD_TRACE("PLATFORM: Windows7")
	#define oWINDOWS_HAS_TRAY_NOTIFYICONIDENTIFIER
	#define oWINDOWS_HAS_TRAY_QUIETTIME
#elif (defined(NTDDI_VISTA) && (NTDDI_VERSION >= NTDDI_VISTA))
	oBUILD_TRACE("PLATFORM: Windows Vista")
#elif (defined(NTDDI_LONGHORN) && (NTDDI_VERSION >= NTDDI_LONGHORN))
	oBUILD_TRACE("PLATFORM: Windows Longhorn")
#elif (defined(NTDDI_WINXP) && (NTDDI_VERSION >= NTDDI_WINXP))
	oBUILD_TRACE("PLATFORM: Windows XP")
#endif

#if (_MSC_VER >= 1600)
	oBUILD_TRACE("COMPILER: Visual Studio 2010")
#elif (_MSC_VER >= 1500)
	oBUILD_TRACE("COMPILER: Visual Studio 2008")
#endif

#if oDXVER >= oDXVER_13_1
	oBUILD_TRACE("DIRECTX: 13.1")
#elif oDXVER >= oDXVER_13
	oBUILD_TRACE("DIRECTX: 13.0")
#elif oDXVER >= oDXVER_12_1
	oBUILD_TRACE("DIRECTX: 12.1")
#elif oDXVER >= oDXVER_12
	oBUILD_TRACE("DIRECTX: 12.0")
#elif oDXVER >= oDXVER_11_1
	oBUILD_TRACE("DIRECTX: 11.1")
#elif oDXVER >= oDXVER_11
	oBUILD_TRACE("DIRECTX: 11.0")
#elif oDXVER >= oDXVER_10_1
	oBUILD_TRACE("DIRECTX: 10.1")
#elif oDXVER >= oDXVER_10
	oBUILD_TRACE("DIRECTX: 10.0")
#elif oDXVER >= oDXVER_9c
	oBUILD_TRACE("DIRECTX: 9.0c")
#elif oDXVER >= oDXVER_9b
	oBUILD_TRACE("DIRECTX: 9.0b")
#elif oDXVER >= oDXVER_9a
	oBUILD_TRACE("DIRECTX: 9.0a")
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
// Abstract between running with a console and not for release.
// Example:
//
// oMAIN() // will have a console for log output/stdout in debug, and no console in non-debug builds
// {
//   ExecuteMyProgram();
//   return 0;
// }

#ifdef _DEBUG
	#define oMAIN() int main(int argc, const char* argv[])
#else
	#define oMAIN() \
		int release_main(int argc, const char* argv[]); \
		int oWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow, int (*oMain)(int argc, const char* argv[])); \
		int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) { return oWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow, release_main); } \
		int release_main(int argc, const char* argv[])
#endif

// _____________________________________________________________________________
// Error check wrappers. These interpret HRESULTS, so should not be used on 
// anything but WIN32 API. oVB is for the return FALSE then GetLastError() 
// pattern, and oV is for direct HRESULT return values.

#ifdef _DEBUG
	#define oVB(fn) do { if (!(fn)) { oWinSetLastError(::GetLastError()); oASSERT_PRINT(oASSERT_ASSERTION, oASSERT_IGNORE_ONCE, #fn, "%s", oErrorGetLastString()); } } while(false)
	#define oV(fn) do { HRESULT HR__ = fn; if (FAILED(HR__)) { oWinSetLastError(HR__); oASSERT_PRINT(oASSERT_ASSERTION, oASSERT_IGNORE_ONCE, #fn, "%s", oErrorGetLastString()); } } while(false)
#else
	#define oVB(fn) fn
	#define oV(fn) fn
#endif

// In some cases oTRACE is too high-level, so add support for some formatted
// OutputDebugString. This does not use oThreadsafeOutputDebugString because
// this is for lower-level systems such as oSingleton or IOCP etc.
void oOutputDebugStringV(const char* _Format, va_list _Args);
inline void oOutputDebugString(const char* _Format, ...) { va_list args; va_start(args, _Format); oOutputDebugStringV(_Format, args); va_end(args); }

// _____________________________________________________________________________
// Wrappers for the Windows-specific crtdbg API. Prefer oASSERT macros found
// in oAssert.h, but for a few systems that are lower-level than oAssert, these
// may be necessary.

#ifdef _DEBUG
	#include <crtdbg.h>

	#define oCRTASSERT(expr, msg, ...) if (!(expr)) { if (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, "OOOii Debug Library", #expr "\n\n" msg, ## __VA_ARGS__)) oDEBUGBREAK(); }
	#define oCRTWARNING(msg, ...) do { _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, "OOOii Debug Library", "WARNING: " msg, ## __VA_ARGS__); } while(false)
	#define oCRTTRACE(msg, ...) do { _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, "OOOii Debug Library", msg, ## __VA_ARGS__); } while(false)

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

// Given the specified HRESULT, set both the closest errno value and the 
// platform-specific description associated with the error code.
// if oWINDOWS_DEFAULT is specified, ::GetLastError() is used
bool oWinSetLastError(HRESULT _hResult = oDEFAULT, const char* _ErrorDescPrefix = 0);

// oV_RETURN executes a block of code that returns an HRESULT
// if the HRESULT is not S_OK it returns the HRESULT
#define oV_RETURN(fn) do { HRESULT HR__ = fn; if (FAILED(HR__)) return HR__; } while(false)

// oVB_RETURN executes a block of Windows API code that returns bool and populates
// oErrorGetLast() with ::GetLastError() and returns false.
#define oVB_RETURN(fn) do { if (!(fn)) { oWinSetLastError(oDEFAULT, #fn " failed: "); return false; } } while(false)

// oVERIFY_RETURN execues a block of code that returns an HRESULT. If the 
// HRESULT is not S_OK, then this sets last error with the platform error and
// returns false.
#define oVB_RETURN2(fn) do { HRESULT HR__ = fn; if (FAILED(HR__)) { oWinSetLastError(HR__, #fn " failed: "); return false; } } while(false)

// _____________________________________________________________________________
// Smart pointer support

inline ULONG oGetRefCount(IUnknown* unk) { ULONG r = unk->AddRef()-1; unk->Release(); return r; }
inline void intrusive_ptr_add_ref(IUnknown* unk) { unk->AddRef(); }
inline void intrusive_ptr_release(IUnknown* unk) { unk->Release(); }

// _____________________________________________________________________________
// Time conversion

static const FILETIME oFTPreserve = { oInvalid, oInvalid };
void oUnixTimeToFileTime(time_t _Time, FILETIME* _pFileTime);
time_t oFileTimeToUnixTime(const FILETIME* _pFileTime);

void oUnixTimeToSystemTime(time_t _Time, SYSTEMTIME* _pSystemTime);
time_t oSystemTimeToUnixTime(const SYSTEMTIME* _pSystemTime);

// Uses a waitable timer to call the specified function at the specified time.
// If _Alertable is true, this can wake up a sleeping system.
bool oScheduleTask(const char* _DebugName, time_t _AbsoluteTime, bool _Alertable, oTASK _Task);

// _____________________________________________________________________________
// Other conversion

inline const oGUID& oGetGUID(const GUID& _WinGUID) { return *reinterpret_cast<const oGUID*>(&_WinGUID); }

// Converts a unicode 16-bit string to its 8-bit equivalent. If 
// _SizeofMultiByteString is 0, this returns the required buffer size to hold 
// the conversion (includes room for the nul terminator). Otherwise, this 
// returns the length of the string written (not including the nul terminator).
size_t oStrConvert(char* _MultiByteString, size_t _SizeofMultiByteString, const wchar_t* _StrUnicodeSource);
template<size_t size> inline size_t oStrConvert(char (&_MultiByteString)[size], const wchar_t* _StrUnicodeSource) { return oStrConvert(_MultiByteString, size, _StrUnicodeSource); }

// Converts a multi-byte 8-bit string to its 16-bit unicode equivalent. If 
// _NumberOfCharactersInUnicodeString is 0, this returns the NUMBER OF CHARACTERS,
// (not the number of bytes) required for the destination bufferm including the 
// nul terminator. Otherwise, this returns the length of the string written.
size_t oStrConvert(wchar_t* _UnicodeString, size_t _NumberOfCharactersInUnicodeString, const char* _StrMultibyteSource);
template<size_t size> inline size_t oStrConvert(wchar_t (&_UnicodeString)[size], const char* _StrMultibyteSource) { return oStrConvert(_UnicodeString, size, _StrMultibyteSource); }

// An ASCII version of CommandLineToArgvW. Use oWinCommandLineToArgvAFree() to
// free the buffer returned by this function.
const char** oWinCommandLineToArgvA(bool _ExePathAsArg0, const char* pCmdLine, int* _pArgc);

// Frees a buffer returned by oWinCommandLineToArgvA
void oWinCommandLineToArgvAFree(const char** _pArgv);

inline float oPointToDIP(float _Point) { return 96.0f * _Point / 72.0f; }
inline float oDIPToPoint(float _DIP) { return 72.0f * _DIP / 96.0f; }

// _____________________________________________________________________________
// Concurrency

// LoadLibrary holds an internal mutex that can deadlock when OOOii lib executes
// code that loads another library. This has only been observed to happen in 
// early process initialization specifically with oProcessHeap, so use these to
// protect against deadlocks.
HMODULE oThreadsafeLoadLibrary(LPCTSTR _lpFileName);
BOOL oThreadsafeFreeLibrary(HMODULE _hModule);

// So many problems would be solved if this were threadsafe! Be careful, using 
// this in concurrent situations can cause undesired synchronization because of
// the mutex used to protect messages from being stomped on. NOTE: The mutex is
// process-wide, no matter what DLLs or how this code was linked.
void oThreadsafeOutputDebugStringA(const char* _OutputString);

// returns true if wait finished successfully, or false if
// timed out or otherwise errored out.
bool oWaitSingle(HANDLE _Handle, unsigned int _TimeoutMS = oINFINITE_WAIT);

// If _pWaitBreakingIndex is nullptr, this waits for all handles to be 
// unblocked. If a valid pointer, then it is filled with the index of the handle
// that unblocked the wait.
bool oWaitMultiple(HANDLE* _pHandles, size_t _NumberOfHandles, size_t* _pWaitBreakingIndex = nullptr, unsigned int _TimeoutMS = oINFINITE_WAIT);

bool oWaitSingle(DWORD _ThreadID, unsigned int _Timeout = oINFINITE_WAIT);
bool oWaitMultiple(DWORD* _pThreadIDs, size_t _NumberOfThreadIDs, size_t* _pWaitBreakingIndex = nullptr, unsigned int _TimeoutMS = oINFINITE_WAIT);

template<typename GetNativeHandleT> inline bool oTWaitMultiple(threadsafe GetNativeHandleT** _ppWaitable, size_t _NumWaitables, size_t* _pWaitBreakingIndex = nullptr, unsigned int _TimeoutMS = oINFINITE_WAIT)
{
	HANDLE handles[64]; // 64 is a windows limit
	for (size_t i = 0; i < _NumWaitables; i++)
		handles[i] = static_cast<HANDLE>(_ppWaitable[i]->GetNativeHandle());
	return oWaitMultiple(handles, _NumWaitables, _pWaitBreakingIndex, _TimeoutMS);
}

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
bool oEnableAero(bool _Enabled, bool _Force = false);

// Some Windows API take string blocks, that is one buffer of 
// nul-separated strings that end in a nul terminator. This is
// often not convenient to construct, so allow construction in
// another way and use this to convert.
bool oConvertEnvStringToEnvBlock(char* _EnvBlock, size_t _SizeofEnvBlock, const char* _EnvString, char _Delimiter);
template<size_t size> inline bool oConvertEnvStringToEnvBlock(char (&_EnvBlock)[size], const char* _EnvString, char _Delimiter) { return oConvertEnvStringToEnvBlock(_EnvBlock, size, _EnvString, _Delimiter); }

HICON oIconFromBitmap(HBITMAP _hBmp);

// Allocates a new BITMAPINFO (if an 8-bit format, a palette is required
// for proper drawing using GDI). If you pass the address of a valid 
// pointer you don't need to specify an allocate function, but the size
// of the destination BMI must be correct. Use oGetBMISize() to ensure
// the correct size is available. The first parameter should be a pointer to
// a pointer to a BITMAPINFO.
void oAllocateBMI(BITMAPINFO** _ppBITMAPINFO, const oSURFACE_DESC& _Desc, oFUNCTION<void*(size_t _Size)> _Allocate, bool _FlipVertically = true, unsigned int _ARGBMonochrome8Zero = 0xFF000000, unsigned int _ARGBMonochrome8One = 0xFFFFFFFF);

// 8 bit formats won't render correctly because BITMAPINFO infers 
// paletted textures from 8-bit, so allocate enough room for the palette.
size_t oGetBMISize(oSURFACE_FORMAT _Format);

oSURFACE_FORMAT oGetFormat(const BITMAPINFOHEADER& _BitmapInfoHeader);

// @oooii-tony: A wrapper for SetWindowsHookEx that includes a user-specified context.
// (Is there a way to do this?! Please someone let me know!)
typedef LRESULT (CALLBACK* oHOOKPROC)(int _nCode, WPARAM _wParam, LPARAM _lParam, void* _pUserData);
bool oUnhookWindowsHook(HHOOK _hHook);
HHOOK oSetWindowsHook(int _idHook, oHOOKPROC _pHookProc, void* _pUserData, DWORD _dwThreadId);
// Use this form to hook a window in the current process.
inline HHOOK oSetWindowsHook(int _idHook, oHOOKPROC _pHookProc, void* _pUserData, HWND _hWnd) { return oSetWindowsHook(_idHook, _pHookProc, _pUserData, GetWindowThreadProcessId(_hWnd, 0)); }

// _____________________________________________________________________________
// Dialog box helpers

enum oWINDOWS_DIALOG_ITEM_TYPE
{
	oDLG_BUTTON,
	oDLG_EDITBOX,
	oDLG_LABEL_LEFT_ALIGNED,
	oDLG_LABEL_CENTERED,
	oDLG_LABEL_RIGHT_ALIGNED,
	oDLG_LARGELABEL,
	oDLG_ICON,
	oDLG_LISTBOX,
	oDLG_SCROLLBAR,
	oDLG_COMBOBOX,
};

struct oWINDOWS_DIALOG_ITEM
{
	const char* Text;
	oWINDOWS_DIALOG_ITEM_TYPE Type;
	WORD ItemID;
	RECT Rect;
	bool Enabled;
	bool Visible;
	bool TabStop;
};

struct oWINDOWS_DIALOG_DESC
{
	const char* Font;
	const char* Caption;
	const oWINDOWS_DIALOG_ITEM* pItems;
	UINT NumItems;
	UINT FontPointSize;
	RECT Rect;
	bool Center; // if true, ignores Rect.left, Rect.top positioning
	bool SetForeground;
	bool Enabled;
	bool Visible;
	bool AlwaysOnTop;
};

LPDLGTEMPLATE oDlgNewTemplate(const oWINDOWS_DIALOG_DESC& _Desc);
void oDlgDeleteTemplate(LPDLGTEMPLATE _lpDlgTemplate);

struct oWINDOWS_VIDEO_DRIVER_DESC
{
	enum GPU_VENDOR
	{
		UNKNOWN,
		NVIDIA,
		ATI,
	};

	int MajorVersion;
	int MinorVersion;
	oStringL Desc;
	GPU_VENDOR Vendor;
};

bool oWinGetVideoDriverDesc(oWINDOWS_VIDEO_DRIVER_DESC* _pDesc);

// Returns false if the video driver isn't the version blessed for this build
// of the software. (See o???VER_MAJOR and o???VER_MINOR above). If this returns
// false oSetLastError is called with a robust error message describing the 
// current and required versions.
bool oWinVideoDriverIsUpToDate();

// _____________________________________________________________________________
// System API - dealing with Windows as a whole

// Goes through all services/drivers currently on the system
bool oWinServicesEnum(oFUNCTION<bool(SC_HANDLE _hSCManager, const ENUM_SERVICE_STATUS_PROCESS& _Status)> _Enumerator);

// Returns the resolved path to the binary that is the running service
bool oWinGetServiceBinaryPath(char* _StrDestination, size_t _SizeofStrDestination, SC_HANDLE _hSCManager, const char* _ServiceName);

// Returns true if the specified file is a binary compiled for x64. If false,
// it is reasonable to assume x86 (32-bit).
bool oWinSystemIs64BitBinary(const char* _StrPath);

// Returns true if all services on the system are not in a pending state.
bool oWinSystemAllServicesInSteadyState();

// Returns a percent [0,100] of CPU usage across all processes and all cores
// (basically the summary percentage that Task Manager gives you). The value
// returned is over a sample period, so it is required that two values are 
// cached to be compared against for the calculation. Thus this function should
// be called as a regular interval for refreshing the current CPU usage.
double oWinSystemCalculateCPUUsage(unsigned long long* _pPreviousIdleTime, unsigned long long* _pPreviousSystemTime);

// _____________________________________________________________________________
// Identification/ID Conversion API

// Converts a standard C file handle from fopen to a windows handle
HANDLE oGetFileHandle(FILE* _File);

DWORD oGetThreadID(HANDLE _hThread);

// Returns the current module. Unlike GetModuleHandle(NULL), this returns the
// module handle containing the specified pointer. A typical use case would be
// to pass the address of the function calling oGetModule() to return the 
// current module at that point. If NULL is specified, this returns the handle
// of the main process module.
HMODULE oGetModule(void* _ModuleFunctionPointer);

// Returns true if _pOutProcessID has been filled with the ID of the specified
// process image name (exe name). If this returns false, check oErrorGetLast()
// for more information.
bool oWinGetProcessID(const char* _Name, DWORD* _pOutProcessID);

// Call the specified function for each thread in the current process. The 
// function should return true to keep enumerating, or false to exit early. This
// function returns false if there is a failure, check oErrorGetLast() for more
// information.
bool oEnumProcessThreads(DWORD _ProcessID, oFUNCTION<bool(DWORD _ThreadID, DWORD _ParentProcessID)> _Function);

// Call the specified function for each of the child processes of the current
// process. The function should return true to keep enumerating, or false to
// exit early. This function returns false if there is a failure, check 
// oErrorGetLast() for more information. The error can be ECHILD if there are no 
// child processes.
bool oWinEnumProcesses(oFUNCTION<bool(DWORD _ProcessID, DWORD _ParentProcessID, const char* _ProcessExePath)> _Function);

void oSetThreadNameInDebugger(DWORD _ThreadID, const char* _Name);

inline void oSetThreadNameInDebugger(HANDLE _hThread, const char* _Name) { oSetThreadNameInDebugger(oGetThreadID(_hThread), _Name); }

// _____________________________________________________________________________
// Misc

// Adjusts privileges and calls ExitWindowsEx with the specified parameters
bool oWinExitWindows(UINT _uFlags, DWORD _dwReason);

void oGetScreenDPIScale(float* _pScaleX, float* _pScaleY);

#if oDXVER >= oDXVER_11
	float oGetD3DVersion(D3D_FEATURE_LEVEL _Level);
#endif

unsigned int oWinGetDisplayDevice(HMONITOR _hMonitor, DISPLAY_DEVICE* _pDevice);

void oGetVirtualDisplayRect(RECT* _pRect);

inline POINT oAsPOINT(const int2& _P) { POINT p; p.x = _P.x; p.y = _P.y; return p; }

#else
	#error Unsupported platform
#endif

#endif
