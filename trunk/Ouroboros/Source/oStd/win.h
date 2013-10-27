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
#ifndef oStd_win_h
#define oStd_win_h

#if !defined(_WIN32) && !defined(_WIN64)
	#error Including a Windows header for a non-Windows target
#endif

#ifdef _WINDOWS_
	#pragma message("BAD WINDOWS INCLUDE! Applications should #include <win.h> to limit the proliferation of macros and platform interfaces throughout the code.")
#endif

#ifndef NOMINMAX
	#define NOMINMAX
#endif

// GUI/drawing
//#define NODRAWTEXT
//#define NOMENUS
//#define NORASTEROPS

#define NOATOM
#define NOCOMM
#define NOCRYPT
#define NODEFERWINDOWPOS
#define NOGDICAPMASKS
#define NOHELP
#define NOIMAGE
#define NOKANJI
#define NOKERNEL
#define NOMCX
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOPROFILER
#define NOPROXYSTUB
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

#include <system_error>

// Including intrin is necessary for interlocked functions, but it causes
// a warning to spew from math. So squelch that warning since it seems the
// declarations are indeed the same.
#include <intrin.h>
#pragma warning(disable:4985) // 'ceil': attributes not present on previous declaration.
#include <math.h>
#pragma warning (default:4985)

#include <windows.h>
#include <windowsx.h>
#include <winsock2.h>
//#define _NO_CVCONST_H
#include <comutil.h>
#include <Shellapi.h>

//#include <Dwmapi.h>
//#include <tlhelp32.h>
//#include <Wbemidl.h>
//#include <psapi.h>
//#include <PowrProf.h>
//#include <lm.h>

#ifdef interface
	#define INTERFACE_DEFINED
#endif

// _____________________________________________________________________________
// Debug CRT support

#ifdef _DEBUG
	#include <crtdbg.h>
	#define oCRTASSERT(expr, msg, ...) if (!(expr)) { if (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, "Ouroboros Debug Library", #expr "\n\n" msg, ## __VA_ARGS__)) __debugbreak(); }
	#define oCRTTRACE(msg, ...) do { _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, "Ouroboros Debug Library", msg "\n", ## __VA_ARGS__); } while(false)

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
	#define oCRTTRACE(msg, ...) __noop
#endif

// Error checking support
#define oCHECK_SIZE(_WinType, _SizeTValue) if (static_cast<size_t>(static_cast<_WinType>(_SizeTValue)) != static_cast<size_t>(_SizeTValue)) throw std::invalid_argument("out of range: size_t -> " #_WinType);
#define oV(_HRWinFn) do { HRESULT HR__ = _HRWinFn; if (FAILED(HR__)) throw oStd::windows::error(HR__); } while(false)
#define oVB(_BoolWinFn) do { if (!(_BoolWinFn)) throw oStd::windows::error(); } while(false)

// intrusive_ptr support
inline ULONG ref_count(IUnknown* unk) { ULONG r = unk->AddRef()-1; unk->Release(); return r; }
inline void intrusive_ptr_add_ref(IUnknown* unk) { unk->AddRef(); }
inline void intrusive_ptr_release(IUnknown* unk) { unk->Release(); }

namespace oStd {
	namespace windows {

const std::error_category& category();
/*constexpr*/ inline std::error_code make_error_code(HRESULT _hResult) { return std::error_code(_hResult, category()); }
/*constexpr*/ inline std::error_condition make_error_condition(HRESULT _hResult) { return std::error_condition(_hResult, category()); }

class error : public std::system_error
{
public:
	error(const error& _That) : std::system_error(_That) {}
	error() : system_error(make_error_code(GetLastError()), make_error_code(GetLastError()).message()) { trace(); }
	error(HRESULT _hResult) : system_error(make_error_code(_hResult), make_error_code(_hResult).message()) { trace(); }
	error(HRESULT _hResult, const char* _Message) : system_error(make_error_code(_hResult), _Message) { trace(); }
	error(HRESULT _hResult, const std::string& _Message) : system_error(make_error_code(_hResult), _Message) { trace(); }
private:
	void trace() { char buf[1024]; _snprintf_s(buf, sizeof(buf), "\noStd::windows::error: 0x%08x: %s\n\n", code().value(), what()); OutputDebugStringA(buf); }
};

class scoped_handle
{
	scoped_handle(scoped_handle&);
	const scoped_handle& operator=(scoped_handle&);

public:
	scoped_handle() : h(nullptr) {}
	scoped_handle(HANDLE _Handle) : h(_Handle) { if (h == INVALID_HANDLE_VALUE) throw windows::error(); }
	scoped_handle(scoped_handle&& _That) { operator=(std::move(_That)); }
	const scoped_handle& operator=(scoped_handle&& _That)
	{
		if (this != &_That)
		{
			close();
			h = _That.h;
			_That.h = nullptr;
		}
		return *this;
	}

	const scoped_handle& operator=(HANDLE _That)
	{
		close();
		h = _That;
	}

	~scoped_handle() { close(); }

	operator HANDLE() { return h; }

private:
	HANDLE h;
	void close() { if (h && h != INVALID_HANDLE_VALUE) { ::CloseHandle(h); h = nullptr; } }
};

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

const char* as_string(const version::value& _Version);

version::value get_version();

	} // namespace windows
} // namespace oStd

#endif
