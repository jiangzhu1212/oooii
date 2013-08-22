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
#include "oManipulatorScale.h"
#include <oCompute/oComputeConstants.h>

const float SCALE_CAP_OFFSET = 0.1f;
const float SCALE_CAP_SCALE = 0.001f;

oManipulatorScale::oManipulatorScale(const DESC& _Desc, bool *_pSuccess) : oManipulatorBase(_Desc, _pSuccess)
{
	if(!(*_pSuccess)) //base failed to construct
		return;
	*_pSuccess = false;
	Clip[X].resize(2);
	Clip[Y].resize(2);
	Clip[Z].resize(2);
	*_pSuccess = true;
}

void oManipulatorScale::BeginManipulation(AXIS _Axis, const float2& _ScreenPosition)
{
	Picking = true;
	PickAxis = _Axis;
	StartTranslation = _ScreenPosition;
	CurrentTranslation = _ScreenPosition;
}

void oManipulatorScale::GetTransform(float4x4 *_pTransform) const
{	
	if(Desc.Mode == DESC::OBJECT)
		*_pTransform = Transform*World;
	else
		*_pTransform = WorldRotation*Transform*invert(WorldRotation)*World;
}

float3 oManipulatorScale::CalcScale(float3 _axis)
{
	float4x4 WorldRotation = oIDENTITY4x4;
	if(Desc.Mode == DESC::OBJECT)
		WorldRotation = oCreateRotation(oExtractRotation(World));

	float4 nearPlane = InvProj*float4(0,0,0,1);
	nearPlane /= nearPlane.w; //near plane will now be in z
	float3 center = WorldRotation*View*float3(0,0,0);
	float3 axis = WorldRotation*View*_axis;
	float proj = center.z+nearPlane.z;
	axis = axis - center;
	float unitLength = length(axis);
	axis = normalize(axis);
	
	float3 divisor = _axis;
	if(_axis.x == 0)
		divisor.x = 1.0f;
	if(_axis.y == 0)
		divisor.y = 1.0f;
	if(_axis.z == 0)
		divisor.z = 1.0f;

	float3 mouseVector = float3(CurrentTranslation.x - StartTranslation.x, CurrentTranslation.y - StartTranslation.y, 0);
	if(length(mouseVector) == 0)
		return float3(1,1,1);
	mouseVector *= proj;
	mouseVector = InvProj*mouseVector;
	mouseVector.z = 0.0;
	float scale = dot(axis,mouseVector);
	float3 move = scale*axis;
	if(scale > 0)
		scale = length(move)/unitLength + 1;
	else
		scale = -length(move)/unitLength + 1;
	move += center;
	move = invert(View)*move;
	return (scale*_axis)/divisor;
}

bool oManipulatorScale::CalcGeometry(AXIS _Axis)
{
	oASSERT(Lines[_Axis].size() == 2,"CalcGeometry assumes 2 vertices for line, and 4 for the pick rect");
	
	if(Project(Lines[_Axis]))
		return true;

	float zdirection = Lines[_Axis][1].z - Lines[_Axis][0].z;
	Lines[_Axis][0].z = static_cast<float>(0.01);
	Lines[_Axis][1].z = static_cast<float>(0.01);

	//handle screen pick rect. get a perp vector from line and create the 4 corners of the rect
	float4 normal = Lines[_Axis][1]-Lines[_Axis][0];
	float tempf = normal.x;
	normal.x = -normal.y;
	normal.y = tempf;
	normal.z = 0;
	normal.w = 0;
	normal = normalize(normal) * static_cast<float>(Desc.PickWidth);

	PickGeometry[_Axis].resize(4);
	if(zdirection > 0) //so z clipping between the several pick rects, x, y, and z, work a little better. i.e. which will have priority on getting picked
	{
		PickGeometry[_Axis][0] = Lines[_Axis][0]+normal+float4(0,0,0.01f,0);
		PickGeometry[_Axis][2] = Lines[_Axis][0]-normal+float4(0,0,0.01f,0);
		PickGeometry[_Axis][1] = Lines[_Axis][1]+normal+float4(0,0,0.02f,0);
		PickGeometry[_Axis][3] = Lines[_Axis][1]-normal+float4(0,0,0.02f,0);
	}
	else
	{
		PickGeometry[_Axis][0] = Lines[_Axis][0]+normal+float4(0,0,0.01f,0);
		PickGeometry[_Axis][2] = Lines[_Axis][0]-normal+float4(0,0,0.01f,0);
		PickGeometry[_Axis][1] = Lines[_Axis][1]+normal+float4(0,0,0,0);
		PickGeometry[_Axis][3] = Lines[_Axis][1]-normal+float4(0,0,0,0);
	}

	UnProject(Lines[_Axis]);
	UnProject(PickGeometry[_Axis]);
	return false;
}

