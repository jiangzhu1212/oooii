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
// Helper macros to reduce the boilerplate necessary to soft link exported DLL 
// functions. This also enforces consistency and standards in reporting and 
// centralizes where changes might need to be made. This should only be used by 
// the DLL singleton wrappers in this folder.
#pragma once
#ifndef oWinSoftLinkCommon_h
#define oWinSoftLinkCommon_h

#include <oPlatform/oModule.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/Windows/oWindows.h>

// List all Windows API as <retval> (__stdcall *WinAPIFunction)(<params>);
// between calls to oDECLARE_DLL_SINGLETON_BEGIN and oDECLARE_DLL_SINGLETON_END
// and it will create a class with a Singleton() interface from which those
// functions can be called.

#define oDECLARE_DLL_SINGLETON_BEGIN(_ClassName) \
	struct _ClassName : oProcessSingleton<_ClassName> \
	{	static const oGUID GUID; \
		_ClassName(); \
		~_ClassName() { oModuleUnlink(hModule); }

#define oDECLARE_DLL_SINGLETON_END() protected: oHMODULE hModule; };

// Define this in an associated .cpp files to implement the details of loading
// and binding the softlinked functions/procedures. NOTE: There must be an array 
// of strings for each proc name IN THE SAME ORDER as the functions declared. 
// The name of the array must be "static const char* dll_procs[] = {...};". 
// _FirstProc is the first procedure declared in the class so it's function 
// pointer's address can be taken to fill in all the pointers.
#define oDEFINE_DLL_SINGLETON_CTOR(_ClassName, _StrModuleName, _FirstProc) _ClassName::_ClassName() { hModule = oModuleLinkSafe(_StrModuleName, dll_procs, (void**)&_FirstProc, oCOUNTOF(dll_procs)); }

#endif
