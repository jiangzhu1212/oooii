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
#include <oBasis/oMath.h>
#include <oBase/assert.h>
#include <oBase/byte.h>

using namespace ouro;

namespace ouro {

const char* as_string(const oFRUSTUM_CORNER& _Corner)
{
	switch (_Corner)
	{
		case oFRUSTUM_LEFT_TOP_NEAR: return "oFRUSTUM_LEFT_TOP_NEAR";
		case oFRUSTUM_LEFT_TOP_FAR: return "oFRUSTUM_LEFT_TOP_FAR";
		case oFRUSTUM_LEFT_BOTTOM_NEAR: return "oFRUSTUM_LEFT_BOTTOM_NEAR";
		case oFRUSTUM_LEFT_BOTTOM_FAR: return "oFRUSTUM_LEFT_BOTTOM_FAR";
		case oFRUSTUM_RIGHT_TOP_NEAR: return "oFRUSTUM_RIGHT_TOP_NEAR";
		case oFRUSTUM_RIGHT_TOP_FAR: return "oFRUSTUM_RIGHT_TOP_FAR";
		case oFRUSTUM_RIGHT_BOTTOM_NEAR: return "oFRUSTUM_RIGHT_BOTTOM_NEAR";
		case oFRUSTUM_RIGHT_BOTTOM_FAR: return "oFRUSTUM_RIGHT_BOTTOM_FAR";
		oNODEFAULT;
	}
}

} // namespace ouro

float4x4 oFitToView(const float4x4& _View, float _FovYRadians, const oSpheref& _Bounds, float _OffsetMultiplier = 1.0f)
{
	return ouro::fit_to_view(_View, _FovYRadians, _Bounds.xyz(), _Bounds.radius(), _OffsetMultiplier);
}

bool oCalculateAreaAndCentriod(float* _pArea, float2* _pCentroid, const float2* _pVertices, size_t _VertexStride, size_t _NumVertices)
{
	// Bashein, Gerard, Detmer, Paul R. "Graphics Gems IV." 
	// ed. Paul S. Heckbert. pg 3-6. Academic Press, San Diego, 1994.

	*_pArea = float(0);
	if (_NumVertices < 3)
		return false;

	float atmp = float(0), xtmp = float(0), ytmp = float(0);
	const float2* vj = _pVertices;
	const float2* vi = byte_add(_pVertices, _VertexStride, _NumVertices-1);
	const float2* end = byte_add(vi, _VertexStride, 1);
	while (vj < end)
	{
		float ai = vi->x * vj->y - vj->x * vi->y;
		atmp += ai;
		xtmp += (vj->x * vi->x) * ai;
		ytmp += (vj->y * vi->y) * ai;

		vi = vj;
		vj += _VertexStride;
	}

	*_pArea = atmp / 2.0f;
	if (!ouro::equal(atmp, 0.0f))
	{
		_pCentroid->x = xtmp / (3.0f * atmp);
		_pCentroid->y = ytmp / (3.0f * atmp);
	}

	return true;
}

inline float lengthSquared(const float3& x) { return dot(x,x); }

bool oIntersects(const float3& _SphereCenter0, float _Radius0, const float3& _SphereCenter1, float _Radius1)
{
	const float distance2 = dot(_SphereCenter0, _SphereCenter1); // length squared
	float minDistance2 = _Radius0 + _Radius1;
	minDistance2 *= minDistance2;
	return distance2 < minDistance2;
}

template<typename T> static void lookupNP(TVEC3<T>& _N, TVEC3<T>& _P, const oCompute::aabox<T, TVEC3<T>>& _Box, const TVEC4<T>& _Plane)
{
	/** <citation
		usage="Paper" 
		reason="This is part of a reasonably efficient non-SIMD frust cull" 
		author="Daniel S�kora & Josef Jel�nek"
		description="http://www.cescg.org/CESCG-2002/DSykoraJJelinek/index.html"
	/>*/

	#define NP_ASSIGN_COMPONENT(box, planeNormal, component, N, P) \
		if ((planeNormal).component < 0.0f) \
		{	(P).component = box.Min.component; \
			(N).component = box.Max.component; \
		} else \
		{	(P).component = box.Max.component; \
			(N).component = box.Min.component; \
		}

	// This is derived from Table 1
	NP_ASSIGN_COMPONENT(_Box, _Plane.xyz(), x, _N, _P);
	NP_ASSIGN_COMPONENT(_Box, _Plane.xyz(), y, _N, _P);
	NP_ASSIGN_COMPONENT(_Box, _Plane.xyz(), z, _N, _P);

	#undef NP_ASSIGN_COMPONENT
}

