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
#include <oPlatform/Windows/oWinWindowing.h>
#include <oStd/assert.h>
#include <oStd/fixed_string.h>
#include <oStd/oStdChrono.h>
#include <oConcurrency/mutex.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWinAsString.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/Windows/oWinStatusBar.h>
#include "SoftLink/oWinCommCtrl.h"
#include "SoftLink/oWinShlwapi.h"
#include <WindowsX.h>
#include <CdErr.h>
#include <Shellapi.h>

struct oSkeletonInputContext : oProcessSingleton<oSkeletonInputContext>
{
	static const oGUID GUID;

	oSkeletonInputContext() {}
	~oSkeletonInputContext()
	{
		oASSERT(Sources.empty(), "");
	}

	void Register(HSKELETON _hSkeleton, const oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>& _Get)
	{
		oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
		if (Sources.find(_hSkeleton) != Sources.end())
			throw std::exception("redundant registration");
		Sources[_hSkeleton] = _Get;
	}

	void Unregister(HSKELETON _hSkeleton)
	{
		oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
		auto it = Sources.find(_hSkeleton);
		if (it != Sources.end())
			Sources.erase(it);
	}

	bool Get(HSKELETON _hSkeleton, oGUI_BONE_DESC* _pSkeleton)
	{
		// prevent Unregister() from being called during a read
		oConcurrency::shared_lock<oConcurrency::shared_mutex> lock(Mutex);
		auto it = Sources.find(_hSkeleton);
		if (it == Sources.end())
			return oErrorSetLast(std::errc::invalid_argument);
		it->second(_pSkeleton);
		return true;
	}

	bool Close(HSKELETON _hSkeleton)
	{
		return true;
	}

	oConcurrency::shared_mutex Mutex;
	std::map<HSKELETON
		, oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>
		, std::less<HSKELETON>
		, oProcessHeapAllocator<std::pair<HSKELETON, oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>>>> 
	Sources;
};

// {7E5F5608-2C7A-43E7-B7A0-46293FEC653C}
const oGUID oSkeletonInputContext::GUID = { 0x7e5f5608, 0x2c7a, 0x43e7, { 0xb7, 0xa0, 0x46, 0x29, 0x3f, 0xec, 0x65, 0x3c } };

oSINGLETON_REGISTER(oSkeletonInputContext);

void RegisterSkeletonSource(HSKELETON _hSkeleton, const oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>& _Get)
{
	oSkeletonInputContext::Singleton()->Register(_hSkeleton, _Get);
}

void UnregisterSkeletonSource(HSKELETON _hSkeleton)
{
	oSkeletonInputContext::Singleton()->Unregister(_hSkeleton);
}

bool GetSkeletonDesc(HSKELETON _hSkeleton, oGUI_BONE_DESC* _pSkeleton)
{
	return oSkeletonInputContext::Singleton()->Get(_hSkeleton, _pSkeleton);
}

// @oooii-tony: Confirmation of hWnd being on the specified thread is disabled
// for now... I think it might be trying to access the HWND before it's fully
// constructed. First get the massive integration done, then come back to this.

#define oWINV(_hWnd) \
	if (!oWinExists(_hWnd)) \
		return oErrorSetLast(std::errc::invalid_argument, "Invalid HWND %p specified", _hWnd); \
	if (!oWinIsWindowThread(_hWnd)) \
		oASSERT(oWinIsWindowThread(_hWnd), "This function must be called on the window thread %d for %p", oConcurrency::asuint(oStd::this_thread::get_id()), _hWnd)

#define oWINVP(_hWnd) \
	if (!oWinExists(_hWnd)) \
		{ oErrorSetLast(std::errc::invalid_argument, "Invalid HWND %p specified", _hWnd); return nullptr; } \
	oASSERT(oWinIsWindowThread(_hWnd), "This function must be called on the window thread %d for %p", oConcurrency::asuint(oStd::this_thread::get_id()), _hWnd)

inline bool oErrorSetLastBadType(HWND _hControl, oGUI_CONTROL_TYPE _Type) { return oErrorSetLast(std::errc::invalid_argument, "The specified %s %p (%d) is not valid for this operation", oStd::as_string(_Type), _hControl, GetDlgCtrlID(_hControl)); }

inline bool oErrorSetLastBadIndex(HWND _hControl, oGUI_CONTROL_TYPE _Type, int _SubItemIndex) { return oErrorSetLast(std::errc::invalid_argument, "_SubItemIndex %d was not found in %s %p (%d)", _SubItemIndex, oStd::as_string(_Type), _hControl, GetDlgCtrlID(_hControl)); }

inline HWND oWinControlGetBuddy(HWND _hControl) { return (HWND)SendMessage(_hControl, UDM_GETBUDDY, 0, 0); }

bool oWinCreate(HWND* _phWnd, const int2& _ClientPosition, const int2& _ClientSize, WNDPROC _Wndproc, void* _pThis, bool _AsMessagingWindow)
{
	if (!_phWnd)
		return oErrorSetLast(std::errc::invalid_argument);

	oStd::sstring ClassName;
	oPrintf(ClassName, "Ouroboros.Window.WndProc.%x", _Wndproc);

	WNDCLASSEXA wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(0);
	wc.lpfnWndProc = _Wndproc ? _Wndproc : DefWindowProc;                    
	wc.lpszClassName = ClassName;                        
	wc.style = CS_BYTEALIGNCLIENT|CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_DBLCLKS;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	if (0 == RegisterClassEx(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
		return oWinSetLastError();

	*_phWnd = CreateWindowEx(WS_EX_ACCEPTFILES|WS_EX_APPWINDOW, ClassName, ""
		, WS_OVERLAPPEDWINDOW
		, _ClientPosition.x, _ClientPosition.y
		, _ClientSize.x, _ClientSize.y
		, _AsMessagingWindow ? HWND_MESSAGE : nullptr, nullptr, nullptr, _pThis);

	if (!*_phWnd)
	{
		if (GetLastError() == S_OK)
			return oErrorSetLast(std::errc::protocol_error, "CreateWindowEx returned a null HWND (failure condition) but GetLastError is S_OK. This implies that user handling of a WM_CREATE message failed, so start looking there.");
		return oWinSetLastError();
	}

	oTRACE("HWND %x running on thread %d (0x%x)", *_phWnd, oConcurrency::asuint(oStd::this_thread::get_id()), oConcurrency::asuint(oStd::this_thread::get_id()));

	return true;
}

void* oWinGetThis(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	void* pThis = nullptr;
	if (_hWnd)
	{
		switch (_uMsg)
		{
			case WM_CREATE:
			{
				// 'this' pointer was passed during the call to CreateWindow, so put 
				// that in userdata.
				CREATESTRUCTA* cs = (CREATESTRUCTA*)_lParam;
				SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
				pThis = (void*)cs->lpCreateParams;
				break;
			}

			case WM_INITDIALOG:
				// dialogs don't use CREATESTRUCT, so assume it's directly the context
				SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)_lParam);
				pThis = (void*)_lParam;
				break;

			case WM_DESTROY:
				// once WM_DESTROY is called, don't allow custom handling anymore
				pThis = (void*)SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)nullptr);
				break;

			default:
				// For any other message, grab the context and return it
				pThis = (void*)GetWindowLongPtr(_hWnd, GWLP_USERDATA);
				break;
		}

	}
	return pThis;
}

oStd::thread::id oWinGetWindowThread(HWND _hWnd)
{
	oStd::thread::id ID;
	(uint&)ID = GetWindowThreadProcessId(_hWnd, nullptr);
	return ID;
}

void oWinAccelFromHotKeys(ACCEL* _pAccels, const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys)
{
	for (size_t i = 0; i < _NumHotKeys; i++)
	{
		ACCEL& a = _pAccels[i];
		const oGUI_HOTKEY_DESC_NO_CTOR& h = _pHotKeys[i];
		a.fVirt = FVIRTKEY;
		if (h.AltDown) a.fVirt |= FALT;
		if (h.CtrlDown) a.fVirt |= FCONTROL;
		if (h.ShiftDown) a.fVirt |= FSHIFT;
		a.key = oWinKeyFromKey(h.HotKey) & 0xffff;
		a.cmd = h.ID;
	}
}

void oWinAccelToHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, const ACCEL* _pAccels, size_t _NumHotKeys)
{
	for (size_t i = 0; i < _NumHotKeys; i++)
	{
		const ACCEL& a = _pAccels[i];
		oGUI_HOTKEY_DESC_NO_CTOR& h = _pHotKeys[i];
		oASSERT(a.fVirt & FVIRTKEY, "");
		oWINKEY_CONTROL_STATE dummy;
		h.HotKey = oWinKeyToKey(a.key);
		h.AltDown = !!(a.fVirt & FALT);
		h.CtrlDown = !!(a.fVirt & FCONTROL);
		h.ShiftDown = !!(a.fVirt & FSHIFT);
		h.ID = a.cmd;
	}
}

