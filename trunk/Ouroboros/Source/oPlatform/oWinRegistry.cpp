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
#include <oPlatform/Windows/oWinRegistry.h>
#include <oPlatform/Windows/oWindows.h>

// The return values of Reg* is NOT an HRESULT, but can be parsed in the same
// manner. FAILED() does not work because the error results for Reg* are >0.

#define oREG_CHECK(fn) do { long ERR__ = fn; if (ERR__) { return oWinSetLastError(ERR__, #fn " failed:\n"); } } while(false)
#define oREG_CHECKP(fn) do { long ERR__ = fn; if (ERR__) { oWinSetLastError(ERR__, #fn " failed:\n"); return nullptr; } } while(false)

bool oWinRegistrySetValue(const char* _KeyPath, const char* _ValueName, const char* _Value)
{
	oStringPath KP;
	oReplace(KP, _KeyPath, "/", "\\");
	HKEY hKey = nullptr;
	oREG_CHECK(RegCreateKeyEx(HKEY_CURRENT_USER, KP, 0, 0, 0, KEY_SET_VALUE, 0, &hKey, 0));
	oOnScopeExit close([&](){ RegCloseKey(hKey); });
	oREG_CHECK(RegSetValueEx(hKey, _ValueName, 0, REG_SZ, (BYTE*)_Value, (DWORD) (strlen(_Value) + 1))); // +1 for null terminating, the null character must also be counted
	return true;
}

char* oWinRegistryGetValue(char *_pStrDestination, size_t _SizeofStrDestination, const char* _KeyPath, const char* _ValueName)
{
	oStringPath KP;
	oReplace(KP, _KeyPath, "/", "\\");
	oREG_CHECKP(RegGetValue(HKEY_CURRENT_USER, KP, _ValueName, RRF_RT_ANY, 0, _pStrDestination, (LPDWORD)&_SizeofStrDestination));
	return _pStrDestination;
}

bool oWinRegistryDeleteValue(const char* _KeyPath, const char* _ValueName)
{
	oStringPath KP;
	oReplace(KP, _KeyPath, "/", "\\");
	HKEY hKey = nullptr;
	oREG_CHECK(RegOpenKeyEx(HKEY_CURRENT_USER, KP, 0, KEY_ALL_ACCESS, &hKey));
	oOnScopeExit close([&](){ RegCloseKey(hKey); });
	oREG_CHECK(RegDeleteValue(hKey, _ValueName));
	return true;
}

bool oWinRegistryDeleteKey(const char* _KeyPath, bool _Recursive)
{
	oStringPath KP;
	oReplace(KP, _KeyPath, "/", "\\");
	long err = RegDeleteKey(HKEY_CURRENT_USER, KP);
	if (err)
	{
		if (!_Recursive)
			oREG_CHECK(err);

		HKEY hKey = nullptr;
		oREG_CHECK(RegOpenKeyEx(HKEY_CURRENT_USER, KP, 0, KEY_READ, &hKey));
		oOnScopeExit close([&](){ RegCloseKey(hKey); });
		oEnsureSeparator(KP, '\\');
		size_t KPLen = KP.length();
		DWORD dwSize = oUInt(KP.capacity() - KPLen);
		err = RegEnumKeyEx(hKey, 0, &KP[KPLen], &dwSize, nullptr, nullptr, nullptr, nullptr);
		while (!err)
		{
			if (!oWinRegistryDeleteKey(KP, _Recursive))
				return false; // pass through error

			DWORD dwSize = oUInt(KP.capacity() - KPLen);
			err = RegEnumKeyEx(hKey, 0, &KP[KPLen], &dwSize, nullptr, nullptr, nullptr, nullptr);
		}

		KP[KPLen] = 0;
		// try again to delete original
		oREG_CHECK(RegDeleteKey(HKEY_CURRENT_USER, KP));
	}

	return true;
}
