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
#include <oCompute/oComputeMorton.h>
#include <oCompute/oComputeRaycast.h>
#include <oGfx/oGfxHLSL.h>
#include <oStd/algorithm.h>
#include <vector>

namespace oCompute {
	namespace tests {

static bool equal_eps(float a, float b, float eps = FLT_EPSILON)
{
	return abs(a - b) <= eps;
}

//static bool equal_eps(double a, double b, double eps = DBL_EPSILON)
//{
//	return abs(a - b) <= eps;
//}

template<typename T> static T oCalcEPSAtDepth(uint _TargetDepth)
{
	return pow(T(0.5), (T)(_TargetDepth));
}

template<typename T> static bool oEqualAtDepth(T a, T b, uint _TargetDepth = oMORTON_OCTREE_MAX_TARGET_DEPTH)
{
	return equal_eps(a, b, oCalcEPSAtDepth<T>(_TargetDepth));
}

template<typename T> static bool oEqualAtDepth(const TVEC2<T>& a, const TVEC2<T>& b, uint _TargetDepth = oMORTON_OCTREE_MAX_TARGET_DEPTH)
{
	const float EPS = oCalcEPSAtDepth<T>(_TargetDepth);
	return equal_eps(a.x, b.x, EPS) && equal_eps(a.y, b.y, EPS);
}

template<typename T> static bool oEqualAtDepth(const TVEC3<T>& a, const TVEC3<T>& b, uint _TargetDepth = oMORTON_OCTREE_MAX_TARGET_DEPTH)
{
	const float EPS = oCalcEPSAtDepth<T>(_TargetDepth);
	return equal_eps(a.x, b.x, EPS) && equal_eps(a.y, b.y, EPS) && equal_eps(a.z, b.z, EPS);
}

template<typename T> static bool oEqualAtDepth(const TVEC4<T>& a, const TVEC4<T>& b, uint _TargetDepth = oMORTON_OCTREE_MAX_TARGET_DEPTH)
{
	const float EPS = oCalcEPSAtDepth<T>(_TargetDepth);
	return equal_eps(a.x, b.x, EPS) && equal_eps(a.y, b.y, EPS) && equal_eps(a.z, b.z, EPS) && equal_eps(a.w, b.w, EPS);
}

static void test_hemisphere_vectors()
{
	float3x3 Rot = oCreateRotationHLSL((2.0f*oPIf)/3.0f, float3(0.0f, 0.0f, 1.0f));
	float3 TargetAxis = float3(0.0f, 0.0f, 1.0f);

	float3 Hemisphere3Gen[3];
	Hemisphere3Gen[0] = normalize(float3(0.0f, oSQRT3Quarterf, 0.5f));
	Hemisphere3Gen[1] = normalize(mul(Rot, Hemisphere3Gen[0]));
	Hemisphere3Gen[2] = normalize(mul(Rot, Hemisphere3Gen[1]));

	oFORI(i, oHEMISPHERE3)
		oCHECK(oStd::equal(Hemisphere3Gen[i], oHEMISPHERE3[i]), "Hemisphere vector %d wrong", i);
}

static void test_morton_numbers()
{
	auto Result = oMorton3D(uint3(325, 458, 999));
	oCHECK(668861813 == Result, "oMorton3D failed returning %d", Result);

	// This is an easier case to debug if there's an issue
	float3 ToEncode(0.5f, 0.0f, 0.0f);
	auto Encoded = oMorton3DEncodeNormalizedPosition(ToEncode);
	auto Decoded = oMorton3DDecodeNormalizedPosition(Encoded);
	oCHECK(oEqualAtDepth(ToEncode, Decoded), "oMorton3DEncodeNormalizedPos/oMorton3DDecodeNormalizedPos 1 failed");

	// and then the more thorough case...
	ToEncode = float3(0.244f, 0.873f, 0.111f);
	Encoded = oMorton3DEncodeNormalizedPosition(ToEncode);
	Decoded = oMorton3DDecodeNormalizedPosition(Encoded);
	oCHECK(oEqualAtDepth(ToEncode, Decoded), "oMorton3DEncodeNormalizedPos/oMorton3DDecodeNormalizedPos 2 failed");
}

static void test_3dda_and_bresenham_rays()
{
	float3 TestVector = float3(-890.01f, 409.2f, 204.6f);
	float3 NrmVec = normalize(TestVector);
	//NrmVec = float3(0.0f, 1.0f, 0.0f);
	o3DDARay DDARay = oDeltaRayCreate(NrmVec);
	o3DDARayContext DRayContext = o3DDARayContextCreate(DDARay);

	static const int Count = 500;
	std::vector<o3DDARayContext> RasterPoints;
	RasterPoints.reserve(Count);
	for(int i = 0; i < Count; ++i)
	{
		DRayContext = o3DDARayCast(DDARay, DRayContext);
		RasterPoints.push_back(DRayContext);

		float3 FltPos = float3((float)DRayContext.CurrentPosition.x, (float)DRayContext.CurrentPosition.y, (float)DRayContext.CurrentPosition.z);
		float3 Diff = normalize(FltPos) - NrmVec;

		if(i > 5)
		{
			oCHECK(max(abs(Diff)) < 0.15f, "oDeltaRayStep failed to find correct ray at step %d", i);
		}
	}

	// Bresenham rays are a subset of DeltaRays.  Check to make certain this is the case
	float3 BresenhamRay = oBresenhamRay(NrmVec); 
	int MaxLen = abs(RasterPoints[Count - 1].CurrentPosition.x);
	for(int i = 1; i < MaxLen; i++)
	{
		o3DDARayContext RayContext = o3DDARayCastBresenham(DDARay, BresenhamRay, i);
		oCHECK(RasterPoints.end() != std::find(RasterPoints.begin(), RasterPoints.end(), RayContext), "Bresenham generated %d %d %d at %d", RayContext.CurrentPosition.x, RayContext.CurrentPosition.y, RayContext.CurrentPosition.z, i);
	}  
}

void TESTcompute()
{
	test_hemisphere_vectors();
	test_morton_numbers();
	test_3dda_and_bresenham_rays();
}

	} // namespace tests
} // namespace oCompute
