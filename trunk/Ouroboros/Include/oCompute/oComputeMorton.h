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
// This code contains code that cross-compiles in C++ and HLSL. This contains
// utilities for working with Morton Numbers in the context of an octree spatial 
// partition.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oComputeMorton_h
#define oComputeMorton_h

#include <oHLSL/oHLSLMacros.h>
#include <oCompute/oComputeConstants.h>

// We can store 2^10 values for each component of XYZ in a 32-bit Morton number.
// We assume that because the Morton number exists, it's a valid path into an
// Octree (if it was culled by the octree's root bounds, the number would not be 
// up for consideration in whatever algorithm being implemented). Thus we can 
// store one more level than 2^10. Another way to look at this is that 2^11 is
// 1024 and there can be 1024 values in 10 bits.
static const uint oMORTON_OCTREE_MAX_TARGET_DEPTH = 11;
static const int oMORTON_OCTREE_SCALAR = (1 << (oMORTON_OCTREE_MAX_TARGET_DEPTH - 1)) - 1;
static const float oMORTON_OCTREE_SCALAR_FLOAT = (float)oMORTON_OCTREE_SCALAR;

// 32-bit Morton numbers can only use 30-bits for XYZ, 10 each channel, so 
// define a mask used to get rid of any non-Morton usage of the upper two bits.
#define oMORTON_MASK 0x3fffffff

inline uint spreadbits2(uint _x)
{
	/** <citation
		usage="Implementation" 
		reason="std libs don't have bit functions" 
		author="mweerden"
		description="http://stackoverflow.com/questions/1024754/how-to-compute-a-3d-morton-number-interleave-the-bits-of-3-ints?answertab=votes#tab-top"
		license="*** Assumed Public Domain ***"
		licenseurl="http://stackoverflow.com/questions/1024754/how-to-compute-a-3d-morton-number-interleave-the-bits-of-3-ints?answertab=votes#tab-top"
		modification="Isolated bit spreading"
	/>*/
	// $(CitedCodeBegin)
	/* Takes the input value and spreads
	   the bits out such that 2 0s are
	   between each of the original bits.
	   Since this is a uint the input number
	   must be 10-bit or less
	*/
	_x = (_x | (_x << 16)) & 0x030000FF;
	_x = (_x | (_x <<  8)) & 0x0300F00F;
	_x = (_x | (_x <<  4)) & 0x030C30C3;
	_x = (_x | (_x <<  2)) & 0x09249249;
	return _x;
	// $(CitedCodeEnd)
}

// Morton numbers
// http://en.wikipedia.org/wiki/Morton_number_(number_theory)
// Morton numbers take n-dimensional integer coordinates to a single dimension 
// by interleaving the bits of each coordinate. The resulting number then 
// contains a series of n-length bit sequences that describe progressive 
// navigation of quadtrees/octrees and so on.

inline uint oMorton3D(oIN(uint3, _Position))
{
	return (spreadbits2(_Position.x) << oAXIS_X) | (spreadbits2(_Position.y) << oAXIS_Y) | (spreadbits2(_Position.z) << oAXIS_Z);
}

//    +---------+ Returns [0,7] as to what local octant the specified point is 
//   / 1  / 0  /| in. Use oAXIS_X/oAXIS_Y/oAXIS_Z to check specific dimensions 
//  /----/----/ | axis.
//  +---+---+/|0|
//  | 5 | 4 | |/| _DepthIndex=0 for the root, which is not represented in the
//  +---+---+/|2| Morton number because it always evaluates to 0 (the root) or 
//  | 7 | 6 | |/  the number is not in the octree (outside the root bounds). So
//  +---+---+/    for the 1st subdivision (8 octants as pictured) where Depth=1,
//                This would get the highest 3 bits of the 30-bit Morton number.
inline uint oMorton3DToOctree(uint _Morton3D, uint _DepthIndex)
{
	return (_Morton3D >> (30 - _DepthIndex * 3)) & 0x7;
}

// Given a Morton number of arbitrary precision (i.e. one directly calculated
// from any (normalized) float position) truncate values beyond the _DepthIndex 
// such that values at a higher precision evaluate to the same value. Using 
// oMorton3DToOctree with values larger than the one specified here will yield
// invalid information.
inline uint oMortonQuantize(uint _Morton3D, uint _DepthIndex)
{
	// Provides 1 to _DepthIndex inclusively. See above docs describing that 0
	// is assumed.
	return oMORTON_MASK - ((1 << (3*(oMORTON_OCTREE_MAX_TARGET_DEPTH-1-_DepthIndex))) - 1);
}

inline uint oMorton3DEncodeNormalizedPosition(oIN(float3, _NormalizedPosition))
{
	// oMorton3D encodes the 3 integers at 10bit precision so we take the span of 
	// [0,1] to integer span [0,1023] (10-bit). This Morton encoded number can 
	// then be used for octree traversal at several levels.
	float3 FDiscretizedPos = oMORTON_OCTREE_SCALAR_FLOAT * _NormalizedPosition;
	uint3 DiscretizedPos = uint3((uint)FDiscretizedPos.x, (uint)FDiscretizedPos.y, (uint)FDiscretizedPos.z);
	return oMorton3D(DiscretizedPos);
}

inline float3 oMorton3DDecodeNormalizedPosition(uint _Morton3D, uint _TargetDepth = oMORTON_OCTREE_MAX_TARGET_DEPTH)
{
	float3 Position = 0.0f;
	float Offset = 0.5f;
	for(uint i = 1; i < _TargetDepth; ++i)
	{  
		uint OctreeSelect = oMorton3DToOctree(_Morton3D, i);
		Position.x += (OctreeSelect & (0x1 << oAXIS_X)) ? Offset : 0.0f;
		Position.y += (OctreeSelect & (0x1 << oAXIS_Y)) ? Offset : 0.0f;
		Position.z += (OctreeSelect & (0x1 << oAXIS_Z)) ? Offset : 0.0f;
		Offset *= 0.5f;
	}

	Position += Offset;

	return Position;
}

#endif
