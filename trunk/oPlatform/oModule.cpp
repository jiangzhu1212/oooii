// $(header)
#include <oPlatform/oModule.h>
#include <oBasis/oAssert.h>
#include <oBasis/oSize.h>
#include <oBasis/oError.h>
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
		oErrorSetLast(oERROR_NOT_FOUND, "Could not create all interfaces from %s. This might be because the DLLs are out of date, try copying newer versions into the bin dir for this application.", dllPath);
		oThreadsafeFreeLibrary((HMODULE)hModule);
		hModule = nullptr;
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
	size_t length = static_cast<size_t>(GetModuleFileNameA((HMODULE)_hModule, _StrDestination, oSize32(_SizeofStrDestination)));
	if (length+1 == _SizeofStrDestination && GetLastError())
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}
	
	return true;
}
 