template<typename T> static oCONTAINMENT oContainsT(const TVEC4<T>* _pPlanes, size_t _NumPlanes, const oCompute::aabox<T, TVEC3<T>>& _Box)
{
	/** <citation
		usage="Paper" 
		reason="A reasonably efficient non-SIMD frust cull" 
		author="Daniel S�kora & Josef Jel�nek"
		description="http://www.cescg.org/CESCG-2002/DSykoraJJelinek/index.html"
	/>*/

#if 0
	TVEC3<T> n, p;
	oCONTAINMENT result = oWHOLLY_CONTAINED;
	for (size_t i = 0; i < _NumPlanes; i++)
	{
		const TVEC4<T>& plane = _pPlanes[i];
		lookupNP(n, p, _Box, plane);

		if (sdistance(plane, p) < 0.0f)
			return oNOT_CONTAINED;

		if (sdistance(plane, n) < 0.0f)
			result = oPARTIALLY_CONTAINED;
	}

	return result;
#endif
 
  TVEC3<T> vCorner[8];
	int totalInFront = 0;

	// get the corners of the box into the vCorner array
	oDecompose(_Box, vCorner);

	// test all 8 corners against the 6 sides 
	// if all points are behind 1 specific plane, we are out
	// if we are in with all points, then we are fully in
	for(unsigned int p = 0; p < _NumPlanes; ++p) 
  {
		int behindPlaneInCount = 0;
		int allInFrontOfPlane = 1;

    const TVEC4<T> & plane = _pPlanes[p];

		for(int i = 0; i < 8; ++i) 
    {
      TVEC3<T> & pt = vCorner[i];

			// test this point against the planes
      if (sdistance(plane, pt) < 0.0f)
      {
			  allInFrontOfPlane = 0;
				behindPlaneInCount++;
			}
		}

		// were all the points behind plane p?
		//if (behindPlaneInCount == 8)
		//	return oNOT_CONTAINED;

		// check if they were all on the right side of the plane
		totalInFront += allInFrontOfPlane;
	}

	// so if iTotalIn is 6, then all are inside the view
	if (totalInFront == 6)
		return oWHOLLY_CONTAINED;

	return oPARTIALLY_CONTAINED;
}


oCONTAINMENT oContains(const oRECT& _Rect0, const oRECT& _Rect1)
{
	if (_Rect0.Min.x > _Rect1.Max.x
		|| _Rect0.Max.x < _Rect1.Min.x
		|| _Rect0.Min.y > _Rect1.Max.y
		|| _Rect0.Max.y < _Rect1.Min.y)
		return oNOT_CONTAINED;

	if (all(_Rect1.Min >= _Rect0.Min) && all(_Rect1.Max <= _Rect0.Max))
		return oWHOLLY_CONTAINED;

	return oPARTIALLY_CONTAINED;
}

oCONTAINMENT oContains(const oAABoxf& _Box0, const oAABoxf& _Box1)
{
	if (any(_Box0.Min > _Box1.Max) || any(_Box0.Max < _Box1.Min))
		return oNOT_CONTAINED;

	if (all(_Box1.Min >= _Box0.Min) && all(_Box1.Max <= _Box0.Max))
		return oWHOLLY_CONTAINED;

	return oPARTIALLY_CONTAINED;
}

oCONTAINMENT oContains(float3 _Point, const oAABoxf& _Box)
{
	float3 distance = _Point - _Box.center();
	bool bInX = abs(distance.x) < _Box.size().x / 2.f;
	bool bInY = abs(distance.y) < _Box.size().y / 2.f;
	bool bInZ = abs(distance.z) < _Box.size().z / 2.f;

	return (bInX && bInY && bInZ) ? oWHOLLY_CONTAINED : oNOT_CONTAINED;
}

