/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oCore/module.h>
#include <oCore/filesystem.h>
#include <oCore/windows/win_error.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Dbghelp.h>

namespace ouro {

const char* as_string(const module::type::value& _Type)
{
	switch (_Type)
	{
		case module::type::unknown: return "unknown";
		case module::type::app: return "application";
		case module::type::dll: return "dll";
		case module::type::lib: return "library";
		case module::type::font_unknown: return "unknown font";
		case module::type::font_raster: return "raster font";
		case module::type::font_truetype: return "truetype font";
		case module::type::font_vector: return "vector font";
		case module::type::virtual_device: return "virtual device";
		case module::type::drv_unknown: return "unknown driver";
		case module::type::drv_comm: return "comm driver";
		case module::type::drv_display: return "display driver";
		case module::type::drv_installable: return "installable driver";
		case module::type::drv_keyboard: return "keyboard driver";
		case module::type::drv_language: return "language driver";
		case module::type::drv_mouse: return "mouse driver";
		case module::type::drv_network: return "network driver";
		case module::type::drv_printer: return "printer driver";
		case module::type::drv_sound: return "sound driver";
		case module::type::drv_system: return "system driver";
		default: break;
	}
	return "?";
}

	namespace module {

id open(const path& _Path)
{
	id mid;
	*(HMODULE*)&mid = LoadLibrary(_Path);
	return mid;
}

void close(id _ModuleID)
{
	if (_ModuleID)
		FreeLibrary(*(HMODULE*)&_ModuleID);
}

void* sym(id _ModuleID, const char* _SymbolName)
{
	return GetProcAddress(*(HMODULE*)&_ModuleID, _SymbolName);
}

void link(id _ModuleID, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces)
{
	memset(_ppInterfaces, 0, sizeof(void*) * _CountofInterfaces);
	for (size_t i = 0; i < _CountofInterfaces; i++)
	{
		_ppInterfaces[i] = sym(_ModuleID, _pInterfaceFunctionNames[i]);
		if (!_ppInterfaces[i])
			oTHROW(function_not_supported, "'%s' not found in '%s'", _pInterfaceFunctionNames[i], get_path(_ModuleID).c_str());
	}
}

id get_id(const void* _Symbol)
{
	id mid;
	oVB(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)_Symbol, (HMODULE*)&mid));
	return mid;
}

path get_path(id _ModuleID)
{
	static const char sLocalSymbol = 0;

	if (!_ModuleID)
		_ModuleID = get_id(&sLocalSymbol);

	path_string p;
	size_t len = static_cast<size_t>(GetModuleFileNameA(*(HMODULE*)&_ModuleID
		, p
		, static_cast<UINT>(p.capacity())));

	if (len+1 == p.capacity() && GetLastError())
		oTHROW0(no_buffer_space);

	return path(p);
}

static version get_version(DWORD _VersionMS, DWORD _VersionLS)
{
	return version(HIWORD(_VersionMS), LOWORD(_VersionMS), HIWORD(_VersionLS), LOWORD(_VersionLS));
}

