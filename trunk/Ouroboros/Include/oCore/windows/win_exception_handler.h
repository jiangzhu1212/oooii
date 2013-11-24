
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
// A vectored exception handler that redirects some other handlers to the 
// main exception path and calls to the handler when an exception occurs.
#pragma once
#ifndef oCore_win_exception_handler_h
#define oCore_win_exception_handler_h

#include <oBase/path.h>
#include <functional>
#include <string>

#undef interface
#undef INTERFACE_DEFINED

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ATL { struct CAtlException; }
class _com_error;

namespace ouro {
	namespace windows {

namespace exception_type
{	enum value {

	unknown,
	std,
	com,
	atl,

};}

struct cpp_exception
{
	cpp_exception()
		: type(exception_type::unknown)
		, type_name("")
		, what("")
	{ void_exception = nullptr; }

	exception_type::value type;
	const char* type_name;
	std::string what;
	union
	{
		std::exception* std_exception;
		_com_error* com_error;
		ATL::CAtlException* atl_exception;
		void* void_exception;
	};
};

// NOTE: _pStdException may be nullptr if the exception is not derived from 
// std::exception.
typedef std::function<void(const char* _Message
	, const cpp_exception& _CppException
	, uintptr_t _ExceptionContext)> exception_handler;

class exceptions
{
public:
	static exceptions& singleton();

	inline void set_handler(const exception_handler& _Handler) { Handler = _Handler; }

	// Specify a directory where mini/full dumps will be written. Dump filenames 
	// will be generated with a timestamp at the time of exception.
	inline void mini_dump_path(const path& _MiniDumpPath) { MiniDump = _MiniDumpPath; }
	inline const path& mini_dump_path() const { return MiniDump; }

	inline void full_dump_path(const path& _FullDumpPath) { FullDump = _FullDumpPath; }
	inline const path& full_dump_path() const { return FullDump; }

	inline void post_dump_command(const char* _Command) { PostDumpCommand = _Command; }
	inline const char* post_dump_command() const { return PostDumpCommand; }

	inline void prompt_after_dump(bool _Prompt) { PromptAfterDump = _Prompt; }
	inline bool prompt_after_dump() const { return PromptAfterDump; }
	
private:
	exceptions();
	~exceptions();

	static const ::type_info* get_type_info(const EXCEPTION_RECORD& _Record);
	static const void* get_exception(const EXCEPTION_RECORD& _Record);
	static LONG static_on_exception(EXCEPTION_POINTERS* _pExceptionPointers);
	LONG on_exception(EXCEPTION_POINTERS* _pExceptionPointers);

	exception_handler Handler;

	path MiniDump;
	path FullDump;
	xlstring PostDumpCommand;
	bool PromptAfterDump;
};

	} // namespace windows
} // namespace ouro

#endif
