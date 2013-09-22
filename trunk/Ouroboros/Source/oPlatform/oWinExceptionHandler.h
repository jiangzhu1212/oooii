
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
#ifndef oWinExceptionHandler_h
#define oWinExceptionHandler_h

#include <oPlatform/oSingleton.h>
#include <oBase/guid.h>
#include <oStd/mutex.h>
#include <functional>

namespace windows_exception_type
{	enum value {

	unknown,
	std,
	com,
	atl,

};}

namespace ATL { struct CAtlException; }
class _com_error;

struct oWinCppException
{
	oWinCppException()
		: Type(windows_exception_type::unknown)
		, TypeName("")
		, What("")
	{ VoidException = nullptr; }

	windows_exception_type::value Type;
	const char* TypeName;
	const char* What;
	union
	{
		std::exception* StdException;
		_com_error* ComError;
		ATL::CAtlException* AtlException;
		void* VoidException;
	};
};

// NOTE: _pStdException may be nullptr if the exception is not derived from 
// std::exception.
typedef std::function<void(const char* _Message, const oWinCppException& _CppException, uintptr_t _ExceptionContext)> oExceptionHandler;

class oWinExceptionHandler : public oProcessSingleton<oWinExceptionHandler>
{
public:
	static const ouro::guid GUID;
	oWinExceptionHandler();
	~oWinExceptionHandler();

	inline void SetHandler(const oExceptionHandler& _Handler) { Handler = _Handler; }
	
private:
	static const type_info* GetTypeInfo(const EXCEPTION_RECORD& _Record);
	static const void* GetException(const EXCEPTION_RECORD& _Record);
	static LONG CALLBACK StaticOnException(EXCEPTION_POINTERS* _pExceptionPointers) { return Singleton()->OnException(_pExceptionPointers); }
	LONG OnException(EXCEPTION_POINTERS* _pExceptionPointers);

	oStd::recursive_mutex HandlerMutex;
	oExceptionHandler Handler;
};

#endif
