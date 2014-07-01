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
#include "platonic_solids.h"

namespace ouro { namespace mesh {

#define _0 0.0f
#define _1 1.0f

const float3 sOctaVerts[] = 
{
	float3( _0,  _0,  _1),
	float3(-_1,  _0,  _0),
	float3( _0, -_1,  _0),
	float3( _1,  _0,  _0),
	float3( _0,  _1,  _0),
	float3( _0,  _0, -_1),
};

const uint sOctaIndices[] = 
{
	4,5,0, // tfl
	5,6,1, // tfr
	6,8,2, // tbr
	7,4,3, // tbl
	5,4,9, // bfl
	6,5,10, // bfr
	8,6,11, // bbl
	4,7,12, // bbr
};

static const float T = 1.6180339887f; // golden ratio (1+sqrt(5))/2

// http://www.classes.cs.uchicago.edu/archive/2003/fall/23700/docs/handout-04.pdf
const static float3 sIcosaVerts[] = 
{
	float3(  T,  _1,  _0),
	float3( -T,  _1,  _0),
	float3(  T, -_1,  _0),
	float3( -T, -_1,  _0),
	float3( _1,  _0,   T),
	float3( _1,  _0,  -T),
	float3(-_1,  _0,   T),
	float3(-_1,  _0,  -T),
	float3( _0,   T,  _1),
	float3( _0,  -T,  _1),
	float3( _0,   T, -_1),
	float3( _0,  -T, -_1),
};

const static uint sIcosaIndices[] =
{
	0,8,4, 0,5,10, 2,4,9, 2,11,5, 1,6,8, 
	1,10,7, 3,9,6, 3,7,11, 0,10,8, 1,8,10, 
	2,9,11, 3,11,9, 4,2,0, 5,0,2, 6,1,3, 
	7,3,1, 8,6,4, 9,4,6, 10,5,7, 11,7,5,
};

platonic_info get_platonic_info(const platonic::value& type)
{
	static const float3* sVerts[] = { /*sTetraVerts*/nullptr, /*sHexaVerts*/nullptr, sOctaVerts, /*sDodecaVerts*/nullptr, sIcosaVerts, };
	static_assert(oCOUNTOF(sVerts) == platonic::count, "array mismatch");
	static const uint* sIndices[] = { /*sTetraIndices*/nullptr, /*sHexaIndices*/nullptr, sOctaIndices, /*sDodecaIndices*/nullptr, sIcosaIndices, };
	static_assert(oCOUNTOF(sIndices) == platonic::count, "array mismatch");
	static const uchar sNumVerts[] = { 4, 8, 6, 20, 12 };
	static_assert(oCOUNTOF(sNumVerts) == platonic::count, "array mismatch");
	static const uchar sNumIndices[] = { /*oCOUNTOF(sTetraIndices)*/0, /*oCOUNTOF(sHexaIndices)*/0, oCOUNTOF(sOctaIndices), /*oCOUNTOF(sDodecaIndices)*/0, oCOUNTOF(sIcosaIndices), };
	static_assert(oCOUNTOF(sNumIndices) == platonic::count, "array mismatch");
	static const uchar sNumEdges[] = { 6, 12, 12, 30, 30 };
	static_assert(oCOUNTOF(sNumEdges) == platonic::count, "array mismatch");
	static const uchar sNumFaces[] = { 4, 6, 8, 12, 20 };
	static_assert(oCOUNTOF(sNumFaces) == platonic::count, "array mismatch");

	platonic_info i;
	i.positions = sVerts[type];
	i.indices = sIndices[type];
	i.num_indices = sNumIndices[type];
	i.num_vertices = sNumVerts[type];
	i.num_edges = sNumEdges[type];
	i.num_faces = sNumFaces[type];
	return i;
}

}}
