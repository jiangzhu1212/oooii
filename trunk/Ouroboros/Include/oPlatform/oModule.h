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

// Functions for handling loading and linking as well as identification of code
// in a multi-module system (.dll's or .so's).
#pragma once
#ifndef oModule_h
#define oModule_h

#include <oBasis/oStddef.h>
#include <oStd/fixed_string.h>
#include <oBasis/oVersion.h>

oDECLARE_HANDLE(oHMODULE);

enum oMODULE_TYPE
{
	oMODULE_UNKNOWN,
	oMODULE_APP,
	oMODULE_DLL,
	oMODULE_LIB,
	oMODULE_FONT_UNKNOWN,
	oMODULE_FONT_RASTER,
	oMODULE_FONT_TRUETYPE,
	oMODULE_FONT_VECTOR,
	oMODULE_VIRTUAL_DEVICE,
	oMODULE_DRV_UNKNOWN,
	oMODULE_DRV_COMM,
	oMODULE_DRV_DISPLAY,
	oMODULE_DRV_INSTALLABLE,
	oMODULE_DRV_KEYBOARD,
	oMODULE_DRV_LANGUAGE,
	oMODULE_DRV_MOUSE,
	oMODULE_DRV_NETWORK,
	oMODULE_DRV_PRINTER,
	oMODULE_DRV_SOUND,
	oMODULE_DRV_SYSTEM,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oMODULE_TYPE)

struct oMODULE_DESC
{
	oMODULE_DESC()
		: Type(oMODULE_UNKNOWN)
		, IsDebugBuild(false)
		, IsPrereleaseBuild(false)
		, IsPatchedBuild(false)
		, IsPrivateBuild(false)
		, IsSpecialBuild(false)
	{}

	oStd::mstring CompanyName;
	oStd::mstring Description;
	oStd::mstring ProductName;
	oStd::mstring Copyright;
	oStd::mstring OriginalFilename;
	oStd::mstring Comments;
	oStd::mstring PrivateBuild;
	oStd::mstring SpecialBuild;
	oVersion FileVersion;
	oVersion ProductVersion;
	oMODULE_TYPE Type;
	bool IsDebugBuild;
	bool IsPrereleaseBuild;
	bool IsPatchedBuild;
	bool IsPrivateBuild;
	bool IsSpecialBuild;
};

// Give this a lib to load, and a list of strings of functions to find, and this
// function will return a handle to the lib and a list of resolved interfaces.
// If _ModuleName is specified as a filebase only, this function will prepend 
// the app path and append a build suffix. If an extension is included as part 
// of the (i.e. "dbghelp.dll") then this will pass _ModuleName through to the
// platform calls unmodified.
bool oModuleLink(oHMODULE _hModule, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces);
oHMODULE oModuleLink(const char* _ModuleName, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces);
template<size_t size> oHMODULE oModuleLink(const char* _ModuleName, const char* (&_InterfaceFunctionNames)[size], void** _ppInterfaces) { return oModuleLink(_ModuleName, _InterfaceFunctionNames, _ppInterfaces, size);}

// Calls oModuleLink, then checks for error, presenting a dialog box informing
// the user that the DLL or functions are missing and then std::terminates.
oHMODULE oModuleLinkSafe(const char* _ModuleName, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces);
template<size_t size> oHMODULE oModuleLinkSafe(const char* _ModuleName, const char* (&_InterfaceFunctionNames)[size], void** _ppInterfaces) { return oModuleLinkSafe(_ModuleName, _InterfaceFunctionNames, _ppInterfaces, size);}

// Ensure ALL interfaces/memory/pointers from the module are unloaded by this 
// point, otherwise you'll get what looks like a valid object, but a bad virtual 
// dereference or similar dangling pointer problem because the actual code/data
// segments are no longer loaded.
void oModuleUnlink(oHMODULE _hModule);

// Returns the handle of the module from which this function is called, be it a 
// dynamically loaded module or the static/main exe module.
oHMODULE oModuleGetCurrent();

// Fill the specified string with the name of the specified module. If this 
// fails the function returns false; use oErrorGetLast() for more details. If 
// _hModule is zero, then the current module will be used.
char* oModuleGetName(char* _StrDestination, size_t _SizeofStrDestination, oHMODULE _hModule = 0);
template<size_t size> char* oModuleGetName(char (&_StrDestination)[size], oHMODULE _hModule = 0) { return oModuleGetName(_StrDestination, size, _hModule); }
template<size_t capacity> char* oModuleGetName(oStd::fixed_string<char, capacity>& _StrDestination, oHMODULE _hModule = 0) { return oModuleGetName(_StrDestination, _StrDestination.capacity(), _hModule); }

// Returns information about the module, if available.
bool oModuleGetDesc(oHMODULE _hModule, oMODULE_DESC* _pDesc);
bool oModuleGetDesc(const char* _ModulePath, oMODULE_DESC* _pDesc);
inline bool oModuleGetDesc(oMODULE_DESC* _pDesc) { return oModuleGetDesc(oModuleGetCurrent(), _pDesc); }

#endif
