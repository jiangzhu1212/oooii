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
#include "oWinExceptionHandler.h"
#include <oBase/string.h>
#include <oCore/windows/win_error.h>

#include <new.h>
#include <signal.h>
#include <oBase/type_info.h>
#include <comdef.h>

// Additional exceptions. Some here are user-defined to use with RaiseException
// from a condition handler to redirect everything to one path.
#define oEXCEPTION_CPP 0xe06d7363
#define oEXCEPTION_DLL_NOT_FOUND 0xc0000135
#define oEXCEPTION_DLL_BAD_INIT 0xc0000142
#define oEXCEPTION_MODULE_NOT_FOUND 0xc06d007e
#define oEXCEPTION_PROCEDURE_NOT_FOUND 0xc06d007f
#define oEXCEPTION_PURE_VIRTUAL_CALL 0xc0de0000
#define oEXCEPTION_UNEXPECTED 0xc0de0001
#define oEXCEPTION_NEW 0xc0de0002
#define oEXCEPTION_INVALID_PARAMETER 0xc0de0003
#define oEXCEPTION_SIGABRT 0xc0de0004
#define oEXCEPTION_SIGINT 0xc0de0005
#define oEXCEPTION_SIGTERM 0xc0de0006

namespace ouro {

	const char* as_string(const windows::exception_type::value& _Type)
	{
		switch (_Type)
		{
			case windows::exception_type::unknown: return "unknown exception";
			case windows::exception_type::std: return "std::exception";
			case windows::exception_type::com: return "_com_error";
			case windows::exception_type::atl: return "ATL::CAtlException";
			default: break;
		}
		return "?";
	}

