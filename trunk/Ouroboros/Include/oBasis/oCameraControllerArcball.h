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
#include <oBasis/oX11KeyboardSymbols.h>

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
		Controls[ORBIT] = oKB_Pointer_Button_Left;
		Controls[PAN] = oKB_Pointer_Button_Middle;
		Controls[DOLLY] = oKB_Pointer_Button_Right;
	}

	oKEYBOARD_KEY Controls[NUM_CONTROLS];
	float2 RotationSpeed;
	float2 PanSpeed;
	float DollySpeed;
};

interface oCameraControllerArcball : oCameraController
{
	virtual void SetDesc(const oCAMERA_CONTROLLER_ARCBALL_DESC& _Desc) = 0;
	virtual void GetDesc(oCAMERA_CONTROLLER_ARCBALL_DESC* _pDesc) const = 0;
};

bool oCameraControllerArcballCreate(const oCAMERA_CONTROLLER_ARCBALL_DESC& _Desc, oCameraControllerArcball** _ppCameraController);

#endif
