/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oooii/oDebugger.h>
#include <oooii/oSingleton.h>
#include <oooii/oStddef.h>
#include <oooii/oString.h>
#include <oooii/oThreading.h>
#include <oooii/oWindows.h>
#include "oDbgHelp.h"
#include <float.h>

#define HARDLINK_DBGHELP
#define oBUG_921
  
struct oDebuggerContext : public oProcessSingleton<oDebuggerContext>
{
	oDebuggerContext()
	{
		_controlfp_s(&OrigFloatExceptions, 0, 0);
		oDbgHelp::Singleton()->Reference();
	}

	~oDebuggerContext()
	{
		oDbgHelp::Singleton()->Release();
	}

	void EnableFloatExceptions(bool _Enable)
	{
		unsigned int dummy = 0;
		_controlfp_s(&dummy, _Enable ? _EM_INEXACT : OrigFloatExceptions, _MCW_EM);
	}

	unsigned int OrigFloatExceptions;
};

void oDebugger::Reference()
{
	oDebuggerContext::Singleton()->Reference();
}

void oDebugger::Release()
{
	oDebuggerContext::Singleton()->Release();
}

bool oDebugger::FloatExceptionsEnabled()
{
	unsigned int current = 0;
	_controlfp_s(&current, 0, 0);
	return !!(current & _EM_INEXACT);
}

void oDebugger::EnableFloatExceptions(bool _Enable)
{
	oDebuggerContext::Singleton()->EnableFloatExceptions(_Enable);
}

void oDebugger::ReportLeaksOnExit(bool _Report, bool _UseDefaultReporter)
{
	#ifdef _DEBUG
		if (_UseDefaultReporter)
		{
			const int kLeakCheckFlags = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
			int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
			_CrtSetDbgFlag(_Report ? (flags | kLeakCheckFlags) : (flags &~ kLeakCheckFlags));
		}

		else
	#endif
	{
		#ifdef oBUG_921
			oCRTWARNING("CRTTracker disabled. See bug 921. No memory leaks will be reported. Use _UseDefaultReporter=true to work around this issue for now.");
		#else
			oCRTMallocTracker::Singleton()->Report(_Report);
			oCRTMallocTracker::Singleton()->Enable(_Report);
		#endif
	}
}

void oDebugger::BreakOnAlloc(long _RequestNumber)
{
	_CrtSetBreakAlloc(_RequestNumber);
}

void oDebugger::Print(const char* _String)
{
	// MSDN says to ensure thread safety explicitly with OutputDebugString
	oThreadsafeOutputDebugStringA(_String);
}

bool oDebugger::IsAttached()
{
	return IsDebuggerPresent() != 0;
}

void oDebugger::SetThreadName(const char* _Name, void* _NativeThreadHandle)
{
	oSetThreadNameInDebugger((HANDLE)_NativeThreadHandle, _Name);
}

/** $(Citation)
	<citation>
		<usage type="Adaptation" />
		<author name="Jochen Kalmback" />
		<description url="http://jpassing.wordpress.com/category/win32/" />
		<license type="BSD" url="http://www.opensource.org/licenses/bsd-license.php" />
	</citation>
	<citation>
		<usage type="Adaptation" />
		<author name="Geoff Evans" />
		<author name="Paul Haile" />
		<description url="http://nocturnal.insomniacgames.com/index.php/Main_Page" />
		<license type="Insomniac Open License (IOL)" url="http://nocturnal.insomniacgames.com/index.php/Insomniac_Open_License" />
	</citation>
*/

// Several of the API assume use of CHAR, not TCHAR
_STATIC_ASSERT(sizeof(CHAR) == sizeof(char));

static void InitContext(CONTEXT& c)
{
	memset(&c, 0, sizeof(c));
	RtlCaptureContext(&c);
}

