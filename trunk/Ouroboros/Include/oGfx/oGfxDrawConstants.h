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
// This header is compiled both by HLSL and C++. It describes a oGfx-level 
// policy encapsulation of per-draw values for rasterization of 3D scenes.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGfxDrawConstants_h
#define oGfxDrawConstants_h

#include <oGfx/oGfxHLSL.h>
#include <oGfx/oGfxViewConstants.h>

// Optionally redefine which constant buffer register is used. This can be used 
// as an index in C++.
#ifndef oGFX_DRAW_CONSTANTS_REGISTER
	#define oGFX_DRAW_CONSTANTS_REGISTER 1
#endif

#ifndef oHLSL
	#include <oBasis/oMath.h>
#endif

struct oGfxDrawConstants
{
	// Constant buffer to represent object-dependent, non-material parameters used 
	// in most straightforward rendering.

	// WARNING: Use of the WorldViewQuaternion implies that scale is always 
	// uniform and there's no shear.

#ifndef oHLSL

	enum CONSTRUCTION { Identity, };

	oGfxDrawConstants() {}
	oGfxDrawConstants(CONSTRUCTION _Type) { SetIdentity(); }
	oGfxDrawConstants(const float4x4& _World, const float4x4& _View, const float4x4& _Projection, const oAABoxf& _ObjectBound, uint _ObjectID = 0, uint _DrawID = 0) { Set(_World, _View, _Projection, _ObjectBound, _ObjectID, _DrawID); }
	oGfxDrawConstants(const float4x4& _World, const oGfxViewConstants& _ViewConstants, const oAABoxf& _ObjectBound, uint _ObjectID = 0, uint _DrawID = 0) { Set(_World, _ViewConstants.GetView(), _ViewConstants.GetProjection(), _ObjectBound, _ObjectID, _DrawID); }

	inline void Set(const float4x4& _World, const float4x4& _View, const float4x4& _Projection, const oAABoxf& _ObjectBound, uint _ObjectID, uint _DrawID)
	{
		World = _World;
		WorldView = _World * _View;
		WorldViewInverse = invert(WorldView);
		WorldViewProjection = WorldView * _Projection;
		Normalized = oCreateNormalizationMatrix(_ObjectBound.Min, _ObjectBound.Max);
		NormalizedInverse = invert(Normalized);
		WorldQuaternion = oCreateRotationQ(World);
		WorldViewQuaternion = oCreateRotationQ(WorldView);
		Scale = max(_ObjectBound.size());
		ObjectID = _ObjectID;
		DrawID = _DrawID;
		pad0 = 0;
	}

	inline void SetIdentity()
	{
		World = oIDENTITY4x4;
		WorldView = oIDENTITY4x4;
		WorldViewProjection = oIDENTITY4x4;
		Normalized = oIDENTITY4x4;
		NormalizedInverse = oIDENTITY4x4;
		WorldQuaternion = oIDENTITYQ;
		WorldViewQuaternion = oIDENTITYQ;
		Scale = 1.0f;
		ObjectID = 0;
		DrawID = 0;
		pad0 = 0;
	}

#endif

	float4x4 World;
	float4x4 WorldView;
	float4x4 WorldViewInverse;
	float4x4 WorldViewProjection;

	// Transforms a local-space position in the draw call into an 
	// AABox(float3(0,0,0), float3(1,1,1)).
	float4x4 Normalized;
	float4x4 NormalizedInverse;
	
	// Represents the rotation in the World matrix as a quaternion.
	quatf WorldQuaternion;

	// Represents the rotation in the WorldView matrix as a quaternion.
	quatf WorldViewQuaternion;
	
	// Normalized's scaling factor is 1 / Scale. This is basically the length of 
	// the longest size of the object's overall AABox bound.
	float Scale;

	// ObjectID is user-specified (MeshID, LinesID, QuadID) and identifies a group 
	// of meshes that share an attribute in common
	uint ObjectID;
	
	// DrawID is a monotonically increasing value unique for each call to Draw().
	uint DrawID;

	uint pad0;
};

#ifdef oHLSL
	cbuffer cbuffer_oGfxDrawConstants : register(oCONCAT(b, oGFX_DRAW_CONSTANTS_REGISTER)) { oGfxDrawConstants GfxDrawConstants; }

	float4 oGfxLStoSS(float3 _LSPosition)
	{
		return oMul(GfxDrawConstants.WorldViewProjection, float4(_LSPosition, 1));
	}

	float3 oGfxLStoVS(float3 _LSPosition)
	{
		return oMul(GfxDrawConstants.WorldView, float4(_LSPosition, 1)).xyz;
	}

	float3 oGfxLStoWS(float3 _LSPosition)
	{
		return oMul(GfxDrawConstants.World, float4(_LSPosition, 1)).xyz;
	}

	float3 oGfxVStoLS(float3 _VSPosition)
	{
		return oMul(GfxDrawConstants.WorldViewInverse, float4(_VSPosition, 1)).xyz;
	}

	float4 oGfxRotateQuatLStoWS(float4 _LSQuaternion)
	{
		return qmul(GfxDrawConstants.WorldQuaternion, _LSQuaternion);
	}

	float4 oGfxRotateQuatLStoVS(float4 _LSQuaternion)
	{
		return qmul(GfxDrawConstants.WorldViewQuaternion, _LSQuaternion);
	}

	float3 oGfxRotateLStoWS(float3 _LSVector)
	{
		return oMul(oGetUpper3x3(GfxDrawConstants.World), _LSVector);
	}
	
	float3 oGfxRotateLStoVS(float3 _LSVector)
	{
		return oMul(oGetUpper3x3(GfxDrawConstants.WorldView), _LSVector);
	}

	void oGfxRotateBasisLStoWS(float3 _LSNormal, float4 _LSTangent, out float3 _OutWSNormal, out float3 _OutWSTangent, out float3 _OutWSBitangent)
	{
		oTransformTangentBasisVectors(GfxDrawConstants.World, _LSNormal, _LSTangent, _OutWSNormal, _OutWSTangent, _OutWSBitangent);
	}

	void oGfxRotateBasisLStoVS(float3 _LSNormal, float4 _LSTangent, out float3 _OutVSNormal, out float3 _OutVSTangent, out float3 _OutVSBitangent)
	{
		oTransformTangentBasisVectors(GfxDrawConstants.WorldView, _LSNormal, _LSTangent, _OutVSNormal, _OutVSTangent, _OutVSBitangent);
	}

	uint oGfxGetObjectID()
	{
		return GfxDrawConstants.ObjectID;
	}

	uint oGfxGetDrawID()
	{
		return GfxDrawConstants.DrawID;
	}

#endif
#endif