// A wrapper for MS's GetWindow() that changes the rules of traversal. The rules 
// for the "next" window are: _hWnd's first child, if not then the next 
// sibling, and if at end of sibling list then the first child of _hWnd's parent 
// (what should be the first sibling in _hWnd's sibling list. If the next window 
// has a "buddy" window (UPDOWN control), then the buddy becomes the next, since 
// it controls the events of the other buddy anyway.
static HWND oWinGetNextWindow(HWND _hWnd)
{
	oWINVP(_hWnd);
	HWND hNext = GetWindow(_hWnd, GW_CHILD);
	if (!hNext)
		hNext = GetWindow(_hWnd, GW_HWNDNEXT);
	if (!hNext) // try wrap around
	{
		HWND hParent;
		hParent = GetParent(_hWnd);
		if (!GetParent(hParent)) // if is top-level window bounce back down to first child
			hNext = GetWindow(hParent, GW_CHILD);
		else
			hNext = GetWindow(hParent, GW_HWNDNEXT);

		// handle case where there is exactly one control
		if (_hWnd == hNext)
			return nullptr;
	}

	if (hNext)
	{
		HWND hBuddy = oWinControlGetBuddy(hNext);
		if (hBuddy)
			hNext = hBuddy;
	}
	return hNext;
}

// Returns the last HWND in the specified _hWnd's child list. If not a parent, 
// then this returns nullptr.
static HWND oWinGetLastChild(HWND _hWnd)
{
	oWINVP(_hWnd);
	HWND hChild = GetWindow(_hWnd, GW_CHILD);
	if (hChild)
	{
		HWND hLastChild = GetWindow(hChild, GW_HWNDLAST);
		if (hLastChild) hChild = hLastChild;
	}
	return hChild;
}

// A wrapper for MS's GetWindow() that changes the rules of traversal. The rules
// for the "prev" window are: _hWnd's previous sibling, if at start of sibling 
// list, then get the last child of _hWnd, else get the last child of _hWnd's
// parent.
static HWND oWinGetPrevWindow(HWND _hWnd)
{
	oWINVP(_hWnd);
	HWND hPrev = GetWindow(_hWnd, GW_HWNDPREV);
	if (!hPrev)
	{
		hPrev = GetParent(_hWnd);
		if (hPrev && !GetParent(hPrev)) // don't pop up to top-level window, stay in controls
			hPrev = nullptr;
	}

	else
	{
		HWND hLastChild = oWinGetLastChild(hPrev);
		if (hLastChild) hPrev = hLastChild;
	}

	if (!hPrev) // try wrap around
	{
		hPrev = GetWindow(_hWnd, GW_HWNDLAST);
		HWND hLastChild = oWinGetLastChild(hPrev);
		if (hLastChild) hPrev = hLastChild;
	}

	return hPrev;
}

// Cycles through all next windows until the next valid tabstop is found
static HWND oWinGetNextTabStop()
{
	HWND hCurrent = GetFocus();
	HWND hNext = oWinGetNextWindow(hCurrent);
	while (hNext != hCurrent && hNext && !oWinControlIsTabStop(hNext))
		hNext = oWinGetNextWindow(hNext);
	return hNext;
}

// Cycles through all prev windows until the prev valid tabstop is found
static HWND oWinGetPrevTabStop()
{
	HWND hCurrent = GetFocus();
	HWND hPrev = oWinGetPrevWindow(hCurrent);
	while (hPrev != hCurrent && hPrev && !oWinControlIsTabStop(hPrev))
		hPrev = oWinGetPrevWindow(hPrev);
	return hPrev;
}

// No Wisdom from Google has yielded a rational Microsoft-way of handling 
// Ctrl-Tab and Shift-Ctrl-Tab in a way consistent with Microsoft-system 
// dialogs. This function wraps IsDialogMessage in a way that seems to conform
// more to MS standards than their own function by intercepting WM_KEYDOWN for
// VK_TAB.
static bool oIsDialogMessageEx(HWND _hWnd, MSG* _pMsg)
{
	if (!_hWnd)
		return false;

	oASSERT(oWinIsWindowThread(_hWnd) || !oWinExists(_hWnd), "Must be called from windows thread (HWND=%p)", _hWnd);

	if (_pMsg->message == WM_KEYDOWN && _pMsg->wParam == VK_TAB)
	{
		bool CtrlDown = (GetKeyState(VK_LCONTROL) & 0x1000) || (GetKeyState(VK_RCONTROL) & 0x1000);
		bool ShiftDown = (GetKeyState(VK_LSHIFT) & 0x1000) || (GetKeyState(VK_RSHIFT) & 0x1000);
		if (CtrlDown)
		{
			// @oooii-tony: Hmm, this won't behave well with multiple tab controls in
			// a single window
			HWND hTab = FindWindowEx(_hWnd, nullptr, WC_TABCONTROL, nullptr);
			while (hTab && (!IsWindowEnabled(hTab) || !IsWindowVisible(hTab)))
				hTab = FindWindowEx(_hWnd, hTab, WC_TABCONTROL, nullptr);

			if (hTab)
			{
				NMHDR n;
				n.hwndFrom = hTab;
				n.idFrom = GetDlgCtrlID(hTab);
				n.code = TCN_SELCHANGING;
				// @oooii-tony: Not sure what this should be because the docs say it 
				// isn't the case that n.idFrom is the ID to use, but I always observe 
				// the values being the same in debug spew
				WPARAM w = n.idFrom;
				SendMessage(_hWnd, WM_NOTIFY, w, (LPARAM)&n);
				oWinControlSelectSubItemRelative(hTab, ShiftDown ? -1 : 1);
				n.code = TCN_SELCHANGE;
				SendMessage(_hWnd, WM_NOTIFY, w, (LPARAM)&n);
				return true;
			}
		}

		else
		{
			HWND hNext = ShiftDown ? oWinGetPrevTabStop() : oWinGetNextTabStop();
			if (hNext)
			{
				oWinSetFocus(hNext);
				return true;
			}
		}
	}

	return !!IsDialogMessage(_hWnd, _pMsg);
}

thread_local static double sGetDispatchMessageTime = 0.0;
double oWinGetDispatchMessageTime()
{
	return sGetDispatchMessageTime;
}

bool oWinDispatchMessage(HWND _hWnd, HACCEL _hAccel, double _Timestamp, bool _WaitForNext)
{
	sGetDispatchMessageTime = _Timestamp;

	MSG msg;
	bool HasMessage = false;
	if (_WaitForNext)
		HasMessage = GetMessage(&msg, nullptr, 0, 0) > 0;
	else
		HasMessage = !!PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);

	if (HasMessage)
	{
		if (msg.message == WM_QUIT)
			return oErrorSetLast(std::errc::operation_canceled);

		if (!_hWnd)
			_hWnd = msg.hwnd;

		HasMessage = !!TranslateAccelerator(_hWnd, _hAccel, &msg);
		if (!HasMessage)
			HasMessage = oIsDialogMessageEx(_hWnd, &msg);

		if (!HasMessage)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Treat WM_NULL like the noop it is by indicating that there are no new messages
		if (msg.message != WM_NULL)
			return true;
	}

	return oErrorSetLast(std::errc::no_message_available);
}

bool oWinWake(HWND _hWnd)
{
	return !!PostMessage(_hWnd, WM_NULL, 0, 0);
}

bool oWinSetOwner(HWND _hWnd, HWND _hOwner)
{
	// Yes, you've read this right... Check the community add-ons:
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms633591(v=vs.85).aspx
	SetLastError(S_OK);
	if (!SetWindowLongPtr(_hWnd, GWLP_HWNDPARENT, (LONG_PTR)_hOwner) && GetLastError() != S_OK)
		return oWinSetLastError();
	return true;
}

HWND oWinGetOwner(HWND _hWnd)
{
	return (HWND)GetWindowLongPtr(_hWnd, GWLP_HWNDPARENT);
}

bool oWinExists(HWND _hWnd)
{
	return !!::IsWindow(_hWnd);
}

bool oWinHasFocus(HWND _hWnd)
{
	return oWinExists(_hWnd) && _hWnd == ::GetForegroundWindow();
}

bool oWinSetFocus(HWND _hWnd, bool _Focus)
{
	// @oooii-kevin: Technically this can be called from other threads as we can give focus to another window
	//oWINV(_hWnd);
	if (_Focus)
	{
		::SetForegroundWindow(_hWnd);
		::SetActiveWindow(_hWnd);
		::SetFocus(_hWnd);
		return true;
	}
	
	else
		return oWinSetFocus(GetNextWindow(_hWnd, GW_HWNDNEXT), true);
}

bool oWinIsEnabled(HWND _hWnd)
{
	oWINV(_hWnd);
	oVB_RETURN(IsWindowEnabled(_hWnd));
	return true;
}

bool oWinEnable(HWND _hWnd, bool _Enabled)
{
	oWINV(_hWnd);
	EnableWindow(_hWnd, BOOL(_Enabled));
	return true;
}