void oManipulatorScale::CalcCap(AXIS _Axis, float4 vert, const float4x4 &_capRotation)
{
	vert = WorldTransViewProj*vert;

	float scale = static_cast<float>(1.0 - vert.z/vert.w);
	vert /= vert.w;
	vert.z = SCALE_CAP_OFFSET;

	UnProject(vert);

	CapTransforms[_Axis] = _capRotation*oCreateScale(SCALE_CAP_SCALE)*oCreateTranslation(vert.xyz());
}

void oManipulatorScale::UpdateImpl()
{
	Transform = oIDENTITY4x4;
	if(Picking)
	{
		static float3 axes[NUM_AXES] = {float3(7,0,0),float3(0,7,0),float3(0,0,7)};
		static float3 axesMask[NUM_AXES] = {float3(1,0,0),float3(0,1,0),float3(0,0,1)};
		static float3 axesAdd[NUM_AXES] = {float3(0,1,1),float3(1,0,1),float3(1,1,0)};
		if(PickAxis == X || PickAxis == Y || PickAxis == Z)
		{
			float3 axis;
			axis = axes[PickAxis];
			axis = CalcScale(axes[PickAxis]);
			axis *= axesMask[PickAxis];
			axis += axesAdd[PickAxis];

			Transform = oCreateScale(axis);
		}
		else if(PickAxis == SCREEN)
		{
			float3 moveX = CalcScale(axes[X]);
			float3 moveY = CalcScale(axes[Y]);
			float3 moveZ = CalcScale(axes[Z]);
			float avg = (moveX.x + moveY.y + moveZ.z)/3;
			Transform = oCreateScale(float3(avg,avg,avg));
		}
	}
	
	if(Desc.Mode == DESC::WORLD)
		WorldTransViewProj = Transform*WorldTranslation*View*Proj;
	else
		WorldTransViewProj = Transform*WorldRotation*WorldTranslation*View*Proj;


	Lines[X].resize(2);
	Lines[X][0] = float4(0,0,0,1);
	Lines[X][1] = float4(Desc.ManipularScale,0,0,1);
	Lines[Y].resize(2);
	Lines[Y][0] = float4(0,0,0,1);
	Lines[Y][1] = float4(0,Desc.ManipularScale,0,1);
	Lines[Z].resize(2);
	Lines[Z][0] = float4(0,0,0,1);
	Lines[Z][1] = float4(0,0,Desc.ManipularScale,1);

	Clipped[X] = CalcGeometry(X);
	Clipped[Y] = CalcGeometry(Y);
	Clipped[Z] = CalcGeometry(Z);
		
	float4x4 capXRot = oCreateRotation(radians(static_cast<float>(90.0)), float3(0,1,0));
	if(Desc.Mode == DESC::OBJECT)
		capXRot = capXRot * WorldRotation;
	float4x4 capYRot = oCreateRotation(radians(static_cast<float>(-90.0)), float3(1,0,0));
	if(Desc.Mode == DESC::OBJECT)
		capYRot = capYRot * WorldRotation;
	float4x4 capZRot = oIDENTITY4x4;
	if(Desc.Mode == DESC::OBJECT)
		capZRot = capZRot * WorldRotation;

	CalcCap(X, float4(Desc.ManipularScale,0,0,1), capXRot);
	CalcCap(Y, float4(0,Desc.ManipularScale,0,1), capYRot);
	CalcCap(Z, float4(0,0,Desc.ManipularScale,1), capZRot);
	CalcCap(SCREEN, float4(0,0,0,1), oIDENTITY4x4);
	
	float4 center = float4(0,0,0,1);
	center = WorldTransViewProj*center;
	if(center.w > 0)
		Clipped[SCREEN] = false;
	else
		Clipped[SCREEN] = true;
	if(!Clipped[SCREEN])
	{
		center /= center.w;
		center.z = static_cast<float>(0.01);
	}

	PickGeometry[SCREEN].resize(4);
	PickGeometry[SCREEN][0] = center+float4(-Desc.PickWidth, -Desc.PickWidth, 0, 0);
	PickGeometry[SCREEN][1] = center+float4(Desc.PickWidth, -Desc.PickWidth, 0, 0);
	PickGeometry[SCREEN][2] = center+float4(-Desc.PickWidth, Desc.PickWidth, 0, 0);
	PickGeometry[SCREEN][3] = center+float4(Desc.PickWidth, Desc.PickWidth, 0, 0);
	UnProject(PickGeometry[SCREEN]);
}
