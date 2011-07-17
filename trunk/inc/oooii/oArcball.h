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

// Define very basic orbiting camera controls for focus on a particular point.
// This is useful for very early 3D renderer bringup to be able to move around
// a model.
#pragma once
#ifndef oArcball_h
#define oArcball_h

#include <oooii/oInterface.h>
#include <oooii/oMath.h>

interface oArcball : oInterface
{
	enum EVENT
	{
		BEGIN_ROTATING,
		ROTATING,
		BEGIN_TRANSLATING,
		TRANSLATING,
	};

	struct DESC
	{
		DESC()
			: TranslationScale(1.0f, 1.0f, 0.001f)
			, Radius(1.0f)
		{}
		float2 WindowDimensions;

		// XY is screen space, Z is mouse wheel zoom scale. A typical 
		// value would be float3(1.0f, 1.0f, 0.001f)... Z is very 
		// sensitive.
		float3 TranslationScale;
		float Radius;
	};

	static bool Create(const DESC& _Desc, oArcball** _ppArcball);

	virtual void SetDesc(const DESC* _pDesc) = 0;
	virtual void GetDesc(DESC* _pDesc) const = 0;

	// Resets to a camera that uses the specific config
	virtual void Reset(float3 _Eye, float3 _LookAt, float3 _Up) = 0;

	// Resets to an identity rotation and no translation
	virtual void Reset() = 0;

	// Converts a view matrix to a valid Arcball state
	virtual void ToView(float4x4* _pView) const = 0;

	// screenPosition is generally XY, and Z is the wheel value of 
	// the mouse. This method should be hooked up to the application's 
	// input system to drive the state of the arcball.
	virtual void Trigger(EVENT _Event, const float3& _ScreenPosition) = 0;
};

#endif
