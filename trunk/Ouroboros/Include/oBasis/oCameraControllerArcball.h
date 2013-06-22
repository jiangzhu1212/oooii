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
// A controller that orbits around a point of interest. It behaves similar to 
// Maya controls, but uses the classic arcball, which is more screen-confined
// than Maya's controls.
#pragma once
#ifndef oCameraControllerArcball_h
#define oCameraControllerArcball_h

#include <oBasis/oCameraController.h>
#include <oBasis/oGUI.h>
#include <array>

struct oCAMERA_CONTROLLER_ARCBALL_DESC
{
	enum CONTROL
	{
		ORBIT,
		PAN,
		DOLLY,

		NUM_CONTROLS,
	};

	oCAMERA_CONTROLLER_ARCBALL_DESC()
		: RotationSpeed(0.02f, 0.02f)
		, PanSpeed(0.05f)
		, DollySpeed(0.04f)
	{
		Controls[ORBIT] = oGUI_KEY_MOUSE_LEFT;
		Controls[PAN] = oGUI_KEY_MOUSE_MIDDLE;
		Controls[DOLLY] = oGUI_KEY_MOUSE_RIGHT;
	}

	std::array<oGUI_KEY, NUM_CONTROLS> Controls;
	float2 RotationSpeed;
	float2 PanSpeed;
	float DollySpeed;
};

// {A363E3FE-D85D-4C82-BC31-0DD6495A4470}
oDEFINE_GUID_I(oCameraControllerArcball, 0xa363e3fe, 0xd85d, 0x4c82, 0xbc, 0x31, 0xd, 0xd6, 0x49, 0x5a, 0x44, 0x70);
interface oCameraControllerArcball : oCameraController
{
	virtual void SetDesc(const oCAMERA_CONTROLLER_ARCBALL_DESC& _Desc) = 0;
	virtual void GetDesc(oCAMERA_CONTROLLER_ARCBALL_DESC* _pDesc) const = 0;
};

bool oCameraControllerArcballCreate(const oCAMERA_CONTROLLER_ARCBALL_DESC& _Desc, oCameraControllerArcball** _ppCameraController);

#endif