oCONTAINMENT oContains(const oFrustumf& _Frustum, const oAABoxf& _Box)
{
	// @oooii-tony: A reasonable optimization might be to set 6 to 5, thus ignoring
	// far plane clipping. When do we limit view distances these days?
	return oContainsT(&_Frustum.Left, oFRUSTUM_PLANE_COUNT, _Box);
}

template<typename T> void oFrustCullT(const oCompute::frustum<T>* oRESTRICT _pFrustra, size_t _NumberOfFrusta, const oCompute::aabox<T,TVEC3<T> >* oRESTRICT _pVolumes, size_t _NumberOfVolumes, size_t* _pResults, size_t _MaxNumberOfVolumes, size_t* _pNumResults)
{
	if (!_NumberOfFrusta || !_NumberOfVolumes)
		return;

	oASSERT(_NumberOfVolumes <= _MaxNumberOfVolumes, "");
	for (size_t i = 0; i < _NumberOfFrusta; i++)
	{
		size_t* pFrustumResults = _pResults + (i * _MaxNumberOfVolumes);
		size_t& nResults = _pNumResults[i];
		nResults = 0;

		for (size_t j = 0; j < _NumberOfVolumes; j++)
		{
			if (oContainsT(&_pFrustra[i].Left, 6, _pVolumes[j]))
			{
				pFrustumResults[nResults++] = j;
			}
		}
	}
}

void oFrustCull(const oFrustumf* oRESTRICT _pFrustra, size_t _NumberOfFrusta, const oAABoxf* oRESTRICT _pVolumes, size_t _NumberOfVolumes, size_t* _pResults, size_t _MaxNumberOfVolumes, size_t* _pNumResults)
{
	oFrustCullT(_pFrustra, _NumberOfFrusta, _pVolumes, _NumberOfVolumes, _pResults, _MaxNumberOfVolumes, _pNumResults);
}

template<typename T> static oCONTAINMENT oContainsT(const TVEC4<T>* _pPlanes, size_t _NumPlanes, const TVEC4<T>& _Sphere)
{
	oCONTAINMENT result = oWHOLLY_CONTAINED;
	for (size_t i = 0; i < _NumPlanes; i++)
	{
		float sdist = sdistance(_pPlanes[i], _Sphere.xyz());

		if (sdist < -_Sphere.w)
			return oNOT_CONTAINED;

		if (sdist < _Sphere.w)
			result = oPARTIALLY_CONTAINED;
	}

	return result;
}

oCONTAINMENT oContains(const oFrustumf& _Frustum, const float4& _Sphere)
{
	return oContainsT(&_Frustum.Left, oFRUSTUM_PLANE_COUNT, _Sphere);
}

template<typename T> oCONTAINMENT oContainsT(const oCompute::sphere<T>& _Sphere, const oCompute::aabox<T,TVEC3<T>>& _Box)
{
	/** <citation
		usage="Paper" 
		reason="Need sphere box collision and this article describes it succinctly" 
		author="James Arvo"
		description="Graphics Gems, Academic Press, 1990"
		license=""
		licenseurl=""
		modification=""
	/>*/

	const float radiusSq = _Sphere.radius() * _Sphere.radius();

	T dmin = 0;
	for (size_t i = 0; i < 3; i++)
	{
		if (_Sphere[i] < _Box.Min[i])
		{
			float diff = _Sphere[i] - _Box.Min[i];
			dmin += diff * diff;
		}
		
		else if (_Sphere[i] > _Box.Max[i])
		{
			float diff = _Sphere[i] - _Box.Max[i];
			dmin += diff * diff;
		}
	}

	if (dmin > radiusSq)
		return oNOT_CONTAINED;

	const float minSq = dot(_Box.Min, _Box.Min);
	const float maxSq = dot(_Box.Max, _Box.Max);

	if (minSq <= radiusSq && maxSq <= radiusSq)
		return oWHOLLY_CONTAINED;

	return oPARTIALLY_CONTAINED;
}

int oContains(const oSpheref& _Sphere, const oAABoxf& _Box)
{
	return oContainsT(_Sphere, _Box);
}



