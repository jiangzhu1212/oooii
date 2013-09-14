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
#include <oBasis/oGeometry.h>
#include <oStd/algorithm.h>
#include <oBasis/oError.h>
#include <oStd/for.h>
#include <oBasis/oMath.h>
#include <oBasis/oMeshUtil.h>
#include <oBasis/oOBJ.h>
#include <oBasis/oRefCount.h>

#define GEO_CONSTRUCT(fnName, SupportedLayout, InputLayout, FaceType) do { bool success = false; oCONSTRUCT(_ppGeometry, oGeometry_Impl(fnName, SupportedLayout, InputLayout, FaceType, &success)); } while (false); \
	oGeometry_Impl* pGeometry = (oGeometry_Impl*)*_ppGeometry; \
	if (!pGeometry) return false;

static const float oVERYSMALL = 0.000001f;

// we're left-handed at the moment
static const float3 Z_FACING_EYE(0.0f, 0.0f, -1.0f);

// Utility to find or create midpoints of triangles in a mesh
// Basically the main inputs are two indices that define an edge.
// The other parameters are caching values for the results of what
// happened of either finding a prior midpoint or having to create
// a new vertex. This function is meant to be called iteratively 
// while building new midpoints of triangles in a mesh for the 
// purposes of tessellation. _IndexCurrent should be initialized 
// to 0 and be left alone. _Start _End and _Mid should be able to
// accommodate as many edges as are in the mesh. _Positions should 
// be pre-allocated to the maximum number of possible new 
// midpoints. For example if you pass just one triangle (3 positions)
// and tessellate it, then 3 more midpoints will be generated, so
// positions should be allocated to 6.
//
// _Index0: first index of a triangle's edge for which a midpoint is to be found
// _Index1: second index of a triangle's edge for which a midpoint is to be found
// _IndexCurrent: This function is called iteratively, so _IndexCurrent keeps 
//                track of the last index where data was pulled from.
// _pStart: First half of an edge list structure
// _pEnd: Last half of an edge list structure
// _pMid: Keeps a log of the point in the middle of an edge
//        whether it was found or created
// _Positions: vertex positions. Sometimes a new vertex needs to be created
//             for a midpoint, so this will get updated.
// <other vertex attributes>: If the vectors specified are empty, no work is done
// For any non-empty vectors, new vertices get their values from lerping the 
// edge endpoints.
template<typename T> static unsigned int oFindOrCreateMidpointT(unsigned int _Index0, unsigned int _Index1, unsigned int& _IndexCurrent, unsigned int* _pStart, unsigned int* _pEnd, unsigned int* _pMid, std::vector<TVEC3<T>>& _Positions, std::vector<TVEC3<T>>& _Normals, std::vector<TVEC4<T>>& _Tangents, std::vector<TVEC3<T>>& _Texcoords0, std::vector<TVEC3<T>>& _Texcoords1, std::vector<oStd::color>& _Colors, std::vector<unsigned int>& _ContinuityIDs)
{
	/** <citation
		usage="Adaptation" 
		reason="This is actually cleanup of the original that I now apply to more than just sphere tessellation." 
		author="Cedric Laugerotte"
		description="http://student.ulb.ac.be/~claugero/sphere/index.html"
		license="*** Assumed Public Domain ***"
		licenseurl="http://student.ulb.ac.be/~claugero/sphere/index.html"
		modification="More parameterization and separate from original assume-a-sphere algo"
	/>*/

	for (unsigned int i = 0; i < _IndexCurrent; i++)
	{
		if (((_pStart[i] == _Index0) && (_pEnd[i] == _Index1)) || ((_pStart[i] == _Index1) && (_pEnd[i] == _Index0)))
		{
			unsigned int midpoint = _pMid[i];
			_IndexCurrent--; // won't underflow b/c we won't get here unless _IndexCurrent is at least 1 (from for loop)
			_pStart[i] = _pStart[_IndexCurrent];
			_pEnd[i] = _pEnd[_IndexCurrent];
			_pMid[i] = _pMid[_IndexCurrent];
			return midpoint;
		}
	}

	// no vert found, make a new one

	_pStart[_IndexCurrent] = _Index0;
	_pEnd[_IndexCurrent] = _Index1; 
	_pMid[_IndexCurrent] = static_cast<unsigned int>(_Positions.size());

	_Positions.push_back(lerp(_Positions[_Index0], _Positions[_Index1], 0.5f));
	if (!_Normals.empty())
		_Normals.push_back(normalize(_Normals[_Index0] + _Normals[_Index1]));
	if (!_Tangents.empty())
	{
		oASSERT(_Tangents[_Index0].w == _Tangents[_Index1].w, "");
		_Tangents.push_back(float4(normalize(_Tangents[_Index0].xyz() + _Tangents[_Index1].xyz()), _Tangents[_Index0].w));
	}
	if (!_Texcoords0.empty())
		_Texcoords0.push_back(lerp(_Texcoords0[_Index0], _Texcoords0[_Index1], 0.5f));
	if (!_Texcoords1.empty())
		_Texcoords1.push_back(lerp(_Texcoords0[_Index1], _Texcoords1[_Index1], 0.5f));
	if (!_Colors.empty())
		_Colors.push_back(lerp(_Colors[_Index0], _Colors[_Index1], 0.5f));
	if (!_ContinuityIDs.empty())
	{
		oASSERT(_ContinuityIDs[_Index0] == _ContinuityIDs[_Index1], "");
		_ContinuityIDs.push_back(_ContinuityIDs[_Index0]);
	}

	unsigned int midpoint = _pMid[_IndexCurrent++];
	return midpoint;
} 

// @oooii-tony: TODO: subdivide is fairly generic. Clean it
// up to a higher-level static function in this same .cpp,
// but try to use it for:
// 4. CreateFrustum(): support a Divide param in the desc
// 5. CreateCylinder(): for the tube part, do the simple thing and tessellate

// Subdivides each triangle in a mesh by finding the midpoints of each edge and
// creating a new point there, generating 4 new triangles to replace the prior
// triangle. This does not duplicate points for triangles that share an edge.
// _NumEdges: The valid number of edges in the specified mesh must be passed.
//            This value will be filled with the new edge count after 
//            subdivision. In this way oSubdivideMesh() can be called 
//            recursively up to the desired subdivision level.
// _Indices:  List of indices. This will be modified during subdivision
// _Positions, _Normals, _Tangents, _Texcoords0, _Texcoords1, _Colors
template<typename T> static void oSubdivideMeshT(unsigned int& _NumEdges, std::vector<unsigned int>& _Indices, std::vector<TVEC3<T>>& _Positions, std::vector<TVEC3<T>>& _Normals, std::vector<TVEC4<T>>& _Tangents, std::vector<TVEC3<T>>& _Texcoords0, std::vector<TVEC3<T>>& _Texcoords1, std::vector<oStd::color>& _Colors, std::vector<unsigned int>& _ContinuityIDs)
{
	// reserve space for more faces
	_Positions.reserve(_Positions.size() + 2 * _NumEdges);

	_NumEdges = static_cast<unsigned int>(2 * _Positions.size() + _Indices.size());
	std::vector<unsigned int> start(_NumEdges);
	std::vector<unsigned int> end(_NumEdges);
	std::vector<unsigned int> mid(_NumEdges);
	std::vector<unsigned int> oldIndices(_Indices);

	_Indices.resize(4 * _Indices.size());

	unsigned int indexFace = 0;

	unsigned int indexCurrent = 0;
	const unsigned int numFaces = static_cast<unsigned int>(oldIndices.size() / 3);
	for (unsigned int i = 0; i < numFaces; i++) 
	{ 
		unsigned int a = oldIndices[3*i]; 
		unsigned int b = oldIndices[3*i+1]; 
		unsigned int c = oldIndices[3*i+2]; 

		unsigned int abMidpoint = oFindOrCreateMidpointT(b, a, indexCurrent, oStd::data(start), oStd::data(end), oStd::data(mid), _Positions, _Normals, _Tangents, _Texcoords0, _Texcoords1, _Colors, _ContinuityIDs);
		unsigned int bcMidpoint = oFindOrCreateMidpointT(c, b, indexCurrent, oStd::data(start), oStd::data(end), oStd::data(mid), _Positions, _Normals, _Tangents, _Texcoords0, _Texcoords1, _Colors, _ContinuityIDs);
		unsigned int caMidpoint = oFindOrCreateMidpointT(a, c, indexCurrent, oStd::data(start), oStd::data(end), oStd::data(mid), _Positions, _Normals, _Tangents, _Texcoords0, _Texcoords1, _Colors, _ContinuityIDs);

		_Indices[3*indexFace] = a;
		_Indices[3*indexFace+1] = abMidpoint;
		_Indices[3*indexFace+2] = caMidpoint;
		indexFace++;
		_Indices[3*indexFace] = caMidpoint;
		_Indices[3*indexFace+1] = abMidpoint;
		_Indices[3*indexFace+2] = bcMidpoint;
		indexFace++;
		_Indices[3*indexFace] = caMidpoint;
		_Indices[3*indexFace+1] = bcMidpoint;
		_Indices[3*indexFace+2] = c;
		indexFace++;
		_Indices[3*indexFace] = abMidpoint;
		_Indices[3*indexFace+1] = b;
		_Indices[3*indexFace+2] = bcMidpoint;
		indexFace++;
	} 
}

static void oSubdivideMesh(unsigned int& _NumEdges, std::vector<unsigned int>& _Indices, std::vector<float3>& _Positions, std::vector<float3>& _Normals, std::vector<float4>& _Tangents, std::vector<float3>& _Texcoords0, std::vector<float3>& _Texcoords1, std::vector<oStd::color>& _Colors, std::vector<unsigned int>& _ContinuityIDs)
{
	oSubdivideMeshT(_NumEdges, _Indices, _Positions, _Normals, _Tangents, _Texcoords0, _Texcoords1, _Colors, _ContinuityIDs);
}

static bool IsSupported(const char* _CreateFunctionName, const oGeometry::LAYOUT& _Supported, const oGeometry::LAYOUT& _Input, oGeometry::FACE_TYPE _FaceType)
{
	if (!_Supported.Positions)
		return oErrorSetLast(std::errc::invalid_argument, "%s() is not validly implemented", oSAFESTRN(_CreateFunctionName));

	if (_FaceType == oGeometry::OUTLINE && (_Input.Normals || _Input.Tangents || _Input.Texcoords))
		return oErrorSetLast(std::errc::invalid_argument, "Outline face types do not support normals, tangents, or texcoords");

	if (!_Supported.Normals && _Input.Normals)
		return oErrorSetLast(std::errc::invalid_argument, "%s() does not support normals", oSAFESTRN(_CreateFunctionName));

	if (!_Supported.Tangents && _Input.Tangents)
		return oErrorSetLast(std::errc::invalid_argument, "%s() does not support tangents", oSAFESTRN(_CreateFunctionName));

	if (!_Supported.Texcoords && _Input.Texcoords)
		return oErrorSetLast(std::errc::invalid_argument, "%s() does not support texcoords", oSAFESTRN(_CreateFunctionName));

	if (!_Supported.Colors && _Input.Colors)
		return oErrorSetLast(std::errc::invalid_argument, "%s() does not support colors", oSAFESTRN(_CreateFunctionName));

	if (!_Supported.ContinuityIDs && _Input.ContinuityIDs)
		return oErrorSetLast(std::errc::invalid_argument, "%s() does not support continuity IDs", oSAFESTRN(_CreateFunctionName));

	if (!_Input.Positions)
		return oErrorSetLast(std::errc::invalid_argument, "Positions must be specified as true in layout");

	if (_Input.Tangents && (!_Input.Normals || !_Input.Texcoords))
		return oErrorSetLast(std::errc::invalid_argument, "Invalid layout: Tangents require both normals and texcoords");

	return true;
}

static unsigned int CalcNumPrimitives(oGeometry::PRIMITIVE_TYPE _PrimitiveType, size_t _NumIndices, size_t _NumVertices)
{
	size_t n = _NumIndices;
	switch (_PrimitiveType)
	{
		case oGeometry::POINTLIST: n = _NumVertices; break;
		case oGeometry::LINELIST: n /= 2; break;
		case oGeometry::LINESTRIP: n--; break;
		case oGeometry::TRILIST: n /= 3; break;
		case oGeometry::TRISTRIP: n -= 2; break;
		default: n = 0; break;
	}
	
	return static_cast<unsigned int>(n);
}

static float GetNormalSign(oGeometry::FACE_TYPE type)
{
	// This geo lib was brought up as CCW, so invert some values for CW systems

	switch (type)
	{
		case oGeometry::FRONT_CW: return -1.0f;
		case oGeometry::FRONT_CCW: return 1.0f;
		default: break;
	}

	return 0.0f;
}

static oGeometry::FACE_TYPE GetOppositeWindingOrder(oGeometry::FACE_TYPE type)
{
	switch (type)
	{
		case oGeometry::FRONT_CW: return oGeometry::FRONT_CCW;
		case oGeometry::FRONT_CCW: return oGeometry::FRONT_CW;
		default: break;
	}

	return type;
}

void ChangeWindingOrder(size_t _NumberOfIndices, unsigned int* _pIndices, unsigned int _BaseIndexIndex)
{
	oASSERT((_BaseIndexIndex % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded");
	oASSERT((_NumberOfIndices % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded");
	for (size_t i = _BaseIndexIndex; i < _NumberOfIndices; i += 3)
		std::swap(_pIndices[i+1], _pIndices[i+2]);
}

template<typename T, typename A> inline void ChangeWindingOrder(std::vector<T, A>& _Indices, unsigned int _BaseIndexIndex) { ChangeWindingOrder(_Indices.size(), &_Indices[0], _BaseIndexIndex); }

struct oGeometry_Impl : public oGeometry
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGeometry);

	oGeometry_Impl(const char* _CallingFunction, const LAYOUT& _Supported, const LAYOUT& _Input, oGeometry::FACE_TYPE _FaceType, bool* _pSuccess)
		: FaceType(_FaceType)
		, PrimitiveType(_FaceType == oGeometry::OUTLINE ? oGeometry::LINELIST : oGeometry::TRILIST)
	{
		*_pSuccess = IsSupported(_CallingFunction, _Supported, _Input, _FaceType);
	}

	void GetDesc(DESC* _pDesc) const override;
	void Transform(const float4x4& _Matrix) override;
	bool Map(MAPPED* _pMapped) override;
	void Unmap() override;
	bool MapConst(CONST_MAPPED* _pMapped) const override;
	void UnmapConst() const override;

	inline void Clear()
	{
		Bounds.clear();
		Indices.clear();
		Positions.clear();
		Normals.clear();
		Tangents.clear();
		Texcoords.clear();
		Colors.clear();
		ContinuityIDs.clear();
	}

	inline void Reserve(size_t _NumIndices, size_t _NumVertices)
	{
		Indices.reserve(_NumIndices);
		Positions.reserve(_NumVertices);
		Normals.reserve(_NumVertices);
		Tangents.reserve(_NumVertices);
		Texcoords.reserve(_NumVertices);
		ContinuityIDs.reserve(_NumVertices);
	}

	void Transform(const float4x4& _Matrix, unsigned int _BaseVertexIndex);

	inline void CalcVertexNormals(bool _CCW)
	{
		Normals.resize(Positions.size());
		oCalcVertexNormals(oStd::data(Normals), oStd::data(Indices), Indices.size(), oStd::data(Positions), Positions.size(), _CCW);
	}

	inline void CalcTangents(unsigned int _BaseIndexIndex)
	{
		Tangents.resize(Positions.size());
		oCalcTangents(oStd::data(Tangents), &Indices[_BaseIndexIndex], Indices.size() - _BaseIndexIndex, oStd::data(Positions), oStd::data(Normals), oStd::data(Texcoords), Positions.size());
	}

	inline void SetColor(oStd::color _Color, unsigned int _BaseVertexIndex)
	{
		Colors.resize(Positions.size());
		for (size_t i = _BaseVertexIndex; i < Colors.size(); i++)
			Colors[i] = _Color;
	}

	inline void SetContinuityID(unsigned int _ID, unsigned int _BaseVertexIndex = 0, unsigned int _NumVertices = oInvalid)
	{
		ContinuityIDs.resize(Positions.size());
		const size_t count = __min(_NumVertices, ContinuityIDs.size() - _BaseVertexIndex);
		for (size_t i = _BaseVertexIndex; i < count; i++)
			ContinuityIDs[i] = _ID;
	}

	inline void CalculateBounds()
	{
		float3 m, M;
		oCalcMinMaxPoints(oStd::data(Positions), Positions.size(), &m, &M);
		Bounds.Min = m;
		Bounds.Max = M;
	}

	inline void PruneUnindexedVertices()
	{
		size_t newNumVerts = 0;
		oPruneUnindexedVertices(oStd::data(Indices), Indices.size(), oStd::data(Positions), oStd::data(Normals), oStd::data(Tangents), oStd::data(Texcoords), (float3*)0, (unsigned int*)oStd::data(Colors), Positions.size(), &newNumVerts);
		if (newNumVerts != Positions.size())
		{
			Positions.resize(newNumVerts);
			Normals.resize(newNumVerts);
			Tangents.resize(newNumVerts);
			Texcoords.resize(newNumVerts);
			Colors.resize(newNumVerts);
		}
	}

	inline void Subdivide(unsigned int _Divide, unsigned int _NumEdges)
	{
		std::vector<float3> dummy;
		for (size_t i = 1; i < _Divide; i++)
			oSubdivideMesh(_NumEdges, Indices, Positions, Normals, Tangents, Texcoords, dummy, Colors, ContinuityIDs);
	}

	// Calculates all derived attributes if not already created
	// by prior code.
	inline void Finalize(const LAYOUT& _Layout, const oStd::color& _Color)
	{
		if (FaceType != oGeometry::OUTLINE)
		{
			if (_Layout.Normals && Normals.empty())
				CalcVertexNormals(FaceType == oGeometry::FRONT_CCW);
		
			if (_Layout.Tangents && Tangents.empty())
				CalcTangents(0);
		}

		if (_Layout.Colors)
			SetColor(_Color, 0);

		if (_Layout.ContinuityIDs && ContinuityIDs.empty())
			SetContinuityID(0);

		if (Ranges.empty())
		{
			oGPU_RANGE r;
			r.StartPrimitive = 0;
			r.NumPrimitives = oUInt(Indices.size() / 3);
			r.MaxVertex = oUInt(Positions.size());
			Ranges.push_back(r);
		}

		CalculateBounds();
	}

	void FillMapped(MAPPED* _pMapped) const;

	std::vector<oGPU_RANGE> Ranges;
	std::vector<unsigned int> Indices;
	std::vector<float3> Positions;
	std::vector<float3> Normals;
	std::vector<float4> Tangents;
	std::vector<float3> Texcoords;
	std::vector<oStd::color> Colors;
	std::vector<unsigned int> ContinuityIDs;
	FACE_TYPE FaceType;
	PRIMITIVE_TYPE PrimitiveType;
	oAABoxf Bounds;
	oRefCount RefCount;
	//oConcurrency::shared_mutex Mutex;
};

void oGeometry_Impl::FillMapped(MAPPED* _pMapped) const
{
	oGeometry_Impl* pThis = const_cast<oGeometry_Impl*>(this);

	_pMapped->pRanges = oStd::data(pThis->Ranges);
	_pMapped->pIndices = oStd::data(pThis->Indices);
	_pMapped->pPositions = oStd::data(pThis->Positions);
	_pMapped->pNormals = oStd::data(pThis->Normals);
	_pMapped->pTangents = oStd::data(pThis->Tangents);
	_pMapped->pTexcoords = oStd::data(pThis->Texcoords);
	_pMapped->pColors = oStd::data(pThis->Colors);
}

void oGeometry_Impl::GetDesc(DESC* _pDesc) const
{
	//oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
	oGeometry_Impl* pLockedThis = thread_cast<oGeometry_Impl*>(this); // @oooii-tony: safe because we locked above

	_pDesc->NumRanges = static_cast<unsigned int>(pLockedThis->Ranges.size());
	_pDesc->NumVertices = static_cast<unsigned int>(pLockedThis->Positions.size());
	_pDesc->NumIndices = static_cast<unsigned int>(pLockedThis->Indices.size());
	_pDesc->NumPrimitives = CalcNumPrimitives(PrimitiveType, pLockedThis->Indices.size(), pLockedThis->Positions.size());
	_pDesc->FaceType = FaceType;
	_pDesc->PrimitiveType = PrimitiveType;
	_pDesc->Bounds = pLockedThis->Bounds;
	_pDesc->Layout.Positions = !!oStd::data(pLockedThis->Positions);
	_pDesc->Layout.Normals = !!oStd::data(pLockedThis->Normals);
	_pDesc->Layout.Tangents = !!oStd::data(pLockedThis->Tangents);
	_pDesc->Layout.Texcoords = !!oStd::data(pLockedThis->Texcoords);
	_pDesc->Layout.Colors = !!oStd::data(pLockedThis->Colors);
}

void oGeometry_Impl::Transform(const float4x4& _Matrix)
{
	//oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
	oGeometry_Impl* pLockedThis = thread_cast<oGeometry_Impl*>(this); // @oooii-tony: safe because we locked above
	Bounds.clear();
	pLockedThis->Transform(_Matrix, 0);
}

void oGeometry_Impl::Transform(const float4x4& _Matrix, unsigned int _BaseVertexIndex)
{
	for (size_t i = _BaseVertexIndex; i < Positions.size(); i++)
	{
		Positions[i] = _Matrix * Positions[i];
		oExtendBy(Bounds, Positions[i]);
	}

	float3x3 r = (float3x3)_Matrix;

	for (size_t i = _BaseVertexIndex; i < Normals.size(); i++)
		Normals[i] = normalize(r * Normals[i]);
	for (size_t i = _BaseVertexIndex; i < Tangents.size(); i++)
		Tangents[i] = float4(normalize(r * Tangents[i].xyz()), Tangents[i].w);

// @oooii-jeffrey: attempt to generate the z of TexCoords
// 	for (size_t i = _BaseVertexIndex; i < Texcoords.size(); i++)
// 	{
// 		float3 uv = float3(Texcoords[i].xy(), 0);
// 		float3 uvrot = r * uv;
// 		Texcoords[i].z = uvrot.z;
// 	}
};

bool oGeometry_Impl::Map(MAPPED* _pMapped)
{
	//Mutex.Lock();
	oGeometry_Impl* pLockedThis = thread_cast<oGeometry_Impl*>(this); // @oooii-tony: safe because we locked above
	pLockedThis->FillMapped(_pMapped);
	return true;
}

void oGeometry_Impl::Unmap()
{
	//Mutex.Unlock();
}

bool oGeometry_Impl::MapConst(CONST_MAPPED* _pMapped) const
{
	//Mutex.LockRead();
	oGeometry_Impl* pLockedThis = thread_cast<oGeometry_Impl*>(this); // @oooii-tony: safe because we locked above
	pLockedThis->FillMapped((MAPPED*)_pMapped);
	return true;
}

void oGeometry_Impl::UnmapConst() const
{
	//Mutex.UnlockRead();
}

struct oGeometryFactory_Impl : public oGeometryFactory
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGeometry);

	oGeometryFactory_Impl(bool* _pSuccess) { *_pSuccess = true; }
	bool CreateRect(const RECT_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;
	bool CreateBox(const BOX_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;
	bool CreateFrustum(const FRUSTUM_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;
	bool CreateCircle(const CIRCLE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;
	bool CreateWasher(const WASHER_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;
	bool CreateSphere(const SPHERE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;
	bool CreateCylinder(const CYLINDER_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;
	bool CreateCone(const CONE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;
	bool CreateTorus(const TORUS_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;
	bool CreateTeardrop(const TEARDROP_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;
	bool CreateOBJ(const OBJ_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;
	bool CreateMosaic(const MOSAIC_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry) override;

	oRefCount RefCount;
};

bool oGeometryFactoryCreate(oGeometryFactory** _ppGeometryFactory)
{
	if (!_ppGeometryFactory)
		return oErrorSetLast(std::errc::invalid_argument, "NULL pointer specified");
	bool success = false;
	oCONSTRUCT(_ppGeometryFactory, oGeometryFactory_Impl(&success));
	return success;
}

namespace RectDetails
{
	static void AppendRect(oGeometry_Impl* _pGeometry, const oGeometryFactory::RECT_DESC& _Desc, const oGeometry::LAYOUT& _Layout)
	{
		static const float3 kCorners[4] = 
		{
			float3(0.0f, 0.0f, 0.0f),
			float3(1.0f, 0.0f, 0.0f),
			float3(0.0f, 1.0f, 0.0f),
			float3(1.0f, 1.0f, 0.0f),
		};

		static const unsigned int kOutlineIndices[8] = { 0,1, 2,3, 0,2, 1,3 };
		static const unsigned int kTriIndices[6] = { 0,2,1, 1,2,3 };

		float4x4 m = oCreateScale(float3(_Desc.Width, _Desc.Height, 1.0f));
		if (_Desc.Centered)
			m = oCreateTranslation(float3(-0.5f, -0.5f, 0.0f)) * m;

		size_t baseVertexIndex = _pGeometry->Positions.size();
		oFORI(i, kCorners)
			_pGeometry->Positions.push_back(m * kCorners[i]);

		size_t baseIndexIndex = _pGeometry->Indices.size();
		if (_Desc.FaceType == oGeometry::OUTLINE)
		{
			oFORI(i, kOutlineIndices)
				_pGeometry->Indices.push_back(static_cast<unsigned int>(baseVertexIndex) + kOutlineIndices[i]);
		}

		else
		{
			oFORI(i, kTriIndices)
				_pGeometry->Indices.push_back(static_cast<unsigned int>(baseVertexIndex) + kTriIndices[i]);

			if (_Desc.FaceType == oGeometry::FRONT_CCW)
				ChangeWindingOrder(_pGeometry->Indices, static_cast<unsigned int>(baseIndexIndex));
		}

		if (_Layout.Texcoords)
		{
			oFORI(i, kCorners)
			{
				float3 tc = kCorners[i];
				if (!_Desc.FlipTexcoordV) //We are following Directx standards by default
					tc.y = 1.0f - tc.y;
				_pGeometry->Texcoords.push_back(tc);
			}
		}
	}

} // namespace RectDetails

bool oGeometryFactory_Impl::CreateRect(const RECT_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	static const oGeometry::LAYOUT sSupportedLayout = { true, true, true, true, true, true };
	GEO_CONSTRUCT("CreateRect", sSupportedLayout, _Layout, _Desc.FaceType);

	RectDetails::AppendRect(pGeometry, _Desc, _Layout);

	if (_Desc.FaceType != oGeometry::OUTLINE)
		pGeometry->Subdivide(_Desc.Divide, 6);

	if (_Layout.Normals)
		pGeometry->Normals.assign(pGeometry->Positions.size(), GetNormalSign(_Desc.FaceType) * float3(0.0f, 0.0f, 1.0f));
	if (_Layout.Tangents)
		pGeometry->Tangents.assign(pGeometry->Positions.size(), float4(1.0f, 0.0f, 0.0f, 1.0f));

	pGeometry->Finalize(_Layout, _Desc.Color);
	return true;
}

bool oGeometryFactory_Impl::CreateBox(const BOX_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	static const oGeometry::LAYOUT sSupportedLayout = { true, true, true, true, true, true };
	GEO_CONSTRUCT("CreateBox", sSupportedLayout, _Layout, _Desc.FaceType);

	const float s = GetNormalSign(_Desc.FaceType);
	float3 dim = _Desc.Bounds.size();

	if (_Desc.FaceType == oGeometry::OUTLINE)
	{
		const float4x4 m = oCreateTranslation(float3(_Desc.Bounds.center()));
		float3 positions[] =
		{
			float3(-dim.x/2.0f,-dim.y/2.0f,-dim.z/2.0f), //left bottom back
			float3(dim.x/2.0f,-dim.y/2.0f,-dim.z/2.0f), //right bottom back
			float3(-dim.x/2.0f,-dim.y/2.0f,dim.z/2.0f), //left bottom front
			float3(dim.x/2.0f,-dim.y/2.0f,dim.z/2.0f), //right bottom front
			float3(-dim.x/2.0f,dim.y/2.0f,-dim.z/2.0f), //left top back
			float3(dim.x/2.0f,dim.y/2.0f,-dim.z/2.0f), //right top back
			float3(-dim.x/2.0f,dim.y/2.0f,dim.z/2.0f), //left top front
			float3(dim.x/2.0f,dim.y/2.0f,dim.z/2.0f), //right top front
		};
		oFORI(i, positions)
			pGeometry->Positions.push_back(m*positions[i]);

		unsigned int indices[] = 
		{
			0,1, //bottom
			1,3,
			3,2,
			2,0,
			4,5, //top
			5,7,
			7,6,
			6,4,
			0,4, //sides
			1,5,
			2,6,
			3,7,
		};
		oFORI(i, indices)
			pGeometry->Indices.push_back(indices[i]);
	}
	else
	{
		float4x4 tx[6];
		tx[0] = oCalcPlaneMatrix(float4(-1.0f, 0.0f, 0.0f, s*dim.x/2.0f));
		tx[1] = oCalcPlaneMatrix(float4(1.0f, 0.0f, 0.0f, s*dim.x/2.0f));
		tx[2] = oCalcPlaneMatrix(float4(0.0f, 1.0f, 0.0f, s*dim.y/2.0f));
		tx[3] = oCalcPlaneMatrix(float4(0.0f, -1.0f, 0.0f, s*dim.y/2.0f));
		tx[4] = oCalcPlaneMatrix(float4(0.0f, 0.0f, 1.0f, s*dim.z/2.0f));
		tx[5] = oCalcPlaneMatrix(float4(0.0f, 0.0f, -1.0f, s*dim.z/2.0f));

		float boxW[6] = { dim.z, dim.z, dim.x, dim.x, dim.x, dim.x, };
		float boxH[6] = { dim.y, dim.y, dim.z, dim.z, dim.y, dim.y, };

		const float4x4 m = oCreateTranslation(float3(_Desc.Bounds.center()));
		for (unsigned int i = 0; i < 6; i++)
		{
			RECT_DESC d;
			d.FaceType = _Desc.FaceType;
			d.Width = boxW[i];
			d.Height = boxH[i];
			d.Divide = _Desc.Divide;
			d.Color = _Desc.Color;
			d.Centered = true;
			d.FlipTexcoordV = _Desc.FlipTexcoordV;
			RectDetails::AppendRect(pGeometry, d, _Layout);
			pGeometry->Transform(tx[i] * m, i * 4);
		}
	}

	if (_Desc.FaceType != oGeometry::OUTLINE)
		pGeometry->Subdivide(_Desc.Divide, 36);

	pGeometry->Finalize(_Layout, _Desc.Color);
	return true;
}

namespace FrustumDetails {

	void FillPositions(std::vector<float3>& _Positions, const oFrustumf& _Frustum)
	{
		_Positions.resize(8 * 3); // extra verts for normals
		float3* p = oStd::data(_Positions);
		oVERIFY(oExtractFrustumCorners(_Frustum, p));

		// duplicate corners for normals
		memcpy(p + 8, p, 8 * sizeof(float3));
		memcpy(p + 16, p, 8 * sizeof(float3));
	}

	void FillIndices(std::vector<unsigned int>& _Indices)
	{
		const unsigned int kIndices[] =
		{
			// Left
			oFRUSTUM_LEFT_TOP_NEAR,
			oFRUSTUM_LEFT_BOTTOM_NEAR,
			oFRUSTUM_LEFT_TOP_FAR,
			oFRUSTUM_LEFT_TOP_FAR,
			oFRUSTUM_LEFT_BOTTOM_NEAR,
			oFRUSTUM_LEFT_BOTTOM_FAR,

			// Right
			oFRUSTUM_RIGHT_TOP_NEAR,
			oFRUSTUM_RIGHT_TOP_FAR,
			oFRUSTUM_RIGHT_BOTTOM_FAR,
			oFRUSTUM_RIGHT_BOTTOM_FAR,
			oFRUSTUM_RIGHT_BOTTOM_NEAR,
			oFRUSTUM_RIGHT_TOP_NEAR,

			// Top
			oFRUSTUM_LEFT_TOP_NEAR + 8,
			oFRUSTUM_LEFT_TOP_FAR + 8,
			oFRUSTUM_RIGHT_TOP_FAR + 8,
			oFRUSTUM_RIGHT_TOP_FAR + 8,
			oFRUSTUM_RIGHT_TOP_NEAR + 8,
			oFRUSTUM_LEFT_TOP_NEAR + 8,

			// Bottom
			oFRUSTUM_LEFT_BOTTOM_FAR + 8,
			oFRUSTUM_LEFT_BOTTOM_NEAR + 8,
			oFRUSTUM_RIGHT_BOTTOM_FAR + 8,
			oFRUSTUM_RIGHT_BOTTOM_FAR + 8,
			oFRUSTUM_LEFT_BOTTOM_NEAR + 8,
			oFRUSTUM_RIGHT_BOTTOM_NEAR + 8,

			// Near
			oFRUSTUM_LEFT_TOP_NEAR + 16,
			oFRUSTUM_RIGHT_TOP_NEAR + 16,
			oFRUSTUM_RIGHT_BOTTOM_NEAR + 16,
			oFRUSTUM_RIGHT_BOTTOM_NEAR + 16,
			oFRUSTUM_LEFT_BOTTOM_NEAR + 16,
			oFRUSTUM_LEFT_TOP_NEAR + 16,

			// Far
			oFRUSTUM_LEFT_TOP_FAR + 16,
			oFRUSTUM_LEFT_BOTTOM_FAR + 16,
			oFRUSTUM_RIGHT_BOTTOM_FAR + 16,
			oFRUSTUM_RIGHT_BOTTOM_FAR + 16,
			oFRUSTUM_RIGHT_TOP_FAR + 16,
			oFRUSTUM_LEFT_TOP_FAR + 16,
		};

		_Indices.resize(oCOUNTOF(kIndices));
		memcpy(oStd::data(_Indices), kIndices, sizeof(kIndices));
	}

	void FillIndicesOutline(std::vector<unsigned int>& _Indices)
	{
		const unsigned int kIndices[] =
		{
			// Left
			oFRUSTUM_LEFT_TOP_NEAR,
			oFRUSTUM_LEFT_TOP_FAR,
			oFRUSTUM_LEFT_TOP_FAR,
			oFRUSTUM_LEFT_BOTTOM_FAR,
			oFRUSTUM_LEFT_BOTTOM_FAR,
			oFRUSTUM_LEFT_BOTTOM_NEAR,
			oFRUSTUM_LEFT_BOTTOM_NEAR,
			oFRUSTUM_LEFT_TOP_NEAR,

			// Right
			oFRUSTUM_RIGHT_TOP_NEAR,
			oFRUSTUM_RIGHT_TOP_FAR,
			oFRUSTUM_RIGHT_TOP_FAR,
			oFRUSTUM_RIGHT_BOTTOM_FAR,
			oFRUSTUM_RIGHT_BOTTOM_FAR,
			oFRUSTUM_RIGHT_BOTTOM_NEAR,
			oFRUSTUM_RIGHT_BOTTOM_NEAR,
			oFRUSTUM_RIGHT_TOP_NEAR,


			// Top
			oFRUSTUM_LEFT_TOP_NEAR,
			oFRUSTUM_RIGHT_TOP_NEAR,
			oFRUSTUM_LEFT_TOP_FAR,
			oFRUSTUM_RIGHT_TOP_FAR,

			// Bottom
			oFRUSTUM_LEFT_BOTTOM_NEAR,
			oFRUSTUM_RIGHT_BOTTOM_NEAR,
			oFRUSTUM_LEFT_BOTTOM_FAR,
			oFRUSTUM_RIGHT_BOTTOM_FAR,
		};

		_Indices.resize(oCOUNTOF(kIndices));
		memcpy(oStd::data(_Indices), kIndices, sizeof(kIndices));
	}

	void FillContinuityIDs(std::vector<unsigned int>& _ContinuityIDs)
	{
		static const unsigned int kIDs[] =
		{
			0,0,0,0,
			1,1,1,1,
			2,2,3,3,
			2,2,3,3,
			4,5,4,5,
			4,5,4,5,
		};

		_ContinuityIDs.assign(kIDs, kIDs + oCOUNTOF(kIDs));
	};

} // namespace FrustumDetails

bool oGeometryFactory_Impl::CreateFrustum(const FRUSTUM_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	static const oGeometry::LAYOUT sSupportedLayout = { true, true, true, false, true, true };
	GEO_CONSTRUCT("CreateFrustum", sSupportedLayout, _Layout, _Desc.FaceType);

	FrustumDetails::FillPositions(pGeometry->Positions, _Desc.Bounds);
	if(_Desc.FaceType != oGeometry::OUTLINE)
		FrustumDetails::FillIndices(pGeometry->Indices);
	else
		FrustumDetails::FillIndicesOutline(pGeometry->Indices);

	if (_Layout.ContinuityIDs)
		FrustumDetails::FillContinuityIDs(pGeometry->ContinuityIDs);

	if (_Desc.FaceType == oGeometry::FRONT_CCW)
		ChangeWindingOrder(pGeometry->Indices, 0);

	if (_Desc.FaceType != oGeometry::OUTLINE)
		pGeometry->Subdivide(_Desc.Divide, 36);

	pGeometry->Finalize(_Layout, _Desc.Color);
	return true;
}

namespace CircleDetails
{
	// Circle is a bit interesting because vertices are not in CW or CCW order, but rather
	// evens go up one side and odds go up the other. This allows a straightforward way
	// of zig-zag tessellating the circle for more uniform shading than a point in the 
	// center of the circle that radiates outward trifan-style.

	void FillIndices(std::vector<unsigned int>& _Indices, unsigned int _BaseIndexIndex, unsigned int _BaseVertexIndex, unsigned int _Facet, oGeometry::FACE_TYPE _Facetype)
	{
		if (_Facetype == oGeometry::OUTLINE)
		{
			_Indices.reserve(_Indices.size() + _Facet * 2);

			const unsigned int numEven = (_Facet - 1) / 2;
			const unsigned int numOdd = (_Facet / 2) - 1; // -1 for transition from 0 to 1, which does not fit the for loop

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

			_Indices.push_back(_BaseVertexIndex + _Facet - 1);
			_Indices.push_back(_BaseVertexIndex + _Facet - 2);
		}

		else
		{
			const unsigned int count = _Facet - 2;
			_Indices.reserve(3 * count);

			const unsigned int o[2][2] = { { 2, 1 }, { 1, 2 } };
			for (unsigned int i = 0; i < count; i++)
			{
				_Indices.push_back(_BaseVertexIndex + i);
				_Indices.push_back(_BaseVertexIndex + i+o[i&0x1][0]);
				_Indices.push_back(_BaseVertexIndex + i+o[i&0x1][1]);
			}

			if (_Facetype == oGeometry::FRONT_CCW)
				ChangeWindingOrder(_Indices, _BaseIndexIndex);
		}
	}

	void FillIndicesWasher(std::vector<unsigned int>& _Indices, unsigned int _BaseIndexIndex, unsigned int _BaseVertexIndex, unsigned int _Facet, oGeometry::FACE_TYPE _Facetype)
	{
		if (_Facetype == oGeometry::OUTLINE)
		{
			CircleDetails::FillIndices(_Indices, _BaseIndexIndex, _BaseVertexIndex, _Facet, _Facetype);
			CircleDetails::FillIndices(_Indices, _BaseIndexIndex+_Facet*2, _BaseVertexIndex+_Facet, _Facet, _Facetype);
		}

		else
		{
			const unsigned int count = _Facet - 2;
			_Indices.reserve(6 * (_Facet));

			//end caps done out of loop. connecting first even odd pair
			_Indices.push_back(_BaseVertexIndex + 0);
			_Indices.push_back(_BaseVertexIndex + 2);
			_Indices.push_back(_BaseVertexIndex + 1);

			_Indices.push_back(_BaseVertexIndex + 1);
			_Indices.push_back(_BaseVertexIndex + 2);
			_Indices.push_back(_BaseVertexIndex + 3);

			{
				const unsigned int o[2][4] = { { 1, 4, 5, 4 }, { 4, 1, 4, 5 } };
				for (unsigned int i = 0; i < count; i++)
				{
					_Indices.push_back(_BaseVertexIndex + 2*i);
					_Indices.push_back(_BaseVertexIndex + 2*i+o[i&0x1][0]);
					_Indices.push_back(_BaseVertexIndex + 2*i+o[i&0x1][1]);

					_Indices.push_back(_BaseVertexIndex + 2*i+1);
					_Indices.push_back(_BaseVertexIndex + 2*i+o[i&0x1][2]);
					_Indices.push_back(_BaseVertexIndex + 2*i+o[i&0x1][3]);
				}
			}

			{
				//end caps done out of loop. connecting last even odd pair
				const unsigned int o[2][4] = { { 1, 2, 3, 2 }, { 2, 1, 2, 3 } };
				unsigned int i = _Facet*2-4;
				_Indices.push_back(_BaseVertexIndex + i);
				_Indices.push_back(_BaseVertexIndex + i+o[_Facet&0x1][0]);
				_Indices.push_back(_BaseVertexIndex + i+o[_Facet&0x1][1]);

				_Indices.push_back(_BaseVertexIndex + i+1);
				_Indices.push_back(_BaseVertexIndex + i+o[_Facet&0x1][2]);
				_Indices.push_back(_BaseVertexIndex + i+o[_Facet&0x1][3]);
			}

			if (_Facetype == oGeometry::FRONT_CCW)
				ChangeWindingOrder(_Indices, _BaseIndexIndex);
		}
	}

	void FillPositions(std::vector<float3>& _Positions, float _Radius, unsigned int _Facet, float _ZValue)
	{
		_Positions.reserve(_Positions.size() + _Facet);
		float step = (2.0f * oPIf) / static_cast<float>(_Facet);
		float curStep2 = 0.0f;
		float curStep = (2.0f * oPIf) - step;
		unsigned int k = 0;
		for (unsigned int i = 0; i < _Facet && k < _Facet; i++, curStep -= step, curStep2 += step)
		{
			_Positions.push_back(float3(_Radius * cosf(curStep), _Radius * sinf(curStep), _ZValue));
			if (++k >= _Facet)
				break;
			_Positions.push_back(float3(_Radius * cosf(curStep2), _Radius * sinf(curStep2), _ZValue));
			if (++k >= _Facet)
				break;
		}
	}

	// For a planar circle, all normals point in the same planar direction
	void FillNormalsUp(std::vector<float3>& _Normals, size_t _BaseVertexIndex, bool _CCW)
	{
		const float s = _CCW ? -1.0f : 1.0f;
		for (size_t i = _BaseVertexIndex; i < _Normals.size(); i++)
			_Normals[i] = s * float3(0.0f, 0.0f, 1.0f);
	}

	// Point normals out from center of circle co-planar with circle. This 
	// is useful when creating a cylinder.
	void FillNormalsOut(std::vector<float3>& _Normals, unsigned int __Facet)
	{
		FillPositions(_Normals, 1.0f, __Facet, 0.0f);
	}

	// Optionally does not clear the specified geometry, so this can be used to 
	// append circles while building cylinders.
	bool CreateCircleInternal(const oGeometryFactory::CIRCLE_DESC& _Desc
		, const oGeometry::LAYOUT& _Layout
		, oGeometry_Impl* _pGeometry
		, unsigned int _BaseIndexIndex
		, unsigned int _BaseVertexIndex
		, bool _Clear
		, float _ZValue
		, unsigned int _ContinuityID)
	{
		if (_Desc.Radius < oVERYSMALL)
			return oErrorSetLast(std::errc::invalid_argument, "Radius is too small");

		if (_Desc.Facet < 3)
			return oErrorSetLast(std::errc::invalid_argument);

		if (_Clear)
			_pGeometry->Clear();

		CircleDetails::FillPositions(_pGeometry->Positions, _Desc.Radius, _Desc.Facet, _ZValue);
		if (_Desc.FaceType == oGeometry::OUTLINE)
			CircleDetails::FillIndices(_pGeometry->Indices, _BaseIndexIndex, _BaseVertexIndex, _Desc.Facet, _Desc.FaceType);
		else
		{
			CircleDetails::FillIndices(_pGeometry->Indices, _BaseIndexIndex, _BaseVertexIndex, _Desc.Facet, _Desc.FaceType);

			if (_Layout.Normals)
			{
				_pGeometry->Normals.resize(_pGeometry->Positions.size());
				const float3 N = GetNormalSign(_Desc.FaceType) * float3(0.0f, 0.0f, 1.0f);
				for (size_t i = _BaseVertexIndex; i < _pGeometry->Normals.size(); i++)
					_pGeometry->Normals[i] = N;
			}
		}
		
		if (_ContinuityID != oInvalid)
			_pGeometry->SetContinuityID(_ContinuityID, _BaseVertexIndex);

		return true;
	}

	// Optionally does not clear the specified geometry, so this can be used to 
	// append circles while building shells.
	bool CreateWasherInternal(const oGeometryFactory::WASHER_DESC& _Desc
		, const oGeometry::LAYOUT& _Layout
		, oGeometry_Impl* _pGeometry
		, unsigned int _BaseIndexIndex
		, unsigned int _BaseVertexIndex
		, bool _Clear
		, float _ZValue
		, unsigned int _ContinuityID)
	{
		if (_Desc.InnerRadius < oVERYSMALL || _Desc.OuterRadius < oVERYSMALL)
			return oErrorSetLast(std::errc::invalid_argument, "Radius is too small");

		if(_Desc.OuterRadius < _Desc.InnerRadius)
			return oErrorSetLast(std::errc::invalid_argument, "Outer Radius must be larger than inner radius");

		if (_Desc.Facet < 3)
			return oErrorSetLast(std::errc::invalid_argument);

		if (_Clear)
			_pGeometry->Clear();

		std::vector<float3> innerCircle;
		std::vector<float3> outerCircle;
		CircleDetails::FillPositions(innerCircle, _Desc.InnerRadius, _Desc.Facet, _ZValue);
		CircleDetails::FillPositions(outerCircle, _Desc.OuterRadius, _Desc.Facet, _ZValue);
		
		//For outlines, the vertex layout is the full inner circle, followed by the full outer circle
		if (_Desc.FaceType == oGeometry::OUTLINE)
		{
			_pGeometry->Positions.insert(end(_pGeometry->Positions), begin(innerCircle), end(innerCircle));
			_pGeometry->Positions.insert(end(_pGeometry->Positions), begin(outerCircle), end(outerCircle));
			CircleDetails::FillIndicesWasher(_pGeometry->Indices, _BaseIndexIndex, _BaseVertexIndex, _Desc.Facet, _Desc.FaceType);
		}
		else //For a mesh, the vertex layout is the similar to a normal circle. interleaved even odd pairs of vertices. For each pair
			//	first vertex is from inner circle, and second is from outer. even pairs going one direction around the circle, odd going
			//	the opposite direction.
		{
			oASSERT(innerCircle.size() == outerCircle.size(),"oGeometry washer inner circle and outer circle weren't same size.");
			for (size_t i = 0;i < innerCircle.size(); ++i)
			{
				_pGeometry->Positions.push_back(innerCircle[i]);
				_pGeometry->Positions.push_back(outerCircle[i]);
			}
			CircleDetails::FillIndicesWasher(_pGeometry->Indices, _BaseIndexIndex, _BaseVertexIndex, _Desc.Facet, _Desc.FaceType);

			if (_Layout.Normals)
			{
				_pGeometry->Normals.resize(_pGeometry->Positions.size());
				const float3 N = GetNormalSign(_Desc.FaceType) * float3(0.0f, 0.0f, 1.0f);
				for (size_t i = _BaseVertexIndex; i < _pGeometry->Normals.size(); i++)
					_pGeometry->Normals[i] = N;
			}
		}

		if (_ContinuityID != oInvalid)
			_pGeometry->SetContinuityID(_ContinuityID, _BaseVertexIndex);

		return true;
	}
} // namespace CircleDetails

bool oGeometryFactory_Impl::CreateCircle(const CIRCLE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	static const oGeometry::LAYOUT sSupportedLayout = { true, true, true, false, true, true };
	GEO_CONSTRUCT("CreateCircle", sSupportedLayout, _Layout, _Desc.FaceType);
	if (!CircleDetails::CreateCircleInternal(_Desc, _Layout, pGeometry, 0, 0, true, 0.0f, _Layout.ContinuityIDs ? 0 : oInvalid))
	{
		delete pGeometry;
		*_ppGeometry = 0;
		return false;
	}

	pGeometry->Finalize(_Layout, _Desc.Color);
	return true;
}

bool oGeometryFactory_Impl::CreateWasher(const WASHER_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	static const oGeometry::LAYOUT sSupportedLayout = { true, true, true, false, true, true };
	GEO_CONSTRUCT("CreateWasher", sSupportedLayout, _Layout, _Desc.FaceType);
	if (!CircleDetails::CreateWasherInternal(_Desc, _Layout, pGeometry, 0, 0, true, 0.0f, _Layout.ContinuityIDs ? 0 : oInvalid))
	{
		delete pGeometry;
		*_ppGeometry = 0;
		return false;
	}

	pGeometry->Finalize(_Layout, _Desc.Color);
	return true;
}

void Clip(const oPlanef& _Plane, bool _Clip, oGeometry_Impl* _pGeometry)
{
	// @oooii-tony: FIXME: I just wanted this for a skydome, where 
	// the horizon is hidden anyway, so don't be too smart, just cut 
	// out most of the triangles.
	oASSERT(/*"core.geometry", */!_Clip, "clipping not yet implemented");

	// go thru indices and remove triangles on back side of the plane
	std::vector<unsigned int>::iterator it = _pGeometry->Indices.begin();
	while (it != _pGeometry->Indices.end())
	{
		// @oooii-tony: FIXME: I've been making things more consistent WRT handedness
		// and which way normals point, so this sdistance test should be reconfirmed.

		const float3& a = _pGeometry->Positions[*it];
		const float3& b = _pGeometry->Positions[*(it+1)];
		const float3& c = _pGeometry->Positions[*(it+2)];

		oASSERT(false, "@oooii-tony: I flipped signs in sdistance(), so verify sanity here...");
    // @oooii-doug: I flipped it back to positive.

		float A = sdistance(_Plane, a);
		float B = sdistance(_Plane, b);
		float C = sdistance(_Plane, c);

		if (A < 0.0f && B < 0.0f && C < 0.0f)
			it = _pGeometry->Indices.erase(it, it+3);
		else
			it += 3;
	}

	_pGeometry->PruneUnindexedVertices();
}

namespace SphereDetails
{
static void tcs(std::vector<float3>& tc, const std::vector<float3>& positions, bool hemisphere)
{
	tc.clear();
	tc.reserve(positions.size());

	oFOR(const float3& p, positions)
	{
		float phi = acosf(p.z);
		float v = 1.0f - (phi / oPIf);

		// Map from 0 -> 1 rather than 0.5 -> 1 since half the sphere is cut off
		if (hemisphere)
			v = (v - 0.5f) * 2.0f;

		float u = 0.5f;
		if (!oStd::equal(phi, 0.0f))
		{
			float a = clamp(p.y / sinf(phi), -1.0f, 1.0f);
			u = acosf(a) / (2.0f * oPIf);

			if (p.x > 0.0f)
				u = 1.0f - u;
		}

		tc.push_back(float3(u, v, 0.0f));
	}
}

static void fix_apex_tcs_octahedron(std::vector<float3>& tc)
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
	oFORI(i, Us)
	{
		tc[i] = float3(Us[i], tc[i].y, 0.0f);
		tc[i+9] = float3(Us[i], tc[i+9].y, 0.0f);
	}

	tc[8].x = (1.0f);
}

static void fix_apex_tcs_Icosahedron(std::vector<float3>& tc)
{
//	Fatal(/*"core.geometry", */"Icosahedron not yet implemented");
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

static void fix_seam(std::vector<float3>& tc, unsigned int a, unsigned int b, float threshold)
{
	float A = tc[a].x, B = tc[b].x;
	if ((A - B) > threshold)
	{
		unsigned int smaller = (A < B) ? a : b;
		tc[smaller].x = (tc[smaller].x + 1.0f);
	}
}

static void fix_seam_tcs(std::vector<float3>& tc, const std::vector<unsigned int>& indices, float threshold)
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
	
	// @oooii-tony: FIXME: Gah! after all this time
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


// @oooii-tony: FIXME: Find a rotation that produces a single vert at z = +1 and another single vert at z = -1


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

bool oGeometryFactory_Impl::CreateSphere(const SPHERE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	static const oGeometry::LAYOUT sSupportedLayout = { true, true, true, true, true, true };
	GEO_CONSTRUCT("CreateSphere", sSupportedLayout, _Layout, _Desc.FaceType);

	if (_Desc.Icosahedron && _Desc.Divide != 0)
		return oErrorSetLast(std::errc::invalid_argument, "Icosahedron subdividing not yet implemented");

	if (_Desc.Icosahedron && _Layout.Texcoords)
		return oErrorSetLast(std::errc::invalid_argument, "Icosahedron texcoords not yet implemented");

	if (_Desc.FaceType == oGeometry::OUTLINE)
	{
		CIRCLE_DESC c;
		c.FaceType = _Desc.FaceType;
		c.Facet = _Desc.OutLineFacet;
		c.Color = _Desc.Color;

		c.Radius = _Desc.Bounds.radius();
		if (!CircleDetails::CreateCircleInternal(c, _Layout, pGeometry, static_cast<unsigned int>(pGeometry->Indices.size()), static_cast<unsigned int>(pGeometry->Positions.size()), false, 0, _Layout.ContinuityIDs ? 0 : oInvalid))
			return false;

		float4x4 m = oCreateRotation(radians(90.0f), float3(1.0f, 0.0f, 0.0f));
		pGeometry->Transform(m);

		if (!CircleDetails::CreateCircleInternal(c, _Layout, pGeometry, static_cast<unsigned int>(pGeometry->Indices.size()), static_cast<unsigned int>(pGeometry->Positions.size()), false, 0, _Layout.ContinuityIDs ? 0 : oInvalid))
			return false;
	}
	else
	{
		const float3* srcVerts = _Desc.Icosahedron ? SphereDetails::sIcosahedronVerts : SphereDetails::sOctahedronVerts;
		const unsigned int* srcIndices = _Desc.Icosahedron ? SphereDetails::sIcosahedronIndices : SphereDetails::sOctahedronIndices;
		unsigned int numVerts = _Desc.Icosahedron ? SphereDetails::sIcosahedronNumVerts : SphereDetails::sOctahedronNumVerts;
		unsigned int numFaces = _Desc.Icosahedron ? SphereDetails::sIcosahedronNumFaces : SphereDetails::sOctahedronNumFaces;
		unsigned int numEdges = _Desc.Icosahedron ? SphereDetails::sIcosahedronNumEdges : SphereDetails::sOctahedronNumEdges;

		pGeometry->Positions.assign(srcVerts, srcVerts + numVerts);
		pGeometry->Indices.assign(srcIndices, srcIndices + 3*numFaces);

		if (_Desc.FaceType == oGeometry::FRONT_CCW)
			ChangeWindingOrder(pGeometry->Indices, 0);

		if (_Desc.FaceType != oGeometry::OUTLINE)
			pGeometry->Subdivide(_Desc.Divide, numEdges);

		if (!_Desc.Icosahedron)
		{
			if (_Desc.Hemisphere)
				Clip(oPlanef(float3(0.0f, 0.0f, 1.0f), 0.0f), false, pGeometry);

			if (_Desc.FaceType == oGeometry::OUTLINE)
			{
				if (_Desc.Hemisphere)
				{
					delete pGeometry;
					return oErrorSetLast(std::errc::invalid_argument, "Hemisphere not yet supported for this configuration");
				}

				unsigned int* pEdges = 0;
				size_t nEdges = 0;

				oCalcEdges(pGeometry->Positions.size(), oStd::data(pGeometry->Indices), pGeometry->Indices.size(), &pEdges, &nEdges);

				pGeometry->Indices.clear();
				pGeometry->Indices.reserve(nEdges * 2);
				for (size_t i = 0; i < nEdges; i++)
				{
					pGeometry->Indices.push_back(pEdges[i*2]);
					pGeometry->Indices.push_back(pEdges[i*2+1]);
				}

				oFreeEdgeList(pEdges);
			}
		}
	}

	if (_Layout.Texcoords)
	{
		SphereDetails::tcs(pGeometry->Texcoords, pGeometry->Positions, _Desc.Hemisphere);
		if (_Desc.Icosahedron)
			SphereDetails::fix_apex_tcs_Icosahedron(pGeometry->Texcoords);
		else
			SphereDetails::fix_apex_tcs_octahedron(pGeometry->Texcoords);

		SphereDetails::fix_seam_tcs(pGeometry->Texcoords, pGeometry->Indices, 0.85f);
	}

	if (_Layout.Normals)
	{
		pGeometry->Normals.resize(pGeometry->Positions.size());
		for (size_t i = 0; i < pGeometry->Normals.size(); i++)
			pGeometry->Normals[i] = normalize(pGeometry->Positions[i]);
	}

	oFOR(float3& v, pGeometry->Positions)
		v = (normalize(v) * _Desc.Bounds.radius()) + _Desc.Bounds.xyz();

	pGeometry->Finalize(_Layout, _Desc.Color);
	return true;
}

namespace CylinderDetails
{

static void FillIndices(std::vector<unsigned int>& _Indices, unsigned int _Facet, unsigned int _BaseVertexIndex, oGeometry::FACE_TYPE _FaceType)
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

	if (_FaceType == oGeometry::FRONT_CCW)
		ChangeWindingOrder(_Indices, cwoStartIndex);
}

} // namespace CylinderDetails

bool oGeometryFactory_Impl::CreateCylinder(const CYLINDER_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	// Tangents would be supported if texcoords were supported
	// Texcoord support is a bit hard b/c it has the same wrap-around problem spheres have.
	// This means we need to duplicate vertices along the seams and assign a 
	// different texcoord. It's a bit wacky because the circle doesn't align on 0,
	// so really the next steps are:
	// 1. Get Circle to have a vertex on (0,1,0)
	// 2. Be able to duplicate that vert and reindex triangles on the (0.0001,0.9999,0) 
	//    side
	// 3. Also texture a circle.
	static const oGeometry::LAYOUT sSupportedLayout = { true, true, true, false, true, true };
	GEO_CONSTRUCT("CreateCylinder", sSupportedLayout, _Layout, _Desc.FaceType);

	if (_Desc.Facet < 3)
		return oErrorSetLast(std::errc::invalid_argument, "Invalid facet: must be >=3");

	if (_Desc.Divide == 0)
		return oErrorSetLast(std::errc::invalid_argument, "Invalid divide specified");

	const float fStep = _Desc.Height / static_cast<float>(_Desc.Divide);

	if (_Desc.FaceType == oGeometry::OUTLINE)
	{
		CIRCLE_DESC c;
		c.FaceType = _Desc.FaceType;
		c.Facet = _Desc.Facet;
		c.Color = _Desc.Color;

		for (unsigned int i = 0; i <= _Desc.Divide; i++)
		{
			c.Radius = lerp(_Desc.Radius0, _Desc.Radius1, i/static_cast<float>(_Desc.Divide));
			if (!CircleDetails::CreateCircleInternal(c, _Layout, pGeometry, static_cast<unsigned int>(pGeometry->Indices.size()), static_cast<unsigned int>(pGeometry->Positions.size()), false, i * fStep, _Layout.ContinuityIDs ? 0 : oInvalid))
				return false;
		}

		// Now add lines along _Desc.Facet points

		const unsigned int nVertsInOneCircle = static_cast<unsigned int>(pGeometry->Positions.size() / (_Desc.Divide+1));
		pGeometry->Indices.push_back(0);
		pGeometry->Indices.push_back(0 + nVertsInOneCircle * _Desc.Divide);
		unsigned int i = 2*_Desc.OutlineVerticalSkip+1;
		for (; i < nVertsInOneCircle-1; i+=2*(_Desc.OutlineVerticalSkip+1))
		{
			pGeometry->Indices.push_back(i);
			pGeometry->Indices.push_back(i + nVertsInOneCircle * _Desc.Divide);
			pGeometry->Indices.push_back(i+1);
			pGeometry->Indices.push_back(i+1 + nVertsInOneCircle * _Desc.Divide);
		}
		if(i < nVertsInOneCircle)
		{
			pGeometry->Indices.push_back(i);
			pGeometry->Indices.push_back(i + nVertsInOneCircle * _Desc.Divide);
		}
	}

	else
	{
		CircleDetails::FillPositions(pGeometry->Positions, _Desc.Radius0, _Desc.Facet, 0.0f);
		for (unsigned int i = 1; i <= _Desc.Divide; i++)
		{
			CircleDetails::FillPositions(pGeometry->Positions, lerp(_Desc.Radius0, _Desc.Radius1, i/static_cast<float>(_Desc.Divide)), _Desc.Facet, i * fStep);
			CylinderDetails::FillIndices(pGeometry->Indices, _Desc.Facet, (i-1) * _Desc.Facet, _Desc.FaceType);

			if (_Layout.ContinuityIDs)
				pGeometry->SetContinuityID(0);
		}

		if (_Layout.Texcoords)
		{
			pGeometry->Texcoords.resize(pGeometry->Positions.size());
			for (size_t i = 0; i < pGeometry->Texcoords.size(); i++)
			{
				float3& c = pGeometry->Texcoords[i];
				const float3& p = pGeometry->Positions[i];

				float x = ((p.x + _Desc.Radius0) / (2.0f*_Desc.Radius0));
				float v = p.z / _Desc.Height;

				if (p.y <= 0.0f)
					c = float3(x * 0.5f, v, 0.0f);
				else
					c = float3(1.0f - (x * 0.5f), v, 0.0f);
			}
		}

		if (_Desc.IncludeBase)
		{
			CIRCLE_DESC c;
			c.FaceType = _Desc.FaceType;
			c.Color = _Desc.Color;
			c.Facet = _Desc.Facet;
			c.Radius = __max(_Desc.Radius0, oVERYSMALL); // pure 0 causes a degenerate face and thus degenerate normals

			oGeometry::LAYOUT layout = _Layout;
			layout.Normals = false; // normals get created later

			if (!CircleDetails::CreateCircleInternal(c, layout, pGeometry, static_cast<unsigned int>(pGeometry->Indices.size()), static_cast<unsigned int>(pGeometry->Positions.size()), false, 0.0f, _Layout.ContinuityIDs ? 1 : oInvalid))
				return false;

			c.FaceType = GetOppositeWindingOrder(_Desc.FaceType);
			c.Radius = __max(_Desc.Radius1, oVERYSMALL); // pure 0 causes a degenerate face and thus degenerate normals
			if (!CircleDetails::CreateCircleInternal(c, layout, pGeometry, static_cast<unsigned int>(pGeometry->Indices.size()), static_cast<unsigned int>(pGeometry->Positions.size()), false, _Desc.Height, _Layout.ContinuityIDs ? 2 : oInvalid))
				return false;

			//fixme: implement texcoord generation
			if (_Layout.Texcoords)
				pGeometry->Texcoords.resize(pGeometry->Positions.size());
		}
	}

	pGeometry->Finalize(_Layout, _Desc.Color);
	return true;
}

bool oGeometryFactory_Impl::CreateCone(const CONE_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	CYLINDER_DESC desc;
	desc.FaceType = _Desc.FaceType;
	desc.Divide = _Desc.Divide;
	desc.Facet = _Desc.Facet;
	desc.Radius0 = _Desc.Radius;
	desc.Radius1 = oVERYSMALL; // pure 0 causes a degenerate face and thus degenerate normals
	desc.Height = _Desc.Height;
	desc.Color = _Desc.Color;
	desc.IncludeBase = _Desc.IncludeBase;
	desc.OutlineVerticalSkip = _Desc.OutlineVerticalSkip;
	return CreateCylinder(desc, _Layout, _ppGeometry);
}

bool oGeometryFactory_Impl::CreateTorus(const TORUS_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	static const oGeometry::LAYOUT sSupportedLayout = { true, true, true, true, true, true };
	GEO_CONSTRUCT("CreateTorus", sSupportedLayout, _Layout, _Desc.FaceType);

	if (_Desc.Facet < 3)
		return oErrorSetLast(std::errc::invalid_argument, "Invalid facet: must be >=3");

	if (_Desc.Divide < 3)
		return oErrorSetLast(std::errc::invalid_argument, "Invalid divide: must be >=3");

	if (_Desc.InnerRadius < 0.0f || _Desc.OuterRadius < 0.0f || _Desc.InnerRadius > _Desc.OuterRadius)
		return oErrorSetLast(std::errc::invalid_argument, "Invalid radius");

	const float kCenterRadius = (_Desc.InnerRadius + _Desc.OuterRadius) / 2.0f;
	const float kRangeRadius = _Desc.OuterRadius - kCenterRadius;

	if (_Desc.FaceType == oGeometry::OUTLINE)
	{
		CIRCLE_DESC c;
		c.FaceType = _Desc.FaceType;
		c.Facet = _Desc.Facet;
		c.Color = _Desc.Color;

		//main circle
		c.Radius = kCenterRadius;
		if (!CircleDetails::CreateCircleInternal(c, _Layout, pGeometry, static_cast<unsigned int>(pGeometry->Indices.size()), static_cast<unsigned int>(pGeometry->Positions.size()), false, 0, _Layout.ContinuityIDs ? 0 : oInvalid))
			return false;

		float4x4 m = oCreateRotation(radians(90.0f), float3(1.0f, 0.0f, 0.0f));
		pGeometry->Transform(m);

		//small circles
		c.Radius = kRangeRadius;
		unsigned int nextCircleIndex = static_cast<unsigned int>(pGeometry->Positions.size());
		if (!CircleDetails::CreateCircleInternal(c, _Layout, pGeometry, static_cast<unsigned int>(pGeometry->Indices.size()), static_cast<unsigned int>(pGeometry->Positions.size()), false, 0, _Layout.ContinuityIDs ? 0 : oInvalid))
			return false;
		m = oCreateTranslation(float3(kCenterRadius, 0.0f, 0.0f));
		pGeometry->Transform(m,nextCircleIndex);

		nextCircleIndex = static_cast<unsigned int>(pGeometry->Positions.size());
		if (!CircleDetails::CreateCircleInternal(c, _Layout, pGeometry, static_cast<unsigned int>(pGeometry->Indices.size()), static_cast<unsigned int>(pGeometry->Positions.size()), false, 0, _Layout.ContinuityIDs ? 0 : oInvalid))
			return false;
		m = oCreateTranslation(float3(-kCenterRadius, 0.0f, 0.0f));
		pGeometry->Transform(m,nextCircleIndex);

		nextCircleIndex = static_cast<unsigned int>(pGeometry->Positions.size());
		if (!CircleDetails::CreateCircleInternal(c, _Layout, pGeometry, static_cast<unsigned int>(pGeometry->Indices.size()), static_cast<unsigned int>(pGeometry->Positions.size()), false, 0, _Layout.ContinuityIDs ? 0 : oInvalid))
			return false;
		m = oCreateRotation(radians(90.0f), float3(0.0f, 1.0f, 0.0f)) * oCreateTranslation(float3(0.0f, 0.0f, kCenterRadius));
		pGeometry->Transform(m,nextCircleIndex);

		nextCircleIndex = static_cast<unsigned int>(pGeometry->Positions.size());
		if (!CircleDetails::CreateCircleInternal(c, _Layout, pGeometry, static_cast<unsigned int>(pGeometry->Indices.size()), static_cast<unsigned int>(pGeometry->Positions.size()), false, 0, _Layout.ContinuityIDs ? 0 : oInvalid))
			return false;
		m = oCreateRotation(radians(90.0f), float3(0.0f, 1.0f, 0.0f)) * oCreateTranslation(float3(0.0f, 0.0f, -kCenterRadius));
		pGeometry->Transform(m,nextCircleIndex);
	}
	else
	{
		const float kInvDivide = 1.0f / static_cast<float>(_Desc.Divide);
		const float kInvFacet = 1.0f / static_cast<float>(_Desc.Facet);

		const float kDStep = 2.0f * oPIf / static_cast<float>(_Desc.Divide);
		const float kFStep = 2.0f * oPIf / static_cast<float>(_Desc.Facet);

		for (unsigned int i = 0; i < _Desc.Divide; i++)
		{
			const float2 S(sin(kDStep * i), sin(kDStep * (i+1)));
			const float2 C(cos(kDStep * i), cos(kDStep * (i+1)));

			for (unsigned int j = 0; j < _Desc.Facet + 1; j++)
			{
				const float curFacet = (j % _Desc.Facet) * kFStep;
				const float fSin = sinf(curFacet);
				const float fCos = cosf(curFacet);

				for (int k = 0; k < 2; k++)
				{
					float3 center = float3(kCenterRadius * C[k], 0.0f, kCenterRadius* S[k]);

					float3 p = float3((kCenterRadius + kRangeRadius * fCos) * C[k], kRangeRadius * fSin, (kCenterRadius + kRangeRadius * fCos) * S[k]);

					if (_Layout.Positions)
						pGeometry->Positions.push_back(p);

					if (_Layout.Texcoords)
						pGeometry->Texcoords.push_back(float3(1.0f - (i + k) * kInvDivide, j * kInvFacet, 0.0f));

					if (_Layout.Normals)
						pGeometry->Normals.push_back(normalize(p - center));
				}
			}
		}

		// This creates one long tri-strip, so index it out to keep everything as 
		// indexed triangles.

		for (unsigned int i = 2; i < pGeometry->Positions.size(); i++)
		{
			if ((i & 0x1) == 0)
			{
				pGeometry->Indices.push_back(i);
				pGeometry->Indices.push_back(i-1);
				pGeometry->Indices.push_back(i-2);
			}

			else
			{
				pGeometry->Indices.push_back(i-2);
				pGeometry->Indices.push_back(i-1);
				pGeometry->Indices.push_back(i-0);
			}
		}
	}

	if (_Desc.FaceType == oGeometry::FRONT_CCW)
		ChangeWindingOrder(pGeometry->Indices, 0);

	pGeometry->Finalize(_Layout, _Desc.Color);
	return true;
}

namespace TeardropDetails {

template<typename T> TVEC3<T> Eval(const T& _Theta, const T& _Phi)
{
	T o = 0.25f;//T(0.5) * (T(1) - cos(_Theta)) * sin(_Theta);
	T x = o * cos(_Phi);
	T y = o * sin(_Phi);
	T z = cos(_Phi);
	return TVEC3<T>(x, y, z);
}

} // namespace TeardropDetails

void CylinderFillIndices(std::vector<unsigned int>& _Indices, unsigned int _Facet, unsigned int _Divide)
{
	for (unsigned int i = 0; i < _Divide; i++)
	{
		unsigned int base = i * _Facet;
		for (unsigned int j = 0; j < _Facet; j++)
		{
			// four corners of a quad
			unsigned int a = base + j;
			unsigned int b = a + 1;
			unsigned int c = a + _Facet;
			unsigned int d = c + 1;

			// Wrap around to the first vert of the circle
			if (j == _Facet-1)
			{
				b -= _Facet;
				d -= _Facet;
			}

			_Indices.push_back(a); _Indices.push_back(d); _Indices.push_back(c);
			_Indices.push_back(a); _Indices.push_back(b); _Indices.push_back(d);
		}
	}
}

bool oGeometryFactory_Impl::CreateTeardrop(const TEARDROP_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	static const oGeometry::LAYOUT sSupportedLayout = { true, true, true, true, true, true };
	GEO_CONSTRUCT("CreateTeardrop", sSupportedLayout, _Layout, _Desc.FaceType);

	//if (_Desc.Divide < 3)
	//	return oErrorSetLast(std::errc::invalid_argument, "Invalid divide: must be >=3");

	if (_Desc.Facet < 3)
		return oErrorSetLast(std::errc::invalid_argument, "Invalid facet: must be >=3");

	const float kUStep = oPIf / static_cast<float>(_Desc.Divide+1);
	const float kVStep = 2.0f * oPIf / static_cast<float>(_Desc.Facet);

	for (size_t i = 0; i < _Desc.Divide+1; i++)
	{
		for (size_t j = 0; j < _Desc.Facet; j++)
		{
			const float u = kUStep * i;
			//const float u1 = kUStep * (i+1);
			const float v = kVStep * j;
			//const float v1 = kVStep * (j+1);

			float3 p = TeardropDetails::Eval(u, v);

			pGeometry->Positions.push_back(p);
			pGeometry->Normals.push_back(normalize(p));
			//pGeometry->Positions.push_back(TeardropDetails::Eval(u1, v));
			//pGeometry->Positions.push_back(TeardropDetails::Eval(u1, v1));
			//pGeometry->Positions.push_back(TeardropDetails::Eval(u, v1));
		}
	}

	CylinderFillIndices(pGeometry->Indices, _Desc.Facet, _Desc.Divide);

	//if (_Desc.FaceType == oGeometry::FRONT_CCW)
	//	ChangeWindingOrder(pGeometry->Indices, 0);

	pGeometry->Finalize(_Layout, _Desc.Color);
	return true;
}

bool oGeometryFactory_Impl::CreateOBJ(const OBJ_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	static const oGeometry::LAYOUT sSupportedLayout = { true, true, true, true, true, true };
	GEO_CONSTRUCT("CreateOBJ", sSupportedLayout, _Layout, oGeometry::FRONT_CW);

	oOBJ_INIT init;
	init.EstimatedNumVertices = oMB(1);
	init.EstimatedNumIndices = oMB(1);
	init.CounterClockwiseFaces = !_Desc.FlipFaces;
	init.CalcNormalsOnError = true;

	oStd::intrusive_ptr<threadsafe oOBJ> obj;
	if (!oOBJCreate(_Desc.OBJPath, _Desc.OBJString, init, &obj))
		return false;

	oOBJ_DESC d;
	obj->GetDesc(&d);

	pGeometry->Positions.resize(d.NumVertices);
	pGeometry->Normals.resize(d.NumVertices);
	pGeometry->Texcoords.resize(d.NumVertices);
	pGeometry->Indices.resize(d.NumIndices);

	memcpy(oStd::data(pGeometry->Positions), d.pPositions, d.NumVertices * sizeof(float3));
	memcpy(oStd::data(pGeometry->Normals), d.pNormals, d.NumVertices * sizeof(float3));
	memcpy(oStd::data(pGeometry->Texcoords), d.pTexcoords, d.NumVertices * sizeof(float2));
	memcpy(oStd::data(pGeometry->Indices), d.pIndices, d.NumIndices * sizeof(float2));
	
	if (d.NumGroups)
	{
		pGeometry->ContinuityIDs.resize(d.NumVertices);
		for (unsigned int i = 0; i < d.NumGroups; i++)
		{
			const oOBJ_GROUP& g = d.pGroups[i];
			const size_t indexStart = g.Range.StartPrimitive * 3;
			const size_t indexEnd = indexStart + g.Range.NumPrimitives * 3;
			for (size_t j = indexStart; j < indexEnd; j++)
				pGeometry->ContinuityIDs[pGeometry->Indices[j]] = i;
		}
	}

	pGeometry->Ranges.resize(d.NumGroups);
	for (size_t i = 0; i < pGeometry->Ranges.size(); i++)
	{
		pGeometry->Ranges[i] = d.pGroups[i].Range;
		pGeometry->Ranges[i] = d.pGroups[i].Range;
	}

	pGeometry->Finalize(_Layout, oStd::White);

	float3 dim = pGeometry->Bounds.size();
	float s = 1.0f / __max(dim.x, __max(dim.y, dim.z));

	if (!oStd::equal(s, 1.0f))
	{
		float4x4 scale = oCreateScale(s);
		pGeometry->Transform(scale);
	}

#if 0
	char mtlpath[256];
	oStrcpy(mtlpath, obj.OBJPath.c_str());
	oTrimFilename(mtlpath);
	oStrcat(mtlpath, obj.MaterialLibraryPath.c_str());
	
	void* pBuffer = 0;
	size_t size = 0;
	oLoadBuffer(&pBuffer, &size, malloc, mtlpath, true);

	std::vector<oOBJ::MATERIAL> mtllib;
	oLoadMTL(mtlpath, (const char*)pBuffer, &mtllib);

	free(pBuffer);
#endif
	return true;
}

bool oGeometryFactory_Impl::CreateMosaic(const MOSAIC_DESC& _Desc, const oGeometry::LAYOUT& _Layout, oGeometry** _ppGeometry)
{
	if (_Desc.FaceType == oGeometry::OUTLINE)
		return oErrorSetLast(std::errc::invalid_argument, "Face type OUTLINE not supported for mosaics");

	static const oGeometry::LAYOUT sSupportedLayout = { true, false, false, true, false, false };
	GEO_CONSTRUCT("CreateMosaic", sSupportedLayout, _Layout, _Desc.FaceType);
	
	const oRECT kNullDestRect(oRECT::pos_size, int2(0,0), _Desc.DestinationSize);
	const oRECT kNullSourceRect(oRECT::pos_size, int2(0,0), _Desc.SourceSize);

	MOSAIC_DESC LocalDesc = _Desc;
	if (!LocalDesc.pDestinationRects)
	{
		LocalDesc.pDestinationRects = &kNullDestRect;
		LocalDesc.NumRectangles = 1;
	}
	
	if (_Layout.Texcoords && !LocalDesc.pSourceRects)
	{
		// if no orig dest, limit num rects to 1 and thus use the first source rect, 
		// otherwise use source rects to do a 1:1 mapping.
		LocalDesc.pSourceRects = &kNullSourceRect;
		LocalDesc.NumRectangles = 1;
	}

	pGeometry->Reserve(6 * LocalDesc.NumRectangles, 4 * LocalDesc.NumRectangles);

	float2 SourceSize = oCastAsFloat(LocalDesc.SourceSize);
	float2 SourcePaddedSize = oCastAsFloat(LocalDesc.SourceImageSpace.size());
	float2 DestinationSize = oCastAsFloat(LocalDesc.DestinationSize);

	for (int i = 0; i < LocalDesc.NumRectangles; i++)
	{
		// First define positions in clip space. This means (-1,-1) to (1,1), so
		// normalize position within VirtualDesktopSize to [0,1], then *2-1 to get
		// back to clip space.
		float2 DMinClip = (oCastAsFloat(LocalDesc.pDestinationRects[i].Min) / DestinationSize);
		float2 DMaxClip = (oCastAsFloat(LocalDesc.pDestinationRects[i].Max) / DestinationSize);

		// We are following Directx standards by default which means 
		// clip space is defined by -1 to 1 in x and 1 to -1 in y
		if (!LocalDesc.FlipPositionY)
		{
			DMinClip.y = 1.0f - DMinClip.y;
			DMaxClip.y = 1.0f - DMaxClip.y;
		}

		DMinClip = DMinClip * 2.0f - 1.0f;
		DMaxClip = DMaxClip * 2.0f - 1.0f;

		pGeometry->Positions.push_back(float3(DMinClip.x, DMaxClip.y, LocalDesc.ZPosition));
		pGeometry->Positions.push_back(float3(DMaxClip, LocalDesc.ZPosition));
		pGeometry->Positions.push_back(float3(DMinClip, LocalDesc.ZPosition));
		pGeometry->Positions.push_back(float3(DMaxClip.x, DMinClip.y, LocalDesc.ZPosition));

		if (_Layout.Texcoords)
		{
			// Now define texture coords in texel space [0,1]
			float2 SMin = oCastAsFloat(LocalDesc.pSourceRects[i].Min-LocalDesc.SourceImageSpace.Min) / SourcePaddedSize;
			float2 SMax = oCastAsFloat(LocalDesc.pSourceRects[i].Max-LocalDesc.SourceImageSpace.Min) / SourcePaddedSize;

			if (LocalDesc.FlipTexcoordV) //We are following Directx standards by default
				std::swap(SMin.y, SMax.y);

			pGeometry->Texcoords.push_back(float3(SMin.x, SMax.y, 0.0f));
			pGeometry->Texcoords.push_back(float3(SMax, 0.0f));
			pGeometry->Texcoords.push_back(float3(SMin, 0.0f));
			pGeometry->Texcoords.push_back(float3(SMax.x, SMin.y, 0.0f));
		}

		// Finally add indices

		unsigned int offset = i * 4;

		pGeometry->Indices.push_back(offset + 3);
		pGeometry->Indices.push_back(offset + 2);
		pGeometry->Indices.push_back(offset + 0);
		pGeometry->Indices.push_back(offset + 0);
		pGeometry->Indices.push_back(offset + 1);
		pGeometry->Indices.push_back(offset + 3);
	}

	if (_Desc.FaceType == oGeometry::FRONT_CCW)
		ChangeWindingOrder(pGeometry->Indices, 0);

	pGeometry->Finalize(_Layout, oStd::Black);
	
	return true;
}
