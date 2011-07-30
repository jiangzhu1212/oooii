// $(header)
// NOTE: This header is compiled both by HLSL and C++
// This file presents a robust representation of a 
// view/camera/eye for deferred or inferred rendering
#ifndef oHLSL
	#pragma once
#endif
#ifndef oHLSLDeferredView_h
#define oHLSLDeferredView_h

#include <oooii/oHLSL.h>

#ifndef oHLSL_DEFERRED_VIEW_CONSTANT_BUFFER_INDEX
	#define oHLSL_DEFERRED_VIEW_CONSTANT_BUFFER_INDEX b0
#endif

struct oDeferredViewConstants
{
	#ifdef oHLSL

		#define BOTTOM_LEFT 0
		#define BOTTOM_RIGHT 1
		#define TOP_LEFT 2
		#define TOP_RIGHT 3

	#else

		enum FARPLANE_CORNERS
		{
			BOTTOM_LEFT,
			BOTTOM_RIGHT,
			TOP_LEFT,
			TOP_RIGHT,
		};

		enum CONSTRUCTION { Identity, };

		oDeferredViewConstants() {}
		oDeferredViewConstants(CONSTRUCTION _Type, float2 _RenderTargetDimensions, uint _TextureArrayIndex) { Set(float4x4(float4x4::Identity), float4x4(float4x4::Identity), _RenderTargetDimensions, _TextureArrayIndex); }
		oDeferredViewConstants(const float4x4& _View, const float4x4& _Projection, const float2& _RenderTargetDimensions, uint _TextureArrayIndex) { Set(_View, _Projection, _RenderTargetDimensions, _TextureArrayIndex); }
		inline void Set(const float4x4& _View, const float4x4& _Projection, const float2& _RenderTargetDimensions, uint _TextureArrayIndex)
		{
			View = _View;
			InverseView = invert(View);
			Projection = _Projection;
			ViewProjection = View * Projection;

			float3 corners[8];
			oFrustumf psf(_Projection);
			psf.ExtractCorners(corners);
			ViewSpaceFarplaneCorners[TOP_LEFT] = float4(corners[oFrustumf::LEFT_TOP_FAR], 0.0f);
			ViewSpaceFarplaneCorners[TOP_RIGHT] = float4(corners[oFrustumf::RIGHT_TOP_FAR], 0.0f);
			ViewSpaceFarplaneCorners[BOTTOM_LEFT] = float4(corners[oFrustumf::LEFT_BOTTOM_FAR], 0.0f);
			ViewSpaceFarplaneCorners[BOTTOM_RIGHT] = float4(corners[oFrustumf::RIGHT_BOTTOM_FAR], 0.0f);

			RenderTargetDimensions = _RenderTargetDimensions;

			// Store the 1/far plane distance so we can calculate view-space depth,
			// then store it on [0,1] for maximum precision.
			float f = 0.0f;
			oCalculateNearFarPlanesDistance(_View, _Projection, &NearPlaneDistancePlusEpsilon, &f);
			NearPlaneDistancePlusEpsilon += 0.01f;
			InverseFarPlaneDistance = 1.0f / f;

			TextureArrayIndex = _TextureArrayIndex;
			Pad0 = Pad1 = Pad2 = 0;
		}

protected:
	#endif

	// Constant buffer to represent view-dependent parameters used in most 
	// straightforward rendering.

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

#ifdef oHLSL

cbuffer oCBufferHLSLDeferredViewConstants : register(oHLSL_DEFERRED_VIEW_CONSTANT_BUFFER_INDEX) { oDeferredViewConstants DeferredViewConstants; }

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
float3 oCalculateViewSpaceFarPlanePositionFSQ(float2 _Texcoord)
{
	return DeferredViewConstants.ViewSpaceFarplaneCorners[_Texcoord.x + _Texcoord.y * 2].xyz;
}

// This should be called from the vertex shader.
// For non-full screen quads the interpolated value must be calculated explicitly.
// NOTE: Using a WVP to get a screen space position in the vertex shader will yield
// different values that using an SV_Position, so be careful to use this only in a 
// vertex shader.
float3 oCalculateFarPlanePosition(float4 _ScreenSpaceQuadCornerPosition)
{
	float2 Texcoord = oCalculateScreenSpaceTexcoordVS(_ScreenSpaceQuadCornerPosition);
	float4 InterpolatedTop = lerp(DeferredViewConstants.ViewSpaceFarplaneCorners[TOP_LEFT], DeferredViewConstants.ViewSpaceFarplaneCorners[TOP_RIGHT], Texcoord.x);
	float4 InterpolatedBottom = lerp(DeferredViewConstants.ViewSpaceFarplaneCorners[BOTTOM_LEFT], DeferredViewConstants.ViewSpaceFarplaneCorners[BOTTOM_RIGHT], Texcoord.x);
	float4 InterpolatedFinal = lerp(InterpolatedTop, InterpolatedBottom, Texcoord.y);
	return InterpolatedFinal.xyz;
}

// NOTE: This must be called from a pixel shader
float2 oCalculateScreenSpaceTexcoordPS(float4 _SVPosition)
{
	return oCalculateScreenSpaceTexcoordPS(_SVPosition, DeferredViewConstants.RenderTargetDimensions);
}

// Returns the view pace depth of an object-space position
float oCalculateViewSpaceDepth(float4x4 _WorldView, float3 _ObjectSpacePosition)
{
	return oMul(_WorldView, float4(_ObjectSpacePosition, 1)).z;
}

// Keep depth values between 0 and 1 to maximize precision
float oEncodeNormalizedViewSpaceDepth(float _ViewSpaceDepth)
{
	return _ViewSpaceDepth * DeferredViewConstants.InverseFarPlaneDistance;
}

// Use the encoded depth and the far plane's position to recreate a 3D position
float3 oReconstructViewSpacePosition(float3 _ViewSpaceFarPlanePosition, float _EncodedViewSpaceDepth)
{
	return _EncodedViewSpaceDepth * _ViewSpaceFarPlanePosition;
}

// Calculate the vector pointing from the eye to the specified
// point on a surface
float3 oCalculateEyeVectorFromViewSpace(float3 _ViewSpaceSurfacePosition)
{
	// In view space the eye is at the origin so we only 
	// need to normalize position and negate
	return -normalize(_ViewSpaceSurfacePosition);
}

// For screen-space quad representations of volumes (like light volumes)
// ensure that if a volume still influences the sceen from behind the eye
// that we can position the quad geometry in a renderable position.
float3 oClampViewSpacePositionToNearPlane(float3 _ViewSpacePosition)
{
	return float3(_ViewSpacePosition.xy, max(_ViewSpacePosition.z, DeferredViewConstants.NearPlaneDistancePlusEpsilon));
}

#endif
#endif
