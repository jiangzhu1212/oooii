/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
// Declare the public types for oMath. This header can be included separately 
// from oMath to keep public header contents simple. The basic types are 
// intended to be the same as in HLSL Shader Model 5.0.
#pragma once
#ifndef oMathTypes_h
#define oMathTypes_h

#include <oBasis/oMathInternalTypes.h>
#include <oBasis/oTypes.h>

typedef TVEC2<char> char2; typedef TVEC2<uchar> uchar2;
typedef TVEC3<char> char3; typedef TVEC3<uchar> uchar3;
typedef TVEC4<char> char4; typedef TVEC4<uchar> uchar4;

// _____________________________________________________________________________
// HLSL types

typedef TVEC2<short> short2; typedef TVEC2<ushort> ushort2;
typedef TVEC3<short> short3; typedef TVEC3<ushort> ushort3;
typedef TVEC4<short> short4; typedef TVEC4<ushort> ushort4;

typedef TVEC2<int> int2; typedef TVEC2<uint> uint2;
typedef TVEC3<int> int3; typedef TVEC3<uint> uint3;
typedef TVEC4<int> int4; typedef TVEC4<uint> uint4;

typedef TVEC2<llong> llong2; typedef TVEC2<ullong> ullong2;
typedef TVEC3<llong> llong3; typedef TVEC3<ullong> ullong3;
typedef TVEC4<llong> llong4; typedef TVEC4<ullong> ullong4;

typedef TVEC2<float> float2; typedef TVEC2<double> double2;
typedef TVEC3<float> float3; typedef TVEC3<double> double3;
typedef TVEC4<float> float4; typedef TVEC4<double> double4;

typedef TMAT3<float> float3x3; typedef TMAT3<double> double3x3;
typedef TMAT4<float> float4x4; typedef TMAT4<double> double4x4;

// _____________________________________________________________________________
// Additional types

typedef TQUAT<float> quatf; typedef TQUAT<double> quatd;
typedef TFRUSTUM<float> oFrustumf; //typedef TFRUSTUM<double> oFrustumd; // @oooii-tony: Need an oIntersects for double
typedef TSPHERE<float> oSpheref; typedef TSPHERE<double> oSphered;
typedef TPLANE<float> oPlanef; typedef TPLANE<double> oPlaned;
typedef TAABOX<float, TVEC3<float>> oAABoxf; typedef TAABOX<double, TVEC3<float>> oAABoxd;
typedef TAABOX<int, TVEC2<int>> oRECT;
typedef TAABOX<float, TVEC2<float>> oRECTF;

// _____________________________________________________________________________
// Commonly used values

static const float2 oZERO2(0.0f, 0.0f);
static const float3 oZERO3(0.0f, 0.0f, 0.0f);
static const float4 oZERO4(0.0f, 0.0f, 0.0f, 0.0f);

#endif
