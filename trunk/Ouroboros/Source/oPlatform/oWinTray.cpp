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
#include <oPlatform/Windows/oWinTray.h>
#include <oConcurrency/mutex.h>
#include <oStd/oStdThread.h>
#include <oBasis/oString.h>
#include <oPlatform/oReporting.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/Windows/oWinWindowing.h>
#include <shellapi.h>
#include <Windowsx.h>

HWND oTrayGetHwnd()
{
	static const char* sHierarchy[] = 
	{
		"Shell_TrayWnd",
		"TrayNotifyWnd",
		"SysPager",
		"ToolbarWindow32",
	};

	size_t i = 0;
	HWND hWnd = FindWindow(sHierarchy[i++], nullptr);
	while (hWnd && i < oCOUNTOF(sHierarchy))
		hWnd = FindWindowEx(hWnd, nullptr, sHierarchy[i++], nullptr);

	return hWnd;
}

#ifndef oWINDOWS_HAS_TRAY_NOTIFYICONIDENTIFIER
static bool Shell_NotifyIconGetRect_WAR(HWND _hWnd, UINT _ID, RECT* _pRect)
{
	// http://social.msdn.microsoft.com/forums/en-US/winforms/thread/4ac8d81e-f281-4b32-9407-e663e6c234ae/
	
	HWND hTray = oTrayGetHwnd();
	DWORD TrayProcID;
	GetWindowThreadProcessId(hTray, &TrayProcID);
	HANDLE hTrayProc = OpenProcess(PROCESS_ALL_ACCESS, 0, TrayProcID);
	bool success = false;
	if (!hTrayProc)
	{
		TBBUTTON* lpBI = (TBBUTTON*)VirtualAllocEx(hTrayProc, nullptr, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);
		int nButtons = (int)SendMessage(hTray, TB_BUTTONCOUNT, 0, 0);
		for (int i = 0; i < nButtons; i++)
		{
			if (SendMessage(hTray, TB_GETBUTTON, i, (LPARAM)lpBI))
			{
				TBBUTTON bi;
				DWORD extraData[2];
				ReadProcessMemory(hTrayProc, lpBI, &bi, sizeof(TBBUTTON), nullptr);
				ReadProcessMemory(hTrayProc, (LPCVOID)bi.dwData, extraData, sizeof(DWORD) * 2, nullptr);
				HWND IconNotifiesThisHwnd = (HWND)extraData[0];
				UINT IconID = extraData[1];

				if (_hWnd == IconNotifiesThisHwnd && _ID == IconID)
				{
					RECT r;
					RECT* lpRect = (RECT*)VirtualAllocEx(hTrayProc, nullptr, sizeof(RECT), MEM_COMMIT, PAGE_READWRITE);
					SendMessage(hTray, TB_GETITEMRECT, i, (LPARAM)lpRect);
					ReadProcessMemory(hTrayProc, lpRect, &r, sizeof(RECT), nullptr);
					VirtualFreeEx(hTrayProc, lpRect, 0, MEM_RELEASE);
					MapWindowPoints(hTray, nullptr, (LPPOINT)&r, 2);
					success = true;
					break;
				}
			}
		}

		VirtualFreeEx(hTrayProc, lpBI, 0, MEM_RELEASE);
		CloseHandle(hTrayProc);
	}

	return success;
}
#endif

// This API can be used to determine if the specified
// icon exists at all.
bool oTrayGetIconRect(HWND _hWnd, UINT _ID, RECT* _pRect)
{
	#ifdef oWINDOWS_HAS_TRAY_NOTIFYICONIDENTIFIER
		NOTIFYICONIDENTIFIER nii;
		memset(&nii, 0, sizeof(nii));
		nii.cbSize = sizeof(nii);
		nii.hWnd = _hWnd;
		nii.uID = _ID;
		HRESULT hr = Shell_NotifyIconGetRect(&nii, _pRect);
		return SUCCEEDED(hr);
	#else
		return Shell_NotifyIconGetRect_WAR(_hWnd, _ID, _pRect);
	#endif
}

struct REMOVE_TRAY_ICON
{
	HWND hWnd;
	UINT ID;
	UINT TimeoutMS;
};

bool operator==(const REMOVE_TRAY_ICON& _RTI1, const REMOVE_TRAY_ICON& _RTI2) { return _RTI1.hWnd == _RTI2.hWnd && _RTI1.ID == _RTI2.ID; }

struct oTrayCleanup : public oProcessSingleton<oTrayCleanup>
{
	oTrayCleanup()
		: AllowInteraction(true)
	{
		oReportingReference();
	}

