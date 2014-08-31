// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// A controller that orbits around a point of interest. It behaves similar to 
// Maya controls, but uses the classic arcball, which is more screen-confined
// than Maya's controls.
#pragma once
#ifndef oCameraControllerArcball_h
#define oCameraControllerArcball_h

#include <oBasis/oCameraController.h>
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
		Controls[ORBIT] = ouro::input::mouse_left;
		Controls[PAN] = ouro::input::mouse_middle;
		Controls[DOLLY] = ouro::input::mouse_right;
	}

	std::array<ouro::input::key, NUM_CONTROLS> Controls;
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
