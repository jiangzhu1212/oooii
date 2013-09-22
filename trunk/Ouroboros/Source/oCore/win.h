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
#pragma once
#ifndef oCore_win_h
#define oCore_win_h

#if !defined(_WIN32) && !defined(_WIN64)
	#error Including a Windows header for a non-Windows target
#endif

#ifdef _WINDOWS_
	#pragma message("BAD WINDOWS INCLUDE! Applications should #include <win.h> to limit the proliferation of macros and platform interfaces throughout the code.")
#endif

#ifndef NOMINMAX
	#define NOMINMAX
#endif

#define NOATOM
#define NOCOMM
#define NOCRYPT
#define NODEFERWINDOWPOS
#define NODRAWTEXT
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
#define NORASTEROPS
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
#include <DShow.h>
#include <shlobj.h>
#define _NO_CVCONST_H
#include <comutil.h>
#include <DbgHelp.h>
#include <Dwmapi.h>
#include <tlhelp32.h>
#include <Wbemidl.h>
#include <psapi.h>
#include <PowrProf.h>
#include <lm.h>

#if (defined(NTDDI_WIN7) && (NTDDI_VERSION >= NTDDI_WIN7))
	#define oWINDOWS_HAS_TRAY_NOTIFYICONIDENTIFIER
	#define oWINDOWS_HAS_TRAY_QUIETTIME
	#define oWINDOWS_HAS_SHMUTEX_TRYLOCK
	#define oWINDOWS_HAS_REGISTERTOUCHWINDOW
#endif

// Define something like WINVER, but for DX functionality
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

#if (defined(NTDDI_WIN7) && (NTDDI_VERSION >= NTDDI_WIN7))
	#define oDXVER oDXVER_11
#elif (defined(NTDDI_LONGHORN) && (NTDDI_VERSION >= NTDDI_LONGHORN)) || (defined(NTDDI_VISTA) && (NTDDI_VERSION >= NTDDI_VISTA))
	#define oDXVER oDXVER_11 //oDXVER_10_1
#else
	#define oDXVER oDXVER_9c
#endif

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

// RAII and size_t support
#include <oStd/finally.h>
#include <system_error>

#define oCLOSE(hHandle) do { if (hHandle && hHandle != INVALID_HANDLE_VALUE) ::CloseHandle(hHandle); } while(false)
#define oFINALLY_CLOSE(hHandle) oStd::finally Close##hHandle([&] { oCLOSE(hHandle); })
#define oCHECK_SIZE(_WinType, _SizeTValue) if (static_cast<size_t>(static_cast<_WinType>(_SizeTValue)) != (_SizeTValue)) throw std::invalid_argument("out of range: size_t -> " #_WinType);

// Error checking support
#define oV(_HRWinFn) do { HRESULT HR__ = _HRWinFn; if (FAILED(HR__)) throw oCore::windows_error(HR__); } while(false)
#define oVB(_BoolWinFn) do { if (!(_BoolWinFn)) throw oCore::windows_error(); } while(false)

// oStd::ref support
inline ULONG ref_count(IUnknown* unk) { ULONG r = unk->AddRef()-1; unk->Release(); return r; }
inline void intrusive_ptr_add_ref(IUnknown* unk) { unk->AddRef(); }
inline void intrusive_ptr_release(IUnknown* unk) { unk->Release(); }

namespace oCore {

const std::error_category& windows_category();
/*constexpr*/ inline std::error_code make_error_code(HRESULT _hResult) { return std::error_code(_hResult, windows_category()); }
/*constexpr*/ inline std::error_condition make_error_condition(HRESULT _hResult) { return std::error_condition(_hResult, windows_category()); }

class windows_error : public std::system_error
{
public:
	windows_error(const windows_error& _That) : std::system_error(_That) {}

	windows_error()
		: system_error(make_error_code(GetLastError()), make_error_code(GetLastError()).message())
	{ trace(); }

	windows_error(HRESULT _hResult)
		: system_error(make_error_code(_hResult), make_error_code(_hResult).message())
	{ trace(); }

	windows_error(HRESULT _hResult, const char* _Message)
		: system_error(make_error_code(_hResult), _Message)
	{ trace(); }

	windows_error(HRESULT _hResult, const std::string& _Message)
		: system_error(make_error_code(_hResult), _Message)
	{ trace(); }

private:
	void trace() { char buf[1024]; snprintf(buf, "\noCore::windows_error: 0x%08x: %s\n\n", code().value(), what()); OutputDebugStringA(buf); }
};

	namespace windows {

std::string message(HRESULT _hResult);

const char* as_string_display_code(UINT _DISPCode);

/* enum class */ namespace version
{	enum value {

	unknown,
	win2000,
	xp,
	xp_pro_64bit,
	server_2003,
	home_server,
	server_2003r2,
	vista,
	server_2008,
	vista_sp1,
	server_2008_sp1,
	vista_sp2,
	server_2008_sp2,
	win7,
	server_2008r2,
	win7_sp1,
	server_2008r2_sp1,
	win8,
	server_2012,
	win8_1,
	server_2012_sp1,

};}

version::value get_version();

	} // namespace windows
} // namespace oCore

#endif
