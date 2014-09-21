// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oMesh_mesh_template
#define oMesh_mesh_template

#include <oMesh/mesh.h>
#include <oBase/algorithm.h>
#include <oBase/color.h>
#include <oBase/throw.h>
#include <oConcurrency/concurrency.h>
#include <oHLSL/oHLSLMath.h>
#include <oHLSL/oHLSLTypes.h>
#include <oMemory/byte.h>
#include <vector>

namespace ouro {
	namespace mesh {
		namespace detail {

template<typename T> void calc_min_max_indices(const T* oRESTRICT indices, uint start_index, uint num_indices, uint num_vertices, uint* oRESTRICT out_min_vertex, uint* oRESTRICT out_max_vertex)
{
	if (indices)
	{
		*out_min_vertex = ouro::invalid;
		*out_max_vertex = 0;

		const uint end = start_index + num_indices;
		for (uint i = start_index; i < end; i++)
		{
			*out_min_vertex = min(*out_min_vertex, static_cast<uint>(indices[i]));
			*out_max_vertex = max(*out_max_vertex, static_cast<uint>(indices[i]));
		}
	}

	else
	{
		*out_min_vertex = 0;
		*out_max_vertex = num_vertices - 1;
	}
}

template<typename T> aabox<T, TVEC3<T>> calc_bound(const TVEC3<T>* vertices, uint vertex_stride, uint num_vertices)
{
	aabox<T, TVEC3<T>> b;
	const TVEC3<T>* end = byte_add(vertices, vertex_stride * num_vertices);
	while (vertices < end)
	{
		extend_by(b, *vertices);
		vertices = byte_add(vertices, vertex_stride);
	}

	return b;
}

template<typename T> void transform_points(const TMAT4<T>& matrix, TVEC3<T>* oRESTRICT destination, uint destination_stride, const TVEC3<T>* oRESTRICT source, uint source_stride, uint num_points)
{
	const TVEC3<T>* oRESTRICT end = byte_add(destination, destination_stride * num_points);
	while (destination < end)
	{
		*destination = mul(matrix, TVEC4<T>(*source, T(1))).xyz();
		destination = byte_add(destination, destination_stride);
		source = byte_add(source, source_stride);
	}
}

template<typename T> void transform_vectors(const TMAT4<T>& matrix, TVEC3<T>* oRESTRICT destination, uint destination_stride, const TVEC3<T>* oRESTRICT source, uint source_stride, uint num_vectors)
{
	const TMAT3<T> m_(matrix[0].xyz(), matrix[1].xyz(), matrix[2].xyz());
	const TVEC3<T>* oRESTRICT end = byte_add(destination, destination_stride * num_vectors);
	while (destination < end)
	{
		*destination = mul(m_, *source);
		destination = byte_add(destination, destination_stride);
		source = byte_add(source, source_stride);
	}
}

template<typename T, typename IndexT> void remove_degenerates(const TVEC3<T>* oRESTRICT positions, uint num_positions, IndexT* oRESTRICT indices, uint num_indices, uint* oRESTRICT out_new_num_indices)
{
	if ((num_indices % 3) != 0)
		oTHROW_INVARG("num_indices must be a multiple of 3");

	for (uint i = 0; i < num_indices / 3; i++)
	{
		uint I = i * 3;
		uint J = i * 3 + 1;
		uint K = i * 3 + 2;

		if (indices[I] >= num_positions || indices[J] >= num_positions || indices[K] >= num_positions)
			oTHROW_INVARG("an index value indexes outside the range of vertices specified");

		const TVEC3<T>& a = positions[indices[I]];
		const TVEC3<T>& b = positions[indices[J]];
		const TVEC3<T>& c = positions[indices[K]];

		if (equal(cross(a - b, a - c), TVEC3<T>(T(0), T(0), T(0))))
		{
			indices[I] = invalid;
			indices[J] = invalid;
			indices[K] = invalid;
		}
	}

	*out_new_num_indices = num_indices;
	for (uint i = 0; i < *out_new_num_indices; i++)
	{
		if (indices[i] == invalid)
		{
			memcpy(&indices[i], &indices[i+1], sizeof(uint) * (num_indices - i - 1));
			i--;
			(*out_new_num_indices)--;
		}
	}
}

template<typename T, typename IndexT> void calc_face_normals_task(size_t index, TVEC3<T>* oRESTRICT face_normals, const IndexT* oRESTRICT indices, uint num_indices, const TVEC3<T>* oRESTRICT positions, uint num_positions, T ccwMultiplier, bool* oRESTRICT _pSuccess)
{
	size_t I = index * 3;
	size_t J = index * 3 + 1;
	size_t K = index * 3 + 2;

	if (indices[I] >= num_positions || indices[J] >= num_positions || indices[K] >= num_positions)
	{
		*_pSuccess = false;
		return;
	}

	const TVEC3<T>& a = positions[indices[I]];
	const TVEC3<T>& b = positions[indices[J]];
	const TVEC3<T>& c = positions[indices[K]];

	// gracefully put in a zero vector for degenerate faces
	TVEC3<T> cr = cross(a - b, a - c);
	const TVEC3<T> Zero3(T(0), T(0), T(0));
	face_normals[index] = equal(cr, Zero3) ? Zero3 : normalize(cr) * ccwMultiplier;
}

template<typename T, typename IndexT> void calc_face_normals(TVEC3<T>* oRESTRICT face_normals, const IndexT* oRESTRICT indices, uint num_indices, const TVEC3<T>* oRESTRICT positions, uint num_positions, bool ccw)
{
	if ((num_indices % 3) != 0)
		oTHROW_INVARG("num_indices must be a multiple of 3");

	bool success = true;
	const T s = ccw ? T(-1) : T(1);
	parallel_for( 0, num_indices / 3, std::bind( calc_face_normals_task<T, IndexT>, std::placeholders::_1
		, face_normals, indices, num_indices, positions, num_positions, s, &success));
	if (!success)
		oTHROW_INVARG("an index value indexes outside the range of vertices specified");
}

template<typename InnerContainerT, typename VecT> void average_face_normals(
	const std::vector<InnerContainerT>& container, const std::vector<TVEC3<VecT>>& face_normals
	, TVEC3<VecT>* normals, uint num_vertex_normals, bool overwrite_all)
{
	// Now go through the list and average the normals
	for (uint i = 0; i < num_vertex_normals; i++)
	{
		// If there is length on the data already, leave it alone
		if (!overwrite_all && equal(normals[i], TVEC3<VecT>(VecT(0), VecT(0), VecT(0))))
			continue;

		TVEC3<VecT> N(VecT(0), VecT(0), VecT(0));
		const InnerContainerT& TrianglesUsed = container[i];
		for (size_t t = 0; t < TrianglesUsed.size(); t++)
		{
			uint faceIndex = TrianglesUsed[t];
			if (!equal(dot(face_normals[faceIndex], face_normals[faceIndex]), VecT(0)))
				N += face_normals[faceIndex];
		}

		normals[i] = normalize(N);
	}
}

template<typename T, typename IndexT> void calc_vertex_normals(TVEC3<T>* oRESTRICT vertex_normals, const IndexT* oRESTRICT indices, uint num_indices
	, const TVEC3<T>* oRESTRICT positions, uint num_positions, bool ccw = false, bool overwrite_all = true)
{
	std::vector<TVEC3<T>> faceNormals(num_indices / 3, TVEC3<T>(T(0), T(0), T(0)));

	calc_face_normals(faceNormals.data(), indices, num_indices, positions, num_positions, ccw);

	const uint nFaces = as_uint(num_indices) / 3;
	const uint REASONABLE_MAX_FACES_PER_VERTEX = 32;
	std::vector<fixed_vector<uint, REASONABLE_MAX_FACES_PER_VERTEX>> trianglesUsedByVertexA(num_positions);
	std::vector<std::vector<uint>> trianglesUsedByVertex;
	bool UseVecVec = false;

	// Try with a less-memory-intensive method first. If that overflows, fall back
	// to the alloc-y one. This is to avoid a 5x slowdown when std::vector gets
	// released due to secure CRT over-zealousness in this case.
	// for each vertex, store a list of the faces to which it contributes
	for (uint i = 0; i < nFaces; i++)
	{
		fixed_vector<uint, REASONABLE_MAX_FACES_PER_VERTEX>& a = trianglesUsedByVertexA[indices[i*3]];
		fixed_vector<uint, REASONABLE_MAX_FACES_PER_VERTEX>& b = trianglesUsedByVertexA[indices[i*3+1]];
		fixed_vector<uint, REASONABLE_MAX_FACES_PER_VERTEX>& c = trianglesUsedByVertexA[indices[i*3+2]];

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
		trianglesUsedByVertex.resize(num_positions);

		for (uint i = 0; i < nFaces; i++)
		{
			push_back_unique(trianglesUsedByVertex[indices[i*3]], i);
			push_back_unique(trianglesUsedByVertex[indices[i*3+1]], i);
			push_back_unique(trianglesUsedByVertex[indices[i*3+2]], i);
		}

		average_face_normals(trianglesUsedByVertex, faceNormals, vertex_normals, num_positions, overwrite_all);

		uint MaxValence = 0;
		// print out why we ended up in this path...
		for (uint i = 0; i < num_positions; i++)
			MaxValence = __max(MaxValence, as_uint(trianglesUsedByVertex[i].size()));
		oTRACE("debug-slow path in normals caused by reasonable max valence (%u) being exceeded. Actual valence: %u", REASONABLE_MAX_FACES_PER_VERTEX, MaxValence);
	}

	else
		average_face_normals(trianglesUsedByVertexA, faceNormals, vertex_normals, num_positions, overwrite_all);
}

template<typename T, typename IndexT, typename TexCoordTupleT> void calc_vertex_tangents(TVEC4<T>* oRESTRICT tangents, const IndexT* oRESTRICT indices, uint num_indices
 , const TVEC3<T>* oRESTRICT positions, const TVEC3<T>* oRESTRICT normals, const TexCoordTupleT* oRESTRICT texcoords, uint num_vertices)
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

