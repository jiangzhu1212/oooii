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
#include <oPlatform/oModule.h>
#include <oBasis/oAssert.h>
#include <oBasis/oError.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/Windows/oWindows.h>
#include <oPlatform/oProcessHeap.h>
#include "SoftLink/oWinVersion.h"

bool oModuleLink(oHMODULE _hModule, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces)
{
	memset(_ppInterfaces, 0, sizeof(void*) * _CountofInterfaces);
	bool allInterfacesAcquired = true;
	for (unsigned int i = 0; i < _CountofInterfaces; i++)
	{
		_ppInterfaces[i] = GetProcAddress((HMODULE)_hModule, _pInterfaceFunctionNames[i]);
		if (!_ppInterfaces[i])
		{
			oTRACE("Can't find ::%s", _pInterfaceFunctionNames[i]);
			allInterfacesAcquired = false;
		}
	}

	return allInterfacesAcquired;
}
oHMODULE oModuleLink(const char* _ModuleName, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces)
{
	oHMODULE hModule = 0;
	
	hModule = (oHMODULE)oThreadsafeLoadLibrary(_ModuleName);
	if (!hModule)
	{
		oErrorSetLast(oERROR_IO, "The application has failed to start because %s was not found. Re-installing the application may fix the problem.", oSAFESTRN(_ModuleName));
		return hModule;
	}

	if( !oModuleLink(hModule, _pInterfaceFunctionNames, _ppInterfaces, _CountofInterfaces) )
	{
		oErrorSetLast(oERROR_NOT_FOUND, "Could not create all interfaces from %s. This might be because the DLLs are out of date, try copying a newer version of the DLL into the bin dir for this application.", _ModuleName);
		oThreadsafeFreeLibrary((HMODULE)hModule);
		return 0;
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
		oPrintf(buf, "%s - Unable To Locate Component", oGetFilebase(path));
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

char* oModuleGetName(char* _StrDestination, size_t _SizeofStrDestination, oHMODULE _hModule)
{
	if (!_hModule)
		_hModule = oModuleGetCurrent();

	size_t length = static_cast<size_t>(GetModuleFileNameA((HMODULE)_hModule, _StrDestination, oUInt(_SizeofStrDestination)));
	if (length+1 == _SizeofStrDestination && GetLastError())
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return nullptr;
	}
	
	return _StrDestination;
}

const char* oAsString(const oMODULE_TYPE& _Type)
{
	switch (_Type)
	{
		case oMODULE_UNKNOWN: return "Unknown";
		case oMODULE_APP: return "Application";
		case oMODULE_DLL: return "Application Extension";
		case oMODULE_LIB: return "Object File Library";
		case oMODULE_FONT_UNKNOWN: return "Unknown font file";
		case oMODULE_FONT_RASTER: return "Raster font file";
		case oMODULE_FONT_TRUETYPE: return "TrueType font file";
		case oMODULE_FONT_VECTOR: return "Vector font file";
		case oMODULE_VIRTUAL_DEVICE: return "Virtual Device";
		case oMODULE_DRV_UNKNOWN: return "Unknown driver";
		case oMODULE_DRV_COMM: return "COMM driver";
		case oMODULE_DRV_DISPLAY: return "Display driver";
		case oMODULE_DRV_INSTALLABLE: return "Installable driver";
		case oMODULE_DRV_KEYBOARD: return "Keyboard driver";
		case oMODULE_DRV_LANGUAGE: return "Language driver";
		case oMODULE_DRV_MOUSE: return "Mouse driver";
		case oMODULE_DRV_NETWORK: return "Network driver";
		case oMODULE_DRV_PRINTER: return "Printer driver";
		case oMODULE_DRV_SOUND: return "Sound driver";
		case oMODULE_DRV_SYSTEM: return "System driver";
		oNODEFAULT;
	}
}

static oMODULE_TYPE oGetType(const VS_FIXEDFILEINFO& _FFI)
{
	switch (_FFI.dwFileType)
	{
	case VFT_UNKNOWN: return oMODULE_UNKNOWN;
	case VFT_APP: return oMODULE_APP;
	case VFT_DLL: return oMODULE_DLL;
	case VFT_STATIC_LIB: return oMODULE_LIB;
	case VFT_VXD: return oMODULE_VIRTUAL_DEVICE;
	case VFT_FONT:
		{
			switch (_FFI.dwFileSubtype)
			{
			case VFT2_FONT_RASTER: return oMODULE_FONT_RASTER;
			case VFT2_FONT_VECTOR: return oMODULE_FONT_VECTOR;
			case VFT2_FONT_TRUETYPE: return oMODULE_FONT_TRUETYPE;
			case VFT2_UNKNOWN:
			default: return oMODULE_FONT_UNKNOWN;
			}
		}

	case VFT_DRV:
		{
			switch (_FFI.dwFileSubtype)
			{
			case VFT2_DRV_KEYBOARD: return oMODULE_DRV_KEYBOARD;
			case VFT2_DRV_LANGUAGE: return oMODULE_DRV_LANGUAGE;
			case VFT2_DRV_DISPLAY: return oMODULE_DRV_DISPLAY;
			case VFT2_DRV_MOUSE: return oMODULE_DRV_MOUSE;
			case VFT2_DRV_NETWORK: return oMODULE_DRV_NETWORK;
			case VFT2_DRV_SYSTEM: return oMODULE_DRV_SYSTEM;
			case VFT2_DRV_INSTALLABLE: return oMODULE_DRV_INSTALLABLE;
			case VFT2_DRV_SOUND: return oMODULE_DRV_SOUND;
			case VFT2_DRV_COMM: return oMODULE_DRV_COMM;
				//case VFT2_DRV_INPUTMETHOD: return oMODULE_DRV_?;
			case VFT2_DRV_PRINTER:
			case VFT2_DRV_VERSIONED_PRINTER: return oMODULE_DRV_PRINTER;
			case VFT2_UNKNOWN: 
			default: return oMODULE_DRV_UNKNOWN;
			}
		}
		oNODEFAULT;
	}
}

bool oModuleGetDesc(oHMODULE _hModule, oMODULE_DESC* _pDesc)
{
	oStringPath ModulePath;
	if (!oModuleGetName(ModulePath, _hModule))
		return false; // pass through error

	return oModuleGetDesc(ModulePath, _pDesc);
}

bool oModuleGetDesc(const char* _ModulePath, oMODULE_DESC* _pDesc)
{
	DWORD hFVI = 0;
	DWORD FVISize = oWinVersion::Singleton()->GetFileVersionInfoSizeA(_ModulePath, &hFVI);
	if (!FVISize)
		return oWinSetLastError();

	std::vector<char> buf;
	buf.resize(FVISize);
	void* pData = oGetData(buf);

	if (!oWinVersion::Singleton()->GetFileVersionInfoA(_ModulePath, hFVI, FVISize, pData))
		return oWinSetLastError();

	// http://msdn.microsoft.com/en-us/library/ms647003(VS.85).aspx
	// Based on a comment that questions the reliablility of the return value
	if (GetLastError() != S_OK)
		return oWinSetLastError();

	// _____________________________________________________________________________
	// Get the basics from VS_FIXEDFILEINFO

	VS_FIXEDFILEINFO* pFFI = nullptr;
	UINT FFISize = 0;
	if (!oWinVersion::Singleton()->VerQueryValueA(pData, "\\", (LPVOID*)&pFFI, &FFISize))
		return oWinSetLastError();

	_pDesc->FileVersion = oWinGetVersion(pFFI->dwFileVersionMS, pFFI->dwFileVersionLS);
	_pDesc->ProductVersion = oWinGetVersion(pFFI->dwProductVersionMS, pFFI->dwProductVersionLS);
	_pDesc->Type = oGetType(*pFFI);
	_pDesc->IsDebugBuild = (pFFI->dwFileFlags & VS_FF_DEBUG) == VS_FF_DEBUG;
	_pDesc->IsPrereleaseBuild = (pFFI->dwFileFlags & VS_FF_PRERELEASE) == VS_FF_PRERELEASE;
	_pDesc->IsPatchedBuild = (pFFI->dwFileFlags & VS_FF_PATCHED) == VS_FF_PATCHED;
	_pDesc->IsPrivateBuild = (pFFI->dwFileFlags & VS_FF_PRIVATEBUILD) == VS_FF_PRIVATEBUILD;
	_pDesc->IsSpecialBuild = (pFFI->dwFileFlags & VS_FF_SPECIALBUILD) == VS_FF_SPECIALBUILD;

	// _____________________________________________________________________________
	// Now do some of the more complicated ones

	struct LANGANDCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;
	};

	LANGANDCODEPAGE* pLCP = nullptr;
	UINT LCPSize = 0;
	if (!oWinVersion::Singleton()->VerQueryValueA(pData, "\\VarFileInfo\\Translation", (LPVOID*)&pLCP, &LCPSize))
		return oWinSetLastError();

	UINT nLanguages = LCPSize / sizeof(LANGANDCODEPAGE);
	oASSERT(nLanguages == 1, "There are %u languages in the file. Currently we assume 1: English", nLanguages);

	struct MAPPING
	{
		const char* Key;
		oStringM* Dest;
		bool Required;
	};

	MAPPING m[] = 
	{
		{ "CompanyName", &_pDesc->CompanyName, true },
		{ "FileDescription", &_pDesc->Description, true },
		{ "ProductName", &_pDesc->ProductName, true },
		{ "LegalCopyright", &_pDesc->Copyright, true },
		{ "OriginalFilename", &_pDesc->OriginalFilename, true },
		{ "Comments", &_pDesc->Comments, false },
		{ "PrivateBuild", &_pDesc->PrivateBuild, false },
		{ "SpecialBuild", &_pDesc->SpecialBuild, false },
	};

	for (size_t i = 0; i < oCOUNTOF(m); i++)
	{
		oStringM Key;
		oPrintf(Key, "\\StringFileInfo\\%04x%04x\\%s", pLCP[0].wLanguage, pLCP[0].wCodePage, m[i].Key);

		char* pVal = nullptr;
		UINT ValLen = 0;
		*m[i].Dest = "";
		if (!oWinVersion::Singleton()->VerQueryValueA(pData, Key, (LPVOID*)&pVal, &ValLen) && m[i].Required)
			return oWinSetLastError();
		*m[i].Dest = pVal;
	}

	return true;
}

