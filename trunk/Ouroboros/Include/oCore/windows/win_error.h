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
#ifndef oCore_win_error_h
#define oCore_win_error_h

#include <system_error>
#include <cstdio>

extern "C" {

#ifdef _WIN32
	#define WINERR_APICALL __stdcall
#else
	#define WINERR_APICALL
#endif

void __declspec(dllimport) WINERR_APICALL OutputDebugStringA(const char*);
unsigned long __declspec(dllimport) WINERR_APICALL GetLastError();

} // extern "C"

namespace ouro {
	namespace windows {

const std::error_category& category();

/*constexpr*/ inline std::error_code make_error_code(long _hResult) { return std::error_code(_hResult, category()); }
/*constexpr*/ inline std::error_condition make_error_condition(long _hResult) { return std::error_condition(_hResult, category()); }

class error : public std::system_error
{
public:
	error(const error& _That) : std::system_error(_That) {}
	error() : system_error(make_error_code(GetLastError()), make_error_code(GetLastError()).message()) { trace(); }
	error(const char* _Message) : system_error(make_error_code(GetLastError()), _Message) { trace(); }
	error(long _hResult) : system_error(make_error_code(_hResult), make_error_code(_hResult).message()) { trace(); }
	error(long _hResult, const char* _Message) : system_error(make_error_code(_hResult), _Message) { trace(); }
	error(long _hResult, const std::string& _Message) : system_error(make_error_code(_hResult), _Message) { trace(); }
private:
	void trace() { char msg[1024]; _snprintf_s(msg, sizeof(msg), "\nouro::windows::error: 0x%08x: %s\n\n", code().value(), what()); OutputDebugStringA(msg); }
};

	} // namespace windows
} // namespace ouro

// For Windows API that returns an HRESULT, this captures that value and throws 
// on failure.
#define oV(_HRWinFn) do { long HR__ = _HRWinFn; if (HR__) throw ouro::windows::error(HR__); } while(false)

// For Windows API that returns BOOL and uses GetLastError(), this throws on 
// failure.
#define oVB(_BoolWinFn) do { if (!(_BoolWinFn)) throw ouro::windows::error(); } while(false)

#define oVB_MSG(_BoolWinFn, _Format, ...) do { if (!(_BoolWinFn)) { char msg[1024]; _snprintf_s(msg, sizeof(msg), _Format, ## __VA_ARGS__); throw ouro::windows::error(msg); } } while(false)

// Confirm a size_t doesn't overflow a Windows type.
#define oCHECK_SIZE(_WinType, _SizeTValue) if (static_cast<size_t>(static_cast<_WinType>(_SizeTValue)) != static_cast<size_t>(_SizeTValue)) throw std::invalid_argument("out of range: size_t -> " #_WinType);

#endif
