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
#include <oooii/oMouse.h>
#include <oooii/oRefCount.h>
#include <oooii/oThreading.h>
#include <oooii/oWindows.h>
#include "oKeyState.h"

template<> const char* oAsString(const oMouse::BUTTON& _Button)
{
	switch (_Button)
	{
		case oMouse::BUTTON_LEFT: return "left";
		case oMouse::BUTTON_MIDDLE: return "middle";
		case oMouse::BUTTON_RIGHT: return "right";
		case oMouse::BUTTON_FORWARD: return "forward";
		case oMouse::BUTTON_BACKWARD: return "backward";
		default: break;
	}

	return "unknown oMouse::Button";
}

static void GetButtonState(UINT _uMsg, WPARAM _wParam, oMouse::BUTTON* _pWhich, bool* _pIsDown)
{
	switch (_uMsg)
	{
		case WM_LBUTTONDOWN: *_pWhich = oMouse::BUTTON_LEFT; *_pIsDown = true; break;
		case WM_LBUTTONUP: *_pWhich = oMouse::BUTTON_LEFT; *_pIsDown = false; break;
		case WM_MBUTTONDOWN: *_pWhich = oMouse::BUTTON_MIDDLE; *_pIsDown = true; break;
		case WM_MBUTTONUP: *_pWhich = oMouse::BUTTON_MIDDLE; *_pIsDown = false; break;
		case WM_RBUTTONDOWN: *_pWhich = oMouse::BUTTON_RIGHT; *_pIsDown = true; break;
		case WM_RBUTTONUP: *_pWhich = oMouse::BUTTON_RIGHT; *_pIsDown = false; break;
		case WM_XBUTTONDOWN: *_pWhich = GET_XBUTTON_WPARAM(_wParam) == XBUTTON1 ? oMouse::BUTTON_BACKWARD : oMouse::BUTTON_FORWARD; *_pIsDown = true; break;
		case WM_XBUTTONUP: *_pWhich = GET_XBUTTON_WPARAM(_wParam) == XBUTTON1 ? oMouse::BUTTON_BACKWARD : oMouse::BUTTON_FORWARD; *_pIsDown = false; break;
		default: break;
	}
}

