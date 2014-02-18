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
// This code contains constants that can be used in either HLSL or C++.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oComputeConstants_h
#define oComputeConstants_h

#ifdef oHLSL
	#define FLT_EPSILON 1.192092896e-07F /* smallest such that 1.0+FLT_EPSILON != 1.0 */
	#define FLT_MAX 3.402823466e+38F /* max value */
#else
	#include <float.h>
	#include <oHLSL/oHLSLTypes.h>
#endif

// _____________________________________________________________________________
// Common math constants

#define oPI (3.14159265358979323846)
#define oPIf (float(oPI))
#define oE (2.71828183)
#define oEf (float(oE))

#define oSQRT2 (1.41421356237309504880)
#define oSQRT2f (float(oSQRT2))
#define oHALF_SQRT2 (0.70710678118654752440)
#define oHALF_SQRT2f (float(oHALF_SQRT2))

// sqrt(0.75f) : length from the center of a unit cube to one of the corners
#define oSQRT3Quarter (0.86602540378443860)
#define oSQRT3Quarterf (float(oSQRT3Quarter))

#define oDEFAULT_FOVY_RADIANS (oPIf / 4.0f)
#define oDEFAULT_NEAR (10.0f)
#define oDEFAULT_FAR (10000.0f)

// Used to identify an axis throughout shader code
typedef int oAxis;
#define oAXIS_X 0
#define oAXIS_Y 1
#define oAXIS_Z 2

// _____________________________________________________________________________
// Identities

static const float2 oZERO2 = float2(0.0f, 0.0f);
static const float3 oZERO3 = float3(0.0f, 0.0f, 0.0f);
static const float4 oZERO4 = float4(0.0f, 0.0f, 0.0f, 0.0f);

static const float3 oXAXIS3 = float3(1.0f, 0.0f, 0.0f);
static const float3 oYAXIS3 = float3(0.0f, 1.0f, 0.0f);
static const float3 oZAXIS3 = float3(0.0f, 0.0f, 1.0f);

static const float4 oXAXIS4 = float4(1.0f, 0.0f, 0.0f, 0.0f);
static const float4 oYAXIS4 = float4(0.0f, 1.0f, 0.0f, 0.0f);
static const float4 oZAXIS4 = float4(0.0f, 0.0f, 1.0f, 0.0f);
static const float4 oWAXIS4 = float4(0.0f, 0.0f, 0.0f, 1.0f);

static const float3x3 oIDENTITY3x3 = float3x3(oXAXIS3, oYAXIS3, oZAXIS3);
static const float4x4 oIDENTITY4x4 = float4x4(oXAXIS4, oYAXIS4, oZAXIS4, oWAXIS4);

// @tony: We're currently compiling against SM4, but we need a mechanism
// to pass into the shader which shader model it actually is for stuff like
// this... for now keep doubles in C++ land only.
#ifndef oHLSL
	static const double3x3 oIDENTITY3x3D = double3x3(oXAXIS3, oYAXIS3, oZAXIS3);
	static const double4x4 oIDENTITY4x4D = double4x4(oXAXIS4, oYAXIS4, oZAXIS4, oWAXIS4);
#endif

// _____________________________________________________________________________
// Other constants

static const float3 oVECTOR_UP = oYAXIS3;

#ifdef oRIGHTHANDED
	static const float3 oVECTOR_OUT_OF_SCREEN = oZAXIS3;
	static const float3 oVECTOR_INTO_SCREEN = -oZAXIS3;
#else
	static const float3 oVECTOR_OUT_OF_SCREEN = -oZAXIS3;
	static const float3 oVECTOR_INTO_SCREEN = oZAXIS3;
#endif

static const float3 oBLACK3 = float3(0.0f, 0.0f, 0.0f);
static const float3 oWHITE3 = float3(1.0f, 1.0f, 1.0f);
static const float3 oRED3 = float3(1.0f,0.0f,0.0f);
static const float3 oGREEN3 = float3(0.0f, 1.0f,0.0f);
static const float3 oBLUE3 = float3(0.0f,0.0f, 1.0f);
static const float3 oYELLOW3 = float3(1.0f, 1.0f,0.0f);
static const float3 oMAGENTA3 = float3(1.0f,0.0f, 1.0f);
static const float3 oCYAN3 = float3(0.0f, 1.0f, 1.0f);

static const float4 oBLACK4 = float4(0.0f, 0.0f, 0.0f, 1.0f);
static const float4 oWHITE4 = float4(1.0f, 1.0f, 1.0f, 1.0f);
static const float4 oRED4 = float4(1.0f, 0.0f, 0.0f, 1.0f);
static const float4 oGREEN4 = float4(0.0f, 1.0f, 0.0f, 1.0f);
static const float4 oBLUE4 = float4(0.0f, 0.0f, 1.0f, 1.0f);
static const float4 oYELLOW4 = float4(1.0f, 1.0f, 0.0f, 1.0f);
static const float4 oMAGENTA4 = float4(1.0f, 0.0f, 1.0f, 1.0f);
static const float4 oCYAN4 = float4(0.0f, 1.0f, 1.0f, 1.0f);

static const float3 oCOLORS3[] = { oBLACK3, oBLUE3, oGREEN3, oCYAN3, oRED3, oMAGENTA3, oYELLOW3, oWHITE3, };
static const float4 oCOLORS4[] = { oBLACK4, oBLUE4, oGREEN4, oCYAN4, oRED4, oMAGENTA4, oYELLOW4, oWHITE4, };

// Three unit length vectors that approximate a hemisphere
// with a normal of float3(0.0f, 0.0f, 1.0f)
static const float3 oHEMISPHERE3[3] = 
{
	float3(0.0f, oSQRT3Quarterf, 0.5f ),
	float3(-0.75f, -oSQRT3Quarterf * 0.5f, 0.5f),
	float3(0.75f, -oSQRT3Quarterf * 0.5f, 0.5f),
};

static const float3 oHEMISPHERE4[4] = 
{
	float3(0.0f, oSQRT3Quarterf, 0.5f),
	float3(-oSQRT3Quarterf, 0.0f, 0.5f),
	float3(0.0f, -oSQRT3Quarterf, 0.5f),
	float3(oSQRT3Quarterf, 0.0f, 0.5f),
};

#endif