bool oWinIsAlwaysOnTop(HWND _hWnd)
{
	oWINV(_hWnd);
	return !!(GetWindowLong(_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
}

bool oWinSetAlwaysOnTop(HWND _hWnd, bool _AlwaysOnTop)
{
	oWINV(_hWnd);
	RECT r;
	oVB_RETURN(GetWindowRect(_hWnd, &r));
	return !!::SetWindowPos(_hWnd, _AlwaysOnTop ? HWND_TOPMOST : HWND_TOP, r.left, r.top, oWinRectW(r), oWinRectH(r), IsWindowVisible(_hWnd) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
}

bool oWinRestore(HWND _hWnd)
{
	oWINV(_hWnd);
	HWND hProgMan = FindWindow(0, "Program Manager");
	oASSERT(hProgMan, "Program Manager not found");
	oWinSetFocus(hProgMan);
	oWinSetFocus(_hWnd);
	ShowWindow(_hWnd, SW_SHOWDEFAULT);
	return true;
}

oGUI_WINDOW_STATE oWinGetState(HWND _hWnd)
{
	LONG_PTR style = GetWindowLongPtr(_hWnd, GWL_STYLE);
	if (!oWinExists(_hWnd)) return oGUI_WINDOW_NONEXISTANT;
	else if (!(style & WS_VISIBLE)) return oGUI_WINDOW_HIDDEN;
	else if (IsIconic(_hWnd)) return oGUI_WINDOW_MINIMIZED;
	else if (IsZoomed(_hWnd)) return oGUI_WINDOW_MAXIMIZED;
	else
	{
		if (oGUI_WINDOW_BORDERLESS == oWinGetStyle(_hWnd))
		{
			oDISPLAY_DESC PseudoFullscreen;
			oVERIFY(oDisplayEnum(oWinGetDisplayIndex(_hWnd), &PseudoFullscreen));
			RECT rFullscreen = oWinRectWH(PseudoFullscreen.WorkareaPosition, PseudoFullscreen.Mode.Size);
			RECT rClient;
			GetClientRect(_hWnd, &rClient);
			if (rFullscreen == rClient)
				return oGUI_WINDOW_FULLSCREEN_COOPERATIVE;
		}
	}

	return oGUI_WINDOW_RESTORED;
}

// Returns a value fit to be passed to ShowWindow()
static int oWinGetShowCommand(oGUI_WINDOW_STATE _State, bool _TakeFocus)
{
	switch (_State)
	{
		case oGUI_WINDOW_NONEXISTANT: return SW_HIDE;
		case oGUI_WINDOW_HIDDEN: return SW_HIDE;
		case oGUI_WINDOW_MINIMIZED: return _TakeFocus ? SW_SHOWMINIMIZED : SW_SHOWMINNOACTIVE;
		case oGUI_WINDOW_MAXIMIZED: return SW_SHOWMAXIMIZED;
		case oGUI_WINDOW_RESTORED: return _TakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE;
		case oGUI_WINDOW_FULLSCREEN_COOPERATIVE: return SW_SHOW;
		case oGUI_WINDOW_FULLSCREEN_EXCLUSIVE: return SW_SHOW;
		oNODEFAULT;
	}
}

bool oWinSetState(HWND _hWnd, oGUI_WINDOW_STATE _State, bool _TakeFocus)
{
	oWINV(_hWnd);
	if (oGUIIsFullscreen(_State))
	{
		oDISPLAY_DESC PseudoFullscreen;
		oVERIFY(oDisplayEnum(oWinGetDisplayIndex(_hWnd), &PseudoFullscreen));
		RECT r = oWinRectWH(PseudoFullscreen.WorkareaPosition, PseudoFullscreen.Mode.Size);
		oVERIFY(oWinSetStyle(_hWnd, oGUI_WINDOW_BORDERLESS, false, &r));
		oVERIFY(oWinSetState(_hWnd, oGUI_WINDOW_RESTORED, _TakeFocus));
	}
	
	else
	{
		// There's a known issue that a simple ShowWindow doesn't always work on 
		// some minimized apps. The WAR seems to be to set focus to anything else, 
		// then try to restore the app.
		if (_TakeFocus && oWinGetState(_hWnd) == oGUI_WINDOW_MINIMIZED && _State > oGUI_WINDOW_MINIMIZED)
		{
			HWND hProgMan = FindWindow(nullptr, "Program Manager");
			oASSERT(hProgMan, "Program Manager not found");
			oWinSetFocus(hProgMan);
		}

		if (!ShowWindow(_hWnd, oWinGetShowCommand(_State, _TakeFocus)))
			oVB_RETURN(RedrawWindow(_hWnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW));
	}

	return true;
}

oGUI_WINDOW_STYLE oWinGetStyle(HWND _hWnd)
{
	#define oFIXED_STYLE (WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX)
	LONG_PTR style = GetWindowLongPtr(_hWnd, GWL_STYLE);
	if ((style & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW) return oGUI_WINDOW_SIZEABLE;
	else if ((style & oFIXED_STYLE) == oFIXED_STYLE) return oGUI_WINDOW_FIXED;
	else if ((style & WS_POPUP) == WS_POPUP) return oGUI_WINDOW_BORDERLESS;
	else if ((style & WS_CHILD) == WS_CHILD) return oGUI_WINDOW_EMBEDDED;
	return oGUI_WINDOW_DIALOG;
}

static DWORD oWinGetStyle(oGUI_WINDOW_STYLE _Style)
{
	switch (_Style)
	{
		case oGUI_WINDOW_EMBEDDED: return WS_CHILD;
		case oGUI_WINDOW_BORDERLESS: return WS_POPUP;
		case oGUI_WINDOW_DIALOG: return WS_CAPTION;
		case oGUI_WINDOW_FIXED: return WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
		default: return WS_OVERLAPPEDWINDOW;
	}
}

bool oWinSetStyle(HWND _hWnd, oGUI_WINDOW_STYLE _Style, bool _HasStatusBar, const RECT* _prClient)
{
	oWINV(_hWnd);

	// Basically only change the bits we mean to change and preserve the others
	DWORD dwCurrentStyleFlags = oWinGetStyle(oWinGetStyle(_hWnd));
	DWORD dwAllFlags = (DWORD)GetWindowLongPtr(_hWnd, GWL_STYLE);
	dwAllFlags &=~ dwCurrentStyleFlags;
	dwAllFlags |= oWinGetStyle(_Style);

	UINT uFlags = SWP_NOZORDER|SWP_FRAMECHANGED;
	if (dwAllFlags & (WS_MAXIMIZE|WS_MINIMIZE)) // ignore user size/move settings if maximized or minimized
		uFlags |= SWP_NOMOVE|SWP_NOSIZE;

	RECT r;
	if (_prClient)
	{
		r = *_prClient;

		// Don't do a NOSIZE because we need to adjust for statusbar
		if (r.right == oDEFAULT || r.bottom == oDEFAULT)
		{
			RECT rCurrent;
			GetClientRect(_hWnd, &rCurrent);
			if (r.right == oDEFAULT) r.right = rCurrent.right;
			if (r.bottom == oDEFAULT) r.bottom = rCurrent.bottom;
		}

		if (_HasStatusBar)
			oWinStatusBarAdjustClientRect(_hWnd, &r);
	}

	else
		oVB_RETURN(oWinGetClientScreenRect(_hWnd, _HasStatusBar, &r));

	bool HasMenu = !!GetMenu(_hWnd);

	oVB_RETURN(AdjustWindowRect(&r, dwAllFlags, HasMenu));

	SetLastError(0); // http://msdn.microsoft.com/en-us/library/ms644898(VS.85).aspx
	oVB_RETURN(SetWindowLongPtr(_hWnd, GWL_STYLE, dwAllFlags));
	oVB_RETURN(SetWindowPos(_hWnd, 0, r.left, r.top, oWinRectW(r), oWinRectH(r), uFlags));
	return true;
}

bool oWinGetClientRect(HWND _hWnd, RECT* _pRect)
{
	oWINV(_hWnd);
	if (!_pRect)
		return oErrorSetLast(std::errc::invalid_argument);
	oVB_RETURN(GetClientRect(_hWnd, _pRect));

	int StatusBarHeight = oWinStatusBarGetHeight(_hWnd);
	if (StatusBarHeight != oInvalid)
		_pRect->bottom = __max(_pRect->top, _pRect->bottom - StatusBarHeight);
	return true;
}

bool oWinGetClientScreenRect(HWND _hWnd, bool _HasStatusBar, RECT* _pRect)
{
	oWINV(_hWnd);
	if (!_pRect)
		return oErrorSetLast(std::errc::invalid_argument);
	oVB_RETURN(GetClientRect(_hWnd, _pRect));

	if (_HasStatusBar)
		oWinStatusBarAdjustClientRect(_hWnd, _pRect);

	POINT p = { _pRect->left, _pRect->top };
	oVB_RETURN(ClientToScreen(_hWnd, &p));
	*_pRect = oWinRectTranslate(*_pRect, p);
	return true;
}

int2 oWinGetPosition(HWND _hWnd)
{
	if (!IsWindow(_hWnd))
		return int2(oDEFAULT, oDEFAULT);
	POINT p = {0,0};
	ClientToScreen(_hWnd, &p);
	return int2(p.x, p.y);
}

bool oWinSetPosition(HWND _hWnd, const int2& _ScreenPosition)
{
	oWINV(_hWnd);
	RECT r = oWinRectWH(_ScreenPosition, int2(0,0));
	oVB_RETURN(AdjustWindowRect(&r, oWinGetStyle(oWinGetStyle(_hWnd)), FALSE));
	oVB_RETURN(SetWindowPos(_hWnd, 0, r.left, r.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER));
	return true;
}

bool oWinSetPositionAndSize(HWND _hWnd, const int2& _Position, const int2& _Size)
{
	oWINV(_hWnd);
	RECT r = oWinRectWH(_Position, _Size);
	oVB_RETURN(AdjustWindowRect(&r, oWinGetStyle(oWinGetStyle(_hWnd)), FALSE));
	oVB_RETURN(SetWindowPos(_hWnd, 0, r.left, r.top, 0, 0, SWP_NOZORDER));
	return true;
}

bool oWinAnimate(HWND _hWnd, const RECT& _From, const RECT& _To)
{
	oWINV(_hWnd);
	ANIMATIONINFO ai;
	ai.cbSize = sizeof(ai);
	SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);
	if (ai.iMinAnimate)
		return !!DrawAnimatedRects(_hWnd, IDANI_CAPTION, &_From, &_To);
	return true;
}

RECT oWinGetParentRect(HWND _hWnd, HWND _hExplicitParent)
{
	HWND hParent = _hExplicitParent ? _hExplicitParent : GetParent(_hWnd);
	RECT rParent;
	if (hParent)
		oWinGetClientRect(hParent, &rParent);
	else
	{
		oDISPLAY_DESC DDesc;
		oDisplayEnum(oWinGetDisplayIndex(_hWnd), &DDesc);
		rParent = oWinRectWH(DDesc.WorkareaPosition, DDesc.WorkareaSize);
	}
	return rParent;
}

RECT oWinGetRelativeRect(HWND _hWnd, HWND _hExplicitParent)
{
	HWND hParent = _hExplicitParent ? _hExplicitParent : GetParent(_hWnd);
	if (!hParent)
		hParent = GetDesktopWindow();

	RECT r;
	GetWindowRect(_hWnd, &r);
	ScreenToClient(hParent, (POINT*)&r);
	ScreenToClient(hParent, &((POINT*)&r)[1]);
	return r;	
}

int oWinGetDisplayIndex(HWND _hWnd)
{
	DISPLAY_DEVICE dev;
	return oWinGetDisplayDevice(MonitorFromWindow(_hWnd, MONITOR_DEFAULTTONEAREST), &dev);
}

HBRUSH oWinGetBackgroundBrush(HWND _hWnd)
{
	return (HBRUSH)GetClassLongPtr(_hWnd, GCLP_HBRBACKGROUND);
}

HFONT oWinGetFont(HWND _hWnd)
{
	return (HFONT)SendMessage(_hWnd, WM_GETFONT, 0, 0);
}

HFONT oWinGetDefaultFont()
{
	return (HFONT)GetStockObject(DEFAULT_GUI_FONT);
}

bool oWinSetFont(HWND _hWnd, HFONT _hFont)
{
	oWINV(_hWnd);
	if (!_hFont)
		_hFont = oWinGetDefaultFont();
	SendMessage(_hWnd, WM_SETFONT, (WPARAM)_hFont, TRUE);
	return true;
}

bool oWinSetText(HWND _hWnd, const char* _Text, int _SubItemIndex)
{
	oWINV(_hWnd);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	switch (type)
	{
		case oGUI_CONTROL_COMBOTEXTBOX:
			if (_SubItemIndex == oInvalid && !SetWindowText(_hWnd, _Text))
				return oWinSetLastError();
			// pass through
		case oGUI_CONTROL_COMBOBOX:
			if (_SubItemIndex != oInvalid)
			{
				oVERIFY(oWinControlDeleteSubItem(_hWnd, _Text, _SubItemIndex));
				oVERIFY(oWinControlInsertSubItem(_hWnd, _Text, _SubItemIndex));
			}
			break;

		case oGUI_CONTROL_TAB:
		{
			TCITEM tci;
			memset(&tci, 0, sizeof(tci));
			tci.dwStateMask = TCIF_TEXT;
			tci.pszText = (LPSTR)_Text;

			if (!TabCtrl_SetItem(_hWnd, _SubItemIndex, &tci))
				return oWinSetLastError();

			break;
		}

		case oGUI_CONTROL_FLOATBOX:
		case oGUI_CONTROL_FLOATBOX_SPINNER:
			return oErrorSetLast(std::errc::invalid_argument, "For FloatBoxes and FloatBoxSpinners, use oWinControlSetValue instead.");

		default: 
			if (!SetWindowText(_hWnd, _Text))
				return oWinSetLastError();
	}

	return true;
}

size_t oWinGetTruncatedLength(HWND _hWnd, const char* _StrSource)
{
	oStd::xxlstring temp(_StrSource);

	RECT rClient;
	GetClientRect(_hWnd, &rClient);
	oGDIScopedGetDC hDC(_hWnd);

	if (!oWinShlwapi::Singleton()->PathCompactPathA(hDC, temp, oWinRectW(rClient) + 70)) // @oooii-tony: This constant was measured empirically. I think this should work without the +70, but truncation happens too aggressively.
	{
		oErrorSetLast(std::errc::no_buffer_space, "Buffer must be at least MAX_PATH (%u) chars big", MAX_PATH);
		return 0;
	}

	if (!oStrcmp(temp, _StrSource))
	{
		oErrorSetLast(std::errc::operation_in_progress, "Truncation not required");
		return 0;
	}

	return temp.length();
}

char* oWinTruncateLeft(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, const char* _StrSource)
{
	size_t TruncatedLength = oWinGetTruncatedLength(_hWnd, _StrSource);
	if (TruncatedLength)
	{
		oStd::uri_string Truncated;
		const char* pCopy = _StrSource + oStrlen(_StrSource) - TruncatedLength + 3;
		oASSERT(pCopy >= _StrSource, "");
		if (-1 == oPrintf(_StrDestination, _SizeofStrDestination, "...%s", pCopy))
		{
			oErrorSetLast(std::errc::no_buffer_space, "");
			return nullptr;
		}
	}

	return oErrorGetLast() == std::errc::operation_in_progress ? nullptr : _StrDestination;
}

char* oWinTruncatePath(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, const char* _Path)
{
	if (_SizeofStrDestination < MAX_PATH)
		return nullptr; // can't pass this to the function

	oStrcpy(_StrDestination, _SizeofStrDestination, _Path);
	RECT rClient;
	GetClientRect(_hWnd, &rClient);
	oGDIScopedGetDC hDC(_hWnd);

	if (!oWinShlwapi::Singleton()->PathCompactPathA(hDC, _StrDestination, oWinRectW(rClient) + 70)) // @oooii-tony: This constant was measured empirically. I think this should work without the +70, but truncation happens too aggressively.
	{
		oErrorSetLast(std::errc::no_buffer_space, "Buffer must be at least MAX_PATH (%u) chars big", MAX_PATH);
		return nullptr;
	}

	return _StrDestination;
}

char* oWinGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, int _SubItemIndex)
{
	oWINVP(_hWnd);
	oGUI_CONTROL_TYPE type = oGUI_CONTROL_UNKNOWN;
	if (_SubItemIndex != oInvalid)
		type = oWinControlGetType(_hWnd);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
		{
			size_t len = (size_t)ComboBox_GetLBTextLen(_hWnd, _SubItemIndex);
			if (len > _SizeofStrDestination)
			{
				oErrorSetLast(std::errc::no_buffer_space);
				return nullptr;
			}
			else
			{
				if (CB_ERR != ComboBox_GetLBText(_hWnd, _SubItemIndex, _StrDestination))
					return _StrDestination;
			}
		}

		case oGUI_CONTROL_TAB:
		{
			TCITEM item;
			item.mask = TCIF_TEXT;
			item.pszText = (LPSTR)_StrDestination;
			item.cchTextMax = oInt(_SizeofStrDestination);
			if (TabCtrl_GetItem(_hWnd, _SubItemIndex, &item))
				return _StrDestination;
			else
			{
				oWinSetLastError();
				return nullptr;
			}
		}

		default:
			if (GetWindowText(_hWnd, _StrDestination, oInt(_SizeofStrDestination)))
				return _StrDestination;
			else
			{
				oWinSetLastError();
				return nullptr;
			}
	}
}

HICON oWinGetIcon(HWND _hWnd, bool _BigIcon)
{
	oWINVP(_hWnd);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	switch (type)
	{
		case oGUI_CONTROL_ICON: return (HICON)SendMessage(_hWnd, STM_GETICON, 0, 0);
		default: return (HICON)SendMessage(_hWnd, WM_GETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), 0);
	}
}

