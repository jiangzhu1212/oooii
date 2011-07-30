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

// Functions for handling loading and linking as well as identification of code
// in a multi-module system (.dll's or .so's).
#pragma once
#ifndef oModule_h
#define oModule_h

#include <oooii/oStddef.h>

oDECLARE_HANDLE(oHMODULE);

namespace oModule {

// Give this a lib to load, and a list of strings of functions to find, and this
// function will return a handle to the lib and a list of resolved interfaces.
// If _ModuleName is specified as a filebase only, this function will prepend 
// the app path and append a build suffix. If an extension is included as part 
// of the (i.e. "dbghelp.dll") then this will pass _ModuleName through to the
// platform calls unmodified.
oHMODULE Link(const char* _ModuleName, const char** _InterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces);

// Ensure ALL interfaces/memory/pointers from the module are unloaded by this 
// point, otherwise you'll get what looks like a valid object, but a bad virtual 
// dereference or similar dangling pointer problem because the actual code/data
// segments are no longer loaded.
void Unlink(oHMODULE _hModule);

// Returns the handle of the module from which this function is called, be it a 
// dynamically loaded module or the static/main exe module.
oHMODULE GetCurrent();

// Fill the specified string with the name of the specified module. If this 
// fails the function returns false; use oGetLastError() for more details
bool GetModuleName(char* _StrDestination, size_t _SizeofStrDestination, oHMODULE _hModule);

template<size_t size> inline bool GetModuleName(char (&_StrDestination)[size], oHMODULE _hModule) { return GetModuleName(_StrDestination, size, _hModule); }

//Normally you should use Link instead of this. Never call win32 LoadLibrary directly though, as it holds an internal mutex,
//	and you can deadlock fairly easily if at least one call to LoadLibrary is for an oooii lib that will execute code (with a locks of its own) when loaded.
//	This safety isn't fool proof. Currently it only protects deadlocks associated with the process heap.
oHMODULE oSafeLoadLibrary(const char *_LibraryPath);
void oSafeFreeLibrary(oHMODULE _module);

} // namespace oModule

#endif
