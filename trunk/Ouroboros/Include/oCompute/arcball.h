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
// Encapsulate the math for how to interact with a planar pointer (i.e. mouse) 
// to orbit in a sphere around a particular point. Arcball can be constrained 
// to two axes or left unconstrained. Unconstrained is the "classic" arcball 
// like what you would find on the web. It approximates a sphere that can be 
// grabbed in its center and rotated. The constrained versions are found in 
// tools like 3DSMax and Maya.
#pragma once
#ifndef oCompute_arcball_h
#define oCompute_arcball_h

#include <oCompute/linear_algebra.h>
#include <oCompute/oComputeConstants.h>
#include <oCompute/oQuaternion.h>
#include <oBase/macros.h>

namespace ouro {

class arcball
{
public:
	enum constraint_t
	{
		none, // classic arcball behavior
		y_up, // Maya-like, prevents non-explicit roll
		z_up, // Max-like, prevents non-explicit roll
	};

	arcball(constraint_t _Constraint = none)
		: Constraint(_Constraint)
		, R(oIDENTITYQ)
		, T(oZERO3)
		, LookAt(oZERO3)
		, UpsideDownScalar(1.0f)
	{}

	inline void constraint(constraint_t _Constraint) { Constraint = _Constraint; }
	inline constraint_t constraint() const { return Constraint; }

	inline void rotation(const quatf& _Rotation) { R = _Rotation; }
	inline quatf rotation() const { return R; }

	inline void translation(const float3& _Translation) { T = _Translation; }
	inline float3 translation() const { return T; }

	inline void lookat(const float3& _LookAt) { LookAt = _LookAt; }
	inline float3 lookat() const { return LookAt; }

	// Pans LookAt as well. _ScreenPointDelta is the amount to pan by in 
	// screen-space, so a typical calculation might be: 
	// (LastPos - PointerPos) * SpeedScalar
	inline void pan(const float2& _ScreenPointDelta)
	{
		float3 LocalDelta = qmul(R, float3(-_ScreenPointDelta.x, _ScreenPointDelta.y, 0.0f));
		T += LocalDelta;
		LookAt += LocalDelta;
	}

	// Does not zoom/magnify, but moves forward/backward towards (and past) look 
	// at. _ScreenPointDelta is the amount to dolly by, so a typical 
	// calculation might be: (LastPos - PointerPos) * SpeedScalar
	inline void dolly(const float2& _ScreenPointDelta)
	{
		T += qmul(R, float3(0.0f, 0.0f, _ScreenPointDelta.x + _ScreenPointDelta.y));
	}

	inline void view(const float4x4& _View)
	{
		float4x4 invView = invert(_View);
		R = make_quaternion(invView);
		T = extract_translation(invView);
	}

	inline float4x4 view() const { return invert(make_matrix(R, T)); }

	// Call this on the start of rotation, then call Rotate() on the same frame
	// and consequent frames of rotation. This addresses Maya-style rules for 
	// constrained rotation where if you flip a model upside down, then horizontal
	// controls are reversed until you release the mouse and repress, then even
	// upside down models rotation as expected, matching mouse motion. This is not 
	// needed when all 3 axes are respected in the non-constrained arcball.
	inline void begin_rotation()
	{
		// This is invalid for X-up, which is currently not supported... i.e. the
		// upside-down scalar gets ignored.

		// @tony: I can't quite wrap my head around where the breakdown is occurring, 
		// but when using Z-up, the dot-product is rotated 90 degrees. This might be 
		// because we're not fully any-up compatible elsewhere and if I switch to 
		// Z-up something gets missed. Come back to this if we really need z-up, 
		// otherwise move forward with y-up like most of the rest of the world.

		float4x4 rotation = make_rotation(R);

		// If left-handled reverse direction.
		// If the model is upside-down, reverse rotation directions.
		UpsideDownScalar = (dot(rotation[Constraint].xyz(), oIDENTITY3x3[Constraint]) > 0.0f ? 1.0f : -1.0f);
	}

	// Call this any time while rotating. Any scaling of the screenpoint 
	// difference between this and last frame should be done before passing the 
	// result to this function. (i.e. a typical calculation is: 
	// (LastPos - PointerPos) * SpeedScalar
	inline void rotate(const float2& _ScreenPointDelta)
	{
		if (!equal(_ScreenPointDelta, oZERO2))
		{
			float3 OldT = qmul(::invert(R), T - LookAt);
			// Order matters on these multiplies.
			switch (Constraint)
			{
				case y_up:
					R = qmul(R, make_quaternion(_ScreenPointDelta.y, oXAXIS3));
					R = qmul(make_quaternion(UpsideDownScalar * _ScreenPointDelta.x, oYAXIS3), R);
					break;
				case z_up:
					R = qmul(R, make_quaternion(_ScreenPointDelta.y, oXAXIS3));
					R = qmul(make_quaternion(UpsideDownScalar * _ScreenPointDelta.x, oZAXIS3), R);
					break;
				case none:
					R = qmul(R, make_quaternion(float3(_ScreenPointDelta.yx(), 0.0f)));
					break;
				oNODEFAULT;
			}

			T = qmul(normalize(R), OldT) + LookAt;
		}
	}
private:
	constraint_t Constraint;
	quatf R;
	float3 T;
	float3 LookAt;
	float UpsideDownScalar;
};

} // namespace ouro

#endif