bool oWinSetIconAsync(HWND _hWnd, HICON _hIcon, bool _BigIcon)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	if (oWinIsWindowThread(_hWnd))
	{
		switch (type)
		{
			case oGUI_CONTROL_ICON: SendMessage(_hWnd, STM_SETICON, (WPARAM)_hIcon, 0); break;
			default: SendMessage(_hWnd, WM_SETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), (LPARAM)_hIcon); break;
		}

		return true;
	}
	
	else
	{
		switch (type)
		{
			case oGUI_CONTROL_ICON: PostMessage(_hWnd, STM_SETICON, (WPARAM)_hIcon, 0); break;
			default: PostMessage(_hWnd, WM_SETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), (LPARAM)_hIcon); break;
		}

		return oErrorSetLast(std::errc::operation_not_permitted);
	}
}

bool oWinSetIcon(HWND _hWnd, HICON _hIcon, bool _BigIcon)
{
	oWINVP(_hWnd);
	return oWinSetIconAsync(_hWnd, _hIcon, _BigIcon);
}

static const char* oSubclassFloatBoxFormat = "%.04f";
static UINT_PTR oSubclassFloatBoxID = 0x13370000;
static LRESULT CALLBACK oSubclassProcFloatBox(HWND _hControl, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, UINT_PTR _uIdSubclass, DWORD_PTR _dwRefData)
{
	if (_uIdSubclass == oSubclassFloatBoxID)
	{
		switch (_uMsg)
		{
			case WM_KILLFOCUS:
			{
				oStd::mstring text;
				GetWindowText(_hControl, text, (int)text.capacity());
				float f = 0.0f;
				oStd::atof(text, &f);
				oPrintf(text, oSubclassFloatBoxFormat, f);
				SetWindowText(_hControl, text);
				break;
			}

			case WM_SETTEXT:
			{
				float f = 0.0f;
				oStd::mstring text = (const wchar_t*)_lParam; // handle wchar
				if (!oStd::atof(text, &f))
					return FALSE;
				
				// Ensure consistent formatting
				oPrintf(text, oSubclassFloatBoxFormat, f);
				oStd::mwstring wtext = text;
				oWinCommCtrl::Singleton()->DefSubclassProc(_hControl, _uMsg, _wParam, (LPARAM)wtext.c_str());
				return FALSE;
			}

			case WM_GETDLGCODE:
			{
				// probably copy/paste, so let that through
				if ((GetKeyState(VK_LCONTROL) & 0x1000) || (GetKeyState(VK_RCONTROL) & 0x1000))
					return DLGC_WANTALLKEYS;

				oStd::mstring text;
				GetWindowText(_hControl, text, (int)text.capacity());

				// refresh/commit if the user presses enter
				if (_wParam == VK_RETURN)
				{
					SetWindowText(_hControl, text.c_str());
					SendMessage(_hControl, EM_SETSEL, text.length(), text.length());
					return DLGC_WANTARROWS;
				}

				// Only consider allowable keys at all
				static const WPARAM sAllowableKeys[] = 
				{
					'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '.', '-', '+',
					VK_BACK, VK_DELETE, VK_HOME, VK_END,
				};
				int i = 0;
				for (; i < oCOUNTOF(sAllowableKeys); i++)
					if (sAllowableKeys[i] == _wParam) break;
				
				if (i >= oCOUNTOF(sAllowableKeys))
					return DLGC_WANTARROWS;

				DWORD dwStart = oInvalid, dwEnd = oInvalid;
				SendMessage(_hControl, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

				// allow only one '.', unless it's in the selecte d text about to be replaced
				bool selHasDot = false;
				for (DWORD i = dwStart; i < dwEnd; i++)
					if (text[i] == '.')
					{
						selHasDot = true;
						break;
					}

				if (!selHasDot && _wParam == '.' && strchr(text, '.'))
					return DLGC_WANTARROWS;

				// only one of either + or -, and it better be at the front
				if ((_wParam == '+' || _wParam == '-') && (dwStart != 0 || strchr(text, '+') || strchr(text, '-')))
					return DLGC_WANTARROWS;

				return DLGC_WANTALLKEYS;
			}

			default:
				break;
		}
	}

	return oWinCommCtrl::Singleton()->DefSubclassProc(_hControl, _uMsg, _wParam, _lParam);
}

static UINT_PTR oSubclassTabID = 0x13370001;
static LRESULT CALLBACK oSubclassProcTab(HWND _hControl, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, UINT_PTR _uIdSubclass, DWORD_PTR _dwRefData)
{
	if (_uIdSubclass == oSubclassTabID)
	{
		switch (_uMsg)
		{
			case WM_CTLCOLORSTATIC:
				return (INT_PTR)0;
			case WM_COMMAND:
			case WM_NOTIFY:
				return SendMessage(GetParent(_hControl), _uMsg, _wParam, _lParam);
			default:
				break;
		}
	}

	return oWinCommCtrl::Singleton()->DefSubclassProc(_hControl, _uMsg, _wParam, _lParam);
}

static UINT_PTR oSubclassGroupID = 0x13370002;
static LRESULT CALLBACK oSubclassProcGroup(HWND _hControl, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, UINT_PTR _uIdSubclass, DWORD_PTR _dwRefData)
{
	if (_uIdSubclass == oSubclassGroupID)
	{
		switch (_uMsg)
		{
			case WM_CTLCOLORSTATIC:
			{
				HBRUSH hBrush = oWinGetBackgroundBrush(_hControl);
				return (INT_PTR)hBrush;
			}
			case WM_COMMAND:
			case WM_NOTIFY:
				return SendMessage(GetParent(_hControl), _uMsg, _wParam, _lParam);

			default:
				break;
		}
	}

	return oWinCommCtrl::Singleton()->DefSubclassProc(_hControl, _uMsg, _wParam, _lParam);
}

static bool OnCreateAddSubItems(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	if (strchr(_Desc.Text, '|'))
	{
		if (!oWinControlAddSubItems(_hControl, _Desc.Text))
			return false;
		if (!oWinControlSelectSubItem(_hControl, 0))
			return false;
	}

	return true;
}

static bool OnCreateTab(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	if (!oWinCommCtrl::Singleton()->SetWindowSubclass(_hControl, oSubclassProcTab, oSubclassTabID, (DWORD_PTR)nullptr))
		return false;
	return OnCreateAddSubItems(_hControl, _Desc);
}

static bool OnCreateGroup(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	return !!oWinCommCtrl::Singleton()->SetWindowSubclass(_hControl, oSubclassProcGroup, oSubclassGroupID, (DWORD_PTR)nullptr);
}

static bool OnCreateFloatBox(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	if (!oWinCommCtrl::Singleton()->SetWindowSubclass(_hControl, oSubclassProcFloatBox, oSubclassFloatBoxID, (DWORD_PTR)nullptr))
		return false;

	SetWindowText(_hControl, _Desc.Text);
	return true;
}

static bool OnCreateIcon(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	oWinControlSetIcon(_hControl, (HICON)_Desc.Text);
	return true;
}

static bool OnCreateFloatBoxSpinner(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	oGUI_CONTROL_DESC ActualFloatBoxDesc = _Desc;
	ActualFloatBoxDesc.Type = oGUI_CONTROL_FLOATBOX;
	HWND hActualFloatBox = oWinControlCreate(ActualFloatBoxDesc);
	if (hActualFloatBox)
	{
		SendMessage(_hControl, UDM_SETBUDDY, (WPARAM)hActualFloatBox, 0);
		SendMessage(_hControl, UDM_SETRANGE32, (WPARAM)std::numeric_limits<int>::lowest(), (LPARAM)std::numeric_limits<int>::max());
	}

	else
		return oWinSetLastError();

	return true;
}

static bool OnCreateMarquee(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	SendMessage(_hControl, PBM_SETMARQUEE, 1, 0);
	return true;
}

static int2 GetSizeExplicit(const oGUI_CONTROL_DESC& _Desc)
{
	return _Desc.Size;
}

static int2 GetSizeButtonDefault(const oGUI_CONTROL_DESC& _Desc)
{
	return oWinRectResolveSize(_Desc.Size, int2(75,23));
}

static int2 GetSizeIcon(const oGUI_CONTROL_DESC& _Desc)
{
	int2 iconSize = oGDIGetIconSize((HICON)_Desc.Text);
	return oWinRectResolveSize(_Desc.Size, iconSize);
}

struct CONTROL_CREATION_DESC
{
	const char* ClassName;
	DWORD dwStyle;
	DWORD dwExStyle;
	bool SetText;
	bool (*FinishCreation)(HWND _hControl, const oGUI_CONTROL_DESC& _Desc);
	int2 (*GetSize)(const oGUI_CONTROL_DESC& _Desc);
};

static const CONTROL_CREATION_DESC& oWinControlGetCreationDesc(oGUI_CONTROL_TYPE _Type)
{
	// === IF ADDING A NEW TYPE, MAKE SURE YOU ADD AN ENTRY TO oWinControlGetType ===

	static const CONTROL_CREATION_DESC sDescs[] = 
	{
		{ "Unknown", 0, 0, false, nullptr, GetSizeExplicit, },
		{ "Button", WS_VISIBLE|WS_CHILD|BS_GROUPBOX, 0, true, OnCreateGroup, GetSizeExplicit, },
		{ "Button", WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_MULTILINE|BS_PUSHBUTTON, 0, true, nullptr, GetSizeButtonDefault, },
		{ "Button", WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_MULTILINE|BS_TOP|BS_AUTOCHECKBOX, 0, true, nullptr, GetSizeExplicit, },
		{ "Button", WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_MULTILINE|BS_TOP|BS_AUTORADIOBUTTON, 0, true, nullptr, GetSizeExplicit, },
		{ "Static", WS_VISIBLE|WS_CHILD|SS_EDITCONTROL, 0, true, nullptr, GetSizeExplicit, },
		{ "SysLink", WS_VISIBLE|WS_CHILD|WS_TABSTOP|LWS_NOPREFIX, 0, true, nullptr, GetSizeExplicit, },
		{ "Edit", WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_READONLY|ES_NOHIDESEL|ES_AUTOHSCROLL, 0, true, nullptr, GetSizeExplicit, },
		{ "Static", WS_VISIBLE|WS_CHILD|SS_ICON|SS_REALSIZECONTROL, 0, false, OnCreateIcon, GetSizeIcon, },
		{ "Edit", WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_NOHIDESEL|ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_MULTILINE|ES_WANTRETURN, WS_EX_CLIENTEDGE, 0, nullptr, GetSizeExplicit, },
		{ "Edit", WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_NOHIDESEL|ES_AUTOHSCROLL|ES_AUTOVSCROLL|WS_VSCROLL|ES_MULTILINE|ES_WANTRETURN, WS_EX_CLIENTEDGE, 0, nullptr, GetSizeExplicit, },
		{ "Edit", WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_NOHIDESEL|ES_AUTOHSCROLL|ES_RIGHT, WS_EX_CLIENTEDGE, false, OnCreateFloatBox, GetSizeExplicit, },
		{ UPDOWN_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP|UDS_ALIGNRIGHT|UDS_ARROWKEYS|UDS_HOTTRACK|UDS_NOTHOUSANDS|UDS_WRAP, 0,false, OnCreateFloatBoxSpinner, GetSizeExplicit, },
		{ "Combobox", WS_VISIBLE|WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST|CBS_HASSTRINGS, 0, false, OnCreateAddSubItems, GetSizeExplicit, },
		{ "Combobox", WS_VISIBLE|WS_CHILD|WS_TABSTOP|CBS_DROPDOWN|CBS_HASSTRINGS, 0, false, OnCreateAddSubItems, GetSizeExplicit, },
		{ PROGRESS_CLASS, WS_VISIBLE|WS_CHILD, 0, true, nullptr, GetSizeExplicit, },
		{ PROGRESS_CLASS, WS_VISIBLE|WS_CHILD|PBS_MARQUEE, 0, true, OnCreateMarquee, GetSizeExplicit, },
		{ WC_TABCONTROL, WS_VISIBLE|WS_CHILD|WS_TABSTOP, WS_EX_CONTROLPARENT, true, OnCreateTab, GetSizeExplicit, },
		{ TRACKBAR_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP|TBS_NOTICKS, 0, true, nullptr, GetSizeExplicit, },
		{ TRACKBAR_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP|TBS_NOTICKS|TBS_ENABLESELRANGE, 0, true, nullptr, GetSizeExplicit, },
		{ TRACKBAR_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP, 0, true, nullptr, GetSizeExplicit, }, 
	};
	static_assert(oGUI_CONTROL_TYPE_COUNT == oCOUNTOF(sDescs), "oGUI_CONTROL_TYPE_COUNT count mismatch");
	return sDescs[_Type];
}

HWND oWinControlCreate(const oGUI_CONTROL_DESC& _Desc)
{
	if (!_Desc.hParent || _Desc.Type == oGUI_CONTROL_UNKNOWN)
		return (HWND)oErrorSetLast(std::errc::invalid_argument);
	oWINVP((HWND)_Desc.hParent);

	const CONTROL_CREATION_DESC& CCDesc = oWinControlGetCreationDesc(_Desc.Type);
	int2 size = CCDesc.GetSize(_Desc);

	HWND hWnd = CreateWindowEx(
		CCDesc.dwExStyle
		, CCDesc.ClassName
		, (CCDesc.SetText && _Desc.Text) ? _Desc.Text : ""
		, CCDesc.dwStyle | (_Desc.StartsNewGroup ? WS_GROUP : 0)
		, _Desc.Position.x
		, _Desc.Position.y
		, size.x
		, size.y
		, (HWND)_Desc.hParent
		, (HMENU)_Desc.ID
		, nullptr
		, nullptr);

	if (hWnd)
	{
		oWinControlSetFont(hWnd, (HFONT)_Desc.hFont);
		if (CCDesc.FinishCreation)
		{
			if (!CCDesc.FinishCreation(hWnd, _Desc))
			{
				DestroyWindow(hWnd);
				hWnd = nullptr;
			}
		}
	}

	else
		oWinSetLastError();

	return hWnd;
}

bool oWinControlDefaultOnNotify(HWND _hControl, const NMHDR& _NotifyMessageHeader, LRESULT* _plResult, oGUI_CONTROL_TYPE _Type)
{
	oWINV(_hControl);
	bool ShortCircuit = false;
	if (_Type == oGUI_CONTROL_UNKNOWN)
		_Type = oWinControlGetType(_NotifyMessageHeader.hwndFrom);
	switch (_Type)
	{
		case oGUI_CONTROL_FLOATBOX_SPINNER:
			if (_NotifyMessageHeader.code == UDN_DELTAPOS)
			{
				HWND hBuddyFloatBox =oWinControlGetBuddy(_NotifyMessageHeader.hwndFrom);
				if (hBuddyFloatBox)
				{
					const NMUPDOWN& ud = (const NMUPDOWN&)_NotifyMessageHeader;
					float f = oWinControlGetFloat(hBuddyFloatBox);
					oWinControlSetValue(hBuddyFloatBox, f + 0.01f * ud.iDelta);
				}
			}
			ShortCircuit = true;
			break;

		case oGUI_CONTROL_HYPERLABEL:
		{
			if (_NotifyMessageHeader.code == NM_CLICK || _NotifyMessageHeader.code == NM_RETURN)
			{
				const NMLINK& NMLink = (const NMLINK&)_NotifyMessageHeader;
				oStd::xlstring asMultiByte(NMLink.item.szUrl);
				oVERIFY(oWinSystemOpenDocument(asMultiByte));
			}

			ShortCircuit = true;
			break;
		}

		default: break;
	}

	if (_plResult)
		*_plResult = FALSE;

	return ShortCircuit;
}

bool oWinControlToAction(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, oGUI_ACTION_DESC* _pAction, LRESULT* _pLResult)
{
	*_pLResult = 0;
	bool Handled = true;

	switch (_uMsg)
	{
		case WM_COMMAND:
		{
			_pAction->DeviceType = oGUI_INPUT_DEVICE_CONTROL;
			_pAction->DeviceID = LOWORD(_wParam);
			_pAction->hWindow = (oGUI_WINDOW)_hWnd;
			_pAction->Key = oGUI_KEY_NONE;
			_pAction->Position = 0.0f;

			if (!_lParam)
			{
				_pAction->Action = (HIWORD(_wParam) == 1) ? oGUI_ACTION_HOTKEY : oGUI_ACTION_MENU;
				_pAction->ActionCode = oInvalid;
			}
			else
			{
				_pAction->hWindow = (oGUI_WINDOW)_lParam;
				_pAction->Action = oGUI_ACTION_CONTROL_ACTIVATED;
				_pAction->ActionCode = HIWORD(_wParam);
			}

			break;
		}

		case WM_HSCROLL:
		{
			if (_lParam != 0)
			{
				_pAction->hWindow = (oGUI_WINDOW)(_lParam);
				_pAction->DeviceType = oGUI_INPUT_DEVICE_CONTROL;
				_pAction->DeviceID = oInvalid;
				_pAction->Key = oGUI_KEY_NONE;
				_pAction->Position = 0.0f;
				_pAction->ActionCode = oInt(_wParam);
				switch (LOWORD(_wParam))
				{
					case TB_ENDTRACK: _pAction->Action = oGUI_ACTION_CONTROL_DEACTIVATED; break;
					default: _pAction->Action = oGUI_ACTION_CONTROL_ACTIVATED; break;
				}
				break;
			}

			Handled = false;
			break;
		}

		case WM_NOTIFY:
		{
			LRESULT lResult = FALSE;
			const NMHDR& nmhdr = *(const NMHDR*)_lParam;
			*_pAction = oGUI_ACTION_DESC();
			_pAction->DeviceType = oGUI_INPUT_DEVICE_CONTROL;
			_pAction->DeviceID = oInt(nmhdr.idFrom);
			_pAction->hWindow = (oGUI_WINDOW)nmhdr.hwndFrom;
			_pAction->Position = 0.0f;
			_pAction->ActionCode = nmhdr.code;

			oGUI_CONTROL_TYPE type = oWinControlGetType(nmhdr.hwndFrom);
			switch (type)
			{
				case oGUI_CONTROL_TAB:
				{
					switch (_pAction->ActionCode)
					{
					case TCN_SELCHANGING: _pAction->Action = oGUI_ACTION_CONTROL_SELECTION_CHANGING; break;
					case TCN_SELCHANGE: _pAction->Action = oGUI_ACTION_CONTROL_SELECTION_CHANGED; break;
					default: break;
					}

					break;
				}

				case oGUI_CONTROL_BUTTON:
				{
					switch (_pAction->ActionCode)
					{
					case BN_CLICKED: _pAction->Action = oGUI_ACTION_CONTROL_ACTIVATED; break;
					default: break;
					}

					break;
				}

				default: break;
			}

			Handled = !oWinControlDefaultOnNotify(_hWnd, nmhdr, _pLResult, type);
			break;
		}

		default: Handled = false; break;
	}

	return Handled;
}

int oWinControlGetNumSubItems(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX: return ComboBox_GetCount(_hControl);
		case oGUI_CONTROL_TAB: return TabCtrl_GetItemCount(_hControl);
		default: return 0;
	}
}

