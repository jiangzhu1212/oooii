/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// An FPS-like control that allows for up/down/strafe left/strafe right controls
// with both a walk and run mode. Mouse control is slightly different than the 
// typical FPS in that the right mouse button must be held down to change the 
// rotation of the view. This is so the use cases of mouse-controls-pointer and 
// mouse-controls-camera can be quickly toggled to support use of the UI and 
// steering at the same time.
#pragma once
#ifndef oCameraControllerMMO_h
#define oCameraControllerMMO_h

#include <oBasis/oCameraController.h>
#include <array>

struct oCAMERA_CONTROLLER_MMO_DESC
{
	enum CONTROL
	{
		TRANSLATION_FIRST,
		STRAFE_RIGHT = TRANSLATION_FIRST,
		STRAFE_LEFT,
		SWIM_UP,
		SWIM_DOWN,
		FORWARD,
		BACKWARD,
		TRANSLATION_LAST = BACKWARD,
		ROLL_LEFT,
		ROLL_RIGHT,
		RUN,
		ROTATE_VIEW,
		ROTATE_VIEW_AND_FORWARD,

		NUM_CONTROLS,
	};

	oCAMERA_CONTROLLER_MMO_DESC()
		: WalkSpeed(1.0f)
		, RunSpeed(2.0f)
		, RotationSpeed(0.6f)
		, Acceleration(1.05f)
		, WalkSpeedMin(1e-5f)
		, WalkSpeedMax(1e5f)
		, RunSpeedMin(2e-5f)
		, RunSpeedMax(2e5f)
		, AllowMouseWheelAcceleration(false)
	{
		Controls[STRAFE_RIGHT] = ouro::input::d;
		Controls[STRAFE_LEFT] = ouro::input::a;
		Controls[SWIM_UP] = ouro::input::space;
		Controls[SWIM_DOWN] = ouro::input::z;
		Controls[FORWARD] = ouro::input::w;
		Controls[BACKWARD] = ouro::input::s;
		Controls[ROLL_LEFT] = ouro::input::q;
		Controls[ROLL_RIGHT] = ouro::input::e;
		Controls[RUN] = ouro::input::lshift;
		Controls[ROTATE_VIEW] = ouro::input::mouse_right;
		Controls[ROTATE_VIEW_AND_FORWARD] = ouro::input::mouse_left;

		ouro::input::key* Secondary = &Controls[NUM_CONTROLS];
		Secondary[STRAFE_RIGHT] = ouro::input::right;
		Secondary[STRAFE_LEFT] = ouro::input::left;
		Secondary[SWIM_UP] = ouro::input::pgup;
		Secondary[SWIM_DOWN] = ouro::input::pgdn;
		Secondary[FORWARD] = ouro::input::up;
		Secondary[BACKWARD] = ouro::input::down;
		Secondary[ROLL_LEFT] = ouro::input::home;
		Secondary[ROLL_RIGHT] = ouro::input::end;
		Secondary[RUN] = ouro::input::rshift;
		Secondary[ROTATE_VIEW] = ouro::input::none;
		Secondary[ROTATE_VIEW_AND_FORWARD] = ouro::input::none;
	}

	// Define controls for each key, plus another set for 2nd-ary assignment.
	std::array<ouro::input::key, NUM_CONTROLS * 2> Controls;
	float WalkSpeed;
	float RunSpeed;
	float3 RotationSpeed;
	float Acceleration;
	float WalkSpeedMin;
	float WalkSpeedMax;
	float RunSpeedMin;
	float RunSpeedMax;

	// If true, Walk/Run Speed /= acceleration for wheel back, and 
	// Walk/Run Speed *= acceleration for wheel forward
	bool AllowMouseWheelAcceleration;
};

// {B6D0E69B-D305-47FF-AC05-F64303106D1E}
oDEFINE_GUID_I(oCameraControllerMMO, 0xb6d0e69b, 0xd305, 0x47ff, 0xac, 0x5, 0xf6, 0x43, 0x3, 0x10, 0x6d, 0x1e);
interface oCameraControllerMMO : oCameraController
{
	virtual void SetDesc(const oCAMERA_CONTROLLER_MMO_DESC& _Desc) = 0;
	virtual void GetDesc(oCAMERA_CONTROLLER_MMO_DESC* _pDesc) const = 0;
};

bool oCameraControllerMMOCreate(const oCAMERA_CONTROLLER_MMO_DESC& _Desc, oCameraControllerMMO** _ppCameraController);

#endif
