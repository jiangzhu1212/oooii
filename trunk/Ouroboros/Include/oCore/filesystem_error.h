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
// File system abstraction.
#pragma once
#ifndef oCore_filesystem_error_h
#define oCore_filesystem_error_h

#include <oStd/path.h>

namespace oCore {
	namespace filesystem {

template<typename charT, typename TraitsT = oStd::default_posix_path_traits<charT>>
class basic_filesystem_error : public std::system_error
{
public:
	typedef oStd::basic_path<charT, TraitsT> path_type;

	basic_filesystem_error() {}
	~basic_filesystem_error() {}
	basic_filesystem_error(const basic_filesystem_error<charT, TraitsT>& _That)
		: std::system_error(_That)
		, Path1(_That.Path1)
		, Path2(_That.Path2)
	{}

	basic_filesystem_error(std::error_code _ErrorCode)
		: system_error(_ErrorCode, _ErrorCode.message())
	{}

	basic_filesystem_error(const path_type& _Path1, std::error_code _ErrorCode)
		: system_error(_ErrorCode, _ErrorCode.message())
		, Path1(_Path1)
	{}

	basic_filesystem_error(const path_type& _Path1, const path_type& _Path2, std::error_code _ErrorCode)
		: system_error(_ErrorCode, _ErrorCode.message())
		, Path1(_Path1)
		, Path2(_Path2)
	{}

	basic_filesystem_error(const char* _Message, std::error_code _ErrorCode)
		: system_error(_ErrorCode, _Message)
	{}

	basic_filesystem_error(const char* _Message, const path_type& _Path1, std::error_code _ErrorCode)
		: system_error(_ErrorCode, _Message)
		, Path1(_Path1)
	{}

	basic_filesystem_error(const char* _Message, const path_type& _Path1, const path_type& _Path2, std::error_code _ErrorCode)
		: system_error(_ErrorCode, _Message)
		, Path1(_Path1)
		, Path2(_Path2)
	{}
	
	inline const path_type& path1() const { return Path1; }
	inline const path_type& path2() const { return Path2; }

private:
	path_type Path1;
	path_type Path2;
};

typedef basic_filesystem_error<char> filesystem_error;
typedef basic_filesystem_error<wchar_t> wfilesystem_error;

typedef basic_filesystem_error<char, oStd::default_windows_path_traits<char>> windows_filesystem_error;
typedef basic_filesystem_error<wchar_t, oStd::default_windows_path_traits<wchar_t>> windows_wfilesystem_error;

const std::error_category& filesystem_category();

/*constexpr*/ inline std::error_code make_error_code(std::errc::errc _Errc) { return std::error_code(static_cast<int>(_Errc), filesystem_category()); }
/*constexpr*/ inline std::error_condition make_error_condition(std::errc::errc _Errc) { return std::error_condition(static_cast<int>(_Errc), filesystem_category()); }

	} // namespace filesystem
} // namespace oDevice

#endif
