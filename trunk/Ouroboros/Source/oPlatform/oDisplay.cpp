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
#include <oPlatform/oDisplay.h>
#include <oBasis/oAssert.h>
#include <oPlatform/Windows/oDXGI.h>
#include <oPlatform/Windows/oWindows.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/Windows/oWinAsString.h>

void oDisplayAdapterEnum(const oFUNCTION<bool(int _AdapterIndex, const oDISPLAY_ADAPTER_DRIVER_DESC& _DriverDesc)>& _Enumerator)
{
	oDXGIEnumAdapters([&](int _AdapterIndex, IDXGIAdapter* _pAdapter, const oDISPLAY_ADAPTER_DRIVER_DESC& _DriverDesc)->bool
	{
		return _Enumerator(_AdapterIndex, _DriverDesc);
	});
}

oVersion oDisplayAdapterGetMinimumVersion(oGPU_VENDOR _Vendor)
{
	switch (_Vendor)
	{
		case oGPU_VENDOR_NVIDIA: return oVersion(oNVVER_MAJOR, oNVVER_MINOR);
		case oGPU_VENDOR_AMD: return oVersion(oAMDVER_MAJOR, oAMDVER_MINOR);
		default: return oVersion();
	}
}

bool oDisplayAdapterIsUpToDate()
{
	bool IsUpToDate = true;
	oDisplayAdapterEnum([&](int _AdapterIndex, const oDISPLAY_ADAPTER_DRIVER_DESC& _DriverDesc)->bool
	{
		if (_DriverDesc.Version < oDisplayAdapterGetMinimumVersion(_DriverDesc.Vendor))
			IsUpToDate = false;
		return IsUpToDate;
	});
	return IsUpToDate;
}

static BOOL CALLBACK FindWorkArea(HMONITOR _hMonitor, HDC _hdcMonitor, LPRECT _lprcMonitor, LPARAM _dwData)
{
	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	oVB(GetMonitorInfo(_hMonitor, &mi));
	std::pair<const TCHAR*, oDISPLAY_DESC*>& ctx = *(std::pair<const TCHAR*, oDISPLAY_DESC*>*)_dwData;
	if (!oStricmp(mi.szDevice, ctx.first))
	{
		ctx.second->WorkareaPosition = oWinRectPosition(mi.rcWork);
		ctx.second->WorkareaSize = oWinRectSize(mi.rcWork);
		ctx.second->NativeHandle = (void*)_hMonitor;
		return FALSE;
	}
	return TRUE;
}

bool oDisplayEnum(int _Index, oDISPLAY_DESC* _pDesc)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);

	if (!EnumDisplayDevices(0, _Index, &dev, 0))
		return oErrorSetLast(oERROR_NOT_FOUND);

	_pDesc->Index = _Index;
	_pDesc->IsPrimary = !!(dev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE);

	DEVMODE dm;
	if (EnumDisplaySettings(dev.DeviceName, ENUM_CURRENT_SETTINGS, &dm))
	{
		_pDesc->Position = int2(dm.dmPosition.x, dm.dmPosition.y);
		_pDesc->Mode.Size = int2(dm.dmPelsWidth, dm.dmPelsHeight);
		_pDesc->Mode.RefreshRate = dm.dmDisplayFrequency;
		_pDesc->Mode.Bitdepth = dm.dmBitsPerPel;
		_pDesc->NativeHandle = INVALID_HANDLE_VALUE;
		std::pair<const TCHAR*, oDISPLAY_DESC*> ctx;
		ctx.first = dev.DeviceName;
		ctx.second = _pDesc;
		EnumDisplayMonitors(0, 0, FindWorkArea, (LPARAM)&ctx);
		
		// Why is a handle coming right out of EnumDisplayMonitors considered invalid?
		BOOL isOn = FALSE;
		GetDevicePowerState((HMONITOR)_pDesc->NativeHandle, &isOn);
		_pDesc->IsPowerOn = !!isOn;
		return true;
	}

	else
		oWinSetLastError();

	return false;
}

static bool oDisplaySetMode(const char* _DeviceName, const oDISPLAY_MODE& _Mode)
{
	DEVMODE dm;
	if (!EnumDisplaySettings(_DeviceName, ENUM_CURRENT_SETTINGS, &dm))
		return oErrorSetLast(oERROR_NOT_FOUND);

	// ensure only what we want to set is set
	dm.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL|DM_DISPLAYFREQUENCY;

	#define SET(x, y) if (_Mode.y != oDEFAULT) dm.x = _Mode.y
		SET(dmPelsWidth, Size.x);
		SET(dmPelsHeight, Size.y);
		SET(dmDisplayFrequency, RefreshRate);
		SET(dmBitsPerPel, Bitdepth);
	#undef SET

	LONG result = ChangeDisplaySettingsEx(_DeviceName, &dm, 0, CDS_TEST, 0);
	switch (result)
	{
		case DISP_CHANGE_BADDUALVIEW: 
		case DISP_CHANGE_BADFLAGS: 
		case DISP_CHANGE_BADMODE: 
		case DISP_CHANGE_BADPARAM: 
		case DISP_CHANGE_FAILED: 
		case DISP_CHANGE_NOTUPDATED: 
		case DISP_CHANGE_RESTART:
			return oErrorSetLast(oERROR_INVALID_PARAMETER, "%s", oWinAsStringDISP(result));
		default: 
		case DISP_CHANGE_SUCCESSFUL:
			ChangeDisplaySettingsEx(_DeviceName, &dm, 0, CDS_FULLSCREEN, 0);
			break;
	}

	return true;
}

static bool oDisplayResetMode(const char* _DeviceName)
{
	ChangeDisplaySettingsEx(_DeviceName, 0, 0, CDS_FULLSCREEN, 0);
	return true;
}

bool oDisplaySetMode(int _Index, const oDISPLAY_MODE& _Mode)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);
	if (!EnumDisplayDevices(0, _Index, &dev, 0))
		return oErrorSetLast(oERROR_NOT_FOUND);
	return oDisplaySetMode(dev.DeviceName, _Mode);
}

bool oDisplayResetMode(int _Index)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);
	if (!EnumDisplayDevices(0, _Index, &dev, 0))
		return oErrorSetLast(oERROR_NOT_FOUND);
	return oDisplayResetMode(dev.DeviceName);
}

bool oDisplaySetPowerOn(bool _On)
{
	static const LPARAM OFF = 2;
	static const LPARAM LOWPOWER = 1;
	static const LPARAM ON = -1;
	SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, _On ? ON : LOWPOWER);
	return true;
}

int oDisplayGetPrimaryIndex()
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);
	int index = 0;
	while (EnumDisplayDevices(0, index, &dev, 0))
	{
		if (dev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
			return index;
		index++;
	}
	return oInvalid;
}

int oDisplayGetNum()
{
	return GetSystemMetrics(SM_CMONITORS);
}

int oDisplayFindIndex(const int2& _ScreenPosition)
{
	POINT p = { _ScreenPosition.x, _ScreenPosition.y };

	HMONITOR hMonitor = MonitorFromPoint(p, MONITOR_DEFAULTTONULL);
	if (!hMonitor)
	{
		oErrorSetLast(oERROR_NOT_FOUND);
		return oInvalid;
	}

	DISPLAY_DEVICE dd;
	return oWinGetDisplayDevice(hMonitor, &dd);
}

void oDisplayGetVirtualRect(int2* _pPosition, int2* _pSize)
{
	RECT r;
	oGetVirtualDisplayRect(&r);
	*_pPosition = oWinRectPosition(r);
	*_pSize = oWinRectSize(r);
}