static void InitStackFrame(STACKFRAME64& s, const CONTEXT& c)
{
  memset(&s, 0, sizeof(s));
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Mode = AddrModeFlat;
  s.AddrStack.Mode = AddrModeFlat;

	#ifdef _M_IX86
	  s.AddrPC.Offset = c.Eip;
	  s.AddrFrame.Offset = c.Ebp;
	  s.AddrStack.Offset = c.Esp;
	#elif _M_X64
	  s.AddrPC.Offset = c.Rip;
	  s.AddrFrame.Offset = c.Rsp;
	  s.AddrStack.Offset = c.Rsp;
	#elif _M_IA64
	  s.AddrPC.Offset = c.StIIP;
	  s.AddrFrame.Offset = c.IntSp;
	  s.AddrStack.Offset = c.IntSp;
	  s.AddrBStore.Offset = c.RsBSP;
	  s.AddrBStore.Mode = AddrModeFlat;
	#else
		#error "Platform not supported!"
	#endif
}

size_t oDebugger::GetCallstack(unsigned long long* _pAddresses, size_t _NumberOfAddresses, size_t _Offset)
{
	CONTEXT c;
	InitContext(c);

	STACKFRAME64 s;
	InitStackFrame(s, c);

	size_t n = 0;
	while (n < _NumberOfAddresses)
	{
		#ifdef HARDLINK_DBGHELP
			if (!oDbgHelp::Singleton()->CallStackWalk64_HARDLINKED(GetCurrentThread(), &s, &c))
		#else
			if (!oDbgHelp::Singleton()->CallStackWalk64(GetCurrentThread(), &s, &c))
		#endif
			{
				break;
			}

		if (s.AddrReturn.Offset == 0 || s.AddrReturn.Offset == 0xffffffffcccccccc)
			break;

		if (_Offset)
		{
			_Offset--;
			continue;
		}

		_pAddresses[n++] = s.AddrReturn.Offset;
	}

	return n;
}

bool oDebugger::TranslateSymbol(SYMBOL* _pSymbol, unsigned long long _Address)
{
	if (!_pSymbol)
		return false;

	bool success = true;

	IMAGEHLP_MODULE64 module;
	memset(&module, 0, sizeof(module));
	module.SizeOfStruct = sizeof(module);

	_pSymbol->Address = _Address;
	if (oDbgHelp::Singleton()->CallSymGetModuleInfo64(_Address, &module))
		strcpy_s(_pSymbol->Module, module.ModuleName);
	else
		success = false;

	BYTE buf[sizeof(IMAGEHLP_SYMBOL64) + oCOUNTOF(_pSymbol->Name) * sizeof(TCHAR)];
    IMAGEHLP_SYMBOL64* symbolInfo = (IMAGEHLP_SYMBOL64*)buf;
	memset(buf, 0, sizeof(IMAGEHLP_SYMBOL64));
    symbolInfo->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	symbolInfo->MaxNameLength = oCOUNTOF(_pSymbol->Name);

	DWORD64 displacement = 0;
	if (oDbgHelp::Singleton()->CallSymGetSymFromAddr64(_Address, &displacement, symbolInfo))
	{
		// symbolInfo just contains the first 512 characters and doesn't guarantee
		// they will be null-terminated, so copy the buffer and ensure there's some
		// rational terminator

		//strcpy_s(_pSymbol->Name, symbolInfo->Name);
		memcpy(_pSymbol->Name, symbolInfo->Name, sizeof(_pSymbol->Name)-sizeof(TCHAR));
		oAddTruncationElipse(_pSymbol->Name);
		_pSymbol->SymbolOffset = static_cast<unsigned int>(displacement);
	}

	else
		success = false;

 	IMAGEHLP_LINE64 line;
	memset(&line, 0, sizeof(line));
	line.SizeOfStruct = sizeof(line);

	DWORD disp;
	if (oDbgHelp::Singleton()->CallSymGetLineFromAddr64(_Address, &disp, &line))
	{
		strcpy_s(_pSymbol->Filename, line.FileName);
		_pSymbol->Line = line.LineNumber;
		_pSymbol->CharOffset = static_cast<unsigned int>(displacement);
	}

	else
	{
		strcpy_s(_pSymbol->Filename, "Unknown");
		_pSymbol->Line = 0;
		_pSymbol->CharOffset = 0;
	}

	return success;
}
