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
#pragma once
#ifndef oManipulator_h
#define oManipulator_h

#include <oBasis/oInterface.h>
#include <oBasis/oMathTypes.h>

// Manipulators are geometries that are used to visual manipulate
// the position, scale and rotation of other geometries.  This
// idea is encapsulated here by abstractly manipulating a transform
// that represents the other geometry.  The end result is a series 
// of lines that can be drawn to control the maniuplation.  For 
// a higher level visual manipulator look at oGPUManipulators use
// of oManipulator.
interface oManipulator : oInterface
{
	static const unsigned int ROTATION_CIRCLE_VCOUNT = 360;
	static const unsigned int ROTATION_PICK_TORUS_DIVIDE = 120;
	static const unsigned int ROTATION_PICK_TORUS_FACET = 8;
	static const unsigned int ROTATION_PICK_ARCBALL_VCOUNT = 64;

	enum AXIS
	{
		X,
		Y,
		Z,
		SCREEN,
		FREE, //only for rotation, arcball
		NUM_AXES,
	};
	
	struct DESC
	{
		enum MODE
		{
			OBJECT,
			WORLD,
			NUM_MODES,
		};

		enum TYPE
		{
			TRANSLATION,
			SCALE,
			ROTATION,
			NUM_TYPES,
		};

		DESC()
			: Type(TRANSLATION)
			, Mode(WORLD)
			, PickWidth(0.02f)
			, ManipularScale(1.0f)
		{}
		TYPE Type;
		MODE Mode;
		float PickWidth;
		float ManipularScale;
	};

	virtual void GetDesc(DESC* _pDesc) const = 0;

	virtual void GetTransform(float4x4 *_pTransform) const = 0;

	// This should be called whenever the axis is changed
	virtual void BeginManipulation(AXIS _Axis, const float2& _ScreenPosition) = 0;
	virtual void EndManipulation() = 0;

	// Updates the transform based on new matrices and scren position
	virtual void Update(const float2& _ScreenPosition, const float4x4 &_World, const float4x4 &_View, const float4x4 &_Proj) = 0;

	// Maximum sizes of geometry computed for allocation purposes
	virtual void GetMaxSizes(size_t &_maxNumLines, size_t &_maxNumPickGeometry) const = 0;

	// Returns the visual lines that visualize the axis
	virtual void GetLines(AXIS _Axis, float3 *_lines, size_t &_numWritten) const = 0;

	// Returns a geometry that represents the lines but is larger for picking
	virtual void GetLinesPickGeometry(AXIS _Axis, float3 *_vertices, size_t &_numWritten) const = 0;

	// Returns the transform of the cap which usually contains a translation and scale
	virtual void GetCapTransform(AXIS _Axis, float4x4 &_transforms) const = 0;
	virtual bool IsClipped(AXIS _Axis) const = 0;
};

oAPI bool oManipulatorCreate(const oManipulator::DESC& _Desc, oManipulator** _ppManipulator);

#endif // oManipulator_h