bool oWinControlClearSubItems(HWND _hControl)
{
	oWINV(_hControl);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
			ComboBox_ResetContent(_hControl);
			return true;
		case oGUI_CONTROL_TAB:
			if (!TabCtrl_DeleteAllItems(_hControl))
				return oWinSetLastError();
			return true;
		default:
				break;
	}

	return true;
}

int oWinControlInsertSubItem(HWND _hControl, const char* _SubItemText, int _SubItemIndex)
{
	oWINV(_hControl);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
		{
			int index = CB_ERR;
			if (_SubItemIndex >= 0 && _SubItemIndex < ComboBox_GetCount(_hControl))
				index = ComboBox_InsertString(_hControl, _SubItemIndex, _SubItemText);
			else
				index = ComboBox_AddString(_hControl, _SubItemText);

			if (index == CB_ERRSPACE)
			{
				oErrorSetLast(std::errc::no_buffer_space, "String is too large");
				index = CB_ERR;
			}

			return index;
		}

		case oGUI_CONTROL_TAB:
		{
			TCITEM item;
			item.mask = TCIF_TEXT;
			item.pszText = (LPSTR)_SubItemText;
			int count = TabCtrl_GetItemCount(_hControl);
			int index = TabCtrl_InsertItem(_hControl, ((_SubItemIndex >= 0) ? _SubItemIndex : count), &item);
			if (CB_ERR == index)
				oErrorSetLastBadIndex(_hControl, type, _SubItemIndex);
			return index;
		}

		default:
			break;
	}

	oErrorSetLastBadType(_hControl, type);
	return oInvalid;
}

