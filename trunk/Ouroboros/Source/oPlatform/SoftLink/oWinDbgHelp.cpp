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
#include <string.h>
#include <oBasis/oStddef.h>
#include "oWinDbgHelp.h"

#ifndef _DEBUG
	#pragma optimize("", off)
	#pragma warning(disable:4748) // can not protect parameters and local variables from local buffer overrun because optimizations are disabled in function
#endif

#if 0
	#define oWinDbgHelp_SymCleanup(_hProcess) \
	if (SymCleanup) \
	{	OutputDebugStringA("--- If this is the last line of singleton deinit, it means we're calling SymCleanup and it's prematurely/blindly exiting the app. ---\n"); \
		SymCleanup(_hProcess); \
		OutputDebugStringA("--- If you're reading this, there's probably more to come and we've solved calling oWinDbgHelp before _RTC_Terminate(). ---\n"); \
	}
#else
	#define oWinDbgHelp_SymCleanup(_hProcess) SymCleanup(_hProcess)
#endif

oWinDbgHelp* oWinDbgHelp::sInstanceForDeferredRelease = nullptr;

namespace detail {

static const char* dbghelp_dll_Functions[] = 
{
	"EnumerateLoadedModules64",
	"StackWalk64",
	"SymCleanup",
	"SymFunctionTableAccess64",
	"SymGetLineFromAddr64",
	"SymGetModuleBase64",
	"SymGetModuleInfo64",
	"SymGetOptions",
	"SymGetSearchPath",
	"SymGetSymFromAddr64",
	"SymLoadModule64",
	"SymInitialize",
	"SymSetOptions",
	"UnDecorateSymbolName",
	"SymEnumTypes",
	"SymEnumSymbols",
	"SymGetTypeInfo",
	"ImageNtHeader",
	"MiniDumpWriteDump",
};

bool Exists(const char* _Path)
{
	return GetFileAttributesA(_Path) != INVALID_FILE_ATTRIBUTES;
}

bool GetSDKPath(char* _Path, size_t _SizeofPath, const char* _SDKRelativePath)
{
	bool result = false;
	DWORD len = GetEnvironmentVariableA("ProgramFiles", _Path, static_cast<DWORD>(_SizeofPath));
	if (len && len < _SizeofPath)
	{
		oStrcat(_Path, _SizeofPath, _SDKRelativePath);
		result = Exists(_Path);
	}

	return result;
}

template<size_t size> inline BOOL GetSDKPath(char (&_Path)[size], const char* _SDKRelativePath) { return GetSDKPath(_Path, size, _SDKRelativePath); }

} // namespace detail

BOOL CALLBACK LoadModule(PCSTR ModuleName, DWORD64 ModuleBase, ULONG ModuleSize, PVOID UserContext)
{
	const oWinDbgHelp* d = static_cast<const oWinDbgHelp*>(UserContext);
	bool success = !!d->SymLoadModule64(d->GetProcess(), 0, ModuleName, 0, ModuleBase, ModuleSize);
	if (d->GetModuleLoadedHandler())
		(*d->GetModuleLoadedHandler())(ModuleName, success);
	return success;
}

// {FF2D872A-D8C3-48EC-9096-36A7E2CE15D7}
const oGUID oWinDbgHelp::GUID = { 0xff2d872a, 0xd8c3, 0x48ec, { 0x90, 0x96, 0x36, 0xa7, 0xe2, 0xce, 0x15, 0xd7 } };
oSINGLETON_REGISTER(oWinDbgHelp);

oWinDbgHelp::oWinDbgHelp(HANDLE _hProcess, const char* _SymbolPath, ModuleLoadedHandler _Handler)
	: hProcess(_hProcess ? _hProcess : GetCurrentProcess())
	, hDbgHelp(0)
	, Handler(_Handler)
	, oProcessSingleton<oWinDbgHelp>(2) // keep an extra reference to ourselves and use atexit which orders static deinit a bit better than the static oRef that will be receiving this newly created object
{
	sInstanceForDeferredRelease = this;
	atexit(AtExit);

	bool success = false;
		
	char path[1024];
	GetModuleFileNameA(0, path, oCOUNTOF(path));
	if (!GetLastError())
	{
		// first local override
		oStrcat(path, ".local");
		if (!detail::Exists(path))
		{
			// then for an installed version (32/64-bit)
			if (detail::GetSDKPath(path, "/Debugging Tools for Windows/dbghelp.dll") && detail::Exists(path))
				Link(path);

			if (!hDbgHelp && detail::GetSDKPath(path, "/Debugging Tools for Windows 64-Bit/dbghelp.dll") && detail::Exists(path))
				Link(path);
		}
	}

	// else punt to wherever the system can find it
	if (!hDbgHelp)
		Link("dbghelp.dll");
	
	if (hDbgHelp)
	{
		*SymbolPath = 0;
		*SymbolSearchPath = 0;

		if (SymInitialize(hProcess, _SymbolPath, FALSE))
		{
			SymSetOptions(SymGetOptions() | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_FAIL_CRITICAL_ERRORS);
			SymGetSearchPath(hProcess, SymbolSearchPath, oCOUNTOF(SymbolSearchPath));

			EnumerateLoadedModules64(hProcess, &LoadModule, const_cast<oWinDbgHelp*>(this));
			success = !!GetProcess();
		}

		else
			success = false;
	}

	// Don't use assert, because assert depends on debugger, which depends on this,
	// so using assert would create a cyclic reference.
	oCRTASSERT(success, "Failed to initialize oWinDbgHelp. You'll probably get an access violation soon when calling a DbgHelp API.");
}

