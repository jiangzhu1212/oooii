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
#include "pch.h"
#include <oooii/oDisplay.h>
#include <oooii/oAssert.h>
#include <oooii/oWindows.h>

unsigned int oDisplay::GetNumDisplays()
{
	return static_cast<unsigned int>(GetSystemMetrics(SM_CMONITORS));
}

unsigned int oDisplay::GetPrimary()
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);
	unsigned int index = 0;
	while (EnumDisplayDevices(0, index, &dev, 0))
	{
		if (dev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
			return index;
		index++;
	}

	return ~0u;
}

void oDisplay::GetVirtualRect(int* _pX, int* _pY, int* _pWidth, int* _pHeight)
{
	RECT r;
	oGetVirtualDisplayRect(&r);
	*_pX = r.left;
	*_pY = r.top;
	*_pWidth = r.right - r.left;
	*_pHeight = r.bottom - r.top;
}

struct MonitorProcContext
{
	const TCHAR* DeviceName;
	oDisplay::DESC* pDisplayDesc;
};

BOOL CALLBACK FindWorkArea(HMONITOR _hMonitor, HDC _hdcMonitor, LPRECT _lprcMonitor, LPARAM _dwData)
{
		MONITORINFOEX mi;
		mi.cbSize = sizeof(mi);
		oVB(GetMonitorInfo(_hMonitor, &mi));

		MonitorProcContext& ctx = *(MonitorProcContext*)_dwData;
		
		if (!_stricmp(mi.szDevice, ctx.DeviceName))
		{
			ctx.pDisplayDesc->WorkareaX = mi.rcWork.left;
			ctx.pDisplayDesc->WorkareaY = mi.rcWork.top;
			ctx.pDisplayDesc->WorkareaWidth = mi.rcWork.right - mi.rcWork.left;
			ctx.pDisplayDesc->WorkareaHeight = mi.rcWork.bottom - mi.rcWork.top;
			ctx.pDisplayDesc->NativeHandle = (void*)_hMonitor;
			return FALSE;
		}

		return TRUE;
}

bool oDisplay::GetDesc(unsigned int _Index, DESC* _pDesc)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);
	
	oVB(EnumDisplayDevices(0, _Index, &dev, 0));
	
	_pDesc->Index = _Index;
	_pDesc->IsPrimary = !!(dev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE);
	
	DEVMODE dm;
	if (EnumDisplaySettings(dev.DeviceName, ENUM_CURRENT_SETTINGS, &dm))
	{
		_pDesc->X = dm.dmPosition.x;
		_pDesc->Y = dm.dmPosition.y;
		_pDesc->ModeDesc.Width = dm.dmPelsWidth;
		_pDesc->ModeDesc.Height = dm.dmPelsHeight;
		_pDesc->ModeDesc.RefreshRate = dm.dmDisplayFrequency;
		_pDesc->ModeDesc.Bitdepth = dm.dmBitsPerPel;

		MonitorProcContext context;
		context.DeviceName = dev.DeviceName;
		context.pDisplayDesc = _pDesc;
		EnumDisplayMonitors(0, 0, FindWorkArea, (LPARAM)&context);
		return true;
	}

	return false;
}

static bool SetDescByName(const char* _DeviceName, const oDisplay::DESC* _pDesc)
{
	if (_pDesc)
	{
		DEVMODE dm;
		if (!EnumDisplaySettings(_DeviceName, ENUM_CURRENT_SETTINGS, &dm))
			return false;

		// ensure only what we want to set is set
		dm.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL|DM_DISPLAYFREQUENCY;

		#define SET(x, y) if (_pDesc->y != oDisplay::DEFAULT) dm.x = _pDesc->y
		SET(dmPelsWidth, ModeDesc.Width);
		SET(dmPelsHeight, ModeDesc.Height);
		SET(dmDisplayFrequency, ModeDesc.RefreshRate);
		SET(dmBitsPerPel, ModeDesc.Bitdepth);
		#undef SET

		switch (ChangeDisplaySettingsEx(_DeviceName, &dm, 0, CDS_TEST, 0))
		{
			case DISP_CHANGE_BADDUALVIEW: 
			case DISP_CHANGE_BADFLAGS: 
			case DISP_CHANGE_BADMODE: 
			case DISP_CHANGE_BADPARAM: 
			case DISP_CHANGE_FAILED: 
			case DISP_CHANGE_NOTUPDATED: 
			case DISP_CHANGE_RESTART: return false;
			default: 
			case DISP_CHANGE_SUCCESSFUL:
				ChangeDisplaySettingsEx(_DeviceName, &dm, 0, CDS_FULLSCREEN, 0);
				break;
		}
	}

	else
		ChangeDisplaySettingsEx(_DeviceName, 0, 0, CDS_FULLSCREEN, 0);

	return true;
}

bool oDisplay::SetDesc(unsigned int _Index, const DESC* _pDesc)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);
	
	if (!EnumDisplayDevices(0, _Index, &dev, 0))
		return false;

	return SetDescByName(dev.DeviceName, _pDesc);
}
