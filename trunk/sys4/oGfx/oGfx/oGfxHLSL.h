// $(header)
// NOTE: This header is compiled by both HLSL and C++
#pragma once
#ifndef oGfxHLSL_h
#define oGfxHLSL_h

#include <oooii/oHLSL.h>

struct oGfxViewConstants
{
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

		oGfxViewConstants() {}
		oGfxViewConstants(CONSTRUCTION _Type, float2 _RenderTargetDimensions, uint _TextureArrayIndex = 0) { Set(float4x4(float4x4::Identity), float4x4(float4x4::Identity), _RenderTargetDimensions, _TextureArrayIndex); }
		oGfxViewConstants(const float4x4& _View, const float4x4& _Projection, const float2& _RenderTargetDimensions, uint _TextureArrayIndex) { Set(_View, _Projection, _RenderTargetDimensions, _TextureArrayIndex); }
		inline void Set(const float4x4& _View, const float4x4& _Projection, const float2& _RenderTargetDimensions, uint _TextureArrayIndex = 0)
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
			NearPlaneDistancePlusEpsilon += 0.0001f;
			InverseFarPlaneDistance = 1.0f / f;

			TextureArrayIndex = _TextureArrayIndex;
			Pad0 = Pad1 = Pad2 = 0;
		}
	#endif
};

struct oGfxDrawConstants
{
	// Constant buffer to represent object-dependent parameters used
	// in most straightforward rendering.

	float4x4 World;
	float4x4 WorldView;
	float4x4 WorldViewProjection;
	
	// Represents the rotation in the WordView matrix as a quaternion
	quatf WorldViewQuaternion;
	
	// ObjectID is user-specified (MeshID, LinesID, QuadID) and 
	// identifies a group of meshes that share an attribute in common
	uint ObjectID;
	
	// DrawID is a monotonically increasing value that is unique for
	// each call to Draw().
	uint DrawID;
	uint pad0;
	uint pad1;

	#ifndef oHLSL

		enum CONSTRUCTION { Identity, };

		oGfxDrawConstants() {}
		oGfxDrawConstants(CONSTRUCTION _Type) { SetIdentity(); }
		oGfxDrawConstants(const float4x4& _World, const float4x4& _View, const float4x4& _Projection, uint _ObjectID = 0, uint _DrawID = 0) { Set(_World, _View, _Projection, _ObjectID, _DrawID); }

		inline void Set(const float4x4& _World, const float4x4& _View, const float4x4& _Projection, uint _ObjectID, uint _DrawID)
		{
			World = _World;
			WorldView = _World * _View;
			WorldViewProjection = WorldView * _Projection;
			WorldViewQuaternion = oCreateRotationQ(WorldView);
			ObjectID = _ObjectID;
			DrawID = _DrawID;
		}

		inline void SetIdentity()
		{
			World = float4x4::Identity;
			WorldView = float4x4::Identity;
			WorldViewProjection = float4x4::Identity;
			WorldViewQuaternion = quatf::Identity;
			ObjectID = 0;
			DrawID = 0;
		}
	#endif
};

#ifdef oHLSL
	cbuffer cbuffer_view: register(b0) { oGfxViewConstants oGfxView; }
	cbuffer cbuffer_draw : register(b1) { oGfxDrawConstants oGfxDraw; }

	// Returns the view-space depth of an object-space position
	float oGfxCalculateViewSpaceDepth(float3 _ObjectSpacePosition)
	{
		return oMul(oGfxDraw.WorldView, float4(_ObjectSpacePosition, 1)).z;
	}

	#// Returns the normalized depth for the current projection by dividing by the 
	// distance to the far plane. This is the value that should be written to a 
	// depth buffer.
	float oGfxEncodeDepth(float _ViewSpaceDepth)
	{
		// _EncodedDepthFromVS is the view space z value
		// Write out view-space depth, but normalize it by the distance of the far 
		// plane to keep precision high.
		return _ViewSpaceDepth * oGfxView.InverseFarPlaneDistance;
	}

	// When rendering a transformed quad, calculation of the far plane corners is a 
	// bit more complex, so use this instead. Like the FSQ version, the results of 
	// this should be set as an interpolant to the pixel shader.
	float3 oGfxCalculateFarPlanePosition(float4 _ScreenSpaceQuadCornerPosition)
	{
		float2 Texcoord = oCalculateScreenSpaceTexcoordVS(_ScreenSpaceQuadCornerPosition);
		float4 InterpolatedFarPlanePositionTop = lerp(oGfxView.ViewSpaceFarplaneCorners[TOP_LEFT], oGfxView.ViewSpaceFarplaneCorners[TOP_RIGHT], Texcoord.x);
		float4 InterpolatedFarPlanePositionBottom = lerp(oGfxView.ViewSpaceFarplaneCorners[BOTTOM_LEFT], oGfxView.ViewSpaceFarplaneCorners[BOTTOM_RIGHT], Texcoord.x);
		float4 InterpolatedFarPlanePosition = lerp(InterpolatedFarPlanePositionTop, InterpolatedFarPlanePositionBottom, Texcoord.y);
		return InterpolatedFarPlanePosition.xyz;
	}

	// _____________________________________________________________________________
	// Screen-space rendering (deferred/inferred) utility functions

	float4 oGfxCalculateScreenSpacePosition(float3 _ObjectSpacePosition)
	{
		return oMul(oGfxDraw.WorldViewProjection, float4(_ObjectSpacePosition, 1));
	}

	// Encode a view space normal normal as a 2-float value in a G-Buffer
	// Because there's a singularity when pointing directly away from the viewer,
	// and it doesn't make much sense to specify back face rendering unless you
	// meant it to be lit as a front face, reverse normals on back-faces
	float2 oGfxEncodeNormal(float4 _ViewSpaceNormalRotation, bool _IsFrontFace)
	{
		float3 up = _IsFrontFace ? oVECTOR_UP : -oVECTOR_UP;
		return oFullToHalfSphere(oQRotate(normalize(_ViewSpaceNormalRotation), up));
	}

	// Restore a normal stored in a G-Buffer using oGfxEncodeNormal
	float3 oGfxDecodeNormal(float2 _GBufferSampledNormalXY)
	{
		return oHalfToFullSphere(_GBufferSampledNormalXY);
	}

	// Scale/bias a normal into an RGB color invert Z so that more extreme-in-Z 
	// normals show as brighter rather than darker (because normals towards the 
	// screen are -1)
	float3 oGfxColorizeNormal(float3 _ViewSpaceNormal)
	{
		return _ViewSpaceNormal * float3(0.5, 0.5, -0.5) + 0.5;
	}

	// Store a normalized specular exponent on [0,1] in an 8-bit value
	uint oGfxEncodeSpecularExponent(float _Shininess)
	{
		// Assumes _SpecularExponent is normalized/saturated, so just convert to an
		// 8-bit unorm
		return _Shininess * 255.0;
	}

	static const float oMAX_SPECULAR_EXPONENT = 2000;

	// Restore a normalized shininess value stored with oGfxEncodeSpecularExponent()
	// into a specular exponention
	float oGfxDecodeSpecularExponent(uint _8BitSpecularCode)
	{
		// decode from 8-bit unorm and use that to cover range [0,MAX]
		return (_8BitSpecularCode * oMAX_SPECULAR_EXPONENT / 255.0);
	}

#endif
#endif