	namespace windows {

const char* as_string_exception_code(int _ExceptionCode)
{
	switch (_ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
		case EXCEPTION_DATATYPE_MISALIGNMENT: return "EXCEPTION_DATATYPE_MISALIGNMENT";
		case EXCEPTION_BREAKPOINT: return "EXCEPTION_BREAKPOINT";
		case EXCEPTION_SINGLE_STEP: return "EXCEPTION_SINGLE_STEP";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		case EXCEPTION_FLT_DENORMAL_OPERAND: return "EXCEPTION_FLT_DENORMAL_OPERAND";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
		case EXCEPTION_FLT_INEXACT_RESULT: return "EXCEPTION_FLT_INEXACT_RESULT";
		case EXCEPTION_FLT_INVALID_OPERATION: return "EXCEPTION_FLT_INVALID_OPERATION";
		case EXCEPTION_FLT_OVERFLOW: return "EXCEPTION_FLT_OVERFLOW";
		case EXCEPTION_FLT_STACK_CHECK: return "EXCEPTION_FLT_STACK_CHECK";
		case EXCEPTION_FLT_UNDERFLOW: return "EXCEPTION_FLT_UNDERFLOW";
		case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
		case EXCEPTION_INT_OVERFLOW: return "EXCEPTION_INT_OVERFLOW";
		case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
		case EXCEPTION_IN_PAGE_ERROR: return "EXCEPTION_IN_PAGE_ERROR";
		case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
		case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
		case EXCEPTION_INVALID_DISPOSITION: return "EXCEPTION_INVALID_DISPOSITION";
		case EXCEPTION_GUARD_PAGE: return "EXCEPTION_GUARD_PAGE";
		case EXCEPTION_INVALID_HANDLE: return "EXCEPTION_INVALID_HANDLE";
		//case EXCEPTION_POSSIBLE_DEADLOCK: return "EXCEPTION_POSSIBLE_DEADLOCK";
		case oEXCEPTION_CPP: return "oEXCEPTION_CPP'";
		case oEXCEPTION_DLL_NOT_FOUND: return "oEXCEPTION_DLL_NOT_FOUND'";
		case oEXCEPTION_DLL_BAD_INIT: return "oEXCEPTION_DLL_BAD_INIT'";
		case oEXCEPTION_MODULE_NOT_FOUND: return "oEXCEPTION_MODULE_NOT_FOUND'";
		case oEXCEPTION_PROCEDURE_NOT_FOUND: return "oEXCEPTION_PROCEDURE_NOT_FOUND'";
		case oEXCEPTION_PURE_VIRTUAL_CALL: return "oEXCEPTION_PURE_VIRTUAL_CALL'";
		case oEXCEPTION_UNEXPECTED: return "oEXCEPTION_UNEXPECTED'";
		case oEXCEPTION_NEW: return "oEXCEPTION_NEW'";
		case oEXCEPTION_INVALID_PARAMETER: return "oEXCEPTION_INVALID_PARAMETER'";
		case oEXCEPTION_SIGABRT: return "oEXCEPTION_SIGABRT'";
		case oEXCEPTION_SIGINT: return "oEXCEPTION_SIGINT'";
		case oEXCEPTION_SIGTERM: return "oEXCEPTION_SIGTERM'";
		default: break;
	}

	return "unrecognized Exception Code";
}

static void pure_virtual_call_handler()
{
	RaiseException(oEXCEPTION_PURE_VIRTUAL_CALL, EXCEPTION_NONCONTINUABLE, 0, nullptr);
}

static int new_handler(size_t _Size)
{
	ULONG_PTR sz = (ULONG_PTR)_Size;
	RaiseException(oEXCEPTION_NEW, EXCEPTION_NONCONTINUABLE, 1, &sz);
	return 0;
}

static void invalid_parameter_handler(const wchar_t* _Expression, const wchar_t* _Function, const wchar_t* _File, unsigned int _Line, uintptr_t _pReserved)
{
	ULONG_PTR params[5] = { (ULONG_PTR)_Expression,(ULONG_PTR) _Function, (ULONG_PTR)_File, (ULONG_PTR)_Line, (ULONG_PTR)_pReserved };
	RaiseException(oEXCEPTION_NEW, EXCEPTION_NONCONTINUABLE, 5, params);
}

static void sigabrt_handler(int _SigValue)
{
	ULONG_PTR v = (ULONG_PTR)_SigValue;
	//RaiseException(oEXCEPTION_SIGABRT, EXCEPTION_NONCONTINUABLE, 1, &v);
}

static void sigint_handler(int _SigValue)
{
	ULONG_PTR v = (ULONG_PTR)_SigValue;
	RaiseException(oEXCEPTION_SIGINT, EXCEPTION_NONCONTINUABLE, 1, &v);
}

static void sigterm_handler(int _SigValue)
{
	ULONG_PTR v = (ULONG_PTR)_SigValue;
	RaiseException(oEXCEPTION_SIGTERM, EXCEPTION_NONCONTINUABLE, 1, &v);
}

static void unexpected_handler()
{
	RaiseException(oEXCEPTION_UNEXPECTED, EXCEPTION_NONCONTINUABLE, 0, nullptr);
}

static void redirect_handlers_to_exceptions()
{
	_set_purecall_handler(pure_virtual_call_handler);
	_set_new_handler(new_handler);
	set_unexpected(unexpected_handler);
	_set_invalid_parameter_handler(invalid_parameter_handler); 
	_set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
	signal(SIGABRT, sigabrt_handler);
	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigterm_handler);
}

static const ::type_info* get_veh_type_info(const EXCEPTION_RECORD& _Record)
{
	// http://blogs.msdn.com/b/oldnewthing/archive/2010/07/30/10044061.aspx
	if (_Record.ExceptionCode != oEXCEPTION_CPP)
		return nullptr;
	
	#ifdef _WIN64
		HMODULE hModule = (HMODULE)_Record.ExceptionInformation[3];
	#else
		HMODULE hModule = nullptr;
	#endif
	DWORD A = ((DWORD*)_Record.ExceptionInformation[2])[3];
	DWORD B = ((DWORD*)byte_add(hModule, A))[1];
	DWORD C = ((DWORD*)byte_add(hModule, B))[1];
	return (::type_info*)byte_add(hModule, C);
}

static cpp_exception get_veh_exception(const EXCEPTION_RECORD& _Record)
{
	cpp_exception e;
	if (_Record.ExceptionCode == oEXCEPTION_CPP)
	{
		e.void_exception = (void*)_Record.ExceptionInformation[1];
		const ::type_info* ti = get_veh_type_info(_Record);
		if (ti)
		{
			e.type_name = type_name(ti->name());
			// how can this deal with exceptions derived from std::exception?
			// Weak answer for now: assume it's either namespaced in std:: or
			// it is postfixed with _exception.
			if (strstr(e.type_name, "_com_error"))
			{
				e.type = exception_type::com;
				e.what = category().message(e.com_error->Error());
			}
			
			else if (strstr(e.type_name, "CAtlException"))
			{
				e.type = exception_type::atl;
			}
			
			else if (strstr(e.type_name, "std::") || strstr(e.type_name, "_exception"))
			{
				e.type = exception_type::std;
				e.what = e.std_exception->what();
			}
		}
	}
	return e;
}

// Allows us to break execution when an access violation occurs
LONG exceptions::on_exception(EXCEPTION_POINTERS* _pExceptionPointers)
{
	EXCEPTION_RECORD* pRecord = _pExceptionPointers->ExceptionRecord;
	cpp_exception CppException;
	switch (pRecord->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
		{
			void* pAddress = (void*)pRecord->ExceptionInformation[1];
			const char* err = (0 == pRecord->ExceptionInformation[0]) ? "Read" : "Write";
			lstring ErrorMessage;
			snprintf(ErrorMessage, "%s access violation at 0x%p", err, pAddress);
			Handler(ErrorMessage, CppException, (uintptr_t)_pExceptionPointers);
			break;
		}

		// This handler occurs before C++ gets its chance to catch an exception, so
		// don't get in the way of that.
		case oEXCEPTION_CPP:
		{
			CppException = get_veh_exception(*pRecord);
			if (!CppException.what.empty())
			{
				xlstring msg;
				path ModulePath = std::move(ouro::this_module::path());
				#ifdef _WIN64
					#define LOWER_CASE_PTR_FMT "%016llx"
				#else
					#define LOWER_CASE_PTR_FMT "%08x"
				#endif

				snprintf(msg, "First-chance exception at 0x" LOWER_CASE_PTR_FMT ": in %s: %s: %s\n"
					, pRecord->ExceptionAddress, ModulePath.filename().c_str(), CppException.type_name, CppException.what.c_str());
				OutputDebugStringA(msg);
			}
			break;
		}

		// Ensure any exception that is to be handled is listed explicitly. There
		// are some working-as-intended uses of exceptions by Windows that should 
		// not terminate execution, but instead should be passed through.
		case EXCEPTION_DATATYPE_MISALIGNMENT:
		case EXCEPTION_BREAKPOINT:
		case EXCEPTION_SINGLE_STEP:
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_FLT_INEXACT_RESULT:
		case EXCEPTION_FLT_INVALID_OPERATION:
		case EXCEPTION_FLT_OVERFLOW:
		case EXCEPTION_FLT_STACK_CHECK:
		case EXCEPTION_FLT_UNDERFLOW:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_INT_OVERFLOW:
		case EXCEPTION_PRIV_INSTRUCTION:
		case EXCEPTION_IN_PAGE_ERROR:
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		case EXCEPTION_STACK_OVERFLOW:
		case EXCEPTION_INVALID_DISPOSITION:
		case EXCEPTION_GUARD_PAGE:
		case EXCEPTION_INVALID_HANDLE:
		case oEXCEPTION_PURE_VIRTUAL_CALL:
		case oEXCEPTION_UNEXPECTED:
		case oEXCEPTION_DLL_NOT_FOUND:
		case oEXCEPTION_DLL_BAD_INIT:
		case oEXCEPTION_MODULE_NOT_FOUND:
		case oEXCEPTION_PROCEDURE_NOT_FOUND:
		case oEXCEPTION_SIGINT:
		case oEXCEPTION_SIGTERM:
			Handler(as_string_exception_code(pRecord->ExceptionCode), CppException, (uintptr_t)_pExceptionPointers);
			break;

		default:
			break;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

LONG exceptions::static_on_exception(EXCEPTION_POINTERS* _pExceptionPointers)
{
	return exceptions::singleton().on_exception(_pExceptionPointers);
}

exceptions::exceptions() 
{
	AddVectoredExceptionHandler(0, static_on_exception);
	redirect_handlers_to_exceptions();
}

exceptions::~exceptions()
{
	RemoveVectoredExceptionHandler(static_on_exception);
}

exceptions& exceptions::singleton()
{
	exceptions* sInstance = nullptr;
	if (!sInstance)
	{
		process_heap::find_or_allocate(
			"windows::exceptions"
			, process_heap::per_process
			, process_heap::leak_tracked
			, [=](void* _pMemory) { new (_pMemory) exceptions(); }
			, [=](void* _pMemory) { ((exceptions*)_pMemory)->~exceptions(); }
			, &sInstance);
	}
	return *sInstance;
}

	} // namespace windows
} // namespace ouro
