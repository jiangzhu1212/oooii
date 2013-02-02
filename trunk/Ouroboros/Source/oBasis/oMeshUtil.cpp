/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasis/oMeshUtil.h>
#include <oBasis/oAlgorithm.h>
#include <oBasis/oAssert.h>
#include <oBasis/oByte.h>
#include <oBasis/oError.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oMath.h>
#include <oBasis/oTask.h>
#include <vector>

void oTransformPoints(const float4x4& _Matrix, float3* oRESTRICT _pDestination, unsigned int _DestinationStride, const float3* oRESTRICT _pSource, unsigned int _SourceStride, unsigned int _NumPoints)
{
	for (unsigned int i = 0; i < _NumPoints; i++)
	{
		*_pDestination = mul(_Matrix, float4(*_pSource, 1.0f)).xyz();
		_pDestination = oByteAdd(_pDestination, _DestinationStride);
		_pSource = oByteAdd(_pSource, _SourceStride);
	}
}

void oTransformVectors(const float4x4& _Matrix, float3* oRESTRICT _pDestination, unsigned int _DestinationStride, const float3* oRESTRICT _pSource, unsigned int _SourceStride, unsigned int _NumVectors)
{
	float3x3 m_(_Matrix[0].xyz(), _Matrix[1].xyz(), _Matrix[2].xyz());

	for (unsigned int i = 0; i < _NumVectors; i++)
	{
		*_pDestination = m_ * (*_pSource);
		_pDestination = oByteAdd(_pDestination, _DestinationStride);
		_pSource = oByteAdd(_pSource, _SourceStride);
	}
}

template<typename T> bool oRemoveDegeneratesT(const TVEC3<T>* _pPositions, size_t _NumberOfPositions, unsigned int* _pIndices, size_t _NumberOfIndices, size_t* _pNewNumIndices)
{
	if ((_NumberOfIndices % 3) != 0)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "_NumberOfIndices must be a multiple of 3");

	for (size_t i = 0; i < _NumberOfIndices / 3; i++)
	{
		size_t I = i * 3;
		size_t J = i * 3 + 1;
		size_t K = i * 3 + 2;

		if (_pIndices[I] >= _NumberOfPositions || _pIndices[J] >= _NumberOfPositions || _pIndices[K] >= _NumberOfPositions)
			return oErrorSetLast(oERROR_INVALID_PARAMETER, "an index value indexes outside the range of vertices specified");

		const TVEC3<T>& a = _pPositions[_pIndices[I]];
		const TVEC3<T>& b = _pPositions[_pIndices[J]];
		const TVEC3<T>& c = _pPositions[_pIndices[K]];

		if (oEqual(cross(a - b, a - c), 0.0f))
		{
			_pIndices[I] = oInvalid;
			_pIndices[J] = oInvalid;
			_pIndices[K] = oInvalid;
		}
	}

	*_pNewNumIndices = _NumberOfIndices;
	for (size_t i = 0; i < *_pNewNumIndices; i++)
	{
		if (_pIndices[i] == oInvalid)
		{
			memcpy(&_pIndices[i], &_pIndices[i+1], sizeof(unsigned int) * (_NumberOfIndices - i - 1));
			i--;
			(*_pNewNumIndices)--;
		}
	}

	return true;
}

bool oRemoveDegenerates(const float3* _pVertices, size_t _NumberOfVertices, unsigned int* _pIndices, size_t _NumberOfIndices, size_t* _pNewNumIndices)
{
	return oRemoveDegeneratesT(_pVertices, _NumberOfVertices, _pIndices, _NumberOfIndices, _pNewNumIndices);
}

template<typename T> static void CalculateFace(size_t index, TVEC3<T>* _pFaceNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const TVEC3<T>* _pPositions, size_t _NumberOfPositions, T CCWMultiplier, bool* pSuccess)
{
	size_t I = index * 3;
	size_t J = index * 3 + 1;
	size_t K = index * 3 + 2;

	if (_pIndices[I] >= _NumberOfPositions || _pIndices[J] >= _NumberOfPositions || _pIndices[K] >= _NumberOfPositions)
	{
		*pSuccess = false;
		return;
	}

	const TVEC3<T>& a = _pPositions[_pIndices[I]];
	const TVEC3<T>& b = _pPositions[_pIndices[J]];
	const TVEC3<T>& c = _pPositions[_pIndices[K]];

	// gracefully put in a zero vector for degenerate faces
	float3 cr = cross(a - b, a - c);
	_pFaceNormals[index] = oEqual(cr, 0.0f) ? float3(0.0f, 0.0f, 0.0f) : CCWMultiplier * normalize(cr);
}

