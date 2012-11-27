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

// Encapsulate the math for how to interact with a planar pointer (i.e. mouse) 
// to orbit in a sphere around a particular point. 

// There are two implementations presented here with significant shared code:
// only the method of rotation is overridden. 

// oArcball is what you might find described on the web. It emulates a sphere 
// that can be grabbed and rotated. Only the front of the sphere can be grabbed, 
// so if the sphere is grabbed in its center, the most rotation that can be done 
// in any direction in one drag is 90 degrees.

// oArcballInfinite is more like 3DS Max/Maya where continuous dragging will
// spin the object continuously.

#pragma once
#ifndef oArcball_h
#define oArcball_h

#include <oBasis/oAssert.h>
#include <oBasis/oMath.h>

class oArcballBase
{
	// All ScreenPoint coordinates are in top-down, left-right format: 0,0 is 
	// top-left of the viewport.

public:
	oArcballBase()
		: R(quatf::Identity) 
		, T(0.0f)
		, LookAt(0.0f)
	{}

	inline void SetRotation(const quatf& _Rotation) { R = _Rotation; }
	inline quatf GetRotation() const { return R; }

	inline void SetTranslation(const float3& _Translation) { T = _Translation; }
	inline float3 GetTranslation() const { return T; }

	inline void SetLookAt(const float3& _LookAt) { LookAt = _LookAt; }
	inline float3 GetLookAt() const { return LookAt; }

	inline void SetView(const float4x4& _View)
	{
		float4x4 invView = invert(_View);
		R = oCreateRotationQ(invView);
		T = oExtractTranslation(invView);
	}

	inline float4x4 GetView() const { return invert(float4x4(R, T)); }

	// Pans LookAt as well. _ScreenPointDelta is the amount to pan by in 
	// screen-space, so a typical calculation might be: 
	// (LastPos - PointerPos) * SpeedScalar
	inline void Pan(const float2& _ScreenPointDelta)
	{
		float3 LocalDelta = mul(R, float3(-_ScreenPointDelta.x, _ScreenPointDelta.y, 0.0f));
		T += LocalDelta;
		LookAt += LocalDelta;
	}

	// Does not zoom/magnify, but moves forward/backward towards (and past) look 
	// at. _ScreenPointDelta is the amount to dolly by, so a typical 
	// calculation might be: (LastPos - PointerPos) * SpeedScalar
	inline void Dolly(const float2& _ScreenPointDelta)
	{
		T += mul(R, float3(0.0f, 0.0f, _ScreenPointDelta.x + _ScreenPointDelta.y));
	}

protected:
	quatf R;
	float3 T;
	float3 LookAt;
};

class oArcball : public oArcballBase
{
	// The classic arcball that maps pointer clicks to vectors on a sphere and 
	// uses that angle to rotate the view.

public:
	oArcball()
		: R0(quatf::Identity)
		, T0(0.0f)
		, ArcVector0(0.0f)
		, ViewportDimensions(0.0f)
	{}

	inline void SetViewportDimensions(const int2& _ViewportDimensions) { ViewportDimensions = oCastAsFloat(_ViewportDimensions); }

	// Call this on the first frame of interaction with the arcball. For any given
	// "drag" session, all rotation is relative to the initial point passed to 
	// this method.
	inline void BeginRotation(const float2& _ScreenPoint)
	{
		oASSERT(!oEqual(ViewportDimensions, float2(0.0f)), "ViewportDimensions must be set.");
		R0 = R;
		T0 = T;
		ArcVector0 = mul(R0, ScreenToVector(_ScreenPoint, ViewportDimensions));
	}

	// Call this on the second and subsequent frames during a "drag" session. When
	// the session is over, don't call this anymore. So basically on some 
	// mouse-down event, call BeginRotation and while the mouse is down, call 
	// DragRotation on mouse-move events. When a mouse-up event is received, don't
	// call anything anymore.
	void DragRotation(const float2& _ScreenPoint)
	{
		oASSERT(!oEqual(ViewportDimensions, float2(0.0f)), "ViewportDimensions must be set.");
		float3 ArcVector = mul(R0, ScreenToVector(_ScreenPoint, ViewportDimensions));
		quatf RotationDelta = oCreateRotationQ(ArcVector0, ArcVector);
		R = mul(RotationDelta, R0);
		T = mul(RotationDelta, T0 - LookAt) + LookAt;
	}

private:
	quatf R0;
	float3 T0;
	float3 ArcVector0;
	float2 ViewportDimensions;

	static float3 ScreenToVector(const float2& _ScreenPoint, const float2& _ViewportDimensions)
	{
		float2 S;
		S.x = _ScreenPoint.x / _ViewportDimensions.x * 2.0f - 1.0f;
		S.y = 1.0f - (_ScreenPoint.y / _ViewportDimensions.y * 2.0f);

		float lengthSquared = dot(S, S);
		float z = 0.0f;
		if (lengthSquared > 1.0f)
		{
			float scale = 1.0f / sqrt(lengthSquared);
			S *= scale;
		}

		else
			z = sqrt(1.0f - lengthSquared);

		return float3(S.xy(), z);
	}
};

class oArcballInfinite : public oArcballBase
{
	// Converts the euler angles in the X and Y of screen space to a quaternion
	// and rotates by that then the collective rotation of any prior rotation.

public:

	// Call this any time while rotating. Any scaling of the screenpoint 
	// difference between this and last frame should be done before passing the 
	// result to this function. (i.e. a typical calculation is: 
	// (LastPos - PointerPos) * SpeedScalar
	inline void Rotate(const float2& _ScreenPointDelta)
	{
		if (!oEqual(_ScreenPointDelta, float2(0.0f, 0.0f)))
		{
			float3 OldT = mul(invert(R), T - LookAt);
			R = mul(R, oCreateRotationQ(float3(_ScreenPointDelta.yx(), 0.0f)));
			T = mul(R, OldT) + LookAt;
		}
	}
};

#endif
