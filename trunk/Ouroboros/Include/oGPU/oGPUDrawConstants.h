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
// This header is compiled both by HLSL and C++. It contains a robust set of 
// per-draw-based parameters fit for use as a constant buffer in a 3D rendering
// system. Use of oGPU_ does not require that this object be used, but for the 
// vast majority of use cases this provides a robust solution and thus has been 
// factored out as utility code to use or be the basis of a new, more fitted 
// solution.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGPUDrawConstants_h
#define oGPUDrawConstants_h

#include <oPlatform/oHLSL.h>
#include <oGPU/oGPUViewConstants.h>

#ifdef oHLSL
	// Accessor/helper functions assume a single declared constant buffer, so set 
	// this up so client code's compilation environment can override the name if 
	// desired.
	#ifndef oGPU_DRAW_CONSTANT_BUFFER
		#define oGPU_DRAW_CONSTANT_BUFFER GPUDrawConstants
	#endif

#endif

// Optionally redefine which constant buffer is used. This can be used as an 
// index in C++.
#ifndef oGPU_DRAW_CONSTANT_BUFFER_REGISTER
	#define oGPU_DRAW_CONSTANT_BUFFER_REGISTER 1
#endif

struct oGPUDrawConstants
{
	// Constant buffer to represent object-dependent, non-material parameters used 
	// in most straightforward rendering.

	// WARNING: Use of the WorldViewQuaternion implies that scale is always 
	// uniform and there's no shear. Is such a restriction acceptable?

#ifndef oHLSL

	enum CONSTRUCTION { Identity, };

	oGPUDrawConstants() {}
	oGPUDrawConstants(CONSTRUCTION _Type) { SetIdentity(); }
	oGPUDrawConstants(const float4x4& _World, const float4x4& _View, const float4x4& _Projection, uint _ObjectID = 0, uint _DrawID = 0) { Set(_World, _View, _Projection, _ObjectID, _DrawID); }

	inline void Set(const float4x4& _World, const float4x4& _View, const float4x4& _Projection, uint _ObjectID, uint _DrawID)
	{
		World = _World;
		WorldInverseTranspose = transpose(invert(World));
		WorldView = _World * _View;
		WorldViewProjection = WorldView * _Projection;
		WorldViewQuaternion = oCreateRotationQ(WorldView);
		ObjectID = _ObjectID;
		DrawID = _DrawID;
	}

	inline void SetIdentity()
	{
		World = float4x4::Identity;
		WorldInverseTranspose = transpose(invert(World));
		WorldView = float4x4::Identity;
		WorldViewProjection = float4x4::Identity;
		WorldViewQuaternion = quatf::Identity;
		ObjectID = 0;
		DrawID = 0;
	}

protected:
#endif

	float4x4 World;
	float4x4 WorldInverseTranspose;
	float4x4 WorldView;
	float4x4 WorldViewProjection;
	
	// Represents the rotation in the WorldView matrix as a quaternion
	quatf WorldViewQuaternion;
	
	// ObjectID is user-specified (MeshID, LinesID, QuadID) and identifies a group 
	// of meshes that share an attribute in common
	uint ObjectID;
	
	// DrawID is a monotonically increasing value that is unique for each call to 
	// Draw().
	uint DrawID;
	uint pad0;
	uint pad1;
};

#ifdef oHLSL
	cbuffer cbuffer_oGPUDrawConstants : register(oCONCAT(b, oGPU_DRAW_CONSTANT_BUFFER_REGISTER)) { oGPUDrawConstants oGPU_DRAW_CONSTANT_BUFFER; }

	// _____________________________________________________________________________
	// Screen-space rendering (deferred/inferred) utility functions

	// Transformation of points from one space to another (takes into account 
	// translation)

	float4 oGPULStoSS(float3 _LSPosition)
	{
		return oMul(oGPU_DRAW_CONSTANT_BUFFER.WorldViewProjection, float4(_LSPosition, 1));
	}

	float3 oGPULStoVS(float3 _LSPosition)
	{
		return oMul(oGPU_DRAW_CONSTANT_BUFFER.WorldView, float4(_LSPosition, 1)).xyz;
	}

	float3 oGPULStoWS(float3 _LSPosition)
	{
		return oMul(oGPU_DRAW_CONSTANT_BUFFER.World, float4(_LSPosition, 1)).xyz;
	}

	float4 oGPUQuatLStoVS(float4 _LSQuaternion)
	{
		return oQMul(oGPU_DRAW_CONSTANT_BUFFER.WorldViewQuaternion, _LSQuaternion);
	}

	float3 oGPUNormalLStoWS(float3 _LSNormal)
	{
		return oMul(oGPU_DRAW_CONSTANT_BUFFER.WorldInverseTranspose, float4(_LSNormal, 1)).xyz;
	}
	
	uint oGPUGetObjectID()
	{
		return oGPU_DRAW_CONSTANT_BUFFER.ObjectID;
	}

	uint oGPUGetDrawID()
	{
		return oGPU_DRAW_CONSTANT_BUFFER.DrawID;
	}

#endif
#endif