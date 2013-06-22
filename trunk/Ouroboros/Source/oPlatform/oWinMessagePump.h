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
#pragma once
#ifndef oWinMessagePump_h
#define oWinMessagePump_h

#include <oStd/fixed_vector.h>
#include <oConcurrency/event.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/Windows/oWinKey.h>
#include <oPlatform/Windows/oWinWindowing.h>

struct oWinMessagePump : oProcessSingleton<oWinMessagePump>
{
	static const oGUID GUID;

	oWinMessagePump();
	~oWinMessagePump();

	oDECLARE_WNDPROC(oWinMessagePump);

	inline oStd::thread::id GetMessagePumpThread() const threadsafe { return oThreadsafe(Thread).get_id(); }
	inline bool IsMessagePumpThread() const threadsafe { return oStd::this_thread::get_id() == GetMessagePumpThread(); }

	inline void SetAccel(HACCEL _hAccel) { oASSERT(IsMessagePumpThread(), "This API must be called on the messasge pump thread."); hAccel = _hAccel; }
	inline HACCEL GetAccel() const { oASSERT(IsMessagePumpThread(), "This API must be called on the messasge pump thread."); return hAccel; }

	// Active windows receive oWM_MAINLOOP messages
	void RegisterWindow(HWND _hWnd, bool _Active = false);
	void UnregisterWindow(HWND _hWnd);

	void Dispatch(const oTASK& _Task) threadsafe;

	// The main thread proc
	void MessagePump();

private:
	HWND hWndMessage;
	HACCEL hAccel;
	HHOOK hKeyboardLL;
	oStd::thread Thread;
	oStd::fixed_vector<HWND, 16> Windows;
	oStd::fixed_vector<HWND, 16> ActiveWindows;

	// Sleeps thread if there are no new messages. If this is false, the thread
	// only peeks at the message queue and if there is no message sends out its
	// own oWM_MAINLOOP message.
	bool WaitForMessages;
	bool Looping;

	oRefCount RefCount;
	oConcurrency::event Running;

	oWINKEY_CONTROL_STATE ControlKeyState;
};

#endif
