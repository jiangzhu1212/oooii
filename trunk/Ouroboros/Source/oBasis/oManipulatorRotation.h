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
#pragma once
#ifndef oManipulatorRotation_h
#define oManipulatorRotation_h

#include "oManipulatorBase.h"
#include <oBasis/oArcball.h>

// {82C49DF6-FF6C-4246-A932-655E5A513647}
oDEFINE_GUID_S(oManipulatorRotation, 0x82c49df6, 0xff6c, 0x4246, 0xa9, 0x32, 0x65, 0x5e, 0x5a, 0x51, 0x36, 0x47);
struct oManipulatorRotation : oManipulatorBase
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oManipulatorRotation);

	oManipulatorRotation(const DESC& _Desc, bool *_pSuccess);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,);

	virtual void BeginManipulation(AXIS _Axis, const float2& _ScreenPosition);
	virtual void GetTransform(float4x4 *_pTransform) const;
	virtual void GetMaxSizes(size_t &_maxNumLines, size_t &_maxNumPickGeometry) const {_maxNumLines = LineGeometry.size(); _maxNumPickGeometry = __max(PickGeometryBase.size(), PickGeometryBaseArc.size());}
private:
	void UpdateImpl() override;
	bool CalcGeometry(AXIS _Axis, const float4x4 &_Transform); //returns whether the line/geom should be clipped
	bool CalcGeometry(std::vector<float4> &_vertices, std::deque<bool> &_clip, std::vector<float4> &_base, const float4x4 &_Transform); //returns whether the line/geom should be clipped
	void CalcPickGeometry(std::vector<float4> &_pickGeometry, const float4x4 &_Transform);
	void CalcPickGeometryArc(std::vector<float4> &_pickGeometry, const float4x4 &_Transform);

	float4 ScreenSpaceCenter;

	std::vector<float4> LineGeometry;
	std::vector<float4> PickGeometryBase;
	std::vector<float4> PickGeometryBaseFade;
	std::vector<float4> PickGeometryBaseArc;
	std::vector<float4> AngleGeometryBase;
	std::vector<float4> ClosestList;
	std::deque<bool> ClosestClip;

	float3 StartRotationVector;
	float3 CurrentRotationVector;

	int StartIndex;
	int CurrentIndex;
	
	oArcball Arcball;
	float Arcballradius;
};

#endif // oManipulatorRotation_h