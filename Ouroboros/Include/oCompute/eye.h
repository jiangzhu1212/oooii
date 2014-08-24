/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// Encapsulation of math necessary to rotate around a point while preserving its 
// position as well as another relative position labeled a lookat. The lookat is 
// preserved mainly for interop with other control math such as arcball; it's 
// not primarily utilized by this class, but it is preserved.
#pragma once
#ifndef oCompute_eye_h
#define oCompute_eye_h

#include <oCompute/oComputeConstants.h>
#include <oCompute/linear_algebra.h>

namespace ouro {

class eye
{
	// Often it is useful to determine a basic rotation/translation in unit space, 
	// but have the transform actually occur in a rotated space, so here's a 
	// little wrapper for such logic.
public:
	eye(const float4x4& _View = oIDENTITY4x4) { view(_View); }

	// Set/Get the value used for rotation of the local space
	inline void rotation(const quatf& _Rotation) { R = _Rotation; }
	inline const quatf& rotation() const { return R; }

	// Set/Get the value used for translation of the local space
	inline void translation(const float3& _Translation) { P = _Translation; }
	inline const float3& translation() const { return P; }

	// Set/Get the lookat point
	inline void set_lookat(const float3& _LookAt) { L = _LookAt; }
	inline float3 lookat() const { return L; }

	inline void lookat(const float3& _LookAt)
	{
		set_lookat(_LookAt);
		view(make_lookat_lh(P, L, extract_axis_y(make_matrix(R, P))));
	}

	// Set/Get the view this eye represents
	inline void view(const float4x4& _View)
	{
		float4x4 invView = invert(_View);
		Euler = extract_rotation(invView);
		R = make_quaternion(Euler);
		P = extract_translation(invView);
	}

	inline float4x4 view() const
	{
		return invert(make_matrix(R, P));
	}

	// Rotates around the position of the current transform
	inline void rotate(const float3& _LocalSpaceEulerXYZRotation)
	{
		// accumulate yaw/pitch/roll separately and always define an absolute
		// rotation. This way the up vector is maintained rather than slowly 
		// deteriorating with an incremental update because each change changes
		// the fundamental basis and thus the up vector.
		Euler += _LocalSpaceEulerXYZRotation;
		R = make_quaternion(Euler);
		L = qmul(make_quaternion(_LocalSpaceEulerXYZRotation), L);
	}

	// Movement along canonical X Y Z axes that will be transformed into the 
	// current orientation. So +Z is forward, -Z is backward, +Y is up, -Y is 
	// down, +X is right and -X is left, all relative to the facing of the 
	// orientation of the current transform.
	inline void translate(const float3& _LocalSpaceTranslation)
	{
		float3 tx = qmul(R, _LocalSpaceTranslation);
		P += tx;
		L += tx;
	}

private:
	float3 Euler;
	quatf R;
	float3 P;
	float3 L; // center of interest/focal point/lookat
};

} // namespace ouro

#endif
