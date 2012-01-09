// $(header)
#include <oPlatform/oDisplay.h>
#include <oBasis/oAssert.h>
#include <oPlatform/oWindows.h>
#include <oPlatform/oWinRect.h>
#include <oPlatform/oWinAsString.h>

static BOOL CALLBACK FindWorkArea(HMONITOR _hMonitor, HDC _hdcMonitor, LPRECT _lprcMonitor, LPARAM _dwData)
{
	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	oVB(GetMonitorInfo(_hMonitor, &mi));
	std::pair<const TCHAR*, oDISPLAY_DESC*>& ctx = *(std::pair<const TCHAR*, oDISPLAY_DESC*>*)_dwData;
	if (!_stricmp(mi.szDevice, ctx.first))
	{
		ctx.second->WorkareaPosition = oWinRectPosition(mi.rcWork);
		ctx.second->WorkareaSize = oWinRectSize(mi.rcWork);
		ctx.second->NativeHandle = (void*)_hMonitor;
		return FALSE;
	}
	return TRUE;
}

bool oDisplayEnum(unsigned int _Index, oDISPLAY_DESC* _pDesc)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);

	if (!EnumDisplayDevices(0, _Index, &dev, 0))
	{
		oErrorSetLast(oERROR_NOT_FOUND);
		return false;
	}

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
	{
		oErrorSetLast(oERROR_NOT_FOUND);
		return false;
	}

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
			oErrorSetLast(oERROR_INVALID_PARAMETER, "%s", oWinAsStringDISP(result));
			return false;
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

bool oDisplaySetMode(unsigned int _Index, const oDISPLAY_MODE& _Mode)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);

	if (!EnumDisplayDevices(0, _Index, &dev, 0))
	{
		oErrorSetLast(oERROR_NOT_FOUND);
		return false;
	}

	return oDisplaySetMode(dev.DeviceName, _Mode);
}

bool oDisplayResetMode(unsigned int _Index)
{
	DISPLAY_DEVICE dev;
	dev.cb = sizeof(dev);

	if (!EnumDisplayDevices(0, _Index, &dev, 0))
	{
		oErrorSetLast(oERROR_NOT_FOUND);
		return false;
	}

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

unsigned int oDisplayGetPrimaryIndex()
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
	return oInvalid;
}

unsigned int oDisplayGetNum()
{
	return static_cast<unsigned int>(GetSystemMetrics(SM_CMONITORS));
}

void oDisplayGetVirtualRect(int2* _pPosition, int2* _pSize)
{
	RECT r;
	oGetVirtualDisplayRect(&r);
	*_pPosition = oWinRectPosition(r);
	*_pSize = oWinRectSize(r);
}

