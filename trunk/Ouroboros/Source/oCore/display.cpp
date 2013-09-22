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
#include <oCore/display.h>
#include "../oStd/win.h"

using namespace oStd;

namespace oCore {
	namespace display {

id primary_id()
{
	id ID;
	enumerate([&](const info& _Info)->bool
	{
		if (_Info.is_primary)
		{
			ID = _Info.display_id;
			return false;
		}
		return true;
	});

	if (ID == id())
		oTHROW0(no_such_device);
	return ID;
}

id get_id(void* _NativeHandle)
{
	id ID;
	enumerate([&](const info& _Info)->bool
	{
		if (_Info.native_handle == _NativeHandle)
		{
			ID = _Info.display_id;
			return false;
		}
		return true;
	});

	if (ID == id())
		oTHROW0(no_such_device);

	return ID;
}

int count()
{
	return GetSystemMetrics(SM_CMONITORS);
}

id find(int _ScreenX, int _ScreenY)
{
	POINT p = { _ScreenX, _ScreenY };
	HMONITOR hMonitor = MonitorFromPoint(p, MONITOR_DEFAULTTONULL);
	if (!hMonitor)
		oTHROW0(no_such_device);

	id ID;
	enumerate([&](const info& _Info)->bool
	{
		if (hMonitor == (HMONITOR)_Info.native_handle)
		{
			ID = _Info.display_id;
			return false;
		}
		return true;
	});

	if (ID == id())
		oTHROW0(no_such_device);
	return ID;
}

static BOOL CALLBACK find_work_area(HMONITOR _hMonitor, HDC _hdcMonitor, LPRECT _lprcMonitor, LPARAM _dwData)
{
	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	oVB(GetMonitorInfo(_hMonitor, &mi));
	std::pair<const TCHAR*, info*>& ctx = *(std::pair<const TCHAR*, info*>*)_dwData;
	if (!_stricmp(mi.szDevice, ctx.first))
	{
		ctx.second->workarea_x = mi.rcWork.left;
		ctx.second->workarea_y = mi.rcWork.top;
		ctx.second->workarea_width = mi.rcWork.right - mi.rcWork.left;
		ctx.second->workarea_height = mi.rcWork.bottom - mi.rcWork.top;
		ctx.second->native_handle = (void*)_hMonitor;
		return FALSE;
	}
	return TRUE;
}

static info get_info(int _Index, const DISPLAY_DEVICE& _DisplayDevice)
{
	info di;
	*(int*)&di.display_id = _Index;
	di.is_primary = !!(_DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE);
		
	DEVMODE dm;
	oVB(EnumDisplaySettings(_DisplayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &dm));

	di.mode.width = dm.dmPelsWidth;
	di.mode.height = dm.dmPelsHeight;
	di.mode.depth = dm.dmBitsPerPel;
	di.mode.refresh_rate = dm.dmDisplayFrequency;

	di.x = dm.dmPosition.x;
	di.y = dm.dmPosition.y;

	di.native_handle = INVALID_HANDLE_VALUE;
	std::pair<const TCHAR*, info*> ctx;
	ctx.first = _DisplayDevice.DeviceName;
	ctx.second = &di;
	EnumDisplayMonitors(0, 0, find_work_area, (LPARAM)&ctx);
		
	// Why is a handle coming right out of EnumDisplayMonitors considered invalid?
	BOOL isOn = FALSE;
	GetDevicePowerState((HMONITOR)di.native_handle, &isOn);
	di.is_power_on = !!isOn;
	return std::move(di);
}

info get_info(id _ID)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);
	if (!EnumDisplayDevices(0, *(int*)&_ID, &dev, 0))
		oTHROW0(no_such_device);
	return std::move(get_info(*(int*)&_ID, dev));
}

void enumerate(const std::function<bool(const info& _Info)>& _Enumerator)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);
	int index = 0;

	info di;
	while (EnumDisplayDevices(0, index, &dev, 0))
	{
		info di = get_info(index, dev);
		if (!_Enumerator(di))
			break;
		index++;
	}
}

static void set_mode(const char* _DeviceName, const mode_info& _Mode)
{
	DEVMODE dm;
	if (!EnumDisplaySettings(_DeviceName, ENUM_CURRENT_SETTINGS, &dm))
		oTHROW0(no_such_device);

	// ensure only what we want to set is set
	dm.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL|DM_DISPLAYFREQUENCY;

	#define SET(x, y) if (_Mode.y != oDEFAULT) dm.x = _Mode.y
		SET(dmPelsWidth, width);
		SET(dmPelsHeight, height);
		SET(dmDisplayFrequency, refresh_rate);
		SET(dmBitsPerPel, depth);
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
			throw std::invalid_argument("invalid mode");
		default: 
		case DISP_CHANGE_SUCCESSFUL:
			ChangeDisplaySettingsEx(_DeviceName, &dm, 0, CDS_FULLSCREEN, 0);
			break;
	}
}

void set_mode(id _ID, const mode_info& _Mode)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);
	if (!EnumDisplayDevices(0, *(int*)&_ID, &dev, 0))
		oTHROW0(no_such_device);
	set_mode(dev.DeviceName, _Mode);
}

static void reset_mode(const char* _DeviceName)
{
	ChangeDisplaySettingsEx(_DeviceName, 0, 0, CDS_FULLSCREEN, 0);
}

void reset_mode(id _ID)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);
	if (!EnumDisplayDevices(0, *(int*)&_ID, &dev, 0))
		oTHROW0(no_such_device);
	reset_mode(dev.DeviceName);
}

void virtual_rect(int* _pX, int* _pY, int* _pWidth, int* _pHeight)
{
	*_pX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	*_pY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	*_pWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	*_pHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

void set_power_on(bool _On)
{
	static const LPARAM OFF = 2;
	static const LPARAM LOWPOWER = 1;
	static const LPARAM ON = -1;
	PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, _On ? ON : LOWPOWER);
}

	} // namespace display
} // namespace oCore
