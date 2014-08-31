// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// A controller intended to be able to control either 3DSMax-style or Maya-style
// tools. For Maya, use the activation key. For Max, set activation and any 
// control type not active to ouro::input::none. Maya constraints to XY, Max to XZ
#pragma once
#ifndef oCameraControllerModeler_h
#define oCameraControllerModeler_h

#include <oCompute/arcball.h>
#include <oBasis/oCameraController.h>
#include <array>

struct oCAMERA_CONTROLLER_MODELER_DESC
{
	enum CONTROL
	{
		ACTIVATION,
		TUMBLER, // (orbit/arcball)
		TRACK, // Pan/move in screen space
		DOLLY,

		NUM_CONTROLS,
	};

	// Maya-like defaults
	oCAMERA_CONTROLLER_MODELER_DESC()
		: RotationSpeed(0.01f, 0.01f)
		, PanSpeed(0.05f)
		, DollySpeed(0.04f)
		, Constraint(ouro::arcball::y_up)
	{
		// if activation is ouro::input::none, then control is always-on.
		Controls[ACTIVATION] = ouro::input::lalt;
		Controls[TUMBLER] = ouro::input::mouse_left;
		Controls[TRACK] = ouro::input::mouse_middle;
		Controls[DOLLY] = ouro::input::mouse_right;
	}

	std::array<ouro::input::key, NUM_CONTROLS> Controls;
	float2 RotationSpeed;
	float2 PanSpeed;
	float DollySpeed;
	ouro::arcball::constraint_t Constraint;
};

// {85C0BAD4-D140-48D9-BA97-9E0EE6E2B75C}
oDEFINE_GUID_I(oCameraControllerModeler, 0x85c0bad4, 0xd140, 0x48d9, 0xba, 0x97, 0x9e, 0xe, 0xe6, 0xe2, 0xb7, 0x5c);
interface oCameraControllerModeler : oCameraController
{
	virtual void SetDesc(const oCAMERA_CONTROLLER_MODELER_DESC& _Desc) = 0;
	virtual void GetDesc(oCAMERA_CONTROLLER_MODELER_DESC* _pDesc) const = 0;
};

bool oCameraControllerModelerCreate(const oCAMERA_CONTROLLER_MODELER_DESC& _Desc, oCameraControllerModeler** _ppCameraController);

#endif
