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
#include <oPlatform/Windows/oWinRegistry.h>
#include "../Source/oStd/win.h"

using namespace ouro;

// The return values of Reg* is NOT an HRESULT, but can be parsed in the same
// manner. FAILED() does not work because the error results for Reg* are >0.

static HKEY sRoots[] = { HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, HKEY_USERS, HKEY_PERFORMANCE_DATA, HKEY_PERFORMANCE_TEXT, HKEY_PERFORMANCE_NLSTEXT, HKEY_CURRENT_CONFIG, HKEY_DYN_DATA, HKEY_CURRENT_USER_LOCAL_SETTINGS };

bool oWinRegistrySetValue(oWIN_REGISTRY_ROOT _Root, const char* _KeyPath, const char* _ValueName, const char* _Value)
{
	path_string KP;
	replace(KP, _KeyPath, "/", "\\");
	HKEY hKey = nullptr;
	oV(RegCreateKeyEx(sRoots[_Root], KP, 0, 0, 0, KEY_SET_VALUE, 0, &hKey, 0));
	finally close([&](){ RegCloseKey(hKey); });
	oV(RegSetValueEx(hKey, _ValueName, 0, REG_SZ, (BYTE*)_Value, (DWORD) (strlen(_Value) + 1))); // +1 for null terminating, the null character must also be counted
	return true;
}

char* oWinRegistryGetValue(char* _StrDestination, size_t _SizeofStrDestination, oWIN_REGISTRY_ROOT _Root, const char* _KeyPath, const char* _ValueName)
{
	path_string KP;
	replace(KP, _KeyPath, "/", "\\");

	DWORD type = 0;
	oV(RegGetValue(sRoots[_Root], KP, _ValueName, RRF_RT_ANY, &type, _StrDestination, (LPDWORD)&_SizeofStrDestination));

	switch (type)
	{
		case REG_SZ:
			break;

		case REG_DWORD_LITTLE_ENDIAN: // REG_DWORD
			to_string(_StrDestination, _SizeofStrDestination, *(unsigned int*)_StrDestination);
			break;

		case REG_DWORD_BIG_ENDIAN:
			to_string(_StrDestination, _SizeofStrDestination, endian_swap(*(unsigned int*)_StrDestination));
			break;

		case REG_QWORD:
			to_string(_StrDestination, _SizeofStrDestination, *(unsigned long long*)_StrDestination);
			break;

		default:
			return nullptr;
	}

	return _StrDestination;
}

bool oWinRegistryDeleteValue(oWIN_REGISTRY_ROOT _Root, const char* _KeyPath, const char* _ValueName)
{
	path_string KP;
	replace(KP, _KeyPath, "/", "\\");
	HKEY hKey = nullptr;
	oV(RegOpenKeyEx(sRoots[_Root], KP, 0, KEY_ALL_ACCESS, &hKey));
	finally close([&](){ RegCloseKey(hKey); });
	oV(RegDeleteValue(hKey, _ValueName));
	return true;
}

bool oWinRegistryDeleteKey(oWIN_REGISTRY_ROOT _Root, const char* _KeyPath, bool _Recursive)
{
	path_string KP;
	replace(KP, _KeyPath, "/", "\\");
	long err = RegDeleteKey(sRoots[_Root], KP);
	if (err)
	{
		if (!_Recursive)
			oV(err);

		HKEY hKey = nullptr;
		oV(RegOpenKeyEx(sRoots[_Root], KP, 0, KEY_READ, &hKey));
		finally close([&](){ RegCloseKey(hKey); });
		if (KP[KP.length()-1] != '\\')
			strlcat(KP, "\\");
		size_t KPLen = KP.length();
		DWORD dwSize = oUInt(KP.capacity() - KPLen);
		err = RegEnumKeyEx(hKey, 0, &KP[KPLen], &dwSize, nullptr, nullptr, nullptr, nullptr);
		while (!err)
		{
			if (!oWinRegistryDeleteKey(_Root, KP, _Recursive))
				return false; // pass through error

			DWORD dwSize = oUInt(KP.capacity() - KPLen);
			err = RegEnumKeyEx(hKey, 0, &KP[KPLen], &dwSize, nullptr, nullptr, nullptr, nullptr);
		}

		KP[KPLen] = 0;
		// try again to delete original
		oV(RegDeleteKey(sRoots[_Root], KP));
	}

	return true;
}
