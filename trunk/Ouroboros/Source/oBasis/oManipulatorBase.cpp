// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include "oManipulatorBase.h"
#include <oCompute/oComputeConstants.h>

namespace ouro { 

const char* as_string(const oManipulator::AXIS& _Axis)
{
	switch(_Axis)
	{
	case oManipulator::X:
		return "X";
	case oManipulator::Y:
		return "Y";
	case oManipulator::Z:
		return "Z";
	case oManipulator::SCREEN:
		return "SCREEN";
	case oManipulator::FREE:
		return "FREE";
	default:
		return "UNKNOWN";
	}
}

} // namespace ouro

using namespace ouro;

oManipulatorBase::oManipulatorBase(const DESC& _Desc, bool *_pSuccess) : Desc(_Desc), Picking(false)
{
	*_pSuccess = false;
	for (int i = 0; i < NUM_AXES ; i++)
	{
		CapTransforms[i] = oIDENTITY4x4;
	}
	*_pSuccess = true;
}

void oManipulatorBase::GetLines(AXIS _Axis, float3 *_lines, size_t &_numWritten) const
{
	oASSERT(Lines[_Axis].size() == Clip[_Axis].size(), "clipping information is not the same size as line list");

	unsigned int outIndex = 0;
	for(unsigned int i = 0;i < Lines[_Axis].size();i+=2)
	{
		if(!Clip[_Axis][i] && !Clip[_Axis][i+1])
		{
			_lines[outIndex++] = Lines[_Axis][i].xyz();
			_lines[outIndex++] = Lines[_Axis][i+1].xyz();
		}
	}
	_numWritten = Lines[_Axis].size();
}

void oManipulatorBase::Update(const float2& _ScreenPosition, const float4x4 &_World, const float4x4 &_View, const float4x4 &_Proj)
{
	CurrentTranslation = _ScreenPosition;
	World = _World;
	View = _View;
	Proj = _Proj;
	WorldView = World * View;
	InvProj = invert(Proj);
	InvWorldView = invert(WorldView);
	InvViewProj = invert(View * Proj);

	ViewScale = make_scale(extract_scale(View));
	ViewTrans = make_translation(extract_translation(View));
	ViewRot = make_rotation(extract_rotation(View));

	WorldTranslation = make_translation(extract_translation(World));
	WorldRotation = make_rotation(extract_rotation(World));
	WorldScale = make_scale(extract_scale(World));

	UpdateImpl();
}