struct Mouse_Impl : public oMouse
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);

	Mouse_Impl(HWND _hWnd, bool _ShortCircuitChain, bool* pSuccess = 0)
		: ShortCircuitChain(_ShortCircuitChain)
		, hWnd(_hWnd)
		, VerticalWheelDelta(0)
		, HorizontalWheelDelta(0)
		, MouseWaitCount(0)
		, CursorVisible(true)
		, ShouldShowCursor(true)
	{

		hHook = oSetWindowsHook(WH_MOUSE, MouseHook, this, _hWnd);
		if (pSuccess)
			*pSuccess = !!hHook;

		POINT pt;
		oVB(GetCursorPos(&pt));
	}

	~Mouse_Impl()
	{
		if (hHook)
			oUnhookWindowsHook(hHook);
	}

	void Update() threadsafe override
	{
		State.PromoteReleasedPressedToUpDown();
		oMutex::ScopedLock lock(PositionLock);
		VerticalWheelDelta = 0;
		HorizontalWheelDelta = 0;
	}

	bool IsUp(BUTTON _Button) const threadsafe override { return State.IsUp(_Button); }
	bool IsReleased(BUTTON _Button) const threadsafe override { return State.IsReleased(_Button); }
	bool IsDown(BUTTON _Button) const threadsafe override { return State.IsDown(_Button); }
	bool IsPressed(BUTTON _Button) const threadsafe override { return State.IsPressed(_Button); }

	void GetPosition(int *_pX, int *_pY, int *_pVerticalWheelDelta = 0, int *_pHorizontalWheelDelta = 0) threadsafe override
	{
		oMutex::ScopedLock lock(PositionLock);
		*_pX = Position.x;
		*_pY = Position.y;

		if (_pVerticalWheelDelta)
			*_pVerticalWheelDelta = VerticalWheelDelta;
		if (_pHorizontalWheelDelta)
			*_pHorizontalWheelDelta = HorizontalWheelDelta;
	}

	CURSOR_STATE SetCursorState(CURSOR_STATE _State) threadsafe
	{
		oMutex::ScopedLock lock(CursorStateLock);
		CURSOR_STATE state = thread_cast<Mouse_Impl*>(this)->GetCursorStateUnsafe();
		LPCTSTR cursor = IDC_ARROW;
		if (_State == WAITING_FOREGROUND)
		{
			cursor = IDC_WAIT;
			MouseWaitCount++;
		}
		if (_State == WAITING_BACKGROUND) cursor = IDC_APPSTARTING;

		if (_State == NORMAL)
		{
			MouseWaitCount--;
			//@oooii-Andrew: Only return mouse state to Normal if MouseWaitCount is 0
			if (MouseWaitCount <= 0)
				MouseWaitCount = 0;
			else
				return state;
		}

		if (_State == NONE)
			while (ShowCursor(false) > -1) {}
		else
			ShowCursor(true);

		SetCursor(LoadCursor(0, cursor));
		SetClassLongPtr(hWnd, -12, (LONG_PTR)LoadCursor(0, cursor)); // GCLP_HCURSOR = -12
		ShouldShowCursor = CursorVisible = _State != NONE;

		return state;
	}

	CURSOR_STATE GetCursorStateUnsafe() const
	{
		oMutex::ScopedLock lock(CursorStateLock);

		if (oIsCursorVisible())
		{
			HCURSOR cursors[] = 
			{
				0,
				LoadCursor(0, IDC_ARROW),
				LoadCursor(0, IDC_APPSTARTING),
				LoadCursor(0, IDC_WAIT),
			};

			HCURSOR hCurrent = GetCursor();
			for (int i = 0; i < oCOUNTOF(cursors); i++)
				if (hCurrent == cursors[i])
					return static_cast<CURSOR_STATE>(i);
		}

		return NONE;
	}

	CURSOR_STATE GetCursorState() const threadsafe override
	{
		oMutex::ScopedLock lock(CursorStateLock);
		return thread_cast<Mouse_Impl*>(this)->GetCursorStateUnsafe();
	}

	bool MouseHook(int _nCode, WPARAM _wParam, LPARAM _lParam) threadsafe
	{
		MOUSEHOOKSTRUCTEX* pMHS = (MOUSEHOOKSTRUCTEX*)_lParam;
		// Only hook our specific window
		if (hWnd && pMHS->hwnd != hWnd)
			return false;

		// Mouse wheel's idea of focus is a bit different than other messages,
		// so handle it before the hittest cull
		if (_wParam == WM_MOUSEWHEEL)
			HorizontalWheelDelta = GET_WHEEL_DELTA_WPARAM(pMHS->mouseData);
		if (_wParam == WM_MOUSEHWHEEL)
			VerticalWheelDelta = GET_WHEEL_DELTA_WPARAM(pMHS->mouseData);

		BUTTON which = NUM_BUTTONS;
		bool isDown = false;

		GetButtonState(static_cast<UINT>(_wParam), pMHS->mouseData, &which, &isDown);
		State.RecordUpDown(which, isDown);

		oMutex::ScopedLock lock(PositionLock);
		switch (_wParam)
		{
			case WM_MOUSEMOVE: thread_cast<POINT&>(Position) = pMHS->pt; break;
			case WM_MOUSEWHEEL: VerticalWheelDelta = GET_WHEEL_DELTA_WPARAM(pMHS->mouseData); break;
			case WM_MOUSEHWHEEL: HorizontalWheelDelta = GET_WHEEL_DELTA_WPARAM(pMHS->mouseData); break;
			default: break;
		}

		//@oooii-Andrew: change mouse cursor when out of client area
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hWnd, &pt);

		if (pt.y < 0 && !CursorVisible)
		{
			SetCursorState(NORMAL);
			ShouldShowCursor = false;
		}
		else if (!ShouldShowCursor && CursorVisible && pt.y >= 0)
		{
			SetCursorState(NONE);
		}

		return ShortCircuitChain;
	}

	static LRESULT CALLBACK MouseHook(int _nCode, WPARAM _wParam, LPARAM _lParam, void* _pUserData)
	{
		if (_nCode == HC_ACTION)
		{
			threadsafe Mouse_Impl* pThis = static_cast<threadsafe Mouse_Impl*>(_pUserData);
			if (pThis->MouseHook(_nCode, _wParam, _lParam))
				return 1;
		}

		return CallNextHookEx(0, _nCode, _wParam, _lParam);
	}

	HHOOK hHook;
	HWND hWnd;
	oRefCount RefCount;
	POINT Position;
	int VerticalWheelDelta;
	int HorizontalWheelDelta;
	bool ShortCircuitChain;
	oMutex PositionLock;
	mutable oMutex CursorStateLock;
	oKeyState<NUM_BUTTONS> State;
	int MouseWaitCount;
	bool CursorVisible;
	bool ShouldShowCursor;
};

oMouse::ScopedWait::ScopedWait(threadsafe oMouse* _pMouse, CURSOR_STATE _State)
	: pMouse(_pMouse)
{
	if (pMouse) pMouse->Reference();
	PriorState = pMouse->SetCursorState(_State);
}

oMouse::ScopedWait::~ScopedWait()
{
	//@oooii-Andrew lets restore to normal cursor and not previous state
	// When the app loads if the background wait cursor is set and we cache
	// that off then we will be stuck with a background cursor.
	pMouse->SetCursorState(NORMAL);
	if (pMouse)
		pMouse->Release();
}

bool oMouse::Create(void* _AssociatedNativeHandle, bool _ShortCircuitEvents, threadsafe oMouse** _ppMouse)
{
	if (!_AssociatedNativeHandle || !_ppMouse) return false;
	bool success = false;
	*_ppMouse = new Mouse_Impl((HWND)_AssociatedNativeHandle, _ShortCircuitEvents, &success);
	if (*_ppMouse && !success)
	{
		delete *_ppMouse;
		*_ppMouse = 0;
	}

	return success;
}