template<typename T> bool oCalculateFaceNormalsT(TVEC3<T>* _pFaceNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const TVEC3<T>* _pPositions, size_t _NumberOfPositions, bool _CCW)
{
	if ((_NumberOfIndices % 3) != 0)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "_NumberOfIndices must be a multiple of 3");
		return false;
	}

	bool success = true;
	const T s = _CCW ? T(-1.0) : T(1.0);
	oTaskParallelFor( 0, _NumberOfIndices / 3, oBIND( &CalculateFace<T>, oBIND1, _pFaceNormals, _pIndices, _NumberOfIndices, _pPositions, _NumberOfPositions, s, &success ));
	if( !success )
		oErrorSetLast(oERROR_INVALID_PARAMETER, "an index value indexes outside the range of vertices specified");

	return success;
}

bool oCalculateFaceNormals(float3* _pFaceNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const float3* _pPositions, size_t _NumberOfPositions, bool _CCW)
{
	return oCalculateFaceNormalsT(_pFaceNormals, _pIndices, _NumberOfIndices, _pPositions, _NumberOfPositions, _CCW);
}

template<typename T> bool oCalculateVertexNormalsT(TVEC3<T>* _pVertexNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const TVEC3<T>* _pPositions, size_t _NumberOfVertices, bool _CCW, bool _OverwriteAll)
{											
	std::vector<TVEC3<T>> faceNormals(_NumberOfIndices / 3, TVEC3<T>(T(0.0), T(0.0), T(0.0)));

	if (!oCalculateFaceNormals(oGetData(faceNormals), _pIndices, _NumberOfIndices, _pPositions, _NumberOfVertices, _CCW))
		return false;

	const size_t nFaces = _NumberOfIndices / 3;

	// for each vertex, store a list of the faces to which it contributes
	std::vector<std::vector<size_t> > trianglesUsedByVertex(_NumberOfVertices);

	for (size_t i = 0; i < nFaces; i++)
	{
		oPushBackUnique(trianglesUsedByVertex[_pIndices[i*3]], i);
		oPushBackUnique(trianglesUsedByVertex[_pIndices[i*3+1]], i);
		oPushBackUnique(trianglesUsedByVertex[_pIndices[i*3+2]], i);
	}

	// Now go through the list and average the normals
	for (size_t i = 0; i < _NumberOfVertices; i++)
	{
		// If there is length on the data already, leave it alone
		if (!_OverwriteAll && oEqual(_pVertexNormals[i], 0.0f))
			continue;

		TVEC3<T> N(T(0.0), T(0.0), T(0.0));
		std::vector<size_t>& TrianglesUsed = trianglesUsedByVertex[i];
		for (size_t t = 0; t < TrianglesUsed.size(); t++)
		{
			size_t faceIndex = TrianglesUsed[t];
			if (!oEqual(dot(faceNormals[faceIndex], faceNormals[faceIndex]), 0.0f))
				N += faceNormals[faceIndex];
		}
		
		_pVertexNormals[i] = normalize(N);
	}

	return true;
}

bool oCalculateVertexNormals(float3* _pVertexNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const float3* _pPositions, size_t _NumberOfVertices, bool _CCW, bool _OverwriteAll)
{
	return oCalculateVertexNormalsT(_pVertexNormals, _pIndices, _NumberOfIndices, _pPositions, _NumberOfVertices, _CCW, _OverwriteAll);
}

