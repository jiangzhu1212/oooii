/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Creating a standard exception can be very tedious, so wrap common patterns in 
// convenience macros.
#pragma once
#ifndef oStd_throw_h
#define oStd_throw_h

#include <oStd/string.h>
#include <system_error>

namespace oStd {
	namespace detail {

inline std::string vformatf(const char* _Format, va_list _Args)
{
	char buf[128];
#pragma warning(disable:4996) // secure CRT warning
	const size_t len = vsnprintf(buf, sizeof(buf), _Format, _Args);
#pragma warning(default:4996)
	if (len <= sizeof(buf))
		return std::move(std::string(buf));
	char* bigger_buf = static_cast<char*>(alloca(len));
	snprintf(bigger_buf, len, _Format, _Args);
	return std::move(std::string(bigger_buf));
}

inline std::string formatf(const char* _Format, ...)
{
	va_list a; va_start(a, _Format);
	std::string s = vformatf(_Format, a);
	va_end(a);
	return std::move(s);
}

	} // namespace detail
} // namespace oStd

#define oTHROW(_SystemError, _Message, ...) do { throw std::system_error(std::make_error_code(std::errc::_SystemError), oStd::detail::formatf(_Message, ## __VA_ARGS__)); } while(false)
#define oTHROW0(_SystemError) do { std::error_code ec = std::make_error_code(std::errc::_SystemError); throw std::system_error(ec, ec.message()); } while(false)
#define oCHECK(_Expression, _Message, ...) do { if (!(_Expression)) oTHROW(protocol_error, _Message, ## __VA_ARGS__); } while(false)
#define oCHECK0(_Expression) do { if (!(_Expression)) oTHROW(protocol_error, "%s", #_Expression); } while(false)

#endif
