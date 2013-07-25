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
#include "oWinMessagePump.h"
#include <oStd/oStdFuture.h>
#include <oPlatform/Windows/oWinAsString.h>
#include <oPlatform/oReporting.h>

// {36E5CBF0-784B-435B-8389-6E310E928836}
const oGUID oWinMessagePump::GUID = { 0x36e5cbf0, 0x784b, 0x435b, { 0x83, 0x89, 0x6e, 0x31, 0xe, 0x92, 0x88, 0x36 } };
oSINGLETON_REGISTER(oWinMessagePump);

LRESULT CALLBACK LLKeyboardHook(int _nCode, WPARAM _wParam, LPARAM _lParam)
{
	// It seems just be being hooked at all that extended keys come through
	// as WM_KEYDOWN/WM_KEYUPs, so just be here...
	return CallNextHookEx(0, _nCode, _wParam, _lParam);
}

oWinMessagePump::oWinMessagePump()
	: hWndMessage(nullptr)
	, hAccel(nullptr)
	, WaitForMessages(false)
	, Looping(true)
	, RefCount(0)
{
	oReportingReference();
//	hKeyboardLL = SetWindowsHookExA(WH_KEYBOARD_LL, LLKeyboardHook, GetModuleHandle(0), 0);
	Thread = std::move(oStd::thread(&oWinMessagePump::MessagePump, oThreadsafe(this)));
}

oWinMessagePump::~oWinMessagePump()
{
//	UnhookWindowsHookEx(hKeyboardLL);
	oVB(PostMessage(hWndMessage, WM_CLOSE, 0, 0));
	oASSERT(Windows.empty(), "Windows not empty on oWinMessagePump dtor");
	oASSERT(ActiveWindows.empty(), "Active windows not empty on oWinMessagePump dtor");
	Thread.join();
	oReportingRelease();
}

void oWinMessagePump::RegisterWindow(HWND _hWnd, bool _Active)
{
	oASSERT(IsMessagePumpThread(), "This API must be called on the message pump thread.");
	oTRACE("HWND %x registered%s.", _hWnd, _Active ? " as active" : "");
	oStd::push_back_unique(Windows, _hWnd);
	if (_Active)
		oStd::push_back_unique(ActiveWindows, _hWnd);
}

void oWinMessagePump::UnregisterWindow(HWND _hWnd)
{
	oASSERT(IsMessagePumpThread(), "This API must be called on the message pump thread.");
	bool Active = oStd::find_and_erase(ActiveWindows, _hWnd);
	if (oStd::find_and_erase(Windows, _hWnd))
		oTRACE("HWND %x unregistered.", _hWnd);
}

void oWinMessagePump::Dispatch(const oTASK& _Task) threadsafe
{
	oTASK* pTask = new oTASK(_Task);
	Running.wait();
	PostMessage(hWndMessage, oWM_DISPATCH, 0, (LPARAM)pTask);
};

LRESULT oWinMessagePump::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	static const bool kDebug = false;
	if (kDebug)
	{
		oStd::xlstring s;
		oTRACE("%s", oWinParseWMMessage(s, s.capacity(), &ControlKeyState, _hWnd, _uMsg, _wParam, _lParam));
	}

	switch (_uMsg)
	{
		case WM_CREATE:
			oTRACE("oWinMessagePump: HWND %x created.", _hWnd);
			break;

		case WM_CLOSE:
			oASSERT(Windows.empty(), "All windows must be unregistered from oWinMessagePump before releasing its reference on the pump.");
			oASSERT(ActiveWindows.empty(), "All active windows must be unregistered from oWinMessagePump before releasing its reference on the pump.");
			DestroyWindow(_hWnd);
			Looping = false;
			break;

		case WM_QUIT:
			oTRACE("oWinMessagePump HWND %x destroyed.", _hWnd);
			break;

		case oWM_DISPATCH:
		{
			oTASK* pTask = (oTASK*)_lParam;
			(*pTask)();
			delete pTask;
			return 0;
		}

		default:
			break;
	}

	return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}

void oWinMessagePump::MessagePump()
{
	oConcurrency::begin_thread("oWinMessagePump");

	// Why? Because...
	// http://blogs.msdn.com/b/oldnewthing/archive/2009/09/30/9901065.aspx
	oVERIFY(oWinCreate(&hWndMessage, int2(0,0), int2(0,0), StaticWndProc, this, true));

	Running.set();
	while (Looping)
	{
		if (!oWinDispatchMessage(nullptr, hAccel, WaitForMessages))
		{
			errno_t LastErr = oErrorGetLast();
			if (std::errc::no_message_available == LastErr)
			{
				// make a copy to traverse because a window can remove itself from the
				// active list during the oWM_MAINLOOP event.
				oStd::fixed_vector<HWND, 16> Loopers = ActiveWindows;
				oFOR(const HWND& hWnd, Loopers)
					SendMessage(hWnd, oWM_MAINLOOP, 0, 0);
			}
			
			else if (std::errc::operation_canceled == LastErr)
				break;
		}

		WaitForMessages = ActiveWindows.empty();
	}

	oConcurrency::end_thread();
	Running.reset();
}