template<typename T> void oCalculateTangentsT(TVEC4<T>* _pTangents
 , const unsigned int* _pIndices
 , size_t _NumberOfIndices
 , const TVEC3<T>* _pPositions
 , const TVEC3<T>* _pNormals
 , const TVEC3<T>* _pTexcoords
 , size_t _NumberOfVertices)
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

	std::vector<TVEC3<T> > tan1(_NumberOfVertices, TVEC3<T>(T(0), T(0), T(0)));
	std::vector<TVEC3<T> > tan2(_NumberOfVertices, TVEC3<T>(T(0), T(0), T(0)));

	const size_t count = _NumberOfIndices / 3;
	for (unsigned int i = 0; i < count; i++)
	{
		const unsigned int a = _pIndices[3*i];
		const unsigned int b = _pIndices[3*i+1];
		const unsigned int c = _pIndices[3*i+2];

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

		T r = T(1.0) / (s1 * t2 - s2 * t1);
		TVEC3<T> s((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		TVEC3<T> t((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		tan1[a] += s;
		tan1[b] += s;
		tan1[c] += s;

		tan2[a] += t;
		tan2[b] += t;
		tan2[c] += t;
	}

	for (unsigned int i = 0; i < _NumberOfVertices; i++)
	{
		// Gram-Schmidt orthogonalize + handedness
		const TVEC3<T>& n = _pNormals[i];
		const TVEC3<T>& t = tan1[i];
		_pTangents[i] = TVEC4<T>(normalize(t - n * dot(n, t)), (dot(static_cast<float3>(cross(n, t)), tan2[i]) < T(0)) ? T(-1.0) : T(1.0));
	}

	// $(CitedCodeEnd)
}

void oCalculateTangents(float4* _pTangents
											 , const unsigned int* _pIndices
											 , size_t _NumberOfIndices
											 , const float3* _pPositions
											 , const float3* _pNormals
											 , const float3* _pTexcoords
											 , size_t _NumberOfVertices)
{
	oCalculateTangentsT(_pTangents, _pIndices, _NumberOfIndices, _pPositions, _pNormals, _pTexcoords, _NumberOfVertices);
}

namespace TerathonEdges {

	/** <citation
		usage="Implementation" 
		reason="tangents can be derived, and this is how to do it" 
		author="Eric Lengyel"
		description="http://www.terathon.com/code/edges.html"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.terathon.com/code/edges.html"
		modification="Minor changes to not limit algo to 65536 indices and some fixes to get it compiling"
	/>*/

// $(CitedCodeBegin)

// Building an Edge List for an Arbitrary Mesh
// The following code builds a list of edges for an arbitrary triangle 
// mesh and has O(n) running time in the number of triangles n in the 
// pGeometry-> The edgeArray parameter must point to a previously allocated 
// array of Edge structures large enough to hold all of the mesh's 
// edges, which in the worst possible case is 3 times the number of 
// triangles in the pGeometry->

// An edge list is useful for many geometric algorithms in computer 
// graphics. In particular, an edge list is necessary for stencil 
// shadows.

struct Edge
{
    unsigned int      vertexIndex[2]; 
    unsigned int      faceIndex[2];
};


struct Triangle
{
    unsigned int      index[3];
};


long BuildEdges(long vertexCount, long triangleCount,
                const Triangle *triangleArray, Edge *edgeArray)
{
    long maxEdgeCount = triangleCount * 3;
    unsigned int *firstEdge = new unsigned int[vertexCount + maxEdgeCount];
    unsigned int *nextEdge = firstEdge + vertexCount;
    
    for (long a = 0; a < vertexCount; a++) firstEdge[a] = 0xFFFFFFFF;
    
    // First pass over all triangles. This finds all the edges satisfying the
    // condition that the first vertex index is less than the second vertex index
    // when the direction from the first vertex to the second vertex represents
    // a counterclockwise winding around the triangle to which the edge belongs.
    // For each edge found, the edge index is stored in a linked list of edges
    // belonging to the lower-numbered vertex index i. This allows us to quickly
    // find an edge in the second pass whose higher-numbered vertex index is i.
    
    long edgeCount = 0;
    const Triangle *triangle = triangleArray;
    for (long a = 0; a < triangleCount; a++)
    {
        long i1 = triangle->index[2];
        for (long b = 0; b < 3; b++)
        {
            long i2 = triangle->index[b];
            if (i1 < i2)
            {
                Edge *edge = &edgeArray[edgeCount];
                
                edge->vertexIndex[0] = (unsigned int) i1;
                edge->vertexIndex[1] = (unsigned int) i2;
                edge->faceIndex[0] = (unsigned int) a;
                edge->faceIndex[1] = (unsigned int) a;
                
                long edgeIndex = firstEdge[i1];
                if (edgeIndex == 0xFFFFFFFF)
                {
                    firstEdge[i1] = edgeCount;
                }
                else
                {
                    for (;;)
                    {
                        long index = nextEdge[edgeIndex];
                        if (index == 0xFFFFFFFF)
                        {
                            nextEdge[edgeIndex] = edgeCount;
                            break;
                        }
                        
                        edgeIndex = index;
                    }
                }
                
                nextEdge[edgeCount] = 0xFFFFFFFF;
                edgeCount++;
            }
            
            i1 = i2;
        }
        
        triangle++;
    }
    
    // Second pass over all triangles. This finds all the edges satisfying the
    // condition that the first vertex index is greater than the second vertex index
    // when the direction from the first vertex to the second vertex represents
    // a counterclockwise winding around the triangle to which the edge belongs.
    // For each of these edges, the same edge should have already been found in
    // the first pass for a different triangle. So we search the list of edges
    // for the higher-numbered vertex index for the matching edge and fill in the
    // second triangle index. The maximum number of comparisons in this search for
    // any vertex is the number of edges having that vertex as an endpoint.
    
    triangle = triangleArray;
    for (long a = 0; a < triangleCount; a++)
    {
        long i1 = triangle->index[2];
        for (long b = 0; b < 3; b++)
        {
            long i2 = triangle->index[b];
            if (i1 > i2)
            {
                for (long edgeIndex = firstEdge[i2]; edgeIndex != 0xFFFFFFFF;
                        edgeIndex = nextEdge[edgeIndex])
                {
                    Edge *edge = &edgeArray[edgeIndex];
                    if ((edge->vertexIndex[1] == (unsigned int)i1) &&
                            (edge->faceIndex[0] == edge->faceIndex[1]))
                    {
                        edge->faceIndex[1] = (unsigned int) a;
                        break;
                    }
                }
            }
            
            i1 = i2;
        }
        
        triangle++;
    }
    
    delete[] firstEdge;
    return (edgeCount);
}

// $(CitedCodeEnd)

} // namespace TerathonEdges

void oCalculateEdges(size_t _NumberOfVertices, const unsigned int* _pIndices, size_t _NumberOfIndices, unsigned int** _ppEdges, size_t* _pNumberOfEdges)
{
	const size_t numTriangles = _NumberOfIndices / 3;
	oASSERT((size_t)((long)_NumberOfVertices) == _NumberOfVertices, "");
	oASSERT((size_t)((long)numTriangles) == numTriangles, "");

	TerathonEdges::Edge* edgeArray = new TerathonEdges::Edge[3 * numTriangles];

	size_t numEdges = static_cast<size_t>(TerathonEdges::BuildEdges(static_cast<long>(_NumberOfVertices), static_cast<long>(numTriangles), (const TerathonEdges::Triangle *)_pIndices, edgeArray));

	// @oooii-tony: Should the allocator be exposed?
	*_ppEdges = new unsigned int[numEdges * 2];

	for (size_t i = 0; i < numEdges; i++)
	{
		(*_ppEdges)[i*2] = edgeArray[i].vertexIndex[0];
		(*_ppEdges)[i*2+1] = edgeArray[i].vertexIndex[1];
	}

	*_pNumberOfEdges = numEdges;

	delete [] edgeArray;
}

void oFreeEdgeList(unsigned int* _pEdges)
{
	delete [] _pEdges;
}

static void PruneIndices(const std::vector<bool>& _Refed, unsigned int* _pIndices, size_t _NumberOfIndices)
{
	std::vector<unsigned int> sub(_Refed.size(), 0);

	for (unsigned int i = 0; i < _Refed.size(); i++)
		if (!_Refed[i])
			for (unsigned int j = i; j < sub.size(); j++)
				(sub[j])++;

	for (size_t i = 0; i < _NumberOfIndices; i++)
		_pIndices[i] -= sub[_pIndices[i]];
}

template<typename T> static size_t PruneStream(const std::vector<bool>& _Refed, T* _pStream, size_t _NumberOfElements)
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

	return _NumberOfElements - (r - w);
}

template<typename T> void oPruneUnindexedVerticesT(unsigned int* _pIndices
																									, size_t _NumberOfIndices
																									, TVEC3<T>* _pPositions
																									, TVEC3<T>* _pNormals
																									, TVEC4<T>* _pTangents
																									, TVEC3<T>* _pTexcoords0
																									, TVEC3<T>* _pTexcoords1
																									, unsigned int* _pColors
																									, size_t _NumberOfVertices
																									, size_t *_pNewNumVertices)
{
	std::vector<bool> refed;
	refed.assign(_NumberOfVertices, false);
	for (size_t i = 0; i < _NumberOfIndices; i++)
		refed[_pIndices[i]] = true;
	size_t newNumVertices = PruneStream(refed, _pPositions, _NumberOfVertices);
	PruneStream(refed, _pNormals, _NumberOfVertices);
	PruneStream(refed, _pTangents, _NumberOfVertices);
	PruneStream(refed, _pTexcoords0, _NumberOfVertices);
	PruneStream(refed, _pTexcoords1, _NumberOfVertices);
	PruneStream(refed, _pColors, _NumberOfVertices);
	PruneIndices(refed, _pIndices, _NumberOfIndices);
	*_pNewNumVertices = newNumVertices;
}

void oPruneUnindexedVertices(unsigned int* _pIndices
														 , size_t _NumberOfIndices
														 , float3* _pPositions
														 , float3* _pNormals
														 , float4* _pTangents
														 , float3* _pTexcoords0
														 , float3* _pTexcoords1
														 , unsigned int* _pColors
														 , size_t _NumberOfVertices
														 , size_t *_pNewNumVertices)
{
	oPruneUnindexedVerticesT(_pIndices, _NumberOfIndices, _pPositions, _pNormals, _pTangents, _pTexcoords0, _pTexcoords1, _pColors, _NumberOfVertices, _pNewNumVertices);
}

template<typename T> inline void oCalculateMinMaxPointsT(const TVEC3<T>* oRESTRICT _pPoints, size_t _NumberOfPoints, TVEC3<T>* oRESTRICT _pMinPoint, TVEC3<T>* oRESTRICT _pMaxPoint)
{
	*_pMinPoint = TVEC3<T>(oNumericLimits<T>::GetMax());
	*_pMaxPoint = TVEC3<T>(oNumericLimits<T>::GetSignedMin());

	for (size_t i = 0; i < _NumberOfPoints; i++)
	{
		*_pMinPoint = min(*_pMinPoint, _pPoints[i]);
		*_pMaxPoint = max(*_pMaxPoint, _pPoints[i]);
	}
}

void oCalculateMinMaxPoints(const float3* oRESTRICT _pPoints, size_t _NumberOfPoints, float3* oRESTRICT _pMinPoint, float3* oRESTRICT _pMaxPoint)
{
	oCalculateMinMaxPointsT(_pPoints, _NumberOfPoints, _pMinPoint, _pMaxPoint);
}

void oCalculateMinMaxPoints(const double3* oRESTRICT _pPoints, size_t _NumberOfPoints, double3* oRESTRICT _pMinPoint, double3* oRESTRICT _pMaxPoint)
{
	oCalculateMinMaxPointsT(_pPoints, _NumberOfPoints, _pMinPoint, _pMaxPoint);
}

bool oCalculateTexcoords(const oAABoxf& _Bound, const unsigned int* _pIndices, unsigned int _NumIndices, const float3* _pPositions, float3* _pOutTexcoords, unsigned int _NumVertices, double* _pSolveTime)
{
	// @oooii-tony: I tried integrating OpenNL, but integrating it as-is produced
	// the same result as their sample, which is to say something that isn't 
	// generally useful. As I left off it seems that "seaming" or an algo like 
	// seamster needs to be used as a pre-pass. Other integrations are into tools
	// that allow a human to specify those seams. So I ran out of eval time and 
	// removed all the associated code and OpenNL build. It's in my sandbox, so
	// those with access can go look at that stuff if it needs to be resurrected,
	// but it's mostly just the sample OOOii-fied.

	return oErrorSetLast(oERROR_NOT_FOUND, "oCalculateTexcoords is not yet implemented.");
}
