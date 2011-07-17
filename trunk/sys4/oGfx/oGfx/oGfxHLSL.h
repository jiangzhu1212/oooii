// $(header)
// NOTE: This header is compiled by both HLSL and C++
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGfxHLSL_h
#define oGfxHLSL_h

#include <oooii/oHLSL.h>
#include <oooii/oHLSLDeferredView.h>

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
	cbuffer cbuffer_draw : register(b1) { oGfxDrawConstants oGfxDraw; }

	// Returns the view-space depth of an object-space position
	float oGfxCalculateViewSpaceDepth(float3 _ObjectSpacePosition)
	{
		return oCalculateViewSpaceDepth(oGfxDraw.WorldView, _ObjectSpacePosition);
	}

	// _____________________________________________________________________________
	// Screen-space rendering (deferred/inferred) utility functions

	float4 oGfxCalculateScreenSpacePosition(float3 _ObjectSpacePosition)
	{
		return oMul(oGfxDraw.WorldViewProjection, float4(_ObjectSpacePosition, 1));
	}

#endif
#endif
