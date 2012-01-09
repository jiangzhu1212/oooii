// $(header)

// Functions for handling loading and linking as well as identification of code
// in a multi-module system (.dll's or .so's).
#pragma once
#ifndef oModule_h
#define oModule_h

#include <oPlatform/oStddef.h>

oDECLARE_HANDLE(oHMODULE);

// Give this a lib to load, and a list of strings of functions to find, and this
// function will return a handle to the lib and a list of resolved interfaces.
// If _ModuleName is specified as a filebase only, this function will prepend 
// the app path and append a build suffix. If an extension is included as part 
// of the (i.e. "dbghelp.dll") then this will pass _ModuleName through to the
// platform calls unmodified.
oHMODULE oModuleLink(const char* _ModuleName, const char** _InterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces);
template<size_t size> inline oHMODULE oModuleLink(const char* _ModuleName, const char* (&_InterfaceFunctionNames)[size], void** _ppInterfaces) { return oModuleLink(_ModuleName, _InterfaceFunctionNames, _ppInterfaces, size);}

// Ensure ALL interfaces/memory/pointers from the module are unloaded by this 
// point, otherwise you'll get what looks like a valid object, but a bad virtual 
// dereference or similar dangling pointer problem because the actual code/data
// segments are no longer loaded.
void oModuleUnlink(oHMODULE _hModule);

// Returns the handle of the module from which this function is called, be it a 
// dynamically loaded module or the static/main exe module.
oHMODULE oModuleGetCurrent();

// Fill the specified string with the name of the specified module. If this 
// fails the function returns false; use oErrorGetLast() for more details
bool oModuleGetName(char* _StrDestination, size_t _SizeofStrDestination, oHMODULE _hModule);
template<size_t size> inline bool oModuleGetName(char (&_StrDestination)[size], oHMODULE _hModule) { return oModuleGetName(_StrDestination, size, _hModule); }

#endif
