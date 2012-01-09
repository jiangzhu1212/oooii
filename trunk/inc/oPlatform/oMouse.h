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
#pragma once
#ifndef oMouse_h
#define oMouse_h

#include <oBasis/oInterface.h>

interface oMouse : oInterface
{
	enum BUTTON
	{
		BUTTON_LEFT,
		BUTTON_MIDDLE,
		BUTTON_RIGHT,
		BUTTON_FORWARD,
		BUTTON_BACKWARD,
		NUM_BUTTONS,
	};

	struct DESC
	{
		DESC()
			: ShortCircuitEvents(false)
			, ClipCursorMouseDown(true)
		{}
		
		bool ShortCircuitEvents;	// Prevents event handlers below you to not receive events
		bool ClipCursorMouseDown;	// Clips the cursor to the window if any button is down
	};

	// On event-driven platforms (Windows) where there isn't an explicit update 
	// for the application's context, a button might not be able to be determined
	// as "still down from last frame" until the next event fires. In order to
	// work around this, call this function once a frame and it will change all
	// pressed keys from last frame into down keys and all released keys into up 
	// keyss.
	virtual void Update() threadsafe = 0;

	// Default unpressed state of a button
	virtual bool IsUp(BUTTON _Button) const threadsafe = 0;

	// True on first frame of a transition from down to up
	virtual bool IsReleased(BUTTON _Button) const threadsafe  = 0;

	// Raw state ignoring key repeat
	virtual bool IsDown(BUTTON _Button) const threadsafe = 0;

	// Returns true the first time the key goes from up to down, and again 
	// according to platform key repeat settings.
	virtual bool IsPressed(BUTTON _Button) const threadsafe = 0;

	virtual void GetPosition(int *_pX, int *_pY, int *_pVerticalWheelDelta = 0, int *_pHorizontalWheelDelta = 0) threadsafe = 0;
};

oAPI bool oMouseCreate(const oMouse::DESC& _Desc, void* _WindowNativeHandle, threadsafe oMouse** _ppMouse);

#endif
