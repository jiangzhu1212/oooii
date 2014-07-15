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
// policy encapsulation of per-view values for rasterization of 3D scenes.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGfxViewConstants_h
#define oGfxViewConstants_h

#include <oGfx/oGfxHLSL.h>

#ifndef oHLSL

	#include <oBase/throw.h>
	#include <oCompute/linear_algebra.h>
	#include <oCompute/oFrustum.h>

	enum oGfx_VIEW_FAR_PLANE_CORNER
	{
		oGFX_VIEW_FAR_PLANE_BOTTOM_LEFT,
		oGFX_VIEW_FAR_PLANE_BOTTOM_RIGHT,
		oGFX_VIEW_FAR_PLANE_TOP_LEFT,
		oGFX_VIEW_FAR_PLANE_TOP_RIGHT,
	};

#else
	
	#define oGFX_VIEW_FAR_PLANE_BOTTOM_LEFT 0
	#define oGFX_VIEW_FAR_PLANE_BOTTOM_RIGHT 1
	#define oGFX_VIEW_FAR_PLANE_TOP_LEFT 2
	#define oGFX_VIEW_FAR_PLANE_TOP_RIGHT 3

#endif

// Optionally redefine which constant buffer is used. This can be used as an 
// index in C++.
#ifndef oGFX_VIEW_CONSTANTS_REGISTER
	#define oGFX_VIEW_CONSTANTS_REGISTER 0
#endif

struct oGfxViewConstants
{
	// Constant buffer to represent view-dependent parameters used in most 
	// straightforward rendering.

	#ifndef oHLSL
		enum CONSTRUCTION { Identity, };
		oGfxViewConstants() {}
		oGfxViewConstants(CONSTRUCTION _Type, const int3& _RenderTargetDimensions, uint _TextureArrayIndex) { Set(float4x4(oIDENTITY4x4), float4x4(oIDENTITY4x4), _RenderTargetDimensions, _TextureArrayIndex); }
		oGfxViewConstants(const float4x4& _View, const float4x4& _Projection, const int3& _RenderTargetDimensions, uint _TextureArrayIndex) { Set(_View, _Projection, _RenderTargetDimensions, _TextureArrayIndex); }
		inline void Set(const float4x4& _View, const float4x4& _Projection, const int3& _RenderTargetDimensions, uint _TextureArrayIndex)
		{
			View = _View;
			ViewInverse = ouro::invert(View);
			Projection = _Projection;
			ProjectionInverse = ouro::invert(Projection);
			ViewProjection = View * Projection;
			ViewProjectionInverse = ouro::invert(ViewProjection);

			float3 corners[8];
			oFrustumf psf(_Projection);
			oCHECK0(oExtractFrustumCorners(psf, corners));
			VSFarplaneCorners[oGFX_VIEW_FAR_PLANE_TOP_LEFT] = float4(corners[oFRUSTUM_LEFT_TOP_FAR], 0.0f);
			VSFarplaneCorners[oGFX_VIEW_FAR_PLANE_TOP_RIGHT] = float4(corners[oFRUSTUM_RIGHT_TOP_FAR], 0.0f);
			VSFarplaneCorners[oGFX_VIEW_FAR_PLANE_BOTTOM_LEFT] = float4(corners[oFRUSTUM_LEFT_BOTTOM_FAR], 0.0f);
			VSFarplaneCorners[oGFX_VIEW_FAR_PLANE_BOTTOM_RIGHT] = float4(corners[oFRUSTUM_RIGHT_BOTTOM_FAR], 0.0f);

			oASSERT(_RenderTargetDimensions.z==1, "Expecting render target dimensions without depth (z=1), but z=%d", _RenderTargetDimensions.z);
			RenderTargetDimensions = float2(static_cast<float>(_RenderTargetDimensions.x), static_cast<float>(_RenderTargetDimensions.y));

			// Store the 1/far plane distance so we can calculate view-space depth
			ouro::extract_near_far_distances(_Projection, &NearPlaneDistance, &FarPlaneDistance);
			InverseFarPlaneDistance = 1.0f / FarPlaneDistance;
			NearPlaneDistancePlusEpsilon = NearPlaneDistance + 0.1f;
			TextureArrayIndex = _TextureArrayIndex;
			Pad0 = 0;
		}

		float4x4 GetView() const { return View; }
		float4x4 GetViewInverse() const { return ViewInverse; }
		float4x4 GetProjection() const { return Projection; }

protected:
	#endif

	float4x4 View;
	float4x4 ViewInverse;
	float4x4 Projection;
	float4x4 ProjectionInverse;
	float4x4 ViewProjection;
	float4x4 ViewProjectionInverse;

	// Store the view space four corners of the farplane so a normalized
	// depth value plus the eye point can be converted back into world space
	float4 VSFarplaneCorners[4];

	// Dimensions are needed to convert screen coords from NDC space to texture 
	// space
	float2 RenderTargetDimensions;

	float NearPlaneDistance;
	float FarPlaneDistance;

	// Useful in the screen-space lighting pass to ensure any light that is behind
	// the viewer is pushed to a renderable position. This is just a bit beyond
	// the near plane so that a value with this Z doesn't get clipped.
	float NearPlaneDistancePlusEpsilon;

	// Used to normalize the depth/distance from eye to preserve precision 
	// when writing to the depth buffer, this is 1 / FarPlaneDistance.
	float InverseFarPlaneDistance;

	// Addressing when rendering into texture arrays
	uint TextureArrayIndex;
	uint Pad0;
};

#ifdef oHLSL
cbuffer cbuffer_oGfxViewConstants : register(oCONCAT(b, oGFX_VIEW_CONSTANTS_REGISTER)) { oGfxViewConstants GfxViewConstants; }

// To reconstruct a 3D position from a view's depth, the four corners of the 
// view's frustum are stored. In a vertex shader the corners should be extracted
// and written to interpolants. Then the corners will be interpolated across
// each pixel to the pixel shader where a value for the far plane will be 
// available. Depth is encoded as a normalized value to maximize precision by 
// dividing world depth by the distance to the far plane. Then when decoded,
// the normalized value is multiplied by the distance to the far plane along
// the vector between the eye and far plane point to get the 3D position.

// This should be called from the vertex shader.
// The full-screen quad is easy because the vertices of the FSQ are at the corner 
// pixels, and HW interpolation handles the rest. Given render of a full screen 
// quad with texcoords [0,0] for upper left of FSQ, [1,1] for lower right, this 
// will calculate the position of the far plane in view space.
float3 oGfxCalcViewSpaceFarPlanePositionFSQ(float2 _Texcoord)
{
	return GfxViewConstants.VSFarplaneCorners[_Texcoord.x + _Texcoord.y * 2].xyz;
}

// This should be called from the vertex shader.
// For non-full screen quads the interpolated value must be Calcd explicitly.
// NOTE: Using a WVP to get a screen space position in the vertex shader will yield
// different values that using an SV_Position, so be careful to use this only in a 
// vertex shader.
float3 oGfxCalcFarPlanePosition(float4 _ScreenSpaceQuadCornerPosition)
{
	float2 Texcoord = oCalcSSTexcoordVS(_ScreenSpaceQuadCornerPosition);
	float4 InterpolatedTop = lerp(GfxViewConstants.VSFarplaneCorners[oGFX_VIEW_FAR_PLANE_TOP_LEFT], GfxViewConstants.VSFarplaneCorners[oGFX_VIEW_FAR_PLANE_TOP_RIGHT], Texcoord.x);
	float4 InterpolatedBottom = lerp(GfxViewConstants.VSFarplaneCorners[oGFX_VIEW_FAR_PLANE_BOTTOM_LEFT], GfxViewConstants.VSFarplaneCorners[oGFX_VIEW_FAR_PLANE_BOTTOM_RIGHT], Texcoord.x);
	float4 InterpolatedFinal = lerp(InterpolatedTop, InterpolatedBottom, Texcoord.y);
	return InterpolatedFinal.xyz;
}

float4 oGfxVStoSS(float3 _VSPosition)
{
	return oMul(GfxViewConstants.Projection, float4(_VSPosition, 1));
}

float4 oGfxWStoSS(float3 _WSPosition)
{
	return oMul(GfxViewConstants.ViewProjection, float4(_WSPosition, 1));
}

float4 oGfxWStoVS(float3 _WSPosition)
{
	return oMul(GfxViewConstants.View, float4(_WSPosition, 1));
}

float3 oGfxRotateWStoVS(float3 _WSVector)
{
	return oMul(oGetUpper3x3(GfxViewConstants.View), _WSVector);
}

// NOTE: This must be called from a pixel shader
float2 oGfxCalcScreenSpaceTexcoordPS(float4 _SVPosition)
{
	return oCalcSSTexcoordPS(_SVPosition, GfxViewConstants.RenderTargetDimensions);
}

// encodes depth in a way that enhances precision
float oGfxNormalizeDepth(float _Depth)
{
	return oNormalizeDepth(_Depth, GfxViewConstants.NearPlaneDistance, GfxViewConstants.FarPlaneDistance);
}

// decodes the result of oGfxNormalizeDepth()
float oGfxUnnormalizeDepth(float _EncodedDepth)
{
	return oUnnormalizeDepth(_EncodedDepth, GfxViewConstants.NearPlaneDistance, GfxViewConstants.FarPlaneDistance);
}

// Uses encoded depth (encoded using oGfxNormalizeDepth) to reconstruct a 
// view-space position by interpolating the depth along the scaled vector to the 
// far plane.
float3 oGfxReconstructViewSpacePosition(float3 _ViewSpaceFarPlanePosition, float _NormalizedDepth)
{
	return oGfxUnnormalizeDepth(_NormalizedDepth) * _ViewSpaceFarPlanePosition;
}

// Calculate the vector pointing from the eye to the specified
// point on a surface
float3 oGfxCalcEyeVectorFromViewSpace(float3 _ViewSpaceSurfacePosition)
{
	// In view space the eye is at the origin so we only 
	// need to normalize position and negate
	return -normalize(_ViewSpaceSurfacePosition);
}

// For screen-space quad representations of volumes (like light volumes)
// ensure that if a volume still influences the sceen from behind the eye
// that we can position the quad geometry in a renderable position.
float3 oGfxClampViewSpacePositionToNearPlane(float3 _ViewSpacePosition)
{
	return clamp(_ViewSpacePosition, float3(-1,-1, GfxViewConstants.NearPlaneDistancePlusEpsilon), float3(1,1,1) );
}

float2 oGfxGetRenderTargetDimensions()
{
	return GfxViewConstants.RenderTargetDimensions;
}

uint oGfxGetRenderTargetIndex()
{
	return GfxViewConstants.TextureArrayIndex;
}

float3 oGfxGetEyePosition()
{
	return oGetEyePosition(GfxViewConstants.ViewInverse);
}

#endif
#endif