static type::value get_type(const VS_FIXEDFILEINFO& _FFI)
{
	switch (_FFI.dwFileType)
	{
		case VFT_UNKNOWN: return type::unknown;
		case VFT_APP: return type::app;
		case VFT_DLL: return type::dll;
		case VFT_STATIC_LIB: return type::lib;
		case VFT_VXD: return type::virtual_device;
		case VFT_FONT:
		{
			switch (_FFI.dwFileSubtype)
			{
				case VFT2_FONT_RASTER: return type::font_raster;
				case VFT2_FONT_VECTOR: return type::font_vector;
				case VFT2_FONT_TRUETYPE: return type::font_truetype;
				case VFT2_UNKNOWN:
				default: break;
			}
			return type::font_unknown;
		}

		case VFT_DRV:
		{
			switch (_FFI.dwFileSubtype)
			{
				case VFT2_DRV_KEYBOARD: return type::drv_keyboard;
				case VFT2_DRV_LANGUAGE: return type::drv_language;
				case VFT2_DRV_DISPLAY: return type::drv_display;
				case VFT2_DRV_MOUSE: return type::drv_mouse;
				case VFT2_DRV_NETWORK: return type::drv_network;
				case VFT2_DRV_SYSTEM: return type::drv_system;
				case VFT2_DRV_INSTALLABLE: return type::drv_installable;
				case VFT2_DRV_SOUND: return type::drv_sound;
				case VFT2_DRV_COMM: return type::drv_comm;
				//case VFT2_DRV_INPUTMETHOD: return type::DRV_?;
				case VFT2_DRV_PRINTER:
				case VFT2_DRV_VERSIONED_PRINTER: return type::drv_printer;
				case VFT2_UNKNOWN: 
				default: break;
			}
			return type::drv_unknown;
		}
	}
	return type::unknown;
}

#if 0 // not needed yet
static void get_type(const module::type::value _Type, DWORD* _pType, DWORD* _pSubtype)
{
	*_pType = VFT_UNKNOWN;
	*_pSubtype = VFT2_UNKNOWN;

	switch (_Type)
	{
		case module::type::app: *_pType = VFT_APP; break;
		case module::type::dll: *_pType = VFT_DLL; break;
		case module::type::lib: *_pType = VFT_STATIC_LIB; break;
		case module::type::font_unknown: *_pType = VFT_FONT; break;
		case module::type::font_raster: *_pType = VFT_FONT; *_pSubtype = VFT2_FONT_RASTER; break;
		case module::type::font_truetype: *_pType = VFT_FONT; *_pSubtype = VFT2_FONT_TRUETYPE; break;
		case module::type::font_vector: *_pType = VFT_FONT; *_pSubtype = VFT2_FONT_VECTOR; break;
		case module::type::virtual_device: *_pType = VFT_VXD; break;
		case module::type::drv_unknown: *_pType = VFT_DRV; break;
		case module::type::drv_comm: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_COMM; break;
		case module::type::drv_display: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_DISPLAY; break;
		case module::type::drv_installable: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_INSTALLABLE; break;
		case module::type::drv_keyboard: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_KEYBOARD; break;
		case module::type::drv_language: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_LANGUAGE; break;
		case module::type::drv_mouse: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_MOUSE; break;
		case module::type::drv_network: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_NETWORK; break;
		case module::type::drv_printer: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_PRINTER; break;
		case module::type::drv_sound: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_SOUND; break;
		case module::type::drv_system: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_SYSTEM; break;
		default: break;
	}
}
#endif

static bool is_64bit()
{
	if (sizeof(void*) != 4) // If ptr size is larger than 32-bit we must be on 64-bit windows
		return true;

	// If ptr size is 4 bytes then we're a 32-bit process so check if we're running under
	// wow64 which would indicate that we're on a 64-bit system
	BOOL bWow64 = FALSE;
	IsWow64Process(GetCurrentProcess(), &bWow64);
	return !bWow64;
}

static bool is_64bit(const path& _Path)
{
	bool result = false;

	HMODULE hModule = LoadLibrary(_Path);
	finally Close([&] { FreeLibrary(hModule); });

	// can't map the 'this' module for access, so use another method
	if (GetModuleHandle(nullptr) == hModule)
		return is_64bit();

	unsigned long long size = filesystem::file_size(_Path);

	void* mapped = filesystem::map(_Path, filesystem::map_option::binary_read, 0, size);
	if (mapped)
	{
		IMAGE_NT_HEADERS* pHeader = ImageNtHeader(mapped);
		result = pHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64;
		filesystem::unmap(mapped);
	}

	return result;
}