bool oWinControlDeleteSubItem(HWND _hControl, const char* _SubItemText, int _SubItemIndex)
{
	oWINV(_hControl);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
		{
			if (_SubItemIndex >= 0)
				ComboBox_DeleteString(_hControl, _SubItemIndex);
			break;
		}

		case oGUI_CONTROL_TAB:
			TabCtrl_DeleteItem(_hControl, _SubItemIndex);
			break;

			default:
				break;
		}

	return true;
}

bool oWinControlAddSubItems(HWND _hControl, const char* _DelimitedString, char _Delimiter)
{
	oWINV(_hControl);
	char delim[2] = { _Delimiter, 0 };
	char* ctx = nullptr;
	const char* tok = oStrTok(_DelimitedString, delim, &ctx);
	while (tok)
	{
		oWinControlInsertSubItem(_hControl, tok, oInvalid);
		tok = oStrTok(nullptr, delim, &ctx);
	}

	if (!oStrTokFinishedSuccessfully(&ctx))
		return oErrorSetLast(std::errc::protocol_error, "Failed to parse tokenized string for combobox values");
	return true;
}

int oWinControlFindSubItem(HWND _hControl, const char* _SubItemText)
{
	int index = CB_ERR;
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
		{
			int index = ComboBox_FindStringExact(_hControl, 0, _SubItemText);
			if (index == CB_ERR)
				oErrorSetLast(std::errc::invalid_argument, "Text %s was not found in %s %p (%d)", oSAFESTRN(_SubItemText), oStd::as_string(type), _hControl, GetDlgCtrlID(_hControl));
			break;
		}

		case oGUI_CONTROL_TAB:
		{
			oStd::mstring text;
			TCITEM item;
			item.mask = TCIF_TEXT;
			item.pszText = (LPSTR)text.c_str();
			item.cchTextMax = static_cast<int>(text.capacity());
			const int kCount = TabCtrl_GetItemCount(_hControl);
			for (int i = 0; i < kCount; i++)
			{
				if (TabCtrl_GetItem(_hControl, i, &item))
				{
					if (!oStrcmp(text, _SubItemText))
					{
						index = i;
						break;
					}
				}
			}

			break;
		}

	default:
		oErrorSetLastBadType(_hControl, type);
		break;
	}

	return index;
}

