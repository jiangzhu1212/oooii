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
#pragma once
#ifndef oManipulatorScale_h
#define oManipulatorScale_h

#include "oManipulatorBase.h"

// {143C663B-7BF4-499a-B4BF-F6E852455A15}
oDEFINE_GUID_S(oManipulatorScale, 0x143c663b, 0x7bf4, 0x499a, 0xb4, 0xbf, 0xf6, 0xe8, 0x52, 0x45, 0x5a, 0x15);
struct oManipulatorScale : oManipulatorBase
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oManipulatorScale);

	oManipulatorScale(const DESC& _Desc, bool *_pSuccess);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,);

	virtual void BeginManipulation(AXIS _Axis, const float2& _ScreenPosition);
	virtual void GetTransform(float4x4 *_pTransform) const;
	virtual void GetMaxSizes(size_t &_maxNumLines, size_t &_maxNumPickGeometry) const {_maxNumLines = 4; _maxNumPickGeometry = 4;}
private:
	void UpdateImpl() override;
	float3 CalcScale(float3 axis); 
	bool CalcGeometry(AXIS _Axis); //returns whether the line/geom should be clipped
	void CalcCap(AXIS _Axis, float4 vert, const float4x4 &_capRotation);
};

#endif // oManipulatorScale_h