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
#include <oCore/display.h>
#include <oCore/windows/win_com.h>
#include <oCore/windows/win_util.h>
#include <oBase/guid.h>
#include <oHLSL/oHLSLMath.h>
#include <regex>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <comutil.h>
#include <d3d11.h>
#include <dxgi.h>
#include <Wbemidl.h>

// Some GPU drivers have bugs in newer features that we use, so ensure we're at
// least on this version and hope there aren't regressions.

#define oNVVER_MAJOR 285
#define oNVVER_MINOR 62

#define oAMDVER_MAJOR 8
#define oAMDVER_MINOR 982

using namespace oStd;

namespace ouro {
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

	windows::com::ensure_initialized();

	intrusive_ptr<IWbemLocator> WbemLocator;
	oV(CoCreateInstance((const GUID&)oGUID_CLSID_WbemLocator
		, 0
		, CLSCTX_INPROC_SERVER
		, (const IID&)oGUID_IID_WbemLocator
		, (LPVOID*)&WbemLocator));

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
		Enumerator->Next(WBEM_INFINITE, 1, &WbemClassObject, &uReturn);
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
	// There's a new adapter called teh Basic Render Driver that is not a video driver, so 
	// it won't show up with enumerate_video_drivers, so handle it explicitly here.
	if (ad.VendorId == 0x1414 && ad.DeviceId == 0x8c) // Microsoft Basic Render Driver
	{
		adapter_info.description = "Microsoft Basic Render Driver";
		adapter_info.plugnplay_id = "Microsoft Basic Render Driver";
		adapter_info.version = version(1,0);
		adapter_info.vendor = vendor::microsoft;
		adapter_info.feature_level = version(11,1);
	}

	else
	{
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
	}

	*(int*)&adapter_info.id = _AdapterIndex;

	D3D_FEATURE_LEVEL FeatureLevel;
	// Note that the out-device is null, thus this isn't that expensive a call
	if (SUCCEEDED(D3D11CreateDevice(
		_pAdapter
		, D3D_DRIVER_TYPE_UNKNOWN
		, nullptr
		, 0 // D3D11_CREATE_DEVICE_DEBUG // squelches a _com_error warning
		, nullptr
		, 0
		, D3D11_SDK_VERSION
		, nullptr
		, &FeatureLevel
		, nullptr)))
		adapter_info.feature_level = version((FeatureLevel>>12) & 0xffff, (FeatureLevel>>8) & 0xf);

	return adapter_info;
}

void enumerate(const std::function<bool(const info& _Info)>& _Enumerator)
{
	intrusive_ptr<IDXGIFactory> Factory;
	oV(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory));

	int AdapterIndex = 0;
	intrusive_ptr<IDXGIAdapter> Adapter;
	while (DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(AdapterIndex, &Adapter))
	{
		info adapter_info = get_info(AdapterIndex, Adapter);

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

info find(const int2& _VirtualDesktopPosition, const version& _MinVersion, bool _ExactVersion)
{
	intrusive_ptr<IDXGIFactory> Factory;
	oV(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory));

	const bool LookForOutput = all(_VirtualDesktopPosition != int2(oDEFAULT, oDEFAULT));

	int AdapterIndex = 0;
	intrusive_ptr<IDXGIOutput> Output;
	intrusive_ptr<IDXGIAdapter> Adapter;
	while (DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(AdapterIndex, &Adapter))
	{
		adapter::info adapter_info = get_info(AdapterIndex, Adapter);
		version RequiredVersion = _MinVersion;
		if (RequiredVersion == version())
			RequiredVersion = minimum_version(adapter_info.vendor);

		intrusive_ptr<IDXGIOutput> Output;
		if (LookForOutput)
		{
			int o = 0;
			while (DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(o, &Output))
			{
				DXGI_OUTPUT_DESC od;
				Output->GetDesc(&od);
				if (_VirtualDesktopPosition.x >= od.DesktopCoordinates.left 
					&& _VirtualDesktopPosition.x <= od.DesktopCoordinates.right 
					&& _VirtualDesktopPosition.y >= od.DesktopCoordinates.top 
					&& _VirtualDesktopPosition.y <= od.DesktopCoordinates.bottom)
				{
					sstring StrAdd, StrReq;
					if (_ExactVersion)
					{
						if (adapter_info.version != RequiredVersion)
							oTHROW(no_such_device, "Exact video driver version %s required, but current driver is %s", to_string2(StrReq, RequiredVersion), to_string2(StrAdd, adapter_info.version));
					}

					else if (adapter_info.version < RequiredVersion)
						oTHROW(no_such_device, "Video driver version %s or newer required, but current driver is %s", to_string2(StrReq, RequiredVersion), to_string2(StrAdd, adapter_info.version));

					return adapter_info;
				}
				o++;
				Output = nullptr;
			}
		}

		else if ((_ExactVersion && adapter_info.version == RequiredVersion) || (!_ExactVersion && adapter_info.version >= RequiredVersion))
			return adapter_info;

		AdapterIndex++;
		Adapter = nullptr;
	}

		sstring StrReq;
	if (LookForOutput)
		oTHROW(no_such_device, "no adapter found for the specified virtual desktop coordinates that also matches the %s driver version %s", _ExactVersion ? "exact" : "minimum", to_string2(StrReq, _MinVersion));
	else
		oTHROW(no_such_device, "no adapter found matching the %s driver version %s", _ExactVersion ? "exact" : "minimum", to_string2(StrReq, _MinVersion));
}

info find(const display::id& _DisplayID)
{
	display::info di = display::get_info(_DisplayID);

	intrusive_ptr<IDXGIFactory> Factory;
	oV(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory));

	int AdapterIndex = 0;
	intrusive_ptr<IDXGIOutput> Output;
	intrusive_ptr<IDXGIAdapter> Adapter;
	while (DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(AdapterIndex, &Adapter))
	{
		intrusive_ptr<IDXGIOutput> Output;
		int o = 0;
		while (DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(o, &Output))
		{
			DXGI_OUTPUT_DESC od;
			Output->GetDesc(&od);
			if (od.Monitor == (HMONITOR)di.native_handle)
				return get_info(AdapterIndex, Adapter);
			o++;
			Output = nullptr;
		}

		AdapterIndex++;
		Adapter = nullptr;
	}

	oTHROW(no_such_device, "no adapter matches the specified display id");
}

	} // namespace adapter
} // namespace ouro
