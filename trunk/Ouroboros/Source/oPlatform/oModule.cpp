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
#include <oPlatform/oModule.h>
#include <oStd/assert.h>
#include <oBasis/oError.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/Windows/oWindows.h>

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oMODULE_TYPE)
	oRTTI_ENUM_BEGIN_VALUES(oMODULE_TYPE)
		oRTTI_VALUE_CUSTOM(oMODULE_UNKNOWN, "Unknown")
		oRTTI_VALUE_CUSTOM(oMODULE_APP, "Application")
		oRTTI_VALUE_CUSTOM(oMODULE_DLL, "Application Extension")
		oRTTI_VALUE_CUSTOM(oMODULE_LIB, "Object File Library")
		oRTTI_VALUE_CUSTOM(oMODULE_FONT_UNKNOWN, "Unknown font file")
		oRTTI_VALUE_CUSTOM(oMODULE_FONT_RASTER, "Raster font file")
		oRTTI_VALUE_CUSTOM(oMODULE_FONT_TRUETYPE, "TrueType font file")
		oRTTI_VALUE_CUSTOM(oMODULE_FONT_VECTOR, "Vector font file")
		oRTTI_VALUE_CUSTOM(oMODULE_VIRTUAL_DEVICE, "Virtual Device")
		oRTTI_VALUE_CUSTOM(oMODULE_DRV_UNKNOWN, "Unknown driver")
		oRTTI_VALUE_CUSTOM(oMODULE_DRV_COMM, "COMM driver")
		oRTTI_VALUE_CUSTOM(oMODULE_DRV_DISPLAY, "Display driver")
		oRTTI_VALUE_CUSTOM(oMODULE_DRV_INSTALLABLE, "Installable driver")
		oRTTI_VALUE_CUSTOM(oMODULE_DRV_KEYBOARD, "Keyboard driver")
		oRTTI_VALUE_CUSTOM(oMODULE_DRV_LANGUAGE, "Language driver")
		oRTTI_VALUE_CUSTOM(oMODULE_DRV_MOUSE, "Mouse driver")
		oRTTI_VALUE_CUSTOM(oMODULE_DRV_NETWORK, "Network driver")
		oRTTI_VALUE_CUSTOM(oMODULE_DRV_PRINTER, "Printer driver")
		oRTTI_VALUE_CUSTOM(oMODULE_DRV_SOUND, "Sound driver")
		oRTTI_VALUE_CUSTOM(oMODULE_DRV_SYSTEM, "System driver")
	oRTTI_ENUM_END_VALUES(oMODULE_TYPE)
oRTTI_ENUM_END_DESCRIPTION(oMODULE_TYPE)

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
		oErrorSetLast(std::errc::io_error, "The application has failed to start because %s was not found. Re-installing the application may fix the problem.", oSAFESTRN(_ModuleName));
		return hModule;
	}

	if( !oModuleLink(hModule, _pInterfaceFunctionNames, _ppInterfaces, _CountofInterfaces) )
	{
		oErrorSetLast(std::errc::function_not_supported, "Could not create all interfaces from %s. This might be because the DLLs are out of date, try copying a newer version of the DLL into the bin dir for this application.", _ModuleName);
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
		oStd::path path = oCore::filesystem::app_path(true);
		oStd::path_string buf;
		snprintf(buf, "%s - Unable To Locate Component", oGetFilebase(path));
		oMsgBox(oMSGBOX_DESC(oMSGBOX_ERR, buf), "%s", oErrorGetLastString());
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