bool oWinControlSelectSubItem(HWND _hControl, int _SubItemIndex)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
			if (CB_ERR == ComboBox_SetCurSel(_hControl, _SubItemIndex))
				return oErrorSetLastBadIndex(_hControl, type, _SubItemIndex);
			break;

		case oGUI_CONTROL_TAB:
		{
			HWND hParent = GetParent(_hControl);
			int index = TabCtrl_GetCurSel(_hControl);
			NMHDR nm;
			nm.hwndFrom = _hControl;
			nm.idFrom = GetDlgCtrlID(_hControl);
			if (-1 != index)
			{
				nm.code = TCN_SELCHANGING;
				SendMessage(hParent, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
			}

			index = TabCtrl_SetCurSel(_hControl, _SubItemIndex);
			if (-1 == index)
				return oErrorSetLastBadIndex(_hControl, type, _SubItemIndex);

			nm.code = TCN_SELCHANGE;
			SendMessage(hParent, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);

			break;
		}
		default:
			oErrorSetLastBadType(_hControl, type);
	}

	return true;
}

bool oWinControlSelectSubItemRelative(HWND _hControl, int _Offset)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	int next = -1, count = 0;
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
			count = ComboBox_GetCount(_hControl);
			next = ComboBox_GetCurSel(_hControl) + _Offset;
			while (next < 0) next += count;
			while (next >= count) next -= count;
			if (CB_ERR == ComboBox_SetCurSel(_hControl, next))
				return oErrorSetLastBadIndex(_hControl, type, next);
			break;

		case oGUI_CONTROL_TAB:
			count = TabCtrl_GetItemCount(_hControl);
			next = TabCtrl_GetCurSel(_hControl) + _Offset;
			while (next < 0) next += count;
			while (next >= count) next -= count;
			if (-1 == TabCtrl_SetCurSel(_hControl, next))
				return oErrorSetLastBadIndex(_hControl, type, next);
			break;

		default:
			oErrorSetLastBadType(_hControl, type);
	}

	return true;
}

int oWinControlGetSelectedSubItem(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX: return ComboBox_GetCurSel(_hControl);
		case oGUI_CONTROL_TAB: return TabCtrl_GetCurSel(_hControl);
		default: break;
	}
	return oInvalid;
}

oGUI_CONTROL_TYPE oWinControlGetType(HWND _hControl)
{
	oStd::mstring ClassName;
	if (!GetClassName(_hControl, ClassName, static_cast<int>(ClassName.capacity())))
		return oGUI_CONTROL_UNKNOWN;
	
	if (!oStricmp("Button", ClassName))
	{
		LONG dwStyle = 0xff & GetWindowLong(_hControl, GWL_STYLE);
		switch (dwStyle)
		{
			case BS_GROUPBOX: return oGUI_CONTROL_GROUPBOX;
			case BS_PUSHBUTTON: 
			case BS_DEFPUSHBUTTON: return oGUI_CONTROL_BUTTON;
			case BS_AUTOCHECKBOX: return oGUI_CONTROL_CHECKBOX;
			case BS_AUTORADIOBUTTON: return oGUI_CONTROL_RADIOBUTTON;
			default: return oGUI_CONTROL_UNKNOWN;
		}
	}

	else if (!oStricmp("Static", ClassName))
	{
		LONG dwStyle = 0xff & GetWindowLong(_hControl, GWL_STYLE);
		if (dwStyle & SS_ICON)
			return oGUI_CONTROL_ICON;
		else if (dwStyle & SS_SIMPLE)
			return oGUI_CONTROL_LABEL;
		else
			return oGUI_CONTROL_UNKNOWN;
	}

	else if (!oStricmp("Edit", ClassName))
	{
		DWORD dwStyle = 0xffff & (DWORD)GetWindowLong(_hControl, GWL_STYLE);
		if (dwStyle == (dwStyle & oWinControlGetCreationDesc(oGUI_CONTROL_LABEL_SELECTABLE).dwStyle))
			return oGUI_CONTROL_LABEL_SELECTABLE;

			DWORD_PTR data;
		if (oWinCommCtrl::Singleton()->GetWindowSubclass(_hControl, oSubclassProcFloatBox, oSubclassFloatBoxID, &data))
			return oGUI_CONTROL_FLOATBOX;

		else if (dwStyle == (dwStyle & oWinControlGetCreationDesc(oGUI_CONTROL_TEXTBOX).dwStyle))
			return oGUI_CONTROL_TEXTBOX;

		else
			return oGUI_CONTROL_UNKNOWN;
	}

	else if (!oStricmp("ComboBox", ClassName))
	{
		LONG dwStyle = 0xf & GetWindowLong(_hControl, GWL_STYLE);
		switch (dwStyle)
		{
			case CBS_DROPDOWNLIST: return oGUI_CONTROL_COMBOBOX;
			case CBS_DROPDOWN: return oGUI_CONTROL_COMBOTEXTBOX;
			default: return oGUI_CONTROL_UNKNOWN;
		}
	}

	else if (!oStricmp(oWinControlGetCreationDesc(oGUI_CONTROL_FLOATBOX_SPINNER).ClassName, ClassName))
		return oGUI_CONTROL_FLOATBOX_SPINNER;

	else if (!oStricmp(oWinControlGetCreationDesc(oGUI_CONTROL_HYPERLABEL).ClassName, ClassName))
		return oGUI_CONTROL_HYPERLABEL;

	else if (!oStricmp(oWinControlGetCreationDesc(oGUI_CONTROL_TAB).ClassName, ClassName))
	{
		// Don't identify a generic tab as an oGUI tab, only ones that
		// behave properly
		DWORD_PTR data;
		if (oWinCommCtrl::Singleton()->GetWindowSubclass(_hControl, oSubclassProcTab, oSubclassTabID, &data))
			return oGUI_CONTROL_TAB;
		return oGUI_CONTROL_UNKNOWN;
	}
	else if (!oStricmp(oWinControlGetCreationDesc(oGUI_CONTROL_PROGRESSBAR).ClassName, ClassName))
	{
		DWORD dwStyle = (DWORD)GetWindowLong(_hControl, GWL_STYLE);
		if (dwStyle & PBS_MARQUEE)
			return oGUI_CONTROL_PROGRESSBAR_UNKNOWN;
		return oGUI_CONTROL_PROGRESSBAR;
	}
	else if (!oStricmp(oWinControlGetCreationDesc(oGUI_CONTROL_SLIDER).ClassName, ClassName))
	{
		DWORD dwStyle = (DWORD)GetWindowLong(_hControl, GWL_STYLE);

		if (dwStyle & TBS_ENABLESELRANGE)
			return oGUI_CONTROL_SLIDER_SELECTABLE;
		else if (dwStyle & TBS_NOTICKS)
			return oGUI_CONTROL_SLIDER;
		else
			return oGUI_CONTROL_SLIDER_WITH_TICKS;
	}
	return oGUI_CONTROL_UNKNOWN;
}