	~oTrayCleanup()
	{
		{
			oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
			AllowInteraction = false;
		}

		if (!DeferredHideIconThreads.empty())
		{
			for (auto it = std::begin(DeferredHideIconThreads); it != std::end(DeferredHideIconThreads); ++it)
				it->join();
		}

		if (!Removes.empty())
		{
			oStd::xlstring buf;
			oStd::mstring exec;
			oPrintf(buf, "oWindows Trace %s Cleaning up tray icons\n", oSystemGetExecutionPath(exec));
			oThreadsafeOutputDebugStringA(buf);
		}

		for (size_t i = 0; i < Removes.size(); i++)
			oTrayShowIcon(Removes[i].hWnd, Removes[i].ID, 0, 0, false);

		Removes.clear();

		oReportingRelease();
	}

	void Register(HWND _hWnd, UINT _ID)
	{
		oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
		if (!AllowInteraction)
			return;

		if (Removes.size() < Removes.capacity())
		{
			REMOVE_TRAY_ICON rti;
			rti.hWnd = _hWnd;
			rti.ID = _ID;
			rti.TimeoutMS = 0;
			Removes.push_back(rti);
		}
		else
			oThreadsafeOutputDebugStringA("--- Too many tray icons registered for cleanup: ignoring. ---");
	}

	void Unregister(HWND _hWnd, UINT _ID)
	{
		oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
		if (!AllowInteraction)
			return;

		REMOVE_TRAY_ICON rti;
		rti.hWnd = _hWnd;
		rti.ID = _ID;
		rti.TimeoutMS = 0;

		oStd::find_and_erase(Removes, rti);
	}

	void Register(oStd::thread&& _DeferredHideIconThread)
	{
		oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
		DeferredHideIconThreads.push_back(std::move(_DeferredHideIconThread));
	}

	void Unregister(oStd::thread::id _DeferredHideIconThreadID)
	{
		oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
		for (auto it = std::begin(DeferredHideIconThreads); it != std::end(DeferredHideIconThreads); ++it)
		{
			if (it->get_id() == _DeferredHideIconThreadID)
			{
				*it = oStd::thread();
				break;
			}
		}
	}

	static const oGUID GUID;
	oStd::fixed_vector<REMOVE_TRAY_ICON, 20> Removes;
	oStd::fixed_vector<oStd::thread, 20> DeferredHideIconThreads;
	oConcurrency::shared_mutex Mutex;
	volatile bool AllowInteraction;
};

// {1D072014-4CA5-4BA9-AD2C-96AD1510F2A0}
const oGUID oTrayCleanup::GUID = { 0x1d072014, 0x4ca5, 0x4ba9, { 0xad, 0x2c, 0x96, 0xad, 0x15, 0x10, 0xf2, 0xa0 } };
oSINGLETON_REGISTER(oTrayCleanup);

void oTrayShowIcon(HWND _hWnd, UINT _ID, UINT _CallbackMessage, HICON _hIcon, bool _Show)
{
	NOTIFYICONDATA nid;
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = _hWnd;
	nid.hIcon = _hIcon ? _hIcon : oWinGetIcon(_hWnd, false);
	if (!nid.hIcon)
		nid.hIcon = oWinGetIcon(_hWnd, true);
	nid.uID = _ID;
	nid.uCallbackMessage = _CallbackMessage;
	nid.uFlags = NIF_ICON | (_CallbackMessage ? NIF_MESSAGE : 0);
	nid.uVersion = NOTIFYICON_VERSION_4;
	oV(Shell_NotifyIcon(_Show ? NIM_ADD : NIM_DELETE, &nid));

	// we should always be valid when showing, but there may be a scheduled hide 
	// that sneaks in after static deinit clears this singleton
	oTrayCleanup* pTrayCleanup = oTrayCleanup::Singleton();
	if (pTrayCleanup)
	{
		// Ensure we know exactly what version behavior we're dealing with
		oV(Shell_NotifyIcon(NIM_SETVERSION, &nid));
		pTrayCleanup->Register(_hWnd, _ID);
	}

	else
	{
		if (pTrayCleanup)
			pTrayCleanup->Unregister(_hWnd, _ID);
	}
}

void oTraySetFocus()
{
	oV(Shell_NotifyIcon(NIM_SETFOCUS, nullptr));
}

// Because this can call a timeout value that can occur after a synchronous 
// report leaks, ensure we wait for it before reporting a false positive...
#include "oCRTLeakTracker.h"

static void DeferredHideIcon(HWND _hWnd, UINT _ID, unsigned int _TimeoutMS)
{
	Sleep(_TimeoutMS);
	oTRACE("Auto-closing tray icon HWND=0x%p ID=%u", _hWnd, _ID);
	oTrayShowIcon(_hWnd, _ID, 0, 0, false);

	oTrayCleanup::Singleton()->Unregister(oStd::this_thread::get_id());
	oCRTLeakTracker::Singleton()->ReleaseDelay();
}

