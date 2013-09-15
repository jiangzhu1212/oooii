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
// Interface for accessing common debugger features
#pragma once
#ifndef oCore_debugger_h
#define oCore_debugger_h

#include <oStd/path.h>

namespace oCore {
	namespace debugger {

/* enum class */ namespace exception_code
{	enum value {

	pure_virtual_call,
	unexpected,
	new_out_of_memory,
	invalid_parameter,
	debugger_exception,
	access_violation,
	illegal_instruction,
	stack_overflow,
	guard_page,
	array_bounds_exceeded,
	flt_denormal_operand,
	flt_divide_by_zero,
	flt_inexact_result,
	flt_invalid_operation,
	flt_overflow,
	flt_stack_check,
	flt_underflow,
	int_divide_by_zero,
	int_overflow,
	datatype_misalignment,
	priv_instruction,
	in_page_error,
	noncontinuable_exception,
	invalid_disposition,
	invalid_handle,
	breakpoint,
	single_step,
	sigabrt,
	sigint,
	sigterm,

};}

struct symbol_info
{
	unsigned long long address;
	oStd::path module;
	oStd::path filename;
	oStd::mstring name;
	unsigned int symbol_offset;
	unsigned int line;
	unsigned int char_offset;
};

typedef void* symbol;

// Sets the name of the specified thread in the debugger's UI. If the default id 
// value is specified then the id of this_thread is used.
void thread_name(const char* _Name, oStd::thread::id _ID = oStd::thread::id());

// Print the specified string to a debug window
void print(const char* _String);

// uses print() to produce a summary of memory that is still allocated from the 
// crt heap when the program exists.
void report_crt_leaks_on_exit(bool _Enable = true);

// Breaks in the debugger when the specified allocation occurs. The id can be
// determined from the output of a leak report.
void break_on_alloc(uintptr_t _AllocationID);

// Fills the array pointed to by _pSymbols with up to _NumSymbols symbols of 
// functions in the current callstack from main(). This includes every function 
// that led to the GetCallstack() call. Offset ignores the first _Offset number 
// of functions at the top of the stack so a system can simplify/hide details of 
// any debug reporting code and keep the callstack started from the meaningful 
// user code where an error or assertion occurred. This returns the actual 
// number of addresses retrieved.
size_t callstack(symbol* _pSymbols, size_t _NumSymbols, size_t _Offset);
template<size_t size> inline size_t callstack(symbol (&_pSymbols)[size], size_t _Offset) { return callstack(_pSymbols, size, _Offset); }

// Convert a symbol retrieved using oDebuggerGetCallstack() into more 
// descriptive parts.
symbol_info translate(symbol _Symbol);

// uses snprintf to format the specified symbol to _StrDestination. If the bool
// pointed to by _pIsStdBind is false, then this will print an "it's all 
// std::bind" line and update the value to true. If the bool contains true no
// string will be emitted and the return value will be zero. This way cluttering
// runs of std::bind code can be reduced to just one line.
int format(char* _StrDestination, size_t _SizeofStrDestination, symbol _Symbol, const char* _Prefix, bool* _pIsStdBind = nullptr);
template<size_t size> int format(char (&_StrDestination)[size], symbol _Symbol, const char* _Prefix, bool* _pIsStdBind = nullptr) { return format(_StrDestination, size, _Symbol, _Prefix, _pIsStdBind); }

// Saves a file containing debug information that can be used to do postmortem 
// debugging. Exceptions is the platform-specific context during exception
// handling.
bool dump(const oStd::path& _Path, bool _Full, void* _Exceptions);

	} // namespace debugger
} // namespace oCore

#endif
