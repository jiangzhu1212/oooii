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

	ViewScale = oCreateScale(oExtractScale(View));
	ViewTrans = oCreateTranslation(oExtractTranslation(View));
	ViewRot = oCreateRotation(oExtractRotation(View));

	WorldTranslation = oCreateTranslation(oExtractTranslation(World));
	WorldRotation = oCreateRotation(oExtractRotation(World));
	WorldScale = oCreateScale(oExtractScale(World));

	UpdateImpl();
}