info get_info(const path& _Path)
{
	DWORD hFVI = 0;
	DWORD FVISize = GetFileVersionInfoSizeA(_Path, &hFVI);
	if (!FVISize)
		throw windows::error();

	std::vector<char> buf;
	buf.resize(FVISize);
	void* pData = data(buf);

	oVB(GetFileVersionInfoA(_Path, hFVI, FVISize, pData));

	// http://msdn.microsoft.com/en-us/library/ms647003(VS.85).aspx
	// Based on a comment that questions the reliablility of the return value
	if (GetLastError() != S_OK)
		throw windows::error();

	// _____________________________________________________________________________
	// Get the basics from VS_FIXEDFILEINFO

	VS_FIXEDFILEINFO* pFFI = nullptr;
	UINT FFISize = 0;
	oVB(VerQueryValueA(pData, "\\", (LPVOID*)&pFFI, &FFISize));

	info i;
	i.version = get_version(pFFI->dwFileVersionMS, pFFI->dwFileVersionLS);
	i.type = get_type(*pFFI);
	i.is_debug = (pFFI->dwFileFlags & VS_FF_DEBUG) == VS_FF_DEBUG;
	i.is_prerelease = (pFFI->dwFileFlags & VS_FF_PRERELEASE) == VS_FF_PRERELEASE;
	i.is_patched = (pFFI->dwFileFlags & VS_FF_PATCHED) == VS_FF_PATCHED;
	i.is_private = (pFFI->dwFileFlags & VS_FF_PRIVATEBUILD) == VS_FF_PRIVATEBUILD;
	i.is_special = (pFFI->dwFileFlags & VS_FF_SPECIALBUILD) == VS_FF_SPECIALBUILD;
	i.is_64bit_binary = is_64bit(_Path);

	// _____________________________________________________________________________
	// Now do some of the more complicated ones

	struct LANGANDCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;
	};

	LANGANDCODEPAGE* pLCP = nullptr;
	UINT LCPSize = 0;
	oVB(VerQueryValueA(pData, "\\VarFileInfo\\Translation", (LPVOID*)&pLCP, &LCPSize));

	UINT nLanguages = LCPSize / sizeof(LANGANDCODEPAGE);
	
	if (nLanguages != 1)
		oTHROW(protocol_error, "There are %u languages in the file. Currently we assume 1: English", nLanguages);

	struct MAPPING
	{
		const char* Key;
		mstring* Dest;
		bool Required;
	};

	MAPPING m[] = 
	{
		{ "CompanyName", &i.company, true },
		{ "FileDescription", &i.description, true },
		{ "ProductName", &i.product_name, true },
		{ "LegalCopyright", &i.copyright, true },
		{ "OriginalFilename", &i.original_filename, true },
		{ "Comments", &i.comments, false },
		{ "PrivateBuild", &i.private_message, false },
		{ "SpecialBuild", &i.special_message, false },
	};

	oFORI(j, m)
	{
		mstring Key;
		snprintf(Key, "\\StringFileInfo\\%04x%04x\\%s", pLCP[0].wLanguage, pLCP[0].wCodePage, m[j].Key);
		mwstring WKey(Key);
		wchar_t* pVal = nullptr;
		UINT ValLen = 0;
		*m[j].Dest = "";
		// VerQueryValueA version seems bugged... it returns one char short of the
		// proper size.
		if (!VerQueryValueW(pData, WKey, (LPVOID*)&pVal, &ValLen) && m[j].Required)
			throw windows::error();
		*m[j].Dest = pVal;
	}

	return i;
}

info get_info(id _ModuleID)
{
	return get_info(get_path(_ModuleID));
}

	} // namespace module

	namespace this_module {

path get_path()
{
	return module::get_path(module::id());
}

module::info get_info()
{
	return module::get_info(module::id());
}

module::id get_id()
{
	// static is unique per module, so copies and thus unique addresses live in 
	// each module.
	static module::id CurrentId;
	if (!CurrentId)
		CurrentId = module::get_id(&CurrentId);
	return CurrentId;
}

	} // namespace this_module
} // namespace ouro