int2 oWinControlGetInitialSize(oGUI_CONTROL_TYPE _Type, const int2& _Size)
{
	oGUI_CONTROL_DESC d;
	d.Type = _Type;
	d.Size = _Size;
	return oWinControlGetCreationDesc(_Type).GetSize(d);
}

bool oWinControlIsTabStop(HWND _hControl)
{
	oWINV(_hControl);
	return IsWindowEnabled(_hControl) && IsWindowVisible(_hControl) && ((GetWindowLong(_hControl, GWL_STYLE) & WS_TABSTOP) == WS_TABSTOP);
}

int oWinControlGetID(HWND _hControl)
{
	return GetDlgCtrlID(_hControl);
}

HWND oWinControlGetFromID(HWND _hParent, unsigned short _ID)
{
	return GetDlgItem(_hParent, _ID);
}

RECT oWinControlGetRect(HWND _hControl)
{
	RECT r;
	GetClientRect(_hControl, &r);
	HWND hParent = GetParent(_hControl);
	if (hParent)
		MapWindowPoints(_hControl, hParent, (LPPOINT)&r, 2);
	return r;
}

char* oWinControlGetSelectedText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hControl)
{
	oWINVP(_hControl);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
	case oGUI_CONTROL_FLOATBOX_SPINNER:
		_hControl = oWinControlGetBuddy(_hControl);
		// pass through
	case oGUI_CONTROL_TEXTBOX:
	case oGUI_CONTROL_TEXTBOX_SCROLLABLE:
	case oGUI_CONTROL_COMBOTEXTBOX:
	case oGUI_CONTROL_FLOATBOX:
	{
			uint start = 0, end = 0;
			SendMessage(_hControl, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
			size_t len = end - start;
	if (len >= _SizeofStrDestination)
	{
		oErrorSetLast(std::errc::no_buffer_space, "Buffer too small to receive string");
		return nullptr;
	}

			if (!GetWindowText(_hControl, _StrDestination, oInt(_SizeofStrDestination)))
	{
		oWinSetLastError();
		return nullptr;
	}

			if (start != end)
	{
		if (start)
			oStrcpy(_StrDestination, _SizeofStrDestination, _StrDestination + start);
		_StrDestination[len] = 0;
	}

	return _StrDestination;
		}

		default:
		oErrorSetLastBadType(_hControl, type);
		return nullptr;
	}
}

bool oWinControlSelect(HWND _hControl, int _Start, int _Length)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_FLOATBOX_SPINNER:
			_hControl = oWinControlGetBuddy(_hControl);
			// pass thru
		case oGUI_CONTROL_FLOATBOX:
		case oGUI_CONTROL_TEXTBOX:
		case oGUI_CONTROL_TEXTBOX_SCROLLABLE:
		case oGUI_CONTROL_COMBOTEXTBOX:
		case oGUI_CONTROL_LABEL_SELECTABLE:
			Edit_SetSel(_hControl, _Start, _Start+_Length);
			return true;
		case oGUI_CONTROL_SLIDER_SELECTABLE:
			if (_Length)
			{
				SendMessage(_hControl, TBM_SETSELSTART, FALSE, _Start);
				SendMessage(_hControl, TBM_SETSELEND, TRUE, _Start+_Length);
			}
			else
				SendMessage(_hControl, TBM_CLEARSEL, TRUE, 0);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetValue(HWND _hControl, float _Value)
{
	oWINV(_hControl);
	oStd::mstring text;
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_FLOATBOX_SPINNER:
			_hControl = oWinControlGetBuddy(_hControl);
			// pass through
		default:
			oPrintf(text, oSubclassFloatBoxFormat, _Value);
			oVB(SetWindowText(_hControl, text));
			return true;
	}
}

float oWinControlGetFloat(HWND _hControl)
{
	oStd::mstring text;
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_FLOATBOX_SPINNER:
			_hControl = oWinControlGetBuddy(_hControl);
			// pass through
		default: 
			GetWindowText(_hControl, text.c_str(), (int)text.capacity());
			break;
	}

	float f = 0.0f;
	if (!oStd::atof(text, &f))
		return std::numeric_limits<float>::quiet_NaN();
	return f;
}

bool oWinControlSetIcon(HWND _hControl, HICON _hIcon, int _SubItemIndex)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_ICON:
			if (_SubItemIndex != oInvalid)
				return oErrorSetLast(std::errc::invalid_argument, "Invalid _SubItemIndex");
			SendMessage(_hControl, STM_SETICON, (WPARAM)_hIcon, 0);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

HICON oWinControlGetIcon(HWND _hControl, int _SubItemIndex)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_ICON:
			if (_SubItemIndex != oInvalid)
		{
				oErrorSetLast(std::errc::invalid_argument, "Invalid _SubItemIndex");
	return nullptr;
	}
			return oWinGetIcon(_hControl);
		default:
			oErrorSetLastBadType(_hControl, type);
			return nullptr;
	}
}

bool oWinControlIsChecked(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_CHECKBOX:
		case oGUI_CONTROL_RADIOBUTTON:
	{
			oErrorSetLast(0);
			LRESULT State = Button_GetState(_hControl);
			return (State & BST_CHECKED) == BST_CHECKED;
			}
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetChecked(HWND _hControl, bool _Checked)
{
	oWINV(_hControl);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_CHECKBOX:
		case oGUI_CONTROL_RADIOBUTTON:
			Button_SetCheck(_hControl, _Checked ? BST_CHECKED : BST_UNCHECKED);
	return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetRange(HWND _hControl, int _Min, int _Max)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_SLIDER:
		case oGUI_CONTROL_SLIDER_WITH_TICKS:
		case oGUI_CONTROL_SLIDER_SELECTABLE:
			SendMessage(_hControl, TBM_SETRANGEMIN, TRUE, _Min);
			SendMessage(_hControl, TBM_SETRANGEMAX, TRUE, _Max);
			return true;
		case oGUI_CONTROL_PROGRESSBAR:
			SendMessage(_hControl, PBM_SETRANGE32, _Min, _Max);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlGetRange(HWND _hControl, int* _pMin, int* _pMax)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_SLIDER:
		case oGUI_CONTROL_SLIDER_WITH_TICKS:
		case oGUI_CONTROL_SLIDER_SELECTABLE:
			if (_pMin) *_pMin = (int)SendMessage(_hControl, TBM_GETRANGEMIN, 0, 0);
			if (_pMax) *_pMax = (int)SendMessage(_hControl, TBM_GETRANGEMAX, 0, 0);
			return true;
		case oGUI_CONTROL_PROGRESSBAR:
		{
			PBRANGE pbr;
			SendMessage(_hControl, PBM_GETRANGE, 0, (LPARAM)&pbr);
			if (_pMin) *_pMin = pbr.iLow;
			if (_pMax) *_pMax = pbr.iHigh;
		}
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetRangePosition(HWND _hControl, int _Position, bool _bNotify /*=true*/)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_SLIDER:
		case oGUI_CONTROL_SLIDER_WITH_TICKS:
		case oGUI_CONTROL_SLIDER_SELECTABLE:
			SendMessage(_hControl, _bNotify ? TBM_SETPOSNOTIFY : TBM_SETPOS, 0, _Position);
			return true;
		case oGUI_CONTROL_PROGRESSBAR:
			SendMessage(_hControl, PBM_SETPOS, _Position, 0);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

int oWinControlGetRangePosition(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_SLIDER:
		case oGUI_CONTROL_SLIDER_WITH_TICKS:
		case oGUI_CONTROL_SLIDER_SELECTABLE: return (int)SendMessage(_hControl, TBM_GETPOS, 0, 0);
		case oGUI_CONTROL_PROGRESSBAR: return (int)SendMessage(_hControl, PBM_GETPOS, 0, 0);
		default: return oErrorSetLastBadType(_hControl, type);
	}
}

oAPI bool oWinControlSetTick(HWND _hControl, int _Position)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch(type)
	{
	case oGUI_CONTROL_SLIDER_WITH_TICKS:
		SendMessage(_hControl, TBM_SETTIC, 0, _Position);
		return true;
	default:
		return oErrorSetLastBadType(_hControl, type);
	}
}

oAPI bool oWinControlClearTicks(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch(type)
	{
	case oGUI_CONTROL_SLIDER_WITH_TICKS:
		SendMessage(_hControl, TBM_CLEARTICS, 1, 0);
		return true;
	default:
		return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetErrorState(HWND _hControl, bool _InErrorState)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_PROGRESSBAR:
			SendMessage(_hControl, PBM_SETSTATE, _InErrorState ? PBST_ERROR : PBST_NORMAL, 0);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlGetErrorState(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_PROGRESSBAR:
			oErrorSetLast(0);
			return PBST_ERROR == SendMessage(_hControl, PBM_GETSTATE, 0, 0);
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlClampPositionToSelected(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_SLIDER_SELECTABLE:
		{
			int Position = (int)SendMessage(_hControl, TBM_GETPOS, 0, 0);
			int SelectionStart = (int)SendMessage(_hControl, TBM_GETSELSTART, 0, 0);
			int SelectionEnd = (int)SendMessage(_hControl, TBM_GETSELEND, 0, 0);
			if (Position < SelectionStart)
				SendMessage(_hControl, TBM_SETPOS, TRUE, SelectionStart);
			else if (Position > SelectionEnd)
				SendMessage(_hControl, TBM_SETPOS, TRUE, SelectionEnd);
		}

		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}
