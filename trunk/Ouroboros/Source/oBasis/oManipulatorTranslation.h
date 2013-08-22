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
#ifndef oManipulatorTransition_h
#define oManipulatorTransition_h

#include "oManipulatorbase.h"

// {0E3FDA0A-2397-4964-A3C6-3E013E63C476}
oDEFINE_GUID_S(oManipulatorTranslation, 0xe3fda0a, 0x2397, 0x4964, 0xa3, 0xc6, 0x3e, 0x1, 0x3e, 0x63, 0xc4, 0x76);
struct oManipulatorTranslation : oManipulatorBase
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oManipulatorTranslation);

	oManipulatorTranslation(const DESC& _Desc, bool *_pSuccess);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,);

	virtual void BeginManipulation(AXIS _Axis, const float2& _ScreenPosition);
	virtual void GetTransform(float4x4 *_pTransform) const;
	virtual void GetMaxSizes(size_t &_maxNumLines, size_t &_maxNumPickGeometry) const {_maxNumLines = 4; _maxNumPickGeometry = 4;}
private:
	void UpdateImpl() override;
	float3 CalcTranslation(float3 axis); 
	bool CalcGeometry(AXIS _Axis); //returns whether the line/geom should be clipped
	void CalcCap(AXIS _Axis, float4 vert, const float4x4 &_capRotation); 
};

#endif // oManipulatorTransition_h