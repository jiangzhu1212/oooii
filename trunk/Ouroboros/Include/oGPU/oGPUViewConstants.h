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
// view-based parameters fit for use as a constant buffer in a 3D rendering 
// system, including forward, deferred and inferred setups. Use of oGPU_ does 
// not require that this be used, but for the vast majority of use cases this 
// provides a robust solution and thus has been factored out as utility code to 
// use, or be the basis of a new, more fitted solution.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGPUViewConstants_h
#define oGPUViewConstants_h

#include <oPlatform/oHLSL.h>

#ifndef oHLSL
	enum oGPU_VIEW_FAR_PLANE_CORNER
	{
		oGPU_VIEW_FAR_PLANE_BOTTOM_LEFT,
		oGPU_VIEW_FAR_PLANE_BOTTOM_RIGHT,
		oGPU_VIEW_FAR_PLANE_TOP_LEFT,
		oGPU_VIEW_FAR_PLANE_TOP_RIGHT,
	};

#else
	#define oGPU_VIEW_FAR_PLANE_BOTTOM_LEFT 0
	#define oGPU_VIEW_FAR_PLANE_BOTTOM_RIGHT 1
	#define oGPU_VIEW_FAR_PLANE_TOP_LEFT 2
	#define oGPU_VIEW_FAR_PLANE_TOP_RIGHT 3

	// Accessor/helper functions assume a single declared constant buffer, so set 
	// this up so client code's compilation environment can override the name if 
	// desired.
	#ifndef oGPU_VIEW_CONSTANT_BUFFER
		#define oGPU_VIEW_CONSTANT_BUFFER GPUViewConstants
	#endif

#endif

// Optionally redefine which constant buffer is used. This can be used as an 
// index in C++.
#ifndef oGPU_VIEW_CONSTANT_BUFFER_REGISTER
	#define oGPU_VIEW_CONSTANT_BUFFER_REGISTER 0
#endif

struct oGPUViewConstants
{
	// Constant buffer to represent view-dependent parameters used in most 
	// straightforward rendering.

	#ifndef oHLSL
		enum CONSTRUCTION { Identity, };
		oGPUViewConstants() {}
		oGPUViewConstants(CONSTRUCTION _Type, float2 _RenderTargetDimensions, uint _TextureArrayIndex) { Set(float4x4(float4x4::Identity), float4x4(float4x4::Identity), _RenderTargetDimensions, _TextureArrayIndex); }
		oGPUViewConstants(const float4x4& _View, const float4x4& _Projection, const float2& _RenderTargetDimensions, uint _TextureArrayIndex) { Set(_View, _Projection, _RenderTargetDimensions, _TextureArrayIndex); }
		inline void Set(const float4x4& _View, const float4x4& _Projection, const float2& _RenderTargetDimensions, uint _TextureArrayIndex)
		{
			View = _View;
			InverseView = invert(View);
			Projection = _Projection;
			ViewProjection = View * Projection;

			float3 corners[8];
			oFrustumf psf(_Projection);
			oVERIFY(oExtractFrustumCorners(corners, psf));
			ViewSpaceFarplaneCorners[oGPU_VIEW_FAR_PLANE_TOP_LEFT] = float4(corners[oFrustumf::LEFT_TOP_FAR], 0.0f);
			ViewSpaceFarplaneCorners[oGPU_VIEW_FAR_PLANE_TOP_RIGHT] = float4(corners[oFrustumf::RIGHT_TOP_FAR], 0.0f);
			ViewSpaceFarplaneCorners[oGPU_VIEW_FAR_PLANE_BOTTOM_LEFT] = float4(corners[oFrustumf::LEFT_BOTTOM_FAR], 0.0f);
			ViewSpaceFarplaneCorners[oGPU_VIEW_FAR_PLANE_BOTTOM_RIGHT] = float4(corners[oFrustumf::RIGHT_BOTTOM_FAR], 0.0f);

			RenderTargetDimensions = _RenderTargetDimensions;

			// Store the 1/far plane distance so we can calculate view-space depth
			oCalculateNearInverseFarPlanesDistance(_Projection, &NearPlaneDistancePlusEpsilon, &InverseFarPlaneDistance);

			NearPlaneDistancePlusEpsilon += 0.1f;
			TextureArrayIndex = _TextureArrayIndex;
			Pad0 = Pad1 = Pad2 = 0;
		}

protected:
	#endif

	float4x4 View;
	float4x4 InverseView;
	float4x4 Projection;
	float4x4 ViewProjection;

	// Store the view space four corners of the farplane so a normalized
	// depth value plus the eye point can be converted back into world space
	float4 ViewSpaceFarplaneCorners[4];

	// Dimensions are needed to convert screen coords from NDC space to texture 
	// space
	float2 RenderTargetDimensions;

	// Useful in the screen-space lighting pass to ensure any light that is behind
	// the viewer is pushed to a renderable position. This is just a bit beyond
	// the near plane so that a value with this Z doesn't get clipped.
	float NearPlaneDistancePlusEpsilon;

	// Used to normalize the depth/distance from eye to preserve precision 
	// when writing to the depth buffer
	float InverseFarPlaneDistance;

	// Addressing when rendering into texture arrays
	uint TextureArrayIndex;
	uint Pad0;
	uint Pad1;
	uint Pad2;
};

// Refactor note...
// Biggest challenge to making methods cross-compile: swizzle operators. In C++
// they have to be a method (uses parens) and in HLSL it's native and 
// convenient. We could put in GetXYZ, GetX, GetY, GetZ. But should we obfuscate
// the code which probably won't be called too often in C++?
#ifdef oHLSL
cbuffer cbuffer_GPUViewConstants : register(oCONCAT(b, oGPU_VIEW_CONSTANT_BUFFER_REGISTER)) { oGPUViewConstants oGPU_VIEW_CONSTANT_BUFFER; }

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
// will Calc the position of the far plane in view space.
float3 oGPUCalcViewSpaceFarPlanePositionFSQ(float2 _Texcoord)
{
	return oGPU_VIEW_CONSTANT_BUFFER.ViewSpaceFarplaneCorners[_Texcoord.x + _Texcoord.y * 2].xyz;
}

// This should be called from the vertex shader.
// For non-full screen quads the interpolated value must be Calcd explicitly.
// NOTE: Using a WVP to get a screen space position in the vertex shader will yield
// different values that using an SV_Position, so be careful to use this only in a 
// vertex shader.
float3 oGPUCalcFarPlanePosition(float4 _ScreenSpaceQuadCornerPosition)
{
	float2 Texcoord = oCalculateScreenSpaceTexcoordVS(_ScreenSpaceQuadCornerPosition);
	float4 InterpolatedTop = lerp(oGPU_VIEW_CONSTANT_BUFFER.ViewSpaceFarplaneCorners[oGPU_VIEW_FAR_PLANE_TOP_LEFT], oGPU_VIEW_CONSTANT_BUFFER.ViewSpaceFarplaneCorners[oGPU_VIEW_FAR_PLANE_TOP_RIGHT], Texcoord.x);
	float4 InterpolatedBottom = lerp(oGPU_VIEW_CONSTANT_BUFFER.ViewSpaceFarplaneCorners[oGPU_VIEW_FAR_PLANE_BOTTOM_LEFT], oGPU_VIEW_CONSTANT_BUFFER.ViewSpaceFarplaneCorners[oGPU_VIEW_FAR_PLANE_BOTTOM_RIGHT], Texcoord.x);
	float4 InterpolatedFinal = lerp(InterpolatedTop, InterpolatedBottom, Texcoord.y);
	return InterpolatedFinal.xyz;
}

float4 oGPUVStoSS(float3 _ViewSpacePosition)
{
	return oMul(oGPU_VIEW_CONSTANT_BUFFER.Projection, float4(_ViewSpacePosition, 1));
}

float4 oGPUWStoSS(float3 _WorldSpacePosition)
{
	return oMul(oGPU_VIEW_CONSTANT_BUFFER.ViewProjection, float4(_WorldSpacePosition, 1));
}

// NOTE: This must be called from a pixel shader
float2 oGPUCalcScreenSpaceTexcoordPS(float4 _SVPosition)
{
	return oCalculateScreenSpaceTexcoordPS(_SVPosition, oGPU_VIEW_CONSTANT_BUFFER.RenderTargetDimensions);
}

// Keep depth values between 0 and 1 to maximize precision
float oGPUEncodeNormalizedViewSpaceDepth(float _ViewSpaceDepth)
{
	return _ViewSpaceDepth * oGPU_VIEW_CONSTANT_BUFFER.InverseFarPlaneDistance;
}

// Use the encoded depth and the far plane's position to recreate a 3D position
float3 oGPU_ReconstructViewSpacePosition(float3 _ViewSpaceFarPlanePosition, float _EncodedViewSpaceDepth)
{
	return _EncodedViewSpaceDepth * _ViewSpaceFarPlanePosition;
}

// Calculate the vector pointing from the eye to the specified
// point on a surface
float3 oGPUCalcEyeVectorFromViewSpace(float3 _ViewSpaceSurfacePosition)
{
	// In view space the eye is at the origin so we only 
	// need to normalize position and negate
	return -normalize(_ViewSpaceSurfacePosition);
}

// For screen-space quad representations of volumes (like light volumes)
// ensure that if a volume still influences the sceen from behind the eye
// that we can position the quad geometry in a renderable position.
float3 oGPUClampViewSpacePositionToNearPlane(float3 _ViewSpacePosition)
{
	return clamp(_ViewSpacePosition, float3(-1,-1, oGPU_VIEW_CONSTANT_BUFFER.NearPlaneDistancePlusEpsilon), float3(1,1,1) );
}

float2 oGPUGetRenderTargetDimensions()
{
	return oGPU_VIEW_CONSTANT_BUFFER.RenderTargetDimensions;
}

uint oGPUGetRenderTargetIndex()
{
	return oGPU_VIEW_CONSTANT_BUFFER.TextureArrayIndex;
}

float3 oGPUGetEyePosition()
{
	return oGetEyePosition(oGPU_VIEW_CONSTANT_BUFFER.View);
}

#endif
#endif