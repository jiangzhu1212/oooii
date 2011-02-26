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
#include "pch.h"
#include <oooii/oGeometry.h>
#include <oooii/oMath.h>
#include <oooii/oErrno.h>

float3 float3_zero(0.0f,0.0f,0.0f);

#define Err(a)
#define ESUCCESS 0
#define EFAIL 1
#define ENYI 2

size_t oGeometry::GetNumPrimitives() const
{
	size_t n = Indices.size();
	switch (PrimitiveType)
	{
		case GEOMETRY_PRIMITIVE_POINTLIST: n = GetNumVertices(); break;
		case GEOMETRY_PRIMITIVE_LINELIST: n /= 2; break;
		case GEOMETRY_PRIMITIVE_LINESTRIP: n--; break;
		case GEOMETRY_PRIMITIVE_TRILIST: n /= 3; break;
		case GEOMETRY_PRIMITIVE_TRISTRIP: n -= 2; break;
		default: n = 0; break;
	}
	return n;
}

bool oGeometry::IsValid() const
{
	if (PrimitiveType == GEOMETRY_PRIMITIVE_UNKNOWN) return false;
	if (Positions.empty()) return false;
	if (!Normals.empty() && Normals.size() != GetNumVertices()) return false;
	if (!Tangents.empty() && Tangents.size() != GetNumVertices()) return false;
	if (!Texcoords.empty() && Texcoords.size() != GetNumVertices()) return false;
	if (!Colors.empty() && Colors.size() != GetNumVertices()) return false;
	return true;
}

void oGeometry::Clear()
{
	PrimitiveType = GEOMETRY_PRIMITIVE_UNKNOWN;
	Indices.clear();
	Positions.clear();
	Normals.clear();
	Tangents.clear();
	Texcoords.clear();
	Colors.clear();
}

void oGeometry::Transform(const float4x4& _Matrix)
{

	oFOREACH(float3& p, Positions) p = _Matrix * p;
	float3x3 r = _Matrix.getUpper3x3();
	oFOREACH(float3& n, Normals) n = r * n;
	oFOREACH(float4& t, Tangents) t = float4(r * t.XYZ(), t.W());
}

void oGeometry::CalculateBounds()
{
	Bounds.Empty();
	oFOREACH(float3&p, Positions)
		Bounds.ExtendBy(p);
}

static bool IsSupported(const GEOMETRY_LAYOUT* _pSupported, const GEOMETRY_LAYOUT* _pInput)
{    
	return !(!_pInput->Positions
		|| (!_pSupported->Normals && _pInput->Normals)
		|| (!_pSupported->Tangents && _pInput->Tangents)
		|| (!_pSupported->Colors && _pInput->Colors)
		|| (!_pSupported->Texcoords && _pInput->Texcoords));
}

static GEOMETRY_FACE_TYPE GetOppositeWindingOrder(GEOMETRY_FACE_TYPE type)
{
	switch (type)
	{
		case GEOMETRY_FACE_SOLID_CW: return GEOMETRY_FACE_SOLID_CCW;
		case GEOMETRY_FACE_SOLID_CCW: return GEOMETRY_FACE_SOLID_CW;
		default: break;
	}

	return type;
}

static float GetNormalSign(GEOMETRY_FACE_TYPE type)
{
	switch (type)
	{
		case GEOMETRY_FACE_SOLID_CW: return 1.0f;
		case GEOMETRY_FACE_SOLID_CCW: return -1.0f;
		default: break;
	}

	return 0.0f;
}

void ChangeWindingOrder(size_t _NumberOfIndices, unsigned int* _pIndices, unsigned int _BaseIndexIndex)
{
	oASSERT(/*"core.geometry", */(_BaseIndexIndex % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded.");
	oASSERT(/*"core.geometry", */(_NumberOfIndices % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded.");
	for (size_t i = _BaseIndexIndex; i < _NumberOfIndices; i += 3)
		std::swap(_pIndices[i+1], _pIndices[i+2]);
}

template<typename T, typename A> inline void ChangeWindingOrder(std::vector<T, A>& _Indices, unsigned int _BaseIndexIndex) { ChangeWindingOrder(_Indices.size(), &_Indices[0], _BaseIndexIndex); }

void CalculateFaceNormals(float3* _pFaceNormals
	, size_t _NumberOfIndices
	, const unsigned int* _pIndices
	, size_t _NumberOfVertices
	, const float3* _pPositions
	, bool _CCW)
{
	oASSERT(/*"core.geometry", */(_NumberOfIndices % 3) == 0, "");
	const float s = _CCW ? 1.0f : -1.0f;

	for (size_t i = 0; i < _NumberOfIndices / 3; i++)
	{
		const float3 a = _pPositions[_pIndices[i*3]];
		const float3 b = _pPositions[_pIndices[i*3+1]];
		const float3 c = _pPositions[_pIndices[i*3+2]];
		_pFaceNormals[i] = s * normalize(cross(a - b, a - c));
	}
}

void CalculateVertexNormals(float3* _pVertexNormals
	, size_t _NumberOfIndices
	, const unsigned int* _pIndices
	, size_t _NumberOfVertices
	, const float3* _pPositions
	, bool _CCW)
{											
	oASSERT(/*"core.geometry", */(_NumberOfIndices % 3) == 0, "");

	std::vector<float3> faceNormals(_NumberOfIndices / 3, float3_zero);
	CalculateFaceNormals(&faceNormals[0], _NumberOfIndices, _pIndices, _NumberOfVertices, _pPositions, _CCW);

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
		float3 N = float3_zero;
		oFOREACH(size_t faceIndex, trianglesUsedByVertex[i])
			N += faceNormals[faceIndex];
		_pVertexNormals[i] = normalize(N);
	}
}

void CalculateTangents(float4* _pTangents
 , size_t _NumberOfIndices
 , const unsigned int* _pIndices
 , size_t _NumberOfVertices
 , const float3* _pPositions
 , const float3* _pNormals
 , const float2* _pTexcoords)
{
	/** $(Citation)
		<citation>
			<usage type="Implementation" />
			<author name="Eric Lengyel" />
			<description url="http://www.terathon.com/code/tangent.html" />
			<license type="*** Assumed Public Domain ***" url="http://www.terathon.com/code/tangent.html" />
		</citation>
	*/

	// $(CitedCodeBegin)

	std::vector<float3> tan1(_NumberOfVertices, float3_zero);
	std::vector<float3> tan2(_NumberOfVertices, float3_zero);

	const size_t count = _NumberOfIndices / 3;
	for (unsigned int i = 0; i < count; i++)
	{
		const unsigned int a = _pIndices[3*i];
		const unsigned int b = _pIndices[3*i+1];
		const unsigned int c = _pIndices[3*i+2];

		const float3& Pa = _pPositions[a];
		const float3& Pb = _pPositions[b];
		const float3& Pc = _pPositions[c];

		const float x1 = Pb.x - Pa.x;
		const float x2 = Pc.x - Pa.x;
		const float y1 = Pb.y - Pa.y;
		const float y2 = Pc.y - Pa.y;
		const float z1 = Pb.z - Pa.z;
		const float z2 = Pc.z - Pa.z;
        
		const float2& TCa = _pTexcoords[a];
		const float2& TCb = _pTexcoords[b];
		const float2& TCc = _pTexcoords[c];

		const float s1 = TCb.x - TCa.x;
		const float s2 = TCc.x - TCa.x;
		const float t1 = TCb.y - TCa.y;
		const float t2 = TCc.y - TCa.y;

		float r = 1.0f / (s1 * t2 - s2 * t1);
		float3 s((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		float3 t((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

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
		const float3& n = _pNormals[i];
		const float3& t = tan1[i];
		_pTangents[i] = float4(normalize(t - n * dot(n, t)), (dot(static_cast<float3>(cross(n, t)), tan2[i]) < 0.0f) ? -1.0f : 1.0f);
	}

	// $(CitedCodeEnd)
}

static inline void CalculateVertexNormals(oGeometry* _pGeometry, bool _CCW)
{
	_pGeometry->Normals.resize(_pGeometry->GetNumVertices());
	CalculateVertexNormals(&_pGeometry->Normals[0], _pGeometry->Indices.size(), &_pGeometry->Indices[0], _pGeometry->GetNumVertices(), &_pGeometry->Positions[0], _CCW);
}

static inline void CalculateTangents(oGeometry* _pGeometry, unsigned int _BaseIndexIndex)
{
	_pGeometry->Tangents.resize(_pGeometry->GetNumVertices());
	CalculateTangents(&_pGeometry->Tangents[0], _pGeometry->Indices.size() - _BaseIndexIndex, &_pGeometry->Indices[_BaseIndexIndex], _pGeometry->GetNumVertices(), &_pGeometry->Positions[0], &_pGeometry->Normals[0], &_pGeometry->Texcoords[0]);
}

namespace TerathonEdges {

/** $(Citation)
	<citation>
		<usage type="Implementation" />
		<author name="Eric Lengyel" />
		<description url="http://www.terathon.com/code/edges.html" />
		<license type="*** Assumed Public Domain ***" url="http://www.terathon.com/code/edges.html" />
	</citation>
*/

// (tony) I know this isn't efficient, but I just want to get 
// something working so I can get a reference against which
// to test any smarter implementation.

// Some minor changes have been made to use local types and not limit
// the algo to 65536 indices. ... um, also to make it compile...

// $(CitedCodeBegin)

// Building an Edge List for an Arbitrary Mesh
// The following code builds a list of edges for an arbitrary triangle 
// mesh and has O(n) running time in the number of triangles n in the 
// _pGeometry-> The edgeArray parameter must point to a previously allocated 
// array of Edge structures large enough to hold all of the mesh's 
// edges, which in the worst possible case is 3 times the number of 
// triangles in the _pGeometry->

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

static void CalculateEdges(std::vector<std::pair<unsigned int, unsigned int> >& _Edges, size_t _NumVertices, const std::vector<unsigned int>& _Indices)
{
	// fixme: pass an allocator to this function so the user can redirect 
	// what can be a rather large temp allocation.

	const size_t numTriangles = _Indices.size() / 3;
	oASSERT(/*"core.geometry", */(size_t)((long)_NumVertices) == _NumVertices, "");
	oASSERT(/*"core.geometry", */(size_t)((long)numTriangles) == numTriangles, "");

	TerathonEdges::Edge* edgeArray = new TerathonEdges::Edge[3 * numTriangles];

	size_t numEdges = static_cast<size_t>(TerathonEdges::BuildEdges(static_cast<long>(_NumVertices), static_cast<long>(numTriangles), (const TerathonEdges::Triangle *)&_Indices[0], edgeArray));
	_Edges.resize(numEdges);
	for (size_t i = 0; i < numEdges; i++)
		_Edges[i] = std::pair<unsigned int, unsigned int>(edgeArray[i].vertexIndex[0], edgeArray[i].vertexIndex[1]);

	delete [] edgeArray;
}

static void PruneIndices(const std::vector<bool>& _Refed, std::vector<unsigned int>& _Indices)
{
	std::vector<unsigned int> sub(_Refed.size(), 0);

	for (unsigned int i = 0; i < _Refed.size(); i++)
		if (!_Refed[i])
			for (unsigned int j = i; j < sub.size(); j++)
				(sub[j])++;

	oFOREACH(unsigned int& in, _Indices)
		in -= sub[in];
}

template<typename T> static void PruneStream(const std::vector<bool>& _Refed, std::vector<T>& _Stream)
{
	if (_Stream.empty())
		return;
	std::vector<bool>::const_iterator itRefed = _Refed.begin();
	std::vector<T>::iterator r = _Stream.begin(), w = _Stream.begin();
	while (itRefed != _Refed.end())
	{
		if (*itRefed++)
			*w++ = *r++;
		else
			++r;
	}
}

static void PruneUnindexedVertices(oGeometry* _pGeometry)
{
	// clean orphaned vertex values
	std::vector<bool> refed;
	refed.assign(_pGeometry->GetNumVertices(), false);
	oFOREACH(const unsigned int& i, _pGeometry->Indices)
		refed[i] = true;
	PruneStream(refed, _pGeometry->Positions);
	if (!_pGeometry->Normals.empty()) PruneStream(refed, _pGeometry->Normals);
	if (!_pGeometry->Tangents.empty()) PruneStream(refed, _pGeometry->Tangents);
	if (!_pGeometry->Texcoords.empty()) PruneStream(refed, _pGeometry->Texcoords);
	if (!_pGeometry->Colors.empty()) PruneStream(refed, _pGeometry->Colors);
	PruneIndices(refed, _pGeometry->Indices);
}

void Clip(const oPlanef& _Plane, bool _Clip, oGeometry* _pGeometry)
{
	// fixme: (tony) I just wanted this for a skydome, where the 
	// horizon is hidden anyway, so don't be too smart, just cut 
	// out most of the triangles.
	oASSERT(/*"core.geometry", */!_Clip, "clipping not yet implemented");

	// go thru indices and remove triangles on back side of the plane
	std::vector<unsigned int>::iterator it = _pGeometry->Indices.begin();
	while (it != _pGeometry->Indices.end())
	{
		float3 a = _pGeometry->Positions[*it];
		float3 b = _pGeometry->Positions[*(it+1)];
		float3 c = _pGeometry->Positions[*(it+2)];

		float A = sdistance(_Plane, a);
		float B = sdistance(_Plane, b);
		float C = sdistance(_Plane, c);

		if (A < 0.0f && B < 0.0f && C < 0.0f)
			it = _pGeometry->Indices.erase(it, it+3);
		else
			it += 3;
	}

	PruneUnindexedVertices(_pGeometry);
}

namespace RectDetails
{

static void FillIndices(std::vector<unsigned int>& _Indices, unsigned int _DivideW, unsigned int _DivideH, unsigned int _BaseVertexIndex)
{
	_Indices.reserve(_Indices.size() + 6 * _DivideW * _DivideH);
	unsigned int pitch = _DivideW + 1;
	unsigned int row = 0;
	for (unsigned int i = 0; i < _DivideH; i++, row += pitch)
		for (unsigned int j = 0; j < _DivideW; j++)
		{
			_Indices.push_back(_BaseVertexIndex + row + j);
			_Indices.push_back(_BaseVertexIndex + row + pitch + j);
			_Indices.push_back(_BaseVertexIndex + row + j+1);
			_Indices.push_back(_BaseVertexIndex + row + pitch + j);
			_Indices.push_back(_BaseVertexIndex + row + pitch + j+1);
			_Indices.push_back(_BaseVertexIndex + row + j+1);
		}
}

static void FillPositions(std::vector<float3>& _Positions, float _Width, float _Height, unsigned int _DivideW, unsigned int _DivideH, const float4x4& _Transform)
{
	_Positions.reserve(_Positions.size() + (_DivideW+1) * (_DivideH+1));
	float stepX = _Width / static_cast<float>(_DivideW);
	float stepY = _Height / static_cast<float>(_DivideH);
	float x, y = -_Height / 2.0f;
	for (unsigned int i = 0; i <= _DivideH; i++, y += stepY)
	{
		x = -_Width / 2.0f;
		for (unsigned int j = 0; j <= _DivideW; j++, x += stepX)
			_Positions.push_back(_Transform * float3(x, y, 0.0f));
	}
}

static void FillTexcoords(std::vector<float2>& _Texcoords, unsigned int _DivideW, unsigned int _DivideH, GEOMETRY_FACE_TYPE __Facetype, bool _FlipU, bool _FlipV)
{
	_Texcoords.reserve(_Texcoords.size() + (_DivideW+1) * (_DivideH+1));
	float stepU = 1.0f / static_cast<float>(_DivideW);
	float stepV = 1.0f / static_cast<float>(_DivideH);
	float u, v = 0.0f;
	for (unsigned int i = 0; i <= _DivideH; i++)
	{
		u = 0.0f;
		for (unsigned int j = 0; j <= _DivideW; j++, u += stepU)
		{
			float2 texcoord;
			texcoord.x = (__Facetype == GEOMETRY_FACE_SOLID_CCW ? 1.0f - u : u);
			texcoord.x = (_FlipU ? 1.0f - texcoord.x : texcoord.x);
			texcoord.y = (_FlipV ? 1.0f - v : v);
			_Texcoords.push_back(texcoord);
		}

		v += stepV;
	}
}

} // namespace RectDetails

errno_t CreateRect(const GEOMETRY_RECT_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry)
{
	if (!_pDesc ||!_pLayout || !_pGeometry) return EINVAL;

	static const GEOMETRY_LAYOUT sSupportedLayout = { true, true, true, true, true };
	if (!IsSupported(&sSupportedLayout, _pLayout))
	{
		Err(/*"core.geometry", */"Invalid layout specified");
		return EINVAL;
	}

	if (!_pDesc->DivideW)
	{
		Err(/*"core.geometry", */"A non-zero value for DivideW must be specified.");
		return EINVAL;
	}

	if (!_pDesc->DivideH)
	{
		Err(/*"core.geometry", */"A non-zero value for DivideH must be specified.");
		return EINVAL;
	}

	unsigned int divideW = (_pDesc->FaceType == GEOMETRY_FACE_OUTLINE) ? 1 : _pDesc->DivideW;
	unsigned int divideH = (_pDesc->FaceType == GEOMETRY_FACE_OUTLINE) ? 1 : _pDesc->DivideH;

	_pGeometry->Clear();
	_pGeometry->PrimitiveType = GEOMETRY_PRIMITIVE_TRILIST;

	float4x4 m(float4x4::Identity);
	if (!_pDesc->Centered)
	{
		m[3] = float4(0.5f, 0.5f, 0.0f, 1.0f);
	}

	RectDetails::FillPositions(_pGeometry->Positions, _pDesc->Width, _pDesc->Height, divideW, divideH, m);
	
	if (_pDesc->FaceType == GEOMETRY_FACE_OUTLINE)
	{
		const unsigned int indices[] = { 0,1, 2,3, 0,2, 1,3 };
		_pGeometry->Indices.resize(oCOUNTOF(indices));
		memcpy(&_pGeometry->Indices[0], indices, sizeof(indices));
		_pGeometry->PrimitiveType = GEOMETRY_PRIMITIVE_LINELIST;
	}

	else
		RectDetails::FillIndices(_pGeometry->Indices, divideW, divideH, 0);

	if (_pLayout->Texcoords)
		RectDetails::FillTexcoords(_pGeometry->Texcoords, divideW, divideH, _pDesc->FaceType, _pDesc->FaceType == GEOMETRY_FACE_SOLID_CCW, _pDesc->FlipTexcoordV);

	// FIXME: maybe this needs to be flipped depending on face order?
	if (_pLayout->Normals)
		_pGeometry->Normals.assign(_pGeometry->GetNumVertices(), float3::zAxis());

	if (_pLayout->Tangents)
	{
		_pGeometry->Tangents.resize(_pGeometry->GetNumVertices());
		CalculateTangents(&_pGeometry->Tangents[0]
		 , _pGeometry->Indices.size()
		 , &_pGeometry->Indices[0]
		 , _pGeometry->GetNumVertices()
		 , &_pGeometry->Positions[0]
		 , &_pGeometry->Normals[0]
		 , &_pGeometry->Texcoords[0]);
	}

	if (_pLayout->Colors)
		_pGeometry->Colors.assign(_pGeometry->GetNumVertices(), _pDesc->Color);

	return ESUCCESS;
}

namespace BoxDetails
{

static void FillIndices(std::vector<unsigned int>& _Indices, unsigned int _DivideW, unsigned int _DivideH)
{
	unsigned int nFaceVerts = (_DivideW+1) * (_DivideH+1);
	unsigned int _BaseVertexIndex = 0;
	_Indices.reserve(_Indices.size() + 6 * 6 * _DivideW * _DivideH); // nFaces * nIndicesPerQuad * nQuadsW * nQuadsH
	RectDetails::FillIndices(_Indices, _DivideW, _DivideH, _BaseVertexIndex); _BaseVertexIndex += nFaceVerts;
	RectDetails::FillIndices(_Indices, _DivideW, _DivideH, _BaseVertexIndex); _BaseVertexIndex += nFaceVerts;
	RectDetails::FillIndices(_Indices, _DivideW, _DivideH, _BaseVertexIndex); _BaseVertexIndex += nFaceVerts;
	RectDetails::FillIndices(_Indices, _DivideW, _DivideH, _BaseVertexIndex); _BaseVertexIndex += nFaceVerts;
	RectDetails::FillIndices(_Indices, _DivideW, _DivideH, _BaseVertexIndex); _BaseVertexIndex += nFaceVerts;
	RectDetails::FillIndices(_Indices, _DivideW, _DivideH, _BaseVertexIndex);
}

static void FillLines(const oAABoxf& box, oGeometry* _pGeometry)
{
	_pGeometry->Indices.reserve(12 * 2);
	const unsigned int indices[12 * 2] = { 0,1, 2,3, 0,2, 1,3, 4,5, 6,7, 4,6, 5,7, 0,4, 1,5, 2,6, 3,7 };
	_pGeometry->Indices.resize(oCOUNTOF(indices));
	memcpy(&_pGeometry->Indices[0], indices, sizeof(indices));
	
	_pGeometry->Positions.resize(8);
	_pGeometry->Positions[0] = float3(box.GetMin().x, box.GetMin().y, box.GetMin().z);
	_pGeometry->Positions[1] = float3(box.GetMax().x, box.GetMin().y, box.GetMin().z);
	_pGeometry->Positions[2] = float3(box.GetMin().x, box.GetMax().y, box.GetMin().z);
	_pGeometry->Positions[3] = float3(box.GetMax().x, box.GetMax().y, box.GetMin().z);
	_pGeometry->Positions[4] = float3(box.GetMin().x, box.GetMin().y, box.GetMax().z);
	_pGeometry->Positions[5] = float3(box.GetMax().x, box.GetMin().y, box.GetMax().z);
	_pGeometry->Positions[6] = float3(box.GetMin().x, box.GetMax().y, box.GetMax().z);
	_pGeometry->Positions[7] = float3(box.GetMax().x, box.GetMax().y, box.GetMax().z);
}

} // namespace BoxDetails

errno_t CreateBox(const GEOMETRY_BOX_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry)
{
	if (!_pDesc ||!_pLayout || !_pGeometry) return EINVAL;

	static const GEOMETRY_LAYOUT sSupportedLayout = { true, true, true, true, true };
	if (!IsSupported(&sSupportedLayout, _pLayout))
	{
		Err(/*"core.geometry", */"Invalid layout specified");
		return EINVAL;
	}

	_pGeometry->Clear();
	_pGeometry->PrimitiveType = GEOMETRY_PRIMITIVE_TRILIST;

	float s = GetNormalSign(_pDesc->FaceType);

	float w, h, d;
	_pDesc->Bound.GetDimensions(&w, &h, &d);

	if (_pDesc->FaceType == GEOMETRY_FACE_OUTLINE)
	{
		if (_pLayout->Normals || _pLayout->Tangents)
		{
			Err(/*"core.geometry", */"Normals and/or tangent cannot be generated when creating a lines-based box.");
			return EINVAL;
		}

		BoxDetails::FillLines(_pDesc->Bound, _pGeometry);
		_pGeometry->PrimitiveType = GEOMETRY_PRIMITIVE_LINELIST;
	}

	else
	{
		if (_pDesc->Divide == 0) return EINVAL;

		unsigned int divideW = _pDesc->Divide, divideH = _pDesc->Divide;
		BoxDetails::FillIndices(_pGeometry->Indices, divideW, divideH);

		float4x4 top, bottom, front, back, left, right;
		oCalcPlaneMatrix(oPlanef(float3::yAxis(), s*h/2.0f), &top);
		oCalcPlaneMatrix(oPlanef(-float3::yAxis(), s*h/2.0f), &bottom);
		oCalcPlaneMatrix(oPlanef(float3::zAxis(), s*d/2.0f), &front);
		oCalcPlaneMatrix(oPlanef(-float3::zAxis(), s*d/2.0f), &back);
		oCalcPlaneMatrix(oPlanef(-float3::xAxis(), s*w/2.0f), &left);
		oCalcPlaneMatrix(oPlanef(float3::xAxis(), s*w/2.0f), &right);

		const size_t nVertsPerFace = (_pDesc->Divide+1) * (_pDesc->Divide+1);
		const size_t nVerts = 6 * nVertsPerFace;

		_pGeometry->Positions.reserve(nVerts);
		RectDetails::FillPositions(_pGeometry->Positions, w, d, divideW, divideH, top);
		RectDetails::FillPositions(_pGeometry->Positions, w, d, divideW, divideH, bottom);
		RectDetails::FillPositions(_pGeometry->Positions, w, h, divideW, divideH, front);
		RectDetails::FillPositions(_pGeometry->Positions, w, h, divideW, divideH, back);
		RectDetails::FillPositions(_pGeometry->Positions, d, h, divideW, divideH, left);
		RectDetails::FillPositions(_pGeometry->Positions, d, h, divideW, divideH, right);

		if (_pLayout->Texcoords)
		{
			const bool flipU = _pDesc->FaceType == GEOMETRY_FACE_SOLID_CCW;
			RectDetails::FillTexcoords(_pGeometry->Texcoords, divideW, divideH, _pDesc->FaceType, flipU, _pDesc->FlipTexcoordV);
			RectDetails::FillTexcoords(_pGeometry->Texcoords, divideW, divideH, _pDesc->FaceType, flipU, _pDesc->FlipTexcoordV);
			RectDetails::FillTexcoords(_pGeometry->Texcoords, divideW, divideH, _pDesc->FaceType, flipU, _pDesc->FlipTexcoordV);
			RectDetails::FillTexcoords(_pGeometry->Texcoords, divideW, divideH, _pDesc->FaceType, flipU, _pDesc->FlipTexcoordV);
			RectDetails::FillTexcoords(_pGeometry->Texcoords, divideW, divideH, _pDesc->FaceType, flipU, _pDesc->FlipTexcoordV);
			RectDetails::FillTexcoords(_pGeometry->Texcoords, divideW, divideH, _pDesc->FaceType, flipU, _pDesc->FlipTexcoordV);
		}

		if (_pLayout->Normals)
		{
			_pGeometry->Normals.reserve(nVerts);
			_pGeometry->Normals.insert(_pGeometry->Normals.end(), nVertsPerFace, s*float3::yAxis());
			_pGeometry->Normals.insert(_pGeometry->Normals.end(), nVertsPerFace, s*-float3::yAxis());
			_pGeometry->Normals.insert(_pGeometry->Normals.end(), nVertsPerFace, s*float3::zAxis());
			_pGeometry->Normals.insert(_pGeometry->Normals.end(), nVertsPerFace, s*-float3::zAxis());
			_pGeometry->Normals.insert(_pGeometry->Normals.end(), nVertsPerFace, s*-float3::xAxis());
			_pGeometry->Normals.insert(_pGeometry->Normals.end(), nVertsPerFace, s*float3::xAxis());
		}

		if (_pLayout->Tangents)
		{
			_pGeometry->Tangents.resize(_pGeometry->GetNumVertices());
			CalculateTangents(&_pGeometry->Tangents[0]
			 , _pGeometry->Indices.size()
			 , &_pGeometry->Indices[0]
			 , _pGeometry->GetNumVertices()
			 , &_pGeometry->Positions[0]
			 , &_pGeometry->Normals[0]
			 , &_pGeometry->Texcoords[0]);
		}

		const float4x4 m = float4x4::translation(float3(_pDesc->Bound.GetMin() - float3(-w/2.0f, -h/2.0f, -d/2.0f)));
		oFOREACH(float3& p, _pGeometry->Positions)
			p = m * p;
	}

	if (_pLayout->Colors)
		_pGeometry->Colors.assign(_pGeometry->GetNumVertices(), _pDesc->Color);

	return _pGeometry->IsValid() ? ESUCCESS : EFAIL;
}

namespace CircleDetails
{
	// Circle is a bit interesting because vertices are not in CW or CCW order, but rather
	// evens go up one side and odds go up the other. This allows a straightforward way
	// of zig-zag tessellating the circle for more uniform shading than a point in the 
	// center of the circle that radiates outward trifan-style.

void FillIndices(std::vector<unsigned int>& _Indices, unsigned int _BaseVertexIndex, unsigned int __Facet, GEOMETRY_FACE_TYPE __Facetype)
{
	if (__Facetype == GEOMETRY_FACE_OUTLINE)
	{
		_Indices.reserve(_Indices.size() + __Facet * 2);

		const unsigned int numEven = (__Facet - 1) / 2;
		const unsigned int numOdd = (__Facet / 2) - 1; // -1 for transition from 0 to 1, which does not fit the for loop

		for (unsigned int i = 0; i < numEven; i++)
		{
			_Indices.push_back(_BaseVertexIndex + i * 2);
			_Indices.push_back(_BaseVertexIndex + (i + 1) * 2);
		}

		for (unsigned int i = 0; i < numOdd; i++)
		{
			unsigned int idx = i * 2 + 1;

			_Indices.push_back(_BaseVertexIndex + idx);
			_Indices.push_back(_BaseVertexIndex + idx + 2);
		}

		// Edge cases are 0 -> 1 and even -> odd

		_Indices.push_back(_BaseVertexIndex);
		_Indices.push_back(_BaseVertexIndex + 1);

		_Indices.push_back(_BaseVertexIndex + __Facet - 1);
		_Indices.push_back(_BaseVertexIndex + __Facet - 2);
	}

	else
	{
		const unsigned int count = __Facet - 2;
		_Indices.reserve(3 * count);

		const unsigned int o[2][2] = { { 2, 1 }, { 1, 2 } };
		for (unsigned int i = 0; i < count; i++)
		{
			_Indices.push_back(_BaseVertexIndex + i);
			_Indices.push_back(_BaseVertexIndex + i+o[i&0x1][0]);
			_Indices.push_back(_BaseVertexIndex + i+o[i&0x1][1]);
		}

		if (__Facetype == GEOMETRY_FACE_SOLID_CCW)
			ChangeWindingOrder(_Indices, 0);
	}
}

void FillPositions(std::vector<float3>& _Positions, float _Radius, unsigned int __Facet, float _ZValue)
{
	_Positions.reserve(_Positions.size() + __Facet);
	float step = (2.0f * oPIf) / static_cast<float>(__Facet);
	float curStep2 = 0.0f;
	float curStep = (2.0f * oPIf) - step;
	unsigned int k = 0;
	for (unsigned int i = 0; i < __Facet && k < __Facet; i++, curStep -= step, curStep2 += step)
	{
		_Positions.push_back(float3(_Radius * cosf(curStep), _Radius * sinf(curStep), _ZValue));
		if (++k >= __Facet)
			break;
		_Positions.push_back(float3(_Radius * cosf(curStep2), _Radius * sinf(curStep2), _ZValue));
		if (++k >= __Facet)
			break;
	}
}

// For a planar circle, all normals point in the same planar direction
void FillNormalsUp(std::vector<float3>& _Normals, size_t _BaseVertexIndex, bool _CCW)
{
	const float s = _CCW ? -1.0f : 1.0f;
	for (size_t i = _BaseVertexIndex; i < _Normals.size(); i++)
		_Normals[i] = s * float3::zAxis();
}

// Point normals out from center of circle co-planar with circle. This 
// is useful when creating a cylinder.
void FillNormalsOut(std::vector<float3>& _Normals, unsigned int __Facet)
{
	FillPositions(_Normals, 1.0f, __Facet, 0.0f);
}

// Optionally does not clear the specified geometry, so this can be used to 
// append circles while building cylinders.
errno_t CreateCircleInternal(const GEOMETRY_CIRCLE_DESC* _pDesc
	, const GEOMETRY_LAYOUT* _pLayout
	, oGeometry* _pGeometry
	, unsigned int _BaseVertexIndex
	, bool _Clear
	, float _ZValue
	, bool _AllowZeroRadius)
{
	if (!_pDesc ||!_pLayout || !_pGeometry) return EINVAL;

	static const GEOMETRY_LAYOUT sSupportedLayout = { true, true, true, false, true };
	if (!IsSupported(&sSupportedLayout, _pLayout))
	{
		Err(/*"core.geometry", */"Invalid layout specified");
		return EINVAL;
	}

	if (!_AllowZeroRadius && _pDesc->Radius <= 0.0f) return EINVAL;
	if (_pDesc->Facet < 3) return EINVAL;

	if (_Clear)
		_pGeometry->Clear();

	CircleDetails::FillPositions(_pGeometry->Positions, _pDesc->Radius, _pDesc->Facet, _ZValue);
	if (_pDesc->FaceType == GEOMETRY_FACE_OUTLINE)
	{
		if (_pLayout->Normals || _pLayout->Tangents)
			return EINVAL;

		_pGeometry->PrimitiveType = GEOMETRY_PRIMITIVE_LINELIST;
		CircleDetails::FillIndices(_pGeometry->Indices, _BaseVertexIndex, _pDesc->Facet, _pDesc->FaceType);
	}

	else
	{
		const unsigned int baseIndexIndex = static_cast<unsigned int>(_pGeometry->Indices.size());
		
		_pGeometry->PrimitiveType = GEOMETRY_PRIMITIVE_TRILIST;
		CircleDetails::FillIndices(_pGeometry->Indices, _BaseVertexIndex, _pDesc->Facet, _pDesc->FaceType);

		if (_pLayout->Normals)
		{
			_pGeometry->Normals.resize(_pGeometry->GetNumVertices());
			const float3 N = GetNormalSign(_pDesc->FaceType) * float3::zAxis();
			for (size_t i = _BaseVertexIndex; i < _pGeometry->Normals.size(); i++)
				_pGeometry->Normals[i] = N;
		}

		if (_pLayout->Tangents)
			CalculateTangents(_pGeometry, baseIndexIndex);
	}

	if (_pLayout->Colors)
	{
		_pGeometry->Colors.resize(_pGeometry->GetNumVertices());
		for (size_t i = _BaseVertexIndex; i < _pGeometry->Colors.size(); i++)
			_pGeometry->Colors[i] = _pDesc->Color;
	}

	return ESUCCESS;
}

} // namespace CircleDetails

errno_t CreateCircle(const GEOMETRY_CIRCLE_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry)
{
	return CircleDetails::CreateCircleInternal(_pDesc, _pLayout, _pGeometry, 0, true, 0.0f, false);
}

namespace SphereDetails
{
// Based on http://student.ulb.ac.be/~claugero/sphere/index.html 
// (avoids vert duplication of more naive algos)

static unsigned int midpoint(unsigned int indexStart
					   , unsigned int indexEnd
					   , unsigned int& indexCurrent
					   , std::vector<unsigned int>& start
					   , std::vector<unsigned int>& end
					   , std::vector<unsigned int>& mid
					   , std::vector<float3>& verts)
{
	for (unsigned int i = 0; i < indexCurrent; i++)
	{
		if (((start[i] == indexStart) && (end[i] == indexEnd)) || 
			((start[i] == indexEnd) && (end[i] == indexStart)))
		{
			unsigned int midpt = mid[i];
			indexCurrent--; // won't underflow b/c we won't get here unless indexCurrent is at least 1 (from for loop)
			start[i] = start[indexCurrent];
			end[i] = end[indexCurrent];
			mid[i] = mid[indexCurrent];
			return midpt;
		}
	}

	// no vert found, make a new one

	start[indexCurrent] = indexStart;
	end[indexCurrent] = indexEnd; 
	mid[indexCurrent] = static_cast<unsigned int>( verts.size() );

	verts.push_back(normalize(float3(
		(verts[indexStart].x + verts[indexEnd].x) / 2.0f,
		(verts[indexStart].y + verts[indexEnd].y) / 2.0f,
		(verts[indexStart].z + verts[indexEnd].z) / 2.0f)));

	unsigned int midpt = mid[indexCurrent++];
	return midpt;
} 

// fixme: (tony) Change this to ChangeWindingOrder when there is a good test case rendering
static void subdivide(std::vector<float3>& verts, std::vector<unsigned int>& indices, unsigned int& numEdges, bool ccw)
{
	// reserve space for more faces
	verts.reserve( verts.size() + 2*numEdges );

	numEdges = static_cast<unsigned int>(2*verts.size() + indices.size());
	std::vector<unsigned int> start( numEdges );
	std::vector<unsigned int> end( numEdges );
	std::vector<unsigned int> mid( numEdges );
	std::vector<unsigned int> oldIndices( indices );

	indices.resize( 4 * indices.size() );

	unsigned int indexFace = 0;

	// abstraction for winding order (o = order)
	unsigned int o[2] = { 1, 2 };
	if (ccw)
	{
		o[0] = 2;
		o[1] = 1;
	}

	unsigned int indexCurrent = 0;
	const unsigned int numFaces = static_cast<unsigned int>(oldIndices.size() / 3);
	for (unsigned int i = 0; i < numFaces; i++) 
	{ 
		unsigned int a = oldIndices[3*i]; 
		unsigned int b = oldIndices[3*i+1]; 
		unsigned int c = oldIndices[3*i+2]; 

		unsigned int abMidpoint = midpoint( b, a, indexCurrent, start, end, mid, verts );
		unsigned int bcMidpoint = midpoint( c, b, indexCurrent, start, end, mid, verts );
		unsigned int caMidpoint = midpoint( a, c, indexCurrent, start, end, mid, verts );

		indices[3*indexFace] = a;
		indices[3*indexFace+o[0]] = abMidpoint;
		indices[3*indexFace+o[1]] = caMidpoint;
		indexFace++;
		indices[3*indexFace] = caMidpoint;
		indices[3*indexFace+o[0]] = abMidpoint;
		indices[3*indexFace+o[1]] = bcMidpoint;
		indexFace++;
		indices[3*indexFace] = caMidpoint;
		indices[3*indexFace+o[0]] = bcMidpoint;
		indices[3*indexFace+o[1]] = c;
		indexFace++;
		indices[3*indexFace] = abMidpoint;
		indices[3*indexFace+o[0]] = b;
		indices[3*indexFace+o[1]] = bcMidpoint;
		indexFace++;
	} 
}

static void tcs(std::vector<float2>& tc, const std::vector<float3>& positions, bool hemisphere)
{
	tc.resize(positions.size());
	tc.clear();
	oFOREACH(const float3& p, positions)
	{
		float phi = acosf(p.z);
		float v = 1.0f - (phi / oPIf);

		// Map from 0 -> 1 rather than 0.5 -> 1 since half the sphere is cut off
		if (hemisphere)
			v = (v - 0.5f) * 2.0f;

		float u = 0.5f;
		if (!oEqual(phi, 0.0f))
		{
			float a = clamp(p.y / sinf(phi), -1.0f, 1.0f);
			u = acosf(a) / (2.0f * oPIf);

			if (p.x > 0.0f)
				u = 1.0f - u;
		}

		tc.push_back(float2(u, v));
	}
}

static void fix_apex_tcs_octahedron(std::vector<float2>& tc)
{
	// Because the apex is a single vert and represents all
	// U texture coords from 0 to 1, make that singularity
	// make a bit more sense by representing the apex with a
	// unique vert for each of the 4 octahedron faces and give
	// it a texture coord that represents the middle of the
	// texture coord range that face uses.

	const float Us[] = 
	{
		0.375f,
		0.625f,
		0.875f,
		0.125f,
	};

	oASSERT(/*"core.geometry", */tc.size() >= 12, "");
	for (int i = 0; i < oCOUNTOF(Us); i++)
	{
		tc[i] = float2(Us[i], tc[i].y);
		tc[i+9] = float2(Us[i], tc[i+9].y);
	}

	tc[8].x = (1.0f);
}

static void fix_apex_tcs_Icosahedron(std::vector<float2>& tc)
{
//	Fatal(/*"core.geometry", */"Icosahedron not yet implemented.");
#if 0
	const float Us[] = 
	{
		0.1f,
		0.3f,
		0.5f,
		0.7f,
		0.9f,
	};
#endif
}

static void fix_seam(std::vector<float2>& tc, unsigned int a, unsigned int b, float threshold)
{
	float A = tc[a].x, B = tc[b].x;
	if ((A - B) > threshold)
	{
		unsigned int smaller = (A < B) ? a : b;
		tc[smaller].x = (tc[smaller].x + 1.0f);
	}
}

static void fix_seam_tcs(std::vector<float2>& tc, const std::vector<unsigned int>& indices, float threshold)
{
	const unsigned int n = static_cast<unsigned int>(indices.size() / 3);
	for (unsigned int i = 0; i < n; i++)
	{
		unsigned int a = indices[3*i];
		unsigned int b = indices[3*i+1];
		unsigned int c = indices[3*i+2];

		fix_seam(tc, a, b, threshold);
		fix_seam(tc, a, c, threshold);
		fix_seam(tc, b, c, threshold);
	}
}

const static float3 sOctahedronVerts[] = 
{
	// 4 copies of the top and bottom apex so that 
	// texture coordinates interpolate better.
	float3( 0.0f, 0.0f, 1.0f ),		// 0 top a
	float3( 0.0f, 0.0f, 1.0f ),		// 1 top b
	float3( 0.0f, 0.0f, 1.0f ),		// 2 top c
	float3( 0.0f, 0.0f, 1.0f ),		// 3 top d

	float3( -1.0f, 0.0f, 0.0f ),	// 4 left
	float3( 0.0f, -1.0f, 0.0f ),	// 5 near
	float3( 1.0f, 0.0f, 0.0f ),		// 6 right
	
	// fixme: (tony) Gah! after all this time
	// I still can't get this texturing right...
	// so hack it with a redundant vert.
	float3( 0.0f, 1.0f, 0.0f ),		// 7 far
	float3( 0.0f, 1.0f, 0.0f ),		// 8 far 2

	float3( 0.0f, 0.0f, -1.0f ),	// 9 bottom a
	float3( 0.0f, 0.0f, -1.0f ),	//10 bottom b
	float3( 0.0f, 0.0f, -1.0f ),	//11 bottom c
	float3( 0.0f, 0.0f, -1.0f ),	//12 bottom d
};

const static unsigned int sOctahedronIndices[] = 
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

const static unsigned int sOctahedronNumVerts = oCOUNTOF(sOctahedronVerts);
const static unsigned int sOctahedronNumFaces = 8;
const static unsigned int sOctahedronNumEdges = 12;

// The problem with these is that the coord formula produces a flat edge
// at z = -+1. I need a single vert at that location for proper cylinder
// texture mapping.
// http://en.wikipedia.org/wiki/Icosahedron
// http://www.classes.cs.uchicago.edu/archive/2003/fall/23700/docs/handout-04.pdf


// fixme: (tony) Find a rotation that produces a single vert at z = +1 and another single vert at z = -1


// OK I hate math, but here we go:

// Golden ratio = (1 + sqrt(5)) / 2 ~= 1.6180339887f

// Golden rect is 1:G

/*

  ------
  |\   |
2G|a\  |
A |  \h|
  |  b\|
  ------
   2 O

a = asin( 1/G )
b = asin( G )

so calc icosa from books, then rotate by this amount on
one axis to get points at -+Z rather than edges

*/

static const float T = 1.6180339887f; // golden ratio (1+sqrt(5))/2

// http://www.classes.cs.uchicago.edu/archive/2003/fall/23700/docs/handout-04.pdf
const static float3 sIcosahedronVerts[] = 
{
	float3(T, 1.0f, 0.0f),
	float3(-T, 1.0f, 0.0f),
	float3(T, -1.0f, 0.0f),
	float3(-T, -1.0f, 0.0f),
	float3(1.0f, 0.0f, T),
	float3(1.0f, 0.0f, -T),
	float3(-1.0f, 0.0f, T),
	float3(-1.0f, 0.0f, -T),
	float3(0.0f, T, 1.0f),
	float3(0.0f, -T, 1.0f),
	float3(0.0f, T, -1.0f),
	float3(0.0f, -T, -1.0f),
};

const static unsigned int sIcosahedronIndices[] =
{
	0,8,4, 0,5,10, 2,4,9, 2,11,5, 1,6,8, 
	1,10,7, 3,9,6, 3,7,11, 0,10,8, 1,8,10, 
	2,9,11, 3,11,9, 4,2,0, 5,0,2, 6,1,3, 
	7,3,1, 8,6,4, 9,4,6, 10,5,7, 11,7,5,
};

const static unsigned int sIcosahedronNumVerts = oCOUNTOF(sIcosahedronVerts);
const static unsigned int sIcosahedronNumFaces = 20;
const static unsigned int sIcosahedronNumEdges = 30;

} // namespace SphereDetails

errno_t CreateSphere(const GEOMETRY_SPHERE_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry)
{
	if (!_pDesc ||!_pLayout || !_pGeometry) return EINVAL;

	static const GEOMETRY_LAYOUT sSupportedLayout = { true, true, true, true, true };
	if (!IsSupported(&sSupportedLayout, _pLayout))
	{
		Err(/*"core.geometry", */"Invalid layout specified");
		return EINVAL;
	}

	if (_pDesc->FaceType == GEOMETRY_FACE_OUTLINE && (_pLayout->Normals || _pLayout->Tangents || _pLayout->Texcoords))
	{
		Err(/*"core.geometry", */"Lines aren't compatible with normals tangents or texcoords.");
		return EINVAL;
	}

	if (_pDesc->Icosahedron && _pDesc->Divide != 0)
	{
		Err(/*"core.geometry", */"Icosahedron subdividing not yet implemented.");
		return ENYI;
	}

	if (_pDesc->Icosahedron && _pLayout->Texcoords)
	{
		Err(/*"core.geometry", */"Icosahedron texcoords not yet implemented.");
		return ENYI;
	}

	const float3* srcVerts = _pDesc->Icosahedron ? SphereDetails::sIcosahedronVerts : SphereDetails::sOctahedronVerts;
	const unsigned int* srcIndices = _pDesc->Icosahedron ? SphereDetails::sIcosahedronIndices : SphereDetails::sOctahedronIndices;
	unsigned int numVerts = _pDesc->Icosahedron ? SphereDetails::sIcosahedronNumVerts : SphereDetails::sOctahedronNumVerts;
	unsigned int numFaces = _pDesc->Icosahedron ? SphereDetails::sIcosahedronNumFaces : SphereDetails::sOctahedronNumFaces;
	unsigned int numEdges = _pDesc->Icosahedron ? SphereDetails::sIcosahedronNumEdges : SphereDetails::sOctahedronNumEdges;

	_pGeometry->Clear();
	_pGeometry->PrimitiveType = GEOMETRY_PRIMITIVE_TRILIST;

	_pGeometry->Positions.assign(srcVerts, srcVerts + numVerts);
	_pGeometry->Indices.assign(srcIndices, srcIndices + 3*numFaces);

	if (_pDesc->FaceType == GEOMETRY_FACE_SOLID_CW)
		ChangeWindingOrder(_pGeometry->Indices, 0);

	for (unsigned int i = 0; i < _pDesc->Divide; i++)
		SphereDetails::subdivide(_pGeometry->Positions, _pGeometry->Indices, numEdges, _pDesc->FaceType == GEOMETRY_FACE_SOLID_CW);

	if (!_pDesc->Icosahedron)
	{
		if (_pDesc->Hemisphere)
			Clip(oPlanef(float3::zAxis(), 0.0f), false, _pGeometry);

		oFOREACH(float3& p, _pGeometry->Positions)
			p = (p * _pDesc->Bound.radius()) + _pDesc->Bound.position();

		if (_pDesc->FaceType == GEOMETRY_FACE_OUTLINE)
		{
			if (_pLayout->Normals || _pLayout->Tangents) return EINVAL;
			if (_pDesc->Hemisphere) return ENYI;

			_pGeometry->PrimitiveType = GEOMETRY_PRIMITIVE_TRILIST;
		
			std::vector<std::pair<unsigned int, unsigned int> > edges;
			CalculateEdges(edges, _pGeometry->GetNumVertices(), _pGeometry->Indices);

			_pGeometry->Indices.clear();
			_pGeometry->Indices.reserve(edges.size() * 2);
			for (size_t i = 0; i < edges.size(); i++)
			{
				_pGeometry->Indices.push_back(edges[i].first);
				_pGeometry->Indices.push_back(edges[i].second);
			}
		}
	}

	if (_pLayout->Texcoords)
	{
		SphereDetails::tcs(_pGeometry->Texcoords, _pGeometry->Positions, _pDesc->Hemisphere);
		if (_pDesc->Icosahedron)
			SphereDetails::fix_apex_tcs_Icosahedron(_pGeometry->Texcoords);
		else
			SphereDetails::fix_apex_tcs_octahedron(_pGeometry->Texcoords);

		SphereDetails::fix_seam_tcs(_pGeometry->Texcoords, _pGeometry->Indices, 0.85f);
	}

	if (_pLayout->Normals)
	{
		_pGeometry->Normals.resize(_pGeometry->GetNumVertices());
		std::vector<float3>::iterator it = _pGeometry->Positions.begin();
		oFOREACH(float3& n, _pGeometry->Normals)
			n = normalize(*it++);
	}

	if (_pLayout->Tangents)
		CalculateTangents(_pGeometry, 0);

	if (_pLayout->Colors)
		_pGeometry->Colors.assign(_pGeometry->GetNumVertices(), _pDesc->Color);

	return ESUCCESS;
}

namespace CylinderDetails
{

static void FillIndices(std::vector<unsigned int>& _Indices, unsigned int _Facet, unsigned int _BaseVertexIndex, GEOMETRY_FACE_TYPE __Facetype)
{
	unsigned int numEvens = (_Facet + 1) / 2;
	unsigned int oddsStartI = _Facet - 1 - (_Facet & 0x1);
	unsigned int numOdds = _Facet / 2 - 1;

	unsigned int cwoStartIndex = static_cast<unsigned int>(_Indices.size());

	for (unsigned int i = 1; i < numEvens; i++)
	{
		_Indices.push_back(_BaseVertexIndex + i*2);
		_Indices.push_back(_BaseVertexIndex + (i-1)*2);
		_Indices.push_back(_BaseVertexIndex + i*2 + _Facet);

		_Indices.push_back(_BaseVertexIndex + (i-1)*2);
		_Indices.push_back(_BaseVertexIndex + (i-1)*2 + _Facet);
		_Indices.push_back(_BaseVertexIndex + i*2 + _Facet);
	}

	// Transition from even to odd
	_Indices.push_back(_BaseVertexIndex + _Facet-1);
	_Indices.push_back(_BaseVertexIndex + _Facet-2);
	_Indices.push_back(_BaseVertexIndex + _Facet-1 + _Facet);
	
	_Indices.push_back(_BaseVertexIndex + _Facet-2);
	_Indices.push_back(_BaseVertexIndex + _Facet-2 + _Facet);
	_Indices.push_back(_BaseVertexIndex + _Facet-1 + _Facet);

	if (_Facet & 0x1)
	{
		std::swap(*(_Indices.end()-5), *(_Indices.end()-4));
		std::swap(*(_Indices.end()-2), *(_Indices.end()-1));
	}
	
	for (unsigned int i = oddsStartI, k = 0; k < numOdds; i -= 2, k++)
	{
		_Indices.push_back(_BaseVertexIndex + i);
		_Indices.push_back(_BaseVertexIndex + i + _Facet);
		_Indices.push_back(_BaseVertexIndex + i - 2);

		_Indices.push_back(_BaseVertexIndex + i - 2);
		_Indices.push_back(_BaseVertexIndex + i + _Facet);
		_Indices.push_back(_BaseVertexIndex + i - 2 + _Facet);
	}

	// Transition from last odd back to 0
	_Indices.push_back(_BaseVertexIndex);
	_Indices.push_back(_BaseVertexIndex + 1);
	_Indices.push_back(_BaseVertexIndex + _Facet);

	_Indices.push_back(_BaseVertexIndex + _Facet);
	_Indices.push_back(_BaseVertexIndex + 1);
	_Indices.push_back(_BaseVertexIndex + 1 + _Facet);

	if (__Facetype == GEOMETRY_FACE_SOLID_CW)
		ChangeWindingOrder(_Indices, cwoStartIndex);
}

} // namespace CylinderDetails

errno_t CreateCylinder(const GEOMETRY_CYLINDER_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry)
{
	if (!_pDesc ||!_pLayout || !_pGeometry) return EINVAL;

	// Tangents would be supported if texcoords were supported
	// Texcoord support is a bit hard b/c it has the same wrap-around problem spheres have.
	// This means we need to duplicate vertices along the seams and assign a 
	// different texcoord. It's a bit wacky because the circle doesn't align on 0,
	// so really the next steps are:
	// 1. Get Circle to have a vertex on (0,1,0)
	// 2. Be able to duplicate that vert and reindex triangles on the (0.0001,0.9999,0) 
	//    side
	// 3. Also texture a circle.
	static const GEOMETRY_LAYOUT sSupportedLayout = { true, false, false, false, true };
	if (!IsSupported(&sSupportedLayout, _pLayout))
	{
		Err(/*"core.geometry", */"Invalid layout specified");
		return EINVAL;
	}

	if (_pDesc->FaceType == GEOMETRY_FACE_OUTLINE && _pLayout->Normals)
		return EINVAL;
	
	if (_pDesc->Facet < 3) return EINVAL;
	if (_pDesc->Divide == 0) return EINVAL;

	_pGeometry->Clear();

	const float fStep = _pDesc->Height / static_cast<float>(_pDesc->Divide);

	if (_pDesc->FaceType == GEOMETRY_FACE_OUTLINE)
	{
		_pGeometry->PrimitiveType = GEOMETRY_PRIMITIVE_LINELIST;

		GEOMETRY_CIRCLE_DESC c;
		c.FaceType = _pDesc->FaceType;
		c.Facet = _pDesc->Facet;
		c.Color = _pDesc->Color;

		for (unsigned int i = 0; i <= _pDesc->Divide; i++)
		{
			c.Radius = lerp(_pDesc->Radius0, _pDesc->Radius1, i/static_cast<float>(_pDesc->Divide));
			errno_t err = CircleDetails::CreateCircleInternal(&c, _pLayout, _pGeometry, static_cast<unsigned int>(_pGeometry->GetNumVertices()), false, i * fStep, true);
			if (err) return err;
		}

		// Now add lines along _pDesc->Facet points

		const unsigned int nVertsInOneCircle = static_cast<unsigned int>(_pGeometry->GetNumVertices() / (_pDesc->Divide+1));
		for (unsigned int i = 0; i < nVertsInOneCircle; i++)
		{
			_pGeometry->Indices.push_back(i);
			_pGeometry->Indices.push_back(i + nVertsInOneCircle * _pDesc->Divide);
		}
	}

	else
	{
		_pGeometry->PrimitiveType = GEOMETRY_PRIMITIVE_TRILIST;
		CircleDetails::FillPositions(_pGeometry->Positions, _pDesc->Radius0, _pDesc->Facet, 0.0f);
		for (unsigned int i = 1; i <= _pDesc->Divide; i++)
		{
			CircleDetails::FillPositions(_pGeometry->Positions, lerp(_pDesc->Radius0, _pDesc->Radius1, i/static_cast<float>(_pDesc->Divide)), _pDesc->Facet, i * fStep );
			CylinderDetails::FillIndices(_pGeometry->Indices, _pDesc->Facet, (i-1) * _pDesc->Facet, _pDesc->FaceType);
		}

		if (_pLayout->Texcoords)
		{
			_pGeometry->Texcoords.resize(_pGeometry->GetNumVertices());

			size_t i = 0;
			oFOREACH(float2& c, _pGeometry->Texcoords)
			{
				const float3& p = _pGeometry->Positions[i++];

				float x = ((p.x + _pDesc->Radius0) / (2.0f*_pDesc->Radius0));
				float v = p.z / _pDesc->Height;

				if (p.y <= 0.0f)
					c = float2(x * 0.5f, v);
				else
					c = float2(1.0f - (x * 0.5f), v);
			}
		}

		if (_pDesc->IncludeBase)
		{
			GEOMETRY_CIRCLE_DESC c;
			c.FaceType = GetOppositeWindingOrder(_pDesc->FaceType);
			c.Color = _pDesc->Color;
			c.Facet = _pDesc->Facet;
			c.Radius = _pDesc->Radius0;
			
			GEOMETRY_LAYOUT layout = *_pLayout;
			layout.Normals = false; // normals get created later
			
			errno_t err = CircleDetails::CreateCircleInternal(&c, &layout, _pGeometry, static_cast<unsigned int>(_pGeometry->GetNumVertices()), false, 0.0f, true);
			if (err) return err;

			c.FaceType = _pDesc->FaceType;
			c.Radius = _pDesc->Radius1;
			err = CircleDetails::CreateCircleInternal(&c, &layout, _pGeometry, static_cast<unsigned int>(_pGeometry->GetNumVertices()), false, _pDesc->Height, true);
			if (err) return err;
			
			//fixme: implement texcoord generation
			if (_pLayout->Texcoords)
				_pGeometry->Texcoords.resize(_pGeometry->GetNumVertices());
		}

		if (_pLayout->Normals)
			CalculateVertexNormals(_pGeometry, _pDesc->FaceType == GEOMETRY_FACE_SOLID_CCW);

		if (_pLayout->Tangents)
			CalculateTangents(_pGeometry, 0);
	}

	if (_pLayout->Colors)
		_pGeometry->Colors.assign(_pGeometry->GetNumVertices(), _pDesc->Color);

	return ESUCCESS;
}

errno_t CreateCone(const GEOMETRY_CONE_DESC* _pDesc, const GEOMETRY_LAYOUT* _pLayout, oGeometry* _pGeometry)
{
	GEOMETRY_CYLINDER_DESC desc;
	desc.FaceType = _pDesc->FaceType;
	desc.Divide = _pDesc->Divide;
	desc.Facet = _pDesc->Facet;
	desc.Radius0 = _pDesc->Radius;
	desc.Radius1 = 0.0f;
	desc.Height = _pDesc->Height;
	desc.Color = _pDesc->Color;
	desc.IncludeBase = _pDesc->IncludeBase;

	return CreateCylinder(&desc, _pLayout, _pGeometry);
}

errno_t CreateGeometryFromOBJ(const char* _pOBJFileString, const GEOMETRY_LAYOUT* _pLoadLayout, oGeometry* _pGeometry)
{
	if (!_pOBJFileString ||!_pLoadLayout || !_pGeometry) return EINVAL;

	static const GEOMETRY_LAYOUT sSupportedLayout = { true, true, false, true, false };
	if (!IsSupported(&sSupportedLayout, _pLoadLayout))
	{
		Err(/*"core.geometry", */"Invalid layout specified");
		return EINVAL;
	}

	size_t groupCount = 0;
	char type = 0;
	float x = 0.0f, y = 0.0f, z = 0.0f;
	const char* r = _pOBJFileString;
	while (*(int*)r)
	{
		switch (*r)
		{
			case 'v':
			{
					r += sscanf_s(r, "v%c %f %f %f\n", &type, &x, &y, &z);
					switch (type)
					{
						case ' ': _pGeometry->Positions.push_back(float3(x,y,z)); break;
						case 'n': _pGeometry->Normals.push_back(float3(x,y,z)); break;
						case 't': _pGeometry->Texcoords.push_back(float2(x,y)); break;
//						default: Assume(0);
					}

					break;
			}

			case 'g':
				groupCount++;
				if (groupCount > 1)
				{
					Err(/*"core.geometry", */"Multi-section geometry not yet supported.");
					return ENYI;
				}

				break;

			case 'f':
				break;
		}

		r += strcspn(r, "\n");
	}

	return ENYI;
}
