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
// A generic interface for camera control in a 3D environment.
#pragma once
#ifndef oCameraController_h
#define oCameraController_h

#include <oBasis/oGUI.h>
#include <oBasis/oInterface.h>
#include <oBasis/oMathTypes.h>

// Yaw is up/down, pitch is left/right, roll is spin left/spin right. This 
// includes rotation around a lookat point/center of interest
#define oCAMERA_CONTROLLER_ROTATING_YAW (1<<0)
#define oCAMERA_CONTROLLER_ROTATING_PITCH (1<<1)
#define oCAMERA_CONTROLLER_ROTATING_ROLL (1<<2)
#define oCAMERA_CONTROLLER_ROTATING (oCAMERA_CONTROLLER_ROTATING_YAW|oCAMERA_CONTROLLER_ROTATING_PITCH|oCAMERA_CONTROLLER_ROTATING_ROLL)

// center of interest. If not set, then rotation is around the eye point
#define oCAMERA_CONTROLLER_ROTATING_AROUND_COI (1<<3)

// Translations are in screen space (X is left/right, Y is up/down, Z is in/out)
#define oCAMERA_CONTROLLER_TRANSLATING_X (1<<4)
#define oCAMERA_CONTROLLER_TRANSLATING_Y (1<<5)
#define oCAMERA_CONTROLLER_TRANSLATING_Z (1<<6)
#define oCAMERA_CONTROLLER_TRANSLATING (oCAMERA_CONTROLLER_TRANSLATING_X|oCAMERA_CONTROLLER_TRANSLATING_Y|oCAMERA_CONTROLLER_TRANSLATING_Z)

// Opposing concepts (i.e. show v. hide) will never be set at the same time
#define oCAMERA_CONTROLLER_SHOW_POINTER (1<<7)
#define oCAMERA_CONTROLLER_HIDE_POINTER (1<<8)
#define oCAMERA_CONTROLLER_LOCK_POINTER (1<<9)
#define oCAMERA_CONTROLLER_UNLOCK_POINTER (1<<10)
#define oCAMERA_CONTROLLER_SAVE_POINTER_POSITION (1<<11)
#define oCAMERA_CONTROLLER_LOAD_POINTER_POSITION (1<<12)

// Expands a bitmask of OR'ed values from above into a string for development/
// debugging purposes.
oAPI char* oCameraControllerParseResponse(char* _StrDestination, size_t _SizeofStrDestination, int _ResponseFlags);
template<size_t size> char* oCameraControllerParseResponse(char (&_StrDestination)[size], int _ResponseFlags) { return oCameraControllerParseResponse(_StrDestination, size, _ResponseFlags); }
template<size_t capacity> char* oCameraControllerParseResponse(oStd::fixed_string<char, capacity>& _StrDestination, int _ResponseFlags) { return oCameraControllerParseResponse(_StrDestination, _StrDestination.capacity(), _ResponseFlags); }

// {D7874299-6F52-4E5F-A76E-9ACB37A35316}
oDEFINE_GUID_I(oCameraController, 0xd7874299, 0x6f52, 0x4e5f, 0xa7, 0x6e, 0x9a, 0xcb, 0x37, 0xa3, 0x53, 0x16);
interface oCameraController : oInterface
{
	// Respond to user input. Attach this to the message pump of an oWindow's
	// action handler. This returns several of the above bit masks to indicate how 
	// the controller has responded to the input.
	virtual int OnAction(const oGUI_ACTION_DESC& _Action) = 0;

	// Call this every update
	virtual void Tick() = 0;

	// Call this when something happens that causes any state retained by an 
	// instance to become invalid, such as the system stealing focus or going 
	// outside a client area in a way that changes focus. Basically this should 
	// reset the state of this object to the idle state where no active Action 
	// would be processed in OnAction().
	virtual void OnLostCapture() = 0;

	// Override user input to set the internal evaluation of the view to the 
	// specified value.
	virtual void SetView(const float4x4& _View) = 0;

	// Convert the internal state as set by OnAction or SetView to a view matrix.
	// Derivations of this class will most likely define scalar rates at which 
	// input devices are translated into rate of change of the view, so _DeltaTime
	// is provided to scale such rates. If _DeltaTime is 0.0f, this should return
	// the current state of the system, for example to initialize a different 
	// controller instance to the same view as this one.
	virtual float4x4 GetView(float _DeltaTime = 0.0f) = 0;

	// Sets the center of interest for the controller. This is set absolutely; it 
	// is not transformed by the state of the controller. Like view, the value may 
	// be modified when reacting in OnAction.
	virtual void SetLookAt(const float3& _LookAt) = 0;

	// Returns the center of interest for the controller.
	virtual float3 GetLookAt() const = 0;
};

#endif