static void oTrayScheduleIconHide(HWND _hWnd, UINT _ID, unsigned int _TimeoutMS)
{
	oCRTLeakTracker::Singleton()->ReferenceDelay();
	oStd::thread t(DeferredHideIcon, _hWnd, _ID, _TimeoutMS);
	oTrayCleanup::Singleton()->Register(std::move(t));
}

bool oTrayShowMessage(HWND _hWnd, UINT _ID, HICON _hIcon, UINT _TimeoutMS, const char* _Title, const char* _Message)
{
	NOTIFYICONDATA nid;
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = _hWnd;
	nid.hIcon = _hIcon ? _hIcon : oWinGetIcon(_hWnd, false);
	if (!nid.hIcon)
		nid.hIcon = oWinGetIcon(_hWnd, true);
	nid.uID = _ID;
	nid.uFlags = NIF_INFO;
	nid.uTimeout = __max(__min(_TimeoutMS, 30000), 10000);

	// MS recommends truncating at 200 for English: http://msdn.microsoft.com/en-us/library/bb773352(v=vs.85).aspx
	static const int MaxInfo = 201;
	oStrncpy(nid.szInfo, MaxInfo, _Message, MaxInfo - 1);
	oAddTruncationElipse(nid.szInfo, MaxInfo);

	// MS recommends truncating at 48 for English: http://msdn.microsoft.com/en-us/library/bb773352(v=vs.85).aspx
	static const int MaxTitle = 49;
	oStrncpy(nid.szInfoTitle, MaxTitle, _Title, MaxTitle - 1);

	nid.dwInfoFlags = NIIF_NOSOUND;

	#ifdef oWINDOWS_HAS_TRAY_QUIETTIME
		nid.dwInfoFlags |= NIIF_RESPECT_QUIET_TIME;
	#endif

	RECT r;
	if (!oTrayGetIconRect(_hWnd, _ID, &r))
	{
		UINT timeout = 0;
		switch (oGetWindowsVersion())
		{
			case oWINDOWS_2000:
			case oWINDOWS_XP:
			case oWINDOWS_SERVER_2003:
				timeout = nid.uTimeout;
				break;
			default:
			{
				ULONG duration = 0;
				oV(SystemParametersInfo(SPI_GETMESSAGEDURATION, 0, &duration, 0));
				timeout = (UINT)duration * 1000;
				break;
			};
		}

		oTrayShowIcon(_hWnd, _ID, 0, _hIcon, true);
		if (timeout != oInfiniteWait)
			oTrayScheduleIconHide(_hWnd, _ID, timeout);
	}

	if (!Shell_NotifyIcon(NIM_MODIFY, &nid))
		return oWinSetLastError();

	return true;
}

void oTrayDecodeCallbackMessageParams(WPARAM _wParam, LPARAM _lParam, UINT* _pNotificationEvent, UINT* _pID, int* _pX, int* _pY)
{
	// http://msdn.microsoft.com/en-us/library/bb773352(v=vs.85).aspx
	// Search for uCallbackMessage

	*_pNotificationEvent = LOWORD(_lParam);
	*_pID = HIWORD(_lParam);
	if (_pX)
		*_pX = GET_X_LPARAM(_wParam);
	if (_pY)
		*_pY = GET_Y_LPARAM(_wParam);
}

// false means animate from sys tray out to window position
static void oTrayRespectfulAnimateWindow(HWND _hWnd, bool _ToSysTray)
{
	RECT rDesktop, rWindow;
	GetWindowRect(GetDesktopWindow(), &rDesktop);
	GetWindowRect(_hWnd, &rWindow);
	rDesktop.left = rDesktop.right;
	rDesktop.top = rDesktop.bottom;
	const RECT& from = _ToSysTray ? rWindow : rDesktop;
	const RECT& to = _ToSysTray ? rDesktop : rWindow;
	oWinAnimate(_hWnd, from, to);
}

void oTrayMinimize(HWND _hWnd, UINT _CallbackMessage, HICON _hIcon)
{
	oTrayRespectfulAnimateWindow(_hWnd, true);
	ShowWindow(_hWnd, SW_HIDE);
	oTrayShowIcon(_hWnd, 0, _CallbackMessage, _hIcon, true);
}

void oTrayRestore(HWND _hWnd)
{
	oTrayRespectfulAnimateWindow(_hWnd, false);
	ShowWindow(_hWnd, SW_SHOW);
	SetActiveWindow(_hWnd);
	SetForegroundWindow(_hWnd);
	oTrayShowIcon(_hWnd, 0, 0, 0, false);
}