	std::vector<TVEC3<T>> tan1(num_vertices, TVEC3<T>(T(0), T(0), T(0)));
	std::vector<TVEC3<T>> tan2(num_vertices, TVEC3<T>(T(0), T(0), T(0)));

	const uint count = num_indices / 3;
	for (uint i = 0; i < count; i++)
	{
		const uint a = indices[3*i];
		const uint b = indices[3*i+1];
		const uint c = indices[3*i+2];

		const TVEC3<T>& Pa = positions[a];
		const TVEC3<T>& Pb = positions[b];
		const TVEC3<T>& Pc = positions[c];

		const T x1 = Pb.x - Pa.x;
		const T x2 = Pc.x - Pa.x;
		const T y1 = Pb.y - Pa.y;
		const T y2 = Pc.y - Pa.y;
		const T z1 = Pb.z - Pa.z;
		const T z2 = Pc.z - Pa.z;
        
		const auto& TCa = texcoords[a];
		const auto& TCb = texcoords[b];
		const auto& TCc = texcoords[c];

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

	parallel_for(0, num_vertices, [&](size_t _Index)
	{
		// Gram-Schmidt orthogonalize + handedness
		const TVEC3<T>& n = normals[_Index];
		const TVEC3<T>& t = tan1[_Index];
		tangents[_Index] = TVEC4<T>(normalize(t - n * dot(n, t)), (dot(cross(n, t), tan2[_Index]) < T(0)) ? T(-1) : T(1));
	});

	// $(CitedCodeEnd)
}

template<typename T, typename IndexT, typename UV0T>
void calc_texcoords(const aabox<T, TVEC3<T>>& bound, const IndexT* indices, uint num_indices, const TVEC3<T>* positions, UV0T* out_texcoords, uint num_vertices, double* out_solve_time)
{
	oTHROW(operation_not_supported, "calc_texcoords not implemented");
}

template<typename IndexT> void prune_indices(const std::vector<bool>& refed, IndexT* indices, uint num_indices)
{
	std::vector<IndexT> sub(refed.size(), 0);
	for (size_t i = 0; i < refed.size(); i++)
		if (!refed[i])
			for (size_t j = i; j < sub.size(); j++)
				(sub[j])++;

	for (uint i = 0; i < num_indices; i++)
		indices[i] -= sub[indices[i]];
}

template<typename T> uint prune_stream(const std::vector<bool>& refed, T* _pStream, uint _NumberOfElements)
{
	if (!_pStream)
		return 0;

	std::vector<bool>::const_iterator itRefed = refed.begin();
	T* r = _pStream, *w = _pStream;
	while (itRefed != refed.end())
	{
		if (*itRefed++)
			*w++ = *r++;
		else
			++r;
	}

	return static_cast<uint>(_NumberOfElements - (r - w));
}

template<typename T, typename IndexT, typename UV0VecT, typename UV1VecT> 
void prune_unindexed_vertices(const IndexT* oRESTRICT indices, uint num_indices
	, TVEC3<T>* oRESTRICT positions, TVEC3<T>* oRESTRICT normals, TVEC4<T>* oRESTRICT tangents, UV0VecT* oRESTRICT texcoords0, UV1VecT* oRESTRICT texcoords1, color* oRESTRICT colors
	, uint num_vertices, uint* oRESTRICT out_new_num_vertices)
{
	std::vector<bool> refed;
	refed.assign(num_vertices, false);
	for (size_t i = 0; i < num_indices; i++)
		refed[indices[i]] = true;
	uint newNumVertices = 0;

	static const uint kNumVertexWhereParallelismHelps = 2000;

	if (num_vertices < kNumVertexWhereParallelismHelps)
	{
		newNumVertices = prune_stream(refed, positions, num_vertices);
		prune_stream(refed, normals, num_vertices);
		prune_stream(refed, tangents, num_vertices);
		prune_stream(refed, texcoords0, num_vertices);
		prune_stream(refed, texcoords1, num_vertices);
		prune_stream(refed, colors, num_vertices);
	}

	else
	{
		task_group* g = new_task_group();
		g->run([&] { newNumVertices = prune_stream(refed, positions, num_vertices); });
		g->run([&] { prune_stream(refed, normals, num_vertices); });
		g->run([&] { prune_stream(refed, tangents, num_vertices); });
		g->run([&] { prune_stream(refed, texcoords0, num_vertices); });
		g->run([&] { prune_stream(refed, texcoords1, num_vertices); });
		g->run([&] { prune_stream(refed, colors, num_vertices); });
		g->wait();
		delete_task_group(g);
	}

	*out_new_num_vertices = newNumVertices;
}

		} // namespace detail
	} // namespace mesh
}

#endif
