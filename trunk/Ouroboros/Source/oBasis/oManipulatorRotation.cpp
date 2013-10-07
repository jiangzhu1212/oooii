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
#include "oManipulatorRotation.h"
#include <oCompute/oComputeConstants.h>
#include <oBasis/oGeometry.h>

using namespace ouro;

oManipulatorRotation::oManipulatorRotation(const DESC& _Desc, bool *_pSuccess) 
	: oManipulatorBase(_Desc, _pSuccess)
	, Arcball(ouro::arcball::none)
{
	if(!(*_pSuccess)) //base failed to construct
		return;
	*_pSuccess = false;

	intrusive_ptr<oGeometry> CircleGeometry; 
	oGeometry::LAYOUT layout;
	layout.Positions = true;
	layout.Normals = false;
	layout.Tangents = false;
	layout.Texcoords = false;
	layout.Colors = false;
	layout.ContinuityIDs = false;

	auto RotationScale = Desc.ManipularScale;

	intrusive_ptr<oGeometryFactory> GeometryFactory;
	if(!oGeometryFactoryCreate(&GeometryFactory))
	{
		oErrorSetLast(std::errc::invalid_argument, "failed to create a geometry factor for manipulator");
		return;
	}

	oGeometryFactory::CIRCLE_DESC d;
	d.FaceType = oGeometry::OUTLINE;
	d.Radius = 1;
	d.Facet = ROTATION_CIRCLE_VCOUNT;
	d.Color = White;

	if(!GeometryFactory->CreateCircle(d, layout, &CircleGeometry))
	{
		oErrorSetLast(std::errc::invalid_argument, "failed to create a circle geometry for rotation manipulator");
		return;
	}

	oGeometry::DESC gdesc;
	CircleGeometry->GetDesc(&gdesc);
	oASSERT(gdesc.PrimitiveType == oGeometry::LINELIST,"created a line list, but actually didn't");

	oGeometry::CONST_MAPPED gmapped;
	if (!CircleGeometry->MapConst(&gmapped) || !gmapped.pIndices || !gmapped.pPositions)
	{
		oErrorSetLast(std::errc::invalid_argument, "Geometry Map() failed");
		return;
	}

	LineGeometry.resize(gdesc.NumIndices);

	for(unsigned int i = 0;i < gdesc.NumIndices;i++)
	{
		LineGeometry[i] = float4(make_scale(RotationScale)*gmapped.pPositions[gmapped.pIndices[i]],1.0f);
	}

	CircleGeometry->UnmapConst();

	d.Facet = ROTATION_CIRCLE_VCOUNT*2;
	if(!GeometryFactory->CreateCircle(d, layout, &CircleGeometry))
	{
		oErrorSetLast(std::errc::invalid_argument, "failed to create a circle geometry for rotation manipulator");
		return;
	}
	CircleGeometry->GetDesc(&gdesc);
	oASSERT(gdesc.PrimitiveType == oGeometry::LINELIST,"created a line list, but actually didn't");

	if (!CircleGeometry->MapConst(&gmapped) || !gmapped.pIndices || !gmapped.pPositions)
	{
		oErrorSetLast(std::errc::invalid_argument, "Geometry Map() failed");
		return;
	}
	AngleGeometryBase.resize(gdesc.NumVertices);

	for(unsigned int i = 0;i < gdesc.NumVertices;i++)
	{
		AngleGeometryBase[i] = float4(make_scale(RotationScale)*gmapped.pPositions[i],1.0f);
	}

	CircleGeometry->UnmapConst();
	
	d.FaceType = oGeometry::FRONT_CW;
	d.Radius = RotationScale;
	d.Facet = ROTATION_PICK_ARCBALL_VCOUNT;
	if(!GeometryFactory->CreateCircle(d, layout, &CircleGeometry))
	{
		oErrorSetLast(std::errc::invalid_argument, "failed to create a circle geometry for rotation manipulator");
		return;
	}
	CircleGeometry->GetDesc(&gdesc);

	if (!CircleGeometry->MapConst(&gmapped) || !gmapped.pIndices || !gmapped.pPositions)
	{
		oErrorSetLast(std::errc::invalid_argument, "Geometry Map() failed");
		return;
	}
	PickGeometryBaseArc.resize(gdesc.NumVertices);

	for(unsigned int i = 0;i < gdesc.NumVertices;i++)
	{
		PickGeometryBaseArc[i] = float4(gmapped.pPositions[i],1.0f);
	}

	CircleGeometry->UnmapConst();

	intrusive_ptr<oGeometry> TorusGeometry; 
	oGeometryFactory::TORUS_DESC td;
	td.FaceType = oGeometry::FRONT_CW;
	td.InnerRadius = RotationScale - Desc.PickWidth;
	td.OuterRadius = RotationScale + Desc.PickWidth;
	td.Divide = ROTATION_PICK_TORUS_DIVIDE;
	td.Facet = ROTATION_PICK_TORUS_FACET;
	td.Color = White;

	if(!GeometryFactory->CreateTorus(td, layout, &TorusGeometry))
	{
		oErrorSetLast(std::errc::invalid_argument, "failed to create a circle geometry for rotation manipulator");
		return;
	}
	if (!TorusGeometry->MapConst(&gmapped) || !gmapped.pIndices || !gmapped.pPositions)
	{
		oErrorSetLast(std::errc::invalid_argument, "Geometry Map() failed");
		return;
	}
	TorusGeometry->GetDesc(&gdesc);
	PickGeometryBase.resize(gdesc.NumVertices);
	for(unsigned int i = 0;i < gdesc.NumVertices;++i)
	{
		PickGeometryBase[i] = float4(gmapped.pPositions[i],1);
	}

	td.InnerRadius = RotationScale;
	td.OuterRadius = RotationScale;

	if(!GeometryFactory->CreateTorus(td, layout, &TorusGeometry))
	{
		oErrorSetLast(std::errc::invalid_argument, "failed to create a circle geometry for rotation manipulator");
		return;
	}
	if (!TorusGeometry->MapConst(&gmapped) || !gmapped.pIndices || !gmapped.pPositions)
	{
		oErrorSetLast(std::errc::invalid_argument, "Geometry Map() failed");
		return;
	}
	TorusGeometry->GetDesc(&gdesc);
	PickGeometryBaseFade.resize(gdesc.NumVertices);
	for(unsigned int i = 0;i < gdesc.NumVertices;++i)
	{
		PickGeometryBaseFade[i] = float4(gmapped.pPositions[i],1);
	}

	*_pSuccess = true;
}

void oManipulatorRotation::BeginManipulation(AXIS _Axis, const float2& _ScreenPosition)
{
	PickAxis = _Axis;
	CurrentTranslation = _ScreenPosition;
	StartTranslation = CurrentTranslation;

	StartIndex = -1;

	Picking = true;
}

void oManipulatorRotation::GetTransform(float4x4 *_pTransform) const
{
	if (Desc.Mode == DESC::WORLD)
	{
		*_pTransform = WorldScale*WorldRotation*Transform*WorldTranslation;
	}
	else
		*_pTransform = WorldScale*Transform*WorldRotation*WorldTranslation;
}

bool oManipulatorRotation::CalcGeometry(AXIS _Axis, const float4x4 &_Transform)
{
	return CalcGeometry(Lines[_Axis], Clip[_Axis], LineGeometry, _Transform);
}

bool oManipulatorRotation::CalcGeometry(std::vector<float4> &_vertices,  std::deque<bool> &_clip, std::vector<float4> &_base, const float4x4 &_Transform)
{
	_vertices.resize(_base.size());
	_clip.resize(_base.size());

	float4 center = float4(0,0,0,1);
	if(Project(center, _Transform))
		return true;

	for(unsigned int i = 0;i < _vertices.size();++i)
	{
		float4 temp = _base[i];
		if(Project(temp, _Transform))
			return true;
		if(temp.z < center.z+0.01f)
			_clip[i] = false;
		else
			_clip[i] = true;

		temp.z = static_cast<float>(0.01);		
		_vertices[i] = temp;	
	}

	return false;
}

void oManipulatorRotation::CalcPickGeometry(std::vector<float4> &_pickGeometry, const float4x4 &_Transform)
{
	float4 center = float4(0,0,0,1);
	Project(center, _Transform);

	_pickGeometry.resize(PickGeometryBase.size());

	float minz,maxz;
	minz = 1.0f;
	maxz = 0.0f;

	for(unsigned int i = 0;i < PickGeometryBase.size();++i)
	{
		float4 tempI = PickGeometryBase[i];
		Project(tempI, _Transform);

		float4 tempF = PickGeometryBaseFade[i];
		Project(tempF, _Transform);

		if(tempI.z > center.z)
		{
			float lerpv = (tempI.z - center.z)/0.01f;
			lerpv = clamp(lerpv,0.0f,1.0f);
			tempI = lerp(tempI,tempF,lerpv);
		}

		if(tempI.z < minz)
			minz = tempI.z;
		if(tempI.z > maxz)
			maxz = tempI.z;

		_pickGeometry[i] = tempI;
	}

	float zrange = maxz - minz;
	for(unsigned int i = 0;i < _pickGeometry.size();++i)
	{
		_pickGeometry[i].z = (_pickGeometry[i].z - minz)/zrange;
		_pickGeometry[i].z = (_pickGeometry[i].z * 0.01f) + 0.01f;
	}
	UnProject(_pickGeometry);
}

void oManipulatorRotation::CalcPickGeometryArc(std::vector<float4> &_pickGeometry, const float4x4 &_Transform)
{
	float4 center = float4(0,0,0,1);
	Project(center, _Transform);

	_pickGeometry.resize(PickGeometryBaseArc.size());
	
	for(unsigned int i = 0;i < PickGeometryBaseArc.size();++i)
	{
		float4 temp = PickGeometryBaseArc[i];
		Project(temp, _Transform);
		temp.z = 0.03f;

		_pickGeometry[i] = temp;
	}
	UnProject(_pickGeometry);
}

void oManipulatorRotation::UpdateImpl()
{
	ScreenSpaceCenter = WorldTranslation*View*Proj*float4(0,0,0,1);
	if(ScreenSpaceCenter.w > 0)
		ScreenSpaceCenter /= ScreenSpaceCenter.w;
	ScreenSpaceCenter.z = 0;
	ScreenSpaceCenter.w = 0;	

	Transform = oIDENTITY4x4;
	if(Picking && StartIndex >= 0)
	{
		StartRotationVector = normalize(AngleGeometryBase[StartIndex].xyz());
		CurrentRotationVector = normalize(AngleGeometryBase[CurrentIndex].xyz());

		float3 angleCross = cross(CurrentRotationVector,StartRotationVector);
		float angle = dot(CurrentRotationVector,StartRotationVector);
		angle = clamp(angle,-1.0f,1.0f); //fix for rounding errors above
		angle = acos(angle);

		if(angleCross.z > 0)
			angle = -angle;

		switch (PickAxis)
		{
		case X:
			Transform = make_rotation(angle,float3(1,0,0));
			break;
		case Y:
			Transform = make_rotation(angle,float3(0,-1,0));
			break;
		case Z:
			Transform = make_rotation(angle,float3(0,0,1));
			break;
		case SCREEN:
			{
				if(Desc.Mode == DESC::OBJECT)
					Transform = make_rotation(angle,invert(WorldRotation)*invert(ViewRot)*float3(0,0,1));
				else
					Transform = make_rotation(angle,invert(ViewRot)*float3(0,0,1));
			}
			break;
		case FREE:
			{
				// this delta probably needs to be scaled
				float2 delta = CurrentTranslation - StartTranslation;
				Arcball.rotate(delta);
				Transform = Arcball.view();
			}
			break;
		}
	}

	if(Desc.Mode == DESC::OBJECT)
		WorldTransViewProj = Transform*WorldRotation*WorldTranslation*View*Proj;
	else
		WorldTransViewProj = WorldTranslation*View*Proj;
	
	float4x4 xTrans = make_rotation(radians(static_cast<float>(90.0)),float3(0,1,0))*WorldTransViewProj;
	float4x4 yTrans = make_rotation(radians(static_cast<float>(90.0)),float3(1,0,0))*WorldTransViewProj;
	float4x4 zTrans = WorldTransViewProj;
	float4x4 cTrans = invert(ViewRot)*make_scale(1.2f)*WorldTranslation*View*Proj;

	Clipped[X] = CalcGeometry(X, xTrans);
	Clipped[Y] = CalcGeometry(Y, yTrans);
	Clipped[Z] = CalcGeometry(Z, zTrans);
	Clipped[SCREEN] = CalcGeometry(SCREEN, cTrans);
	Clipped[FREE] = false;

	float4x4 clTrans;
	if(Desc.Mode == DESC::OBJECT)
		clTrans = WorldRotation*WorldTranslation*View*Proj;
	else
		clTrans = WorldTranslation*View*Proj;
	switch(PickAxis)
	{
	case X:
		clTrans = make_rotation(radians(static_cast<float>(90.0)),float3(0,1,0))*clTrans;
		break;
	case Y:
		clTrans = make_rotation(radians(static_cast<float>(90.0)),float3(1,0,0))*clTrans;
		break;
	case Z:
		clTrans = clTrans;
		break;
	case SCREEN:
		clTrans = invert(ViewRot)*WorldTranslation*View*Proj;
		break;
	}
	CalcGeometry(ClosestList, ClosestClip, AngleGeometryBase, clTrans);
	
	float smallestDistance = std::numeric_limits<float>::max();
	unsigned int smallestIndex = 0;
	for (unsigned int i = 0;i < ClosestList.size();++i)
	{
		float tempDistance = distance(float3(ClosestList[i].xy(),0),float3(CurrentTranslation.x,CurrentTranslation.y,0));
		if(tempDistance < smallestDistance && !ClosestClip[i])
		{
			smallestDistance = tempDistance;
			smallestIndex = i;
		}
	}
	CurrentIndex = smallestIndex;
	if(StartIndex < 0 && Picking)
		StartIndex = CurrentIndex;
	Arcballradius = distance(Lines[SCREEN][0].xy(),ScreenSpaceCenter.xy());

	UnProject(Lines[X]);
	UnProject(Lines[Y]);
	UnProject(Lines[Z]);
	UnProject(Lines[SCREEN]);

	xTrans = make_rotation(radians(static_cast<float>(90.0)),float3(0,0,1))*WorldTransViewProj;
	yTrans = WorldTransViewProj;
	zTrans = make_rotation(radians(static_cast<float>(90.0)),float3(1,0,0))*WorldTransViewProj;
	cTrans = invert(ViewRot)*make_scale(1.2f)*make_rotation(radians(static_cast<float>(90.0)),float3(1,0,0))*WorldTranslation*View*Proj;
	float4x4 aTrans = make_scale(1.0f)*WorldTranslation*ViewScale*ViewTrans*Proj;
	CalcPickGeometry(PickGeometry[X],xTrans);
	CalcPickGeometry(PickGeometry[Y],yTrans);
	CalcPickGeometry(PickGeometry[Z],zTrans);
	CalcPickGeometry(PickGeometry[SCREEN],cTrans);
	CalcPickGeometryArc(PickGeometry[FREE],aTrans);
}
