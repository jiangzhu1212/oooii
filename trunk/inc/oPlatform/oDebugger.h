// $(header)
// Interface for accessing common debugger features
#pragma once
#ifndef oDebugger_h
#define oDebugger_h

#include <stdarg.h>
#include <oBasis/oStdThread.h>

// Sets the name of the specified thread in the debugger's UI. If the default ID
// value is specified then the id of this_thread is used.
void oDebuggerSetThreadName(const char* _Name, oStd::thread::id _ID = oStd::thread::id());

// Print the specified string to a debug window. Threadsafe.
void oDebuggerPrint(const char* _String);

// Uses oDebuggerPrint to produce a summary of memory that is still allocated
// when the program exits from the crt memory heap. Memory from other heaps will
// not be reported.
void oDebuggerReportCRTLeaksOnExit(bool _Enable = true);

// Breaks in the debugger when the specified allocation occurs. The ID can be
// determined from the output of a leak report.
void oDebuggerBreakOnAllocation(uintptr_t _AllocationID);

// Fills the array pointed to by _pSymbols with up to _NumSymbols symbols of 
// functions in the current callstack from main(). This includes every function 
// that led to the GetCallstack() call. Offset ignores the first _Offset number 
// of functions at the top of the stack so a system can simplify/hide details of 
// any debug reporting code and keep the callstack started from the meaningful 
// user code where an error or assertion occurred. This returns the actual 
// number of addresses retrieved.
size_t oDebuggerGetCallstack(unsigned long long* _pSymbols, size_t _NumSymbols, size_t _Offset);
template<size_t size> inline size_t oDebuggerGetCallstack(unsigned long long (&_pSymbols)[size], size_t _Offset) { return oDebuggerGetCallstack(_pSymbols, size, _Offset); }

struct oDEBUGGER_SYMBOL
{
	unsigned long long Address;
	char Module[512];
	char Name[512];
	char Filename[512];
	unsigned int SymbolOffset;
	unsigned int Line;
	unsigned int CharOffset;
};

// Convert a symbol retrieved using oDebuggerGetCallstack() into more 
// descriptive parts.
bool oDebuggerTranslateSymbol(oDEBUGGER_SYMBOL* _pSymbol, unsigned long long _Symbol);

// Uses oDebuggerTranslateSymbol to format a string fit for reporting to a log
// file. If _pIsStdBind is valid, it should be initialized once to false and 
// then left untouched as this function is called in a loop. If this is done,
// std::bind internal implementation details will be concatenated to a single
// line. _PrefixString is useful for adding indentation and must be valid.
int oDebuggerSymbolSPrintf(char* _Buffer, size_t _SizeofBuffer, unsigned long long _Symbol, const char* _PrefixString = "", bool* _pIsStdBind = nullptr);
template<size_t size> int oDebuggerSymbolSPrintf(char (&_Buffer)[size], unsigned long long _Symbol, const char* _PrefixString = "", bool* _pIsStdBind = nullptr) { return oDebuggerSymbolSPrintf(_Buffer, size, _Symbol, _PrefixString, _pIsStdBind); }

#endif
