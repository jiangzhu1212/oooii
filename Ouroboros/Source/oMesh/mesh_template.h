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
#ifndef oMesh_mesh_template
#define oMesh_mesh_template

#include <oMesh/mesh.h>
#include <oBase/algorithm.h>
#include <oBase/byte.h>
#include <oBase/color.h>
#include <oBase/threadpool.h>
#include <oBase/throw.h>
#include <oCompute/linear_algebra.h>
#include <oHLSL/oHLSLTypes.h>
#include <vector>

namespace ouro {
	namespace mesh {
		namespace detail {

template<typename T> void calc_min_max_indices(const T* oRESTRICT _pIndices, uint _StartIndex, uint _NumIndices, uint _NumVertices, uint* oRESTRICT _pMinVertex, uint* oRESTRICT _pMaxVertex)
{
	if (_pIndices)
	{
		*_pMinVertex = ouro::invalid;
		*_pMaxVertex = 0;

		const uint end = _StartIndex + _NumIndices;
		for (uint i = _StartIndex; i < end; i++)
		{
			*_pMinVertex = min(*_pMinVertex, static_cast<uint>(_pIndices[i]));
			*_pMaxVertex = max(*_pMaxVertex, static_cast<uint>(_pIndices[i]));
		}
	}

	else
	{
		*_pMinVertex = 0;
		*_pMaxVertex = _NumVertices - 1;
	}
}

template<typename T> bound<T> calc_bound(const TVEC3<T>* _pVertices, uint _VertexStride, uint _NumVertices)
{
	TVEC3<T> Min = TVEC3<T>(std::numeric_limits<T>::max());
	TVEC3<T> Max = TVEC3<T>(std::numeric_limits<T>::lowest());
	const TVEC3<T>* end = byte_add(_pVertices, _VertexStride * _NumVertices);
	while (_pVertices < end)
	{
		Min = min(Min, *_pVertices);
		Max = max(Max, *_pVertices);
		_pVertices = byte_add(_pVertices, _VertexStride);
	}

	return bound<T>(Min, Max);
}

template<typename T> void transform_points(const TMAT4<T>& _Matrix, TVEC3<T>* oRESTRICT _pDestination, uint _DestinationStride, const TVEC3<T>* oRESTRICT _pSource, uint _SourceStride, uint _NumPoints)
{
	const TVEC3<T>* oRESTRICT end = byte_add(_pDestination, _DestinationStride * _NumPoints);
	while (_pDestination < end)
	{
		*_pDestination = mul(_Matrix, TVEC4<T>(*_pSource, T(1))).xyz();
		_pDestination = byte_add(_pDestination, _DestinationStride);
		_pSource = byte_add(_pSource, _SourceStride);
	}
}

template<typename T> void transform_vectors(const TMAT4<T>& _Matrix, TVEC3<T>* oRESTRICT _pDestination, uint _DestinationStride, const TVEC3<T>* oRESTRICT _pSource, uint _SourceStride, uint _NumVectors)
{
	const TMAT3<T> m_(_Matrix[0].xyz(), _Matrix[1].xyz(), _Matrix[2].xyz());
	const TVEC3<T>* oRESTRICT end = byte_add(_pDestination, _DestinationStride * _NumVectors);
	while (_pDestination < end)
	{
		*_pDestination = mul(m_, *_pSource);
		_pDestination = byte_add(_pDestination, _DestinationStride);
		_pSource = byte_add(_pSource, _SourceStride);
	}
}

template<typename T, typename IndexT> void remove_degenerates(const TVEC3<T>* oRESTRICT _pPositions, uint _NumPositions, IndexT* oRESTRICT _pIndices, uint _NumIndices, uint* oRESTRICT _pNewNumIndices)
{
	if ((_NumIndices % 3) != 0)
		oTHROW_INVARG("_NumIndices must be a multiple of 3");

	for (uint i = 0; i < _NumIndices / 3; i++)
	{
		uint I = i * 3;
		uint J = i * 3 + 1;
		uint K = i * 3 + 2;

		if (_pIndices[I] >= _NumPositions || _pIndices[J] >= _NumPositions || _pIndices[K] >= _NumPositions)
			oTHROW_INVARG("an index value indexes outside the range of vertices specified");

		const TVEC3<T>& a = _pPositions[_pIndices[I]];
		const TVEC3<T>& b = _pPositions[_pIndices[J]];
		const TVEC3<T>& c = _pPositions[_pIndices[K]];

		if (equal(cross(a - b, a - c), TVEC3<T>(T(0), T(0), T(0))))
		{
			_pIndices[I] = invalid;
			_pIndices[J] = invalid;
			_pIndices[K] = invalid;
		}
	}

	*_pNewNumIndices = _NumIndices;
	for (uint i = 0; i < *_pNewNumIndices; i++)
	{
		if (_pIndices[i] == invalid)
		{
			memcpy(&_pIndices[i], &_pIndices[i+1], sizeof(uint) * (_NumIndices - i - 1));
			i--;
			(*_pNewNumIndices)--;
		}
	}
}

template<typename T, typename IndexT> void calc_face(size_t index, TVEC3<T>* oRESTRICT _pFaceNormals, const IndexT* oRESTRICT _pIndices, uint _NumIndices, const TVEC3<T>* oRESTRICT _pPositions, uint _NumPositions, T _CCWMultiplier, bool* oRESTRICT _pSuccess)
{
	size_t I = index * 3;
	size_t J = index * 3 + 1;
	size_t K = index * 3 + 2;

	if (_pIndices[I] >= _NumPositions || _pIndices[J] >= _NumPositions || _pIndices[K] >= _NumPositions)
	{
		*_pSuccess = false;
		return;
	}

	const TVEC3<T>& a = _pPositions[_pIndices[I]];
	const TVEC3<T>& b = _pPositions[_pIndices[J]];
	const TVEC3<T>& c = _pPositions[_pIndices[K]];

	// gracefully put in a zero vector for degenerate faces
	TVEC3<T> cr = cross(a - b, a - c);
	const TVEC3<T> Zero3(T(0), T(0), T(0));
	_pFaceNormals[index] = equal(cr, Zero3) ? Zero3 : normalize(cr) * _CCWMultiplier;
}

template<typename T, typename IndexT> void calc_face_normals(TVEC3<T>* oRESTRICT _pFaceNormals, const IndexT* oRESTRICT _pIndices, uint _NumIndices, const TVEC3<T>* oRESTRICT _pPositions, uint _NumPositions, bool _CCW)
{
	if ((_NumIndices % 3) != 0)
		oTHROW_INVARG("_NumIndices must be a multiple of 3");

	bool success = true;
	const T s = _CCW ? T(-1) : T(1);
	parallel_for( 0, _NumIndices / 3, std::bind( calc_face<T, IndexT>, std::placeholders::_1
		, _pFaceNormals, _pIndices, _NumIndices, _pPositions, _NumPositions, s, &success));
	if (!success)
		oTHROW_INVARG("an index value indexes outside the range of vertices specified");
}

template<typename InnerContainerT, typename VecT> void average_face_normals(
	const std::vector<InnerContainerT>& _Container, const std::vector<TVEC3<VecT>>& _FaceNormals
	, TVEC3<VecT>* _pNormals, uint _NumVertexNormals, bool _OverwriteAll)
{
	// Now go through the list and average the normals
	for (uint i = 0; i < _NumVertexNormals; i++)
	{
		// If there is length on the data already, leave it alone
		if (!_OverwriteAll && equal(_pNormals[i], TVEC3<VecT>(VecT(0), VecT(0), VecT(0))))
			continue;

		TVEC3<VecT> N(VecT(0), VecT(0), VecT(0));
		const InnerContainerT& TrianglesUsed = _Container[i];
		for (size_t t = 0; t < TrianglesUsed.size(); t++)
		{
			uint faceIndex = TrianglesUsed[t];
			if (!equal(dot(_FaceNormals[faceIndex], _FaceNormals[faceIndex]), VecT(0)))
				N += _FaceNormals[faceIndex];
		}

		_pNormals[i] = normalize(N);
	}
}

template<typename T, typename IndexT> void calc_vertex_normals(TVEC3<T>* oRESTRICT _pVertexNormals, const IndexT* oRESTRICT _pIndices, uint _NumIndices
	, const TVEC3<T>* oRESTRICT _pPositions, uint _NumPositions, bool _CCW = false, bool _OverwriteAll = true)
{
	std::vector<TVEC3<T>> faceNormals(_NumIndices / 3, TVEC3<T>(T(0), T(0), T(0)));

	calc_face_normals(faceNormals.data(), _pIndices, _NumIndices, _pPositions, _NumPositions, _CCW);

	const uint nFaces = as_uint(_NumIndices) / 3;
	const uint REASONABLE_MAX_FACES_PER_VERTEX = 32;
	std::vector<fixed_vector<uint, REASONABLE_MAX_FACES_PER_VERTEX>> trianglesUsedByVertexA(_NumPositions);
	std::vector<std::vector<uint>> trianglesUsedByVertex;
	bool UseVecVec = false;

	// Try with a less-memory-intensive method first. If that overflows, fall back
	// to the alloc-y one. This is to avoid a 5x slowdown when std::vector gets
	// released due to secure CRT over-zealousness in this case.
	// for each vertex, store a list of the faces to which it contributes
	for (uint i = 0; i < nFaces; i++)
	{
		fixed_vector<uint, REASONABLE_MAX_FACES_PER_VERTEX>& a = trianglesUsedByVertexA[_pIndices[i*3]];
		fixed_vector<uint, REASONABLE_MAX_FACES_PER_VERTEX>& b = trianglesUsedByVertexA[_pIndices[i*3+1]];
		fixed_vector<uint, REASONABLE_MAX_FACES_PER_VERTEX>& c = trianglesUsedByVertexA[_pIndices[i*3+2]];

		// maybe fixed_vector should throw? and catch it here instead of this?
		if (a.size() == a.capacity() || b.size() == b.capacity() || c.size() == c.capacity())
		{
			UseVecVec = true;
			break;
		}

		push_back_unique(a, i);
		push_back_unique(b, i);
		push_back_unique(c, i);
	}

	if (UseVecVec)
	{
		free_memory(trianglesUsedByVertexA);
		trianglesUsedByVertex.resize(_NumPositions);

		for (uint i = 0; i < nFaces; i++)
		{
			push_back_unique(trianglesUsedByVertex[_pIndices[i*3]], i);
			push_back_unique(trianglesUsedByVertex[_pIndices[i*3+1]], i);
			push_back_unique(trianglesUsedByVertex[_pIndices[i*3+2]], i);
		}

		average_face_normals(trianglesUsedByVertex, faceNormals, _pVertexNormals, _NumPositions, _OverwriteAll);

		uint MaxValence = 0;
		// print out why we ended up in this path...
		for (uint i = 0; i < _NumPositions; i++)
			MaxValence = __max(MaxValence, as_uint(trianglesUsedByVertex[i].size()));
		oTRACE("debug-slow path in normals caused by reasonable max valence (%u) being exceeded. Actual valence: %u", REASONABLE_MAX_FACES_PER_VERTEX, MaxValence);
	}

	else
		average_face_normals(trianglesUsedByVertexA, faceNormals, _pVertexNormals, _NumPositions, _OverwriteAll);
}

template<typename T, typename IndexT> void calc_vertex_tangents(TVEC4<T>* oRESTRICT _pTangents, const IndexT* oRESTRICT _pIndices, uint _NumIndices
 , const TVEC3<T>* oRESTRICT _pPositions, const TVEC3<T>* oRESTRICT _pNormals, const TVEC3<T>* oRESTRICT _pTexcoords, uint _NumVertices)
{
	/** <citation
		usage="Implementation" 
		reason="tangents can be derived, and this is how to do it" 
		author="Eric Lengyel"
		description="http://www.terathon.com/code/tangent.html"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.terathon.com/code/tangent.html"
		modification="Changes types to oMath types"
	/>*/

	// $(CitedCodeBegin)

	std::vector<TVEC3<T>> tan1(_NumVertices, TVEC3<T>(T(0), T(0), T(0)));
	std::vector<TVEC3<T>> tan2(_NumVertices, TVEC3<T>(T(0), T(0), T(0)));

	const uint count = _NumIndices / 3;
	for (uint i = 0; i < count; i++)
	{
		const uint a = _pIndices[3*i];
		const uint b = _pIndices[3*i+1];
		const uint c = _pIndices[3*i+2];

		const TVEC3<T>& Pa = _pPositions[a];
		const TVEC3<T>& Pb = _pPositions[b];
		const TVEC3<T>& Pc = _pPositions[c];

		const T x1 = Pb.x - Pa.x;
		const T x2 = Pc.x - Pa.x;
		const T y1 = Pb.y - Pa.y;
		const T y2 = Pc.y - Pa.y;
		const T z1 = Pb.z - Pa.z;
		const T z2 = Pc.z - Pa.z;
        
		const TVEC3<T>& TCa = _pTexcoords[a];
		const TVEC3<T>& TCb = _pTexcoords[b];
		const TVEC3<T>& TCc = _pTexcoords[c];

		const T s1 = TCb.x - TCa.x;
		const T s2 = TCc.x - TCa.x;
		const T t1 = TCb.y - TCa.y;
		const T t2 = TCc.y - TCa.y;

		T r = T(1) / (s1 * t2 - s2 * t1);
		TVEC3<T> s((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		TVEC3<T> t((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		tan1[a] += s;
		tan1[b] += s;
		tan1[c] += s;

		tan2[a] += t;
		tan2[b] += t;
		tan2[c] += t;
	}

	parallel_for(0, _NumVertices, [&](size_t _Index)
	{
		// Gram-Schmidt orthogonalize + handedness
		const TVEC3<T>& n = _pNormals[_Index];
		const TVEC3<T>& t = tan1[_Index];
		_pTangents[_Index] = TVEC4<T>(normalize(t - n * dot(n, t)), (dot(cross(n, t), tan2[_Index]) < T(0)) ? T(-1) : T(1));
	});

	// $(CitedCodeEnd)
}

template<typename T, typename IndexT, typename UV0T>
void calc_texcoords(const bound<T>& _Bound, const IndexT* _pIndices, uint _NumIndices, const TVEC3<T>* _pPositions, UV0T* _pOutTexcoords, uint _NumVertices, double* _pSolveTime)
{
	oTHROW(operation_not_supported, "calc_texcoords not implemented");
}

template<typename IndexT> void prune_indices(const std::vector<bool>& _Refed, IndexT* _pIndices, uint _NumIndices)
{
	std::vector<IndexT> sub(_Refed.size(), 0);
	for (size_t i = 0; i < _Refed.size(); i++)
		if (!_Refed[i])
			for (size_t j = i; j < sub.size(); j++)
				(sub[j])++;

	for (uint i = 0; i < _NumIndices; i++)
		_pIndices[i] -= sub[_pIndices[i]];
}

template<typename T> uint prune_stream(const std::vector<bool>& _Refed, T* _pStream, uint _NumberOfElements)
{
	if (!_pStream)
		return 0;

	std::vector<bool>::const_iterator itRefed = _Refed.begin();
	T* r = _pStream, *w = _pStream;
	while (itRefed != _Refed.end())
	{
		if (*itRefed++)
			*w++ = *r++;
		else
			++r;
	}

	return static_cast<uint>(_NumberOfElements - (r - w));
}

template<typename T, typename IndexT, typename UV0VecT, typename UV1VecT> 
void prune_unindexed_vertices(const IndexT* oRESTRICT _pIndices, uint _NumIndices
	, TVEC3<T>* oRESTRICT _pPositions, TVEC3<T>* oRESTRICT _pNormals, TVEC4<T>* oRESTRICT _pTangents, UV0VecT* oRESTRICT _pTexcoords0, UV1VecT* oRESTRICT _pTexcoords1, color* oRESTRICT _pColors
	, uint _NumVertices, uint* oRESTRICT _pNewNumVertices)
{
	std::vector<bool> refed;
	refed.assign(_NumVertices, false);
	for (size_t i = 0; i < _NumIndices; i++)
		refed[_pIndices[i]] = true;
	uint newNumVertices = 0;

	static const uint kNumVertexWhereParallelismHelps = 2000;

	if (_NumVertices < kNumVertexWhereParallelismHelps)
	{
		newNumVertices = prune_stream(refed, _pPositions, _NumVertices);
		prune_stream(refed, _pNormals, _NumVertices);
		prune_stream(refed, _pTangents, _NumVertices);
		prune_stream(refed, _pTexcoords0, _NumVertices);
		prune_stream(refed, _pTexcoords1, _NumVertices);
		prune_stream(refed, _pColors, _NumVertices);
	}

	else
	{
		std::shared_ptr<task_group> g = make_task_group();
		g->run([&] { newNumVertices = prune_stream(refed, _pPositions, _NumVertices); });
		g->run([&] { prune_stream(refed, _pNormals, _NumVertices); });
		g->run([&] { prune_stream(refed, _pTangents, _NumVertices); });
		g->run([&] { prune_stream(refed, _pTexcoords0, _NumVertices); });
		g->run([&] { prune_stream(refed, _pTexcoords1, _NumVertices); });
		g->run([&] { prune_stream(refed, _pColors, _NumVertices); });
		g->wait();
	}

	*_pNewNumVertices = newNumVertices;
}

		} // namespace detail
	} // namespace mesh
} // namespace ouro

#endif