void oWinDbgHelp::Link( const char* _pPath )
{
	// Because this library needs to be loaded *VERY* early, don't rely even on
	// the load-and-do-it oModuleLink
	hDbgHelp = (oHMODULE)LoadLibrary(_pPath);
	oModuleLink(hDbgHelp, detail::dbghelp_dll_Functions, (void**)&EnumerateLoadedModules64, oCOUNTOF(detail::dbghelp_dll_Functions));
}

oWinDbgHelp::~oWinDbgHelp()
{
	oWinDbgHelp_SymCleanup(hProcess);
	if (hDbgHelp)
		FreeLibrary((HMODULE)hDbgHelp);
}

void oWinDbgHelp::AtExit()
{
	sInstanceForDeferredRelease->Release();
}

DWORD oWinDbgHelp::GetImageType()
{
	#ifdef _M_IX86
		return IMAGE_FILE_MACHINE_I386;
	#elif _M_X64
		return IMAGE_FILE_MACHINE_AMD64;
	#elif _M_IA64
		return IMAGE_FILE_MACHINE_IA64;
	#else
		#error Unsupported platform
	#endif
}

HANDLE oWinDbgHelp::GetProcess() const { return hProcess; }
const char* oWinDbgHelp::GetSymbolPath() const { return SymbolPath; }
const char* oWinDbgHelp::GetSymbolSearchPath() const { return SymbolSearchPath; }
oWinDbgHelp::ModuleLoadedHandler oWinDbgHelp::GetModuleLoadedHandler() const { return Handler; }
void oWinDbgHelp::SetModuleLoadedHandler(ModuleLoadedHandler _Handler) { Handler = _Handler; }

#pragma warning(default:4748) // an not protect parameters and local variables from local buffer overrun because optimizations are disabled in function

const char* oAsString(enum SymTagEnum _SymTagEnum)
{
	switch (_SymTagEnum) 
	{
		case SymTagNull: return "SymTagNull";
		case SymTagExe: return "SymTagExe";
		case SymTagCompiland: return "SymTagCompiland";
		case SymTagCompilandDetails: return "SymTagCompilandDetails";
		case SymTagCompilandEnv: return "SymTagCompilandEnv";
		case SymTagFunction: return "SymTagFunction";
		case SymTagBlock: return "SymTagBlock";
		case SymTagData: return "SymTagData";
		case SymTagAnnotation: return "SymTagAnnotation";
		case SymTagLabel: return "SymTagLabel";
		case SymTagPublicSymbol: return "SymTagPublicSymbol";
		case SymTagUDT: return "SymTagUDT";
		case SymTagEnum: return "SymTagEnum";
		case SymTagFunctionType: return "SymTagFunctionType";
		case SymTagPointerType: return "SymTagPointerType";
		case SymTagArrayType: return "SymTagArrayType";
		case SymTagBaseType: return "SymTagBaseType";
		case SymTagTypedef:return "SymTagTypedef";
		case SymTagBaseClass: return "SymTagBaseClass";
		case SymTagFriend: return "SymTagFriend";
		case SymTagFunctionArgType: return "SymTagFunctionArgType";
		case SymTagFuncDebugStart: return "SymTagFuncDebugStart";
		case SymTagFuncDebugEnd: return "SymTagFuncDebugEnd";
		case SymTagUsingNamespace: return "SymTagUsingNamespace";
		case SymTagVTableShape: return "SymTagVTableShape";
		case SymTagVTable: return "SymTagVTable";
		case SymTagCustom: return "SymTagCustom";
		case SymTagThunk: return "SymTagThunk";
		case SymTagCustomType: return "SymTagCustomType";
		case SymTagManagedType: return "SymTagManagedType";
		case SymTagDimension: return "SymTagDimension";
		default: break;
	}
	
	return "Unknown";
}
