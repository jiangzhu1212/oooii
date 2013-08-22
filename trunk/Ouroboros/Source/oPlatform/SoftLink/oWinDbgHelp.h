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
#pragma once
#ifndef oWinDbgHelp_h
#define oWinDbgHelp_h

#include <oPlatform/oModule.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/Windows/oWindows.h>
#define _NO_CVCONST_H
#include <DbgHelp.h>
#include <oStd/guid.h>

struct oWinDbgHelp : public oProcessSingleton<oWinDbgHelp>
{
	// Encapsulation of the details of dynamically linking to DbgHelp.dll and the APIs
	// needed to report callstack information. Wow this is a lot of redundant typing,
	// is there no better way? Well, it's all typed-out now.

	// oStd::as_string() implemented for SymTagEnum.

	// NOTE: ModBase/BaseOfDll and GetModuleHandle() are all the same thing.

	// A good starting read for using the output of the symbols retreived by this
	// API: http://www.debuginfo.com/articles/dbghelptypeinfo.html

	typedef void (*ModuleLoadedHandler)(const char* Name, bool successful);

	oWinDbgHelp(HANDLE _hProcess = 0, const char* _SymbolPath = "", ModuleLoadedHandler _Handler = 0);
	~oWinDbgHelp();

	// Yes, mask oProcessSingleton's Singleton() call
	static oWinDbgHelp* Singleton()
	{
		if (!sInstanceForDeferredRelease)
		{
			sInstanceForDeferredRelease = oProcessSingleton<oWinDbgHelp>::Singleton();
			sInstanceForDeferredRelease->Reference();
			atexit(AtExit);
		}
		return sInstanceForDeferredRelease;
	}
	
	BOOL (__stdcall *EnumerateLoadedModules64)(HANDLE hProcess, PENUMLOADED_MODULES_CALLBACK64 EnumLoadedModulesCallback, PVOID UserContext);
	BOOL (__stdcall *StackWalk64)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
	BOOL (__stdcall *SymCleanup)(HANDLE hProcess);
	PVOID (__stdcall *SymFunctionTableAccess64)(HANDLE hProcess, DWORD64 AddrBase);
	BOOL (__stdcall *SymGetLineFromAddr64)(HANDLE hProcess, DWORD64 dwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line);
	DWORD64 (__stdcall *SymGetModuleBase64)(HANDLE hProcess, DWORD64 dwAddr);
	BOOL (__stdcall *SymGetModuleInfo64)(HANDLE hProcess, DWORD64 dwAddr, PIMAGEHLP_MODULE64 ModuleInfo);
	DWORD (__stdcall *SymGetOptions)();
	BOOL (__stdcall *SymGetSearchPath)(HANDLE hProcess, PTSTR SearchPath, DWORD SearchPathLength);
	BOOL (__stdcall *SymGetSymFromAddr64)(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PIMAGEHLP_SYMBOL64 Symbol);
	DWORD64 (__stdcall *SymLoadModule64)(HANDLE hProcess, HANDLE hFile, PCSTR ImageName, PCSTR ModuleName, DWORD64 BaseOfDll, DWORD SizeOfDll);
	BOOL (__stdcall *SymInitialize)(HANDLE hProcess, PCTSTR UserSearchPath, BOOL fInvadeProcess);
	DWORD (__stdcall *SymSetOptions)(DWORD SymOptions);
	DWORD (__stdcall *UnDecorateSymbolName)(PCTSTR DecoratedName, PTSTR UnDecoratedName, DWORD UndecoratedLength, DWORD Flags);

	BOOL (__stdcall *SymEnumTypes)(HANDLE hProcess, ULONG64 BaseOfDll, PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback, PVOID UserContext);
	BOOL (__stdcall *SymEnumSymbols)(HANDLE hProcess, ULONG64 BaseOfDll, PCTSTR Mask, PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback, const PVOID UserContext);
	BOOL (__stdcall *SymGetTypeInfo)(HANDLE hProcess, DWORD64 ModBase, ULONG TypeId, IMAGEHLP_SYMBOL_TYPE_INFO GetType, PVOID pInfo);

	PIMAGE_NT_HEADERS (__stdcall *ImageNtHeader)(PVOID ImageBase);

	BOOL (__stdcall *MiniDumpWriteDump)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

	inline BOOL CallStackWalk64(HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord) const { return StackWalk64(GetImageType(), GetProcess(), hThread, StackFrame, ContextRecord, 0, SymFunctionTableAccess64, SymGetModuleBase64, 0); }

	// Same as CallStackWalk64 above, but uses a hard-linked reference to 
	// ::StackWalk64. During static deinit, use of dbghelp API after 
	// _RTC_Terminate causes the application to immediately abort/close/exit and 
	// thus any subsequent code does not get executed. To avoid this behavior,
	// dbghelp should be hard-linked. The same oWinDbgHelp Singleton() API can be 
	// used and it will evaluate correctly, but of course the library will not be 
	// hard-linked unless at least one symbol is referred to by the source, so in
	// that use, use this one function.
	inline BOOL CallStackWalk64_HARDLINKED(HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord) const { return ::StackWalk64(GetImageType(), GetProcess(), hThread, StackFrame, ContextRecord, 0, SymFunctionTableAccess64, SymGetModuleBase64, 0); }

	inline BOOL CallSymGetModuleInfo64(DWORD64 dwAddr, PIMAGEHLP_MODULE64 ModuleInfo) const { return SymGetModuleInfo64(GetProcess(), dwAddr, ModuleInfo); }
	inline BOOL CallSymGetSymFromAddr64(DWORD64 Address, PDWORD64 Displacement, PIMAGEHLP_SYMBOL64 Symbol) const { return SymGetSymFromAddr64(GetProcess(), Address, Displacement, Symbol); }
	inline BOOL CallSymGetLineFromAddr64(DWORD64 dwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line) const { return SymGetLineFromAddr64(GetProcess(), dwAddr, pdwDisplacement, Line); }

	static DWORD GetImageType();
	HANDLE GetProcess() const;
	const char* GetSymbolPath() const;
	const char* GetSymbolSearchPath() const;

	ModuleLoadedHandler GetModuleLoadedHandler() const;
	void SetModuleLoadedHandler(ModuleLoadedHandler _Handler);

	static const oGUID GUID;

protected:
	HANDLE hProcess;
	oHMODULE hDbgHelp;
	ModuleLoadedHandler Handler;
	char SymbolPath[_MAX_PATH];
	char SymbolSearchPath[_MAX_PATH];

	static oWinDbgHelp* sInstanceForDeferredRelease;
	static void AtExit();
	void Link(const char* _pPath);
};

#endif
