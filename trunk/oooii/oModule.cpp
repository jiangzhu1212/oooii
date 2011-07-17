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
#include <oooii/oModule.h>
#include <oooii/oAssert.h>
#include <oooii/oPath.h>
#include <oooii/oSize.h>
#include <oooii/oString.h>
#include <oooii/oWindows.h>
#include <oooii/oProcessHeap.h>

oHMODULE oModule::Link(const char* _ModuleName, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces)
{
	oHMODULE hModule = 0;
	#ifdef _DEBUG
		const char* BUILDSUFFIX = "D";
	#else
		const char* BUILDSUFFIX = "";
	#endif

	char dllPath[_MAX_PATH];

	const char* ext = oGetFileExtension(_ModuleName);
	if (!strcmp(".dll", ext))
		strcpy_s(dllPath, _ModuleName);
	else
	{
		oGetSysPath(dllPath, oSYSPATH_APP);
		sprintf_s(dllPath, "%s%s%s.dll", dllPath, _ModuleName, BUILDSUFFIX);
	}
	
	hModule = oSafeLoadLibrary(dllPath);

	memset(_ppInterfaces, 0, sizeof(void*) * _CountofInterfaces);

	bool allInterfacesAcquired = false;
	if (hModule)
	{
		allInterfacesAcquired = true;
		for (unsigned int i = 0; i < _CountofInterfaces; i++)
		{
			_ppInterfaces[i] = GetProcAddress((HMODULE)hModule, _pInterfaceFunctionNames[i]);
			if (!_ppInterfaces[i])
			{
				oTRACE("Can't find %s::%s", _ModuleName, _pInterfaceFunctionNames[i]);
				allInterfacesAcquired = false;
			}
		}
	}

	else
		oASSERT(false, "Could not load %s", dllPath);

	if (hModule && !allInterfacesAcquired)
	{
		oASSERT(false, "Could not create all interfaces from %s. This might be because the DLLs are out of date, try copying newer versions into the bin dir for this application.", dllPath);
		oSafeFreeLibrary(hModule);
		hModule = 0;
	}

	return hModule;
}

void oModule::Unlink(oHMODULE _hModule)
{
	if (_hModule)
		oSafeFreeLibrary(_hModule);
}

oHMODULE oModule::GetCurrent()
{
	// static is unique per module, so copies and thus unique addresses live in 
	// each module.
	static HMODULE hCurrentModule = 0;
	if (!hCurrentModule)
		hCurrentModule = oGetModule(&hCurrentModule);
	return (oHMODULE)hCurrentModule;
}

bool oModule::GetModuleName(char* _StrDestination, size_t _SizeofStrDestination, oHMODULE _hModule)
{
	size_t length = static_cast<size_t>(GetModuleFileNameA((HMODULE)_hModule, _StrDestination, oSize32(_SizeofStrDestination)));
	if (length+1 == _SizeofStrDestination && GetLastError())
	{
		oSetLastError(EINVAL);
		return false;
	}
	
	return true;
}
 
 oHMODULE oModule::oSafeLoadLibrary(const char *_LibraryPath)
 {
	 oProcessHeap::ScopedLock lock;
	 return (oHMODULE)LoadLibraryA(_LibraryPath);
 }

 void oModule::oSafeFreeLibrary(oHMODULE _module)
 {
	 oProcessHeap::ScopedLock lock;
	 FreeLibrary((HMODULE)_module);
 }