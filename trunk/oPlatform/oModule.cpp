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
#include <oPlatform/oModule.h>
#include <oBasis/oAssert.h>
#include <oBasis/oError.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oWindows.h>
#include <oPlatform/oProcessHeap.h>

oHMODULE oModuleLink(const char* _ModuleName, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces)
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
		oSystemGetPath(dllPath, oSYSPATH_APP);
		sprintf_s(dllPath, "%s%s%s.dll", dllPath, _ModuleName, BUILDSUFFIX);
	}
	
	hModule = (oHMODULE)oThreadsafeLoadLibrary(dllPath);
	if (!hModule)
	{
		oErrorSetLast(oERROR_IO, "The application has failed to start because %s was not found. Re-installing the application may fix the problem.", oSAFESTRN(dllPath));
		return hModule;
	}

	memset(_ppInterfaces, 0, sizeof(void*) * _CountofInterfaces);
	bool allInterfacesAcquired = true;
	for (unsigned int i = 0; i < _CountofInterfaces; i++)
	{
		_ppInterfaces[i] = GetProcAddress((HMODULE)hModule, _pInterfaceFunctionNames[i]);
		if (!_ppInterfaces[i])
		{
			oTRACE("Can't find %s::%s", _ModuleName, _pInterfaceFunctionNames[i]);
			allInterfacesAcquired = false;
		}
	}

	if (hModule && !allInterfacesAcquired)
	{
		oErrorSetLast(oERROR_NOT_FOUND, "Could not create all interfaces from %s. This might be because the DLLs are out of date, try copying a newer version of the DLL into the bin dir for this application.", dllPath);
		oThreadsafeFreeLibrary((HMODULE)hModule);
		hModule = nullptr;
	}

	return hModule;
}

oHMODULE oModuleLinkSafe(const char* _ModuleName, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces)
{
	oHMODULE hModule = oModuleLink(_ModuleName, _pInterfaceFunctionNames, _ppInterfaces, _CountofInterfaces);
	if (!hModule)
	{
		oMSGBOX_DESC d;
		d.Type = oMSGBOX_ERR;
		d.TimeoutMS = oInfiniteWait;
		d.ParentNativeHandle = nullptr;
		char path[_MAX_PATH];
		oSystemGetPath(path, oSYSPATH_APP_FULL);
		char buf[_MAX_PATH];
		sprintf_s(buf, "%s - Unable To Locate Component", oGetFilebase(path));
		d.Title = buf;
		oMsgBox(d, "%s", oErrorGetLastString());
		std::terminate();
	}

	return hModule;
}

void oModuleUnlink(oHMODULE _hModule)
{
	if (_hModule)
		oThreadsafeFreeLibrary((HMODULE)_hModule);
}

oHMODULE oModuleGetCurrent()
{
	// static is unique per module, so copies and thus unique addresses live in 
	// each module.
	static HMODULE hCurrentModule = 0;
	if (!hCurrentModule)
		hCurrentModule = oGetModule(&hCurrentModule);
	return (oHMODULE)hCurrentModule;
}

bool oModuleGetName(char* _StrDestination, size_t _SizeofStrDestination, oHMODULE _hModule)
{
	size_t length = static_cast<size_t>(GetModuleFileNameA((HMODULE)_hModule, _StrDestination, oUInt(_SizeofStrDestination)));
	if (length+1 == _SizeofStrDestination && GetLastError())
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}
	
	return true;
}
 