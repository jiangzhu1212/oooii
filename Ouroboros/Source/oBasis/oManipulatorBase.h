// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oManipulatorBase_h
#define oManipulatorBase_h

#include <oBasis/oRefCount.h>
#include <oBasis/oManipulator.h>
#include <oCompute/linear_algebra.h>
#include <deque>
#include <vector>

class oManipulatorBase : public oManipulator
{
public:
	virtual void SetDesc(const DESC* _pDesc) {Desc = *_pDesc;}
	void GetCapTransform(AXIS _Axis, float4x4 &_transform) const override {_transform = CapTransforms[_Axis];}
	void GetLinesPickGeometry(AXIS _Axis, float3 *_vertices, size_t &_numWritten) const override
	{  
		for (size_t i = 0; i < PickGeometry[_Axis].size(); ++i)
		{
			_vertices[i] = PickGeometry[_Axis][i].xyz();
		}
		_numWritten = PickGeometry[_Axis].size();
	}
	void GetLines(AXIS _Axis, float3 *_lines, size_t &_numWritten) const override;
	void EndManipulation() override { Picking = false; }
	bool IsClipped(AXIS _Axis) const override {return Clipped[_Axis];}

	void Update(const float2& _ScreenPosition, const float4x4 &_World, const float4x4 &_View, const float4x4 &_Proj) override;
protected:
	oManipulatorBase(const DESC& _Desc, bool *_pSuccess);

	//returns true if clipped by near plane
	bool Project(float4 &_vert, const float4x4 &_Transform)
	{
		_vert = _Transform*_vert;
		if(_vert.w > 0)
			_vert /= _vert.w; 
		else
			return true;
		return false;
	}

	//returns true if clipped by near plane
	bool Project(float4 &_vert)
	{
		return Project(_vert, WorldTransViewProj);
	}

	//returns true if any vert is clipped by near plane
	bool Project(std::vector<float4> &_vertices)
	{
		bool retval = false;
		for (float4 &_temp : _vertices)
		{
			if(Project(_temp))
				retval = true;
		}
		return retval;
	}

	void UnProject(float4 &_vert)
	{
		_vert = InvViewProj*_vert;
		if(_vert.w != 0)
			_vert /= _vert.w; 
	}

	void UnProject(std::vector<float4> &_vertices)
	{
		for(unsigned int i = 0;i < _vertices.size();++i)
		{
			UnProject(_vertices[i]);
		}
	}

	virtual void UpdateImpl() = 0;

	oRefCount RefCount;
	DESC Desc;

	bool Picking;
	AXIS PickAxis;

	float2 StartTranslation;
	float2 CurrentTranslation;

	float4x4 Transform;
	float4x4 World;
	float4x4 View;
	float4x4 Proj;
	float4x4 WorldView;
	float4x4 WorldTransViewProj;
	float4x4 InvWorldView;
	float4x4 InvProj;
	float4x4 InvViewProj;
	float4x4 ViewScale;
	float4x4 ViewTrans;
	float4x4 ViewRot;
	float4x4 WorldTranslation;
	float4x4 WorldRotation;
	float4x4 WorldScale;

	bool Clipped[NUM_AXES];
	float4x4 CapTransforms[NUM_AXES];
	std::vector<float4> PickGeometry[NUM_AXES];
	std::deque<bool> Clip[NUM_AXES];
	std::vector<float4> Lines[NUM_AXES];
};

#endif