unsigned int SplitRect( const oRECT& _SrcRect, const unsigned int _MaxNumSplits, const float* _pOrderedSplitRatio, const unsigned int _XMultiple, const unsigned int _YMultiple, oRECT* _pSplitResults )
{
	typedef int T;
	typedef TVEC2<T> T_VEC;

	T_VEC Dimensions = _SrcRect.size();

	// Split along the larger axis
	bool SplitHorizontally = Dimensions.x > Dimensions.y;
	T SplitMultiple = SplitHorizontally ? _XMultiple : _YMultiple;
	T SplitCount = SplitHorizontally ? ( ( Dimensions.x ) / _XMultiple ) : ( ( Dimensions.y ) / _YMultiple );

	T HorizSplit = 0;
	T VeritcalSplit = 0;
	unsigned int i = 0;
	for(;i < _MaxNumSplits; ++i )
	{
		T SplitAmount		= SplitMultiple * static_cast<T>( _pOrderedSplitRatio[i] * SplitCount );
		T HorizAmount		= SplitHorizontally ? SplitAmount : Dimensions.x;
		T VerticalAmount	= SplitHorizontally ? Dimensions.y : SplitAmount;

		oRECT& rect = _pSplitResults[i];
		rect.Min = T_VEC(HorizSplit, VeritcalSplit);
		rect.Max = T_VEC(HorizSplit + HorizAmount, VeritcalSplit + VerticalAmount);
		HorizSplit += SplitHorizontally ? ( HorizAmount ) : 0;
		VeritcalSplit += SplitHorizontally ? 0 : ( VerticalAmount  );

		T_VEC RMax = rect.Max;
		if( RMax.x > Dimensions.x || RMax.y > Dimensions.y )
		{
			T_VEC minDim = min(Dimensions, RMax);
			rect.Max = min(Dimensions, RMax);
			return i + 1;
		}
	}

	// Give any remainder to the last split
	_pSplitResults[i-1].Max = Dimensions;

	return _MaxNumSplits;
}

/** <citation
	usage="Implementation" 
	reason="9/7 CDF Wavlet transform"
	author="Gregoire Pau"
	description="http://www.embl.de/~gpau/misc/dwt97.c"
	license="*** Assumed Public Domain ***"
	licenseurl="http://www.embl.de/~gpau/misc/dwt97.c"
	modification="switched to floating point, renamed variables, removed malloc"
/>*/

// $(CitedCodeBegin)

void oCDF97Fwd(float* _pValues, size_t _NumValues)
{
	float a;
	size_t i;

	// Predict 1
	a=-1.586134342f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	} 
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];

	// Update 1
	a=-0.05298011854f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2.0f*a*_pValues[1];

	// Predict 2
	a=0.8829110762f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];

	// Update 2
	a=0.4435068522f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2*a*_pValues[1];

	// Scale
	a=1.0f/1.149604398f;
	for (i=0;i<_NumValues;i++) {
		if (i%2) _pValues[i]*=a;
		else _pValues[i]/=a;
	}

	// Pack
	float* TempBank = (float*)_alloca(sizeof(float) * _NumValues);

	for (size_t i = 0; i < _NumValues; i++) 
	{
		if (i%2==0) 
			TempBank[i/2]= _pValues[i];
		else 
			TempBank[_NumValues/2+i/2] = _pValues[i];
	}

	for (size_t i = 0; i < _NumValues; i++) 
		_pValues[i]=TempBank[i];
}

void oCDF97Inv(float* _pValues, size_t _NumValues) 
{
	float a;
	size_t i;

	// Unpack
	float* TempBank = (float*)_alloca(sizeof(float) * _NumValues);
	for (i=0;i<_NumValues/2;i++) {
		TempBank[i*2]=_pValues[i];
		TempBank[i*2+1]=_pValues[i+_NumValues/2];
	}
	for (i=0;i<_NumValues;i++) _pValues[i]=TempBank[i];

	// Undo scale
	a=1.149604398f;
	for (i=0;i<_NumValues;i++) {
		if (i%2) _pValues[i]*=a;
		else _pValues[i]/=a;
	}

	// Undo update 2
	a=-0.4435068522f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2*a*_pValues[1];

	// Undo predict 2
	a=-0.8829110762f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];

	// Undo update 1
	a=0.05298011854f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2*a*_pValues[1];

	// Undo predict 1
	a=1.586134342f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	} 
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];
}

// $(CitedCodeEnd)
