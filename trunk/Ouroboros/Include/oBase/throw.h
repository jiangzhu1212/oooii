// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Creating a standard exception can be very tedious, so wrap common patterns in 
// convenience macros.

#pragma once
#include <oString/string.h>
#include <system_error>

namespace ouro {

inline std::string vformatf(const char* _Format, va_list _Args)
{
	char buf[2048];
	#pragma warning(disable:4996) // secure CRT warning
	const int len = vsnprintf(buf, sizeof(buf), _Format, _Args);
	ellipsize(buf);
	#pragma warning(default:4996)
	if (len < 1) buf[2047] = 0;
	return buf;
}

inline std::string formatf(const char* _Format, ...)
{
	va_list a; va_start(a, _Format);
	std::string s = vformatf(_Format, a);
	va_end(a);
	return s;
}

}

#define oTHROW(_SystemError, _Message, ...) do { throw std::system_error(std::make_error_code(std::errc::_SystemError), ::ouro::formatf(_Message, ## __VA_ARGS__)); } while(false)
#define oTHROW0(_SystemError) do { std::error_code ec = std::make_error_code(std::errc::_SystemError); throw std::system_error(ec, ec.message()); } while(false)
#define oCHECK(_Expression, _Message, ...) do { if (!(_Expression)) oTHROW(protocol_error, _Message, ## __VA_ARGS__); } while(false)
#define oCHECK0(_Expression) do { if (!(_Expression)) oTHROW(protocol_error, "%s", #_Expression); } while(false)
#define oTHROW_INVARG(_Message, ...) do { throw std::invalid_argument(::ouro::formatf(_Message, ## __VA_ARGS__)); } while(false)
#define oTHROW_INVARG0() do { throw std::invalid_argument("invalid argument"); } while(false)
#define oCHECK_ARG(_Expression, _Message, ...) do { if (!(_Expression)) oTHROW_INVARG(#_Expression); } while(false)
#define oCHECK_ARG0(_Expression) do { if (!(_Expression)) oTHROW_INVARG(_Message, ## __VA_ARGS__); } while(false)
