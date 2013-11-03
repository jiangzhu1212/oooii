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
#include <oBasis/oError.h>
#include <oBasis/oBasisRequirements.h>
#include <oBase/fixed_string.h>
#include <oBase/macros.h>

using namespace ouro;

const char* oErrorAsString(errno_t _Error)
{
	switch (_Error)
	{
		case 0: return "none";
		case std::errc::no_such_file_or_directory: return "no_such_file_or_directory";
		case std::errc::no_such_device: return "no_such_device";
		case std::errc::no_such_process: return "no_such_process";
		case std::errc::operation_in_progress: return "operation_in_progress";
		case std::errc::operation_canceled: return "operation_canceled";
		case std::errc::no_buffer_space: return "no_buffer_space";
		case std::errc::operation_not_permitted: return "operation_not_permitted";
		case std::errc::operation_would_block: return "operation_would_block";
		case std::errc::timed_out: return "timed_out";
		case std::errc::invalid_argument: return "invalid_argument";
		case std::errc::io_error: return "io_error";
		case std::errc::permission_denied: return "permission_denied";
		case std::errc::protocol_error: return "protocol_error";
		case std::errc::not_supported: return "not_supported";
		case std::errc::function_not_supported: return "function_not_supported";
		case std::errc::no_child_process: return "no_child_process";
		case std::errc::no_message_available: return "no_message_available";
		case std::errc::connection_aborted: return "connection_aborted";
		oNODEFAULT;
	}
}

const char* oErrorGetDefaultString(errno_t _Error)
{
	switch (_Error)
	{
		case 0: return "operation was successful";
		case std::errc::no_such_file_or_directory: return "no such file or directory";
		case std::errc::no_such_device: return "no such device";
		case std::errc::no_such_process: return "no such process";
		case std::errc::operation_in_progress: return "redundant operation";
		case std::errc::operation_canceled: return "operation canceled";
		case std::errc::no_buffer_space: return "storage is at capacity";
		case std::errc::operation_not_permitted: return "operation performed on wrong thread";
		case std::errc::operation_would_block: return "operation would block";
		case std::errc::timed_out: return "operation timed out";
		case std::errc::invalid_argument: return "invalid parameter specified";
		case std::errc::io_error: return "IO error occurred";
		case std::errc::permission_denied: return "access refused";
		case std::errc::protocol_error: return "data is corrupt";
		case std::errc::not_supported: return "not supported";
		case std::errc::function_not_supported: return "function not supported";
		case std::errc::no_child_process: return "no child processes";
		case std::errc::no_message_available: return "no message available";
		case std::errc::connection_aborted: return "connection aborted";
		oNODEFAULT;
	}
}

struct ERROR_CONTEXT
{
	static const size_t ERROR_STRING_BUFFER_SIZE = 2048;

	size_t ErrorCount;
	errno_t Error;
	char ErrorString[ERROR_STRING_BUFFER_SIZE];
	bool UseDefaultString;
};

ERROR_CONTEXT* GetErrorContext()
{
	static thread_local ERROR_CONTEXT* pErrorContext = nullptr;
	if(!pErrorContext)
	{
		// add ctor to ERROR_CONTEXT
		//if (process_heap::find_or_allocate(sizeof(bool)
		//	, "ERROR_CONTEXT"
		//	, process_heap::per_thread
		//	, process_heap::none
		//	, [=](void* _pMemory) { new (_pMemory) ERROR_CONTEXT(); }
		//	, (void**)&pErrorContext))
		//{
		//	process_heap::deallocate_at_thread_exit(nullptr, pErrorContext);
		//}

		// {99091828-104D-4320-92C9-FD41810C352D}
		static const guid GUIDErrorContext = { 0x99091828, 0x104d, 0x4320, { 0x92, 0xc9, 0xfd, 0x41, 0x81, 0xc, 0x35, 0x2d } };

		oThreadlocalMalloc(GUIDErrorContext, [=](void* _pMemory)
		{
			ERROR_CONTEXT* ctx = (ERROR_CONTEXT*)_pMemory;
			ctx->ErrorCount = 0;
			ctx->Error = 0;
			ctx->ErrorString[0] = 0;
			ctx->UseDefaultString = false;
		}
		, oLIFETIME_TASK(), &pErrorContext);
	}

	return pErrorContext;
}

bool oErrorSetLast(errno_t _Error)
{
	ERROR_CONTEXT* pErrorContext = GetErrorContext();
	pErrorContext->ErrorCount++;
	pErrorContext->Error = _Error;
	pErrorContext->UseDefaultString = true;
	return false;
}

bool oErrorSetLastV(errno_t _Error, const char* _Format, va_list _Args)
{
	ERROR_CONTEXT* pErrorContext = GetErrorContext();
	pErrorContext->ErrorCount++;
	pErrorContext->Error = _Error;
	pErrorContext->UseDefaultString = false;
	_Format = _Format ? _Format : oErrorGetDefaultString(_Error);
	vsnprintf(pErrorContext->ErrorString, _Format, _Args);
	ellipsize(pErrorContext->ErrorString, oCOUNTOF(pErrorContext->ErrorString));
	return false;
}

bool oErrorPrefixLastV(const char* _Format, va_list _Args)
{
	ERROR_CONTEXT* pErrorContext = GetErrorContext();
	xlstring CurrentCopy;
	snprintf(CurrentCopy, pErrorContext->ErrorString);

	vsnprintf(pErrorContext->ErrorString, _Format, _Args);
	sncatf(pErrorContext->ErrorString, "%s", CurrentCopy);
	return false;
}

size_t oErrorGetLastCount()
{
	return GetErrorContext()->ErrorCount;
}

size_t oErrorGetSizeofMessageBuffer()
{
	return ERROR_CONTEXT::ERROR_STRING_BUFFER_SIZE;
}

errno_t oErrorGetLast()
{
	return GetErrorContext()->Error;
}

const char* oErrorGetLastString()
{
	ERROR_CONTEXT* pErrorContext = GetErrorContext();
	return (oSTRVALID(pErrorContext->ErrorString) && !pErrorContext->UseDefaultString) ? pErrorContext->ErrorString : oErrorGetDefaultString(oErrorGetLast());
}
