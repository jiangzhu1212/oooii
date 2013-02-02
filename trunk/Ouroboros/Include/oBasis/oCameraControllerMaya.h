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
// A controller intended to be identical to Maya. This defines an explicit 
// activation key, basic arcball rotation around a specifiable target, screen-
// space panning, and along-the-view dollying.
#pragma once
#ifndef oCameraControllerMaya_h
#define oCameraControllerMaya_h

#include <oBasis/oX11KeyboardSymbols.h>
#include "oCameraController.h"

struct oCAMERA_CONTROLLER_MAYA_DESC
{
	enum CONTROL
	{
		ACTIVATION,
		TUMBLER, // (orbit/arcball)
		TRACK, // Pan/move in screen space
		DOLLY,

		NUM_CONTROLS,
	};

	oCAMERA_CONTROLLER_MAYA_DESC()
		: RotationSpeed(0.01f, 0.01f)
		, PanSpeed(0.05f)
		, DollySpeed(0.04f)
	{
		Controls[ACTIVATION] = oKB_Alt_L;
		Controls[TUMBLER] = oKB_Pointer_Button_Left;
		Controls[TRACK] = oKB_Pointer_Button_Middle;
		Controls[DOLLY] = oKB_Pointer_Button_Right;
	}

	oKEYBOARD_KEY Controls[NUM_CONTROLS];
	float2 RotationSpeed;
	float2 PanSpeed;
	float DollySpeed;
};

interface oCameraControllerMaya : oCameraController
{
	virtual void SetDesc(const oCAMERA_CONTROLLER_MAYA_DESC& _Desc) = 0;
	virtual void GetDesc(oCAMERA_CONTROLLER_MAYA_DESC* _pDesc) const = 0;
};

bool oCameraControllerMayaCreate(const oCAMERA_CONTROLLER_MAYA_DESC& _Desc, oCameraControllerMaya** _ppCameraController);

#endif
