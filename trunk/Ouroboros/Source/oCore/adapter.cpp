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
#include <oCore/adapter.h>
#include <oStd/guid.h>
#include "../oStd/win.h"
#include <regex>

#define oNVVER_MAJOR 285
#define oNVVER_MINOR 62

#define oAMDVER_MAJOR 8
#define oAMDVER_MINOR 982

using namespace oStd;

namespace oCore {
	namespace adapter {
		namespace detail {

// NVIDIA's version string is of the form "x.xx.xM.MMmm" where
// MMM is the major version and mm is the minor version.
// (outside function below so this doesn't get tracked as a leak)
static std::regex reNVVersionString("[0-9]+\\.[0-9]+\\.[0-9]+([0-9])\\.([0-9][0-9])([0-9]+)");

static version from_string_nv(const char* _VersionString)
{
	std::cmatch matches;
	if (!regex_match(_VersionString, matches, reNVVersionString))
		throw std::invalid_argument(formatf("The specified string \"%s\" is not a well-formed NVIDIA version string", oSAFESTRN(_VersionString)));

	char major[4];
	major[0] = *matches[1].first;
	major[1] = *matches[2].first;
	major[2] = *(matches[2].first+1);
	major[3] = 0;
	return version(static_cast<unsigned short>(atoi(major))
		, static_cast<unsigned short>(atoi(matches[3].first)));
}

// AMD's version string is of the form M.mm.x.x where
// M is the major version and mm is the minor version.
// (outside function below so this doesn't get tracked as a leak)
static std::regex reAMDVersionString("([0-9]+)\\.([0-9]+)\\.[0-9]+\\.[0-9]+");

static version from_string_amd(const char* _VersionString)
{
	std::cmatch matches;
	if (!regex_match(_VersionString, matches, reAMDVersionString))
		throw std::invalid_argument(formatf("The specified string \"%s\" is not a well-formed AMD version string", oSAFESTRN(_VersionString)));

	return version(static_cast<unsigned short>(atoi(matches[1].first))
		, static_cast<unsigned short>(atoi(matches[2].first)));
}

static version from_string_intel(const char* _VersionString)
{
	// This initial version was done based on a laptop which appears to have AMD-
	// like drivers for an Intel chip, so use the same parsing for now.
	return from_string_amd(_VersionString);
}

void enumerate_video_drivers(const std::function<bool(const info& _Info)>& _Enumerator)
{
	// redefine some MS GUIDs so we don't have to link to them

	// 4590F811-1D3A-11D0-891F-00AA004B2E24
	static const guid oGUID_CLSID_WbemLocator = { 0x4590F811, 0x1D3A, 0x11D0, { 0x89, 0x1F, 0x00, 0xAA, 0x00, 0x4B, 0x2E, 0x24 } };

	// DC12A687-737F-11CF-884D-00AA004B2E24
	static const guid oGUID_IID_WbemLocator = { 0xdc12a687, 0x737f, 0x11cf, { 0x88, 0x4D, 0x00, 0xAA, 0x00, 0x4B, 0x2E, 0x24 } };

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	finally Deinitialize([&] { CoUninitialize(); });

	intrusive_ptr<IWbemLocator> WbemLocator;
	oV(CoCreateInstance((const GUID&)oGUID_CLSID_WbemLocator
		, 0
		, CLSCTX_INPROC_SERVER
		, (const IID&)oGUID_IID_WbemLocator
		, (LPVOID *)&WbemLocator));

	intrusive_ptr<IWbemServices> WbemServices;
	oV(WbemLocator->ConnectServer(_bstr_t(L"ROOT\\CIMV2")
		, nullptr
		, nullptr
		, 0
		, 0
		, 0
		, 0
		, &WbemServices));

	oV(CoSetProxyBlanket(WbemServices
		, RPC_C_AUTHN_WINNT
		, RPC_C_AUTHZ_NONE
		, nullptr
		, RPC_C_AUTHN_LEVEL_CALL
		, RPC_C_IMP_LEVEL_IMPERSONATE
		, nullptr
		, EOAC_NONE));
	
	intrusive_ptr<IEnumWbemClassObject> Enumerator;
	oV(WbemServices->ExecQuery(bstr_t("WQL")
		, bstr_t("SELECT * FROM Win32_VideoController")
		, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY
		, nullptr, &Enumerator));

	intrusive_ptr<IWbemClassObject> WbemClassObject;
	while (Enumerator)
	{
		info inf;

		ULONG uReturn = 0;
		WbemClassObject = nullptr;
		oV(Enumerator->Next(WBEM_INFINITE, 1, &WbemClassObject, &uReturn));
		if (0 == uReturn)
			break;

		VARIANT vtProp;
		oV(WbemClassObject->Get(L"Description", 0, &vtProp, 0, 0));
		inf.description = vtProp.bstrVal;
		VariantClear(&vtProp);
		oV(WbemClassObject->Get(L"PNPDeviceID", 0, &vtProp, 0, 0));
		inf.plugnplay_id = vtProp.bstrVal;
		VariantClear(&vtProp);
		oV(WbemClassObject->Get(L"DriverVersion", 0, &vtProp, 0, 0));

		sstring StrVersion = vtProp.bstrVal;
		if (strstr(inf.description, "NVIDIA"))
		{
			inf.vendor = vendor::nvidia;
			inf.version = from_string_nv(StrVersion);
		}
		
		else if (strstr(inf.description, "ATI") || strstr(inf.description, "AMD"))
		{
			inf.vendor = vendor::amd;
			inf.version = from_string_amd(StrVersion);
		}

		else if (strstr(inf.description, "Intel"))
		{
			inf.vendor = vendor::intel;
			inf.version = from_string_intel(StrVersion);
		}

		else
		{
			inf.vendor = vendor::unknown;
			inf.version = version();
		}

		if (!_Enumerator(inf))
			break;
	}
}

		} // namespace detail

static info get_info(int _AdapterIndex, IDXGIAdapter* _pAdapter)
{
	DXGI_ADAPTER_DESC ad;
	_pAdapter->GetDesc(&ad);
	info adapter_info;
	detail::enumerate_video_drivers([&](const info& _Info)->bool
	{
		mstring vendor, device;
		snprintf(vendor, "VEN_%X", ad.VendorId);
		snprintf(device, "DEV_%04X", ad.DeviceId);
		if (strstr(_Info.plugnplay_id, vendor) && strstr(_Info.plugnplay_id, device))
		{
			adapter_info = _Info;
			return false;
		}
		return true;
	});

	*(int*)&adapter_info.id = _AdapterIndex;
	return std::move(adapter_info);
}

void enumerate(const std::function<bool(const info& _Info)>& _Enumerator)
{
	oStd::intrusive_ptr<IDXGIFactory> Factory;
	oStd::intrusive_ptr<IDXGIAdapter> Adapter;
	oV(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory));

	int AdapterIndex = 0;
	while (DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(AdapterIndex, &Adapter))
	{
		info adapter_info = std::move(get_info(AdapterIndex, Adapter));

		if (!_Enumerator(adapter_info))
			return;

		AdapterIndex++;
		Adapter = nullptr;
	}
}

version minimum_version(vendor::value _Vendor)
{
	switch (_Vendor)
	{
		case vendor::nvidia: return version(oNVVER_MAJOR, oNVVER_MINOR);
		case vendor::amd: return version(oAMDVER_MAJOR, oAMDVER_MINOR);
		default: break;
	}
	return version();
}

	} // namespace adapter
} // namespace oCore
