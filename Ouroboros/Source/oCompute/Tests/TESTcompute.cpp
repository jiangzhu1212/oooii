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
#include <oCompute/oComputeMorton.h>
#include <oCompute/oComputeRaycast.h>
#include <oGfx/oGfxHLSL.h>
#include <oBase/throw.h>
#include <vector>

namespace ouro {
	namespace tests {

template<typename T> bool equal_eps(const T& a, const T& b, float eps) { return abs(a - b) <= eps; }
template<typename T> bool equal_eps(const T& a, const T& b, double eps) { return abs(a - b) <= eps; }
template<typename T> bool equal_eps(const T& a, const T& b) { return equal_eps(a, b, FLT_EPSILON); }
template<> inline bool equal_eps(const double& a, const double& b) { return equal_eps(a, b, DBL_EPSILON); }

template<> inline static bool equal_eps(const TVEC2<float>& a, const TVEC2<float>& b, float eps) { return equal_eps(a.x, b.x, eps) && equal_eps(a.y, b.y, eps); }
template<> inline static bool equal_eps(const TVEC3<float>& a, const TVEC3<float>& b, float eps) { return equal_eps(a.x, b.x, eps) && equal_eps(a.y, b.y, eps); }
template<> inline static bool equal_eps(const TVEC4<float>& a, const TVEC4<float>& b, float eps) { return equal_eps(a.x, b.x, eps) && equal_eps(a.y, b.y, eps); }
template<> inline static bool equal_eps(const TMAT3<float>& a, const TMAT3<float>& b, float eps) { return equal_eps(a.Column0, b.Column0, eps) && equal_eps(a.Column1, b.Column1, eps) && equal_eps(a.Column2, b.Column2, eps); }
template<> inline static bool equal_eps(const TMAT4<float>& a, const TMAT4<float>& b, float eps) { return equal_eps(a.Column0, b.Column0, eps) && equal_eps(a.Column1, b.Column1, eps) && equal_eps(a.Column2, b.Column2, eps) && equal_eps(a.Column3, b.Column3, eps); }
template<> inline static bool equal_eps(const TVEC2<double>& a, const TVEC2<double>& b, double eps) { return equal_eps(a.x, b.x, eps) && equal_eps(a.y, b.y, eps); }
template<> inline static bool equal_eps(const TVEC3<double>& a, const TVEC3<double>& b, double eps) { return equal_eps(a.x, b.x, eps) && equal_eps(a.y, b.y, eps); }
template<> inline static bool equal_eps(const TVEC4<double>& a, const TVEC4<double>& b, double eps) { return equal_eps(a.x, b.x, eps) && equal_eps(a.y, b.y, eps); }
template<> inline static bool equal_eps(const TMAT3<double>& a, const TMAT3<double>& b, double eps) { return equal_eps(a.Column0, b.Column0, eps) && equal_eps(a.Column1, b.Column1, eps) && equal_eps(a.Column2, b.Column2, eps); }
template<> inline static bool equal_eps(const TMAT4<double>& a, const TMAT4<double>& b, double eps) { return equal_eps(a.Column0, b.Column0, eps) && equal_eps(a.Column1, b.Column1, eps) && equal_eps(a.Column2, b.Column2, eps) && equal_eps(a.Column3, b.Column3, eps); }

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
	float3x3 Rot = make_rotationHLSL((2.0f*oPIf)/3.0f, float3(0.0f, 0.0f, 1.0f));
	float3 TargetAxis = float3(0.0f, 0.0f, 1.0f);

	float3 Hemisphere3Gen[3];
	Hemisphere3Gen[0] = normalize(float3(0.0f, oSQRT3Quarterf, 0.5f));
	Hemisphere3Gen[1] = normalize(mul(Rot, Hemisphere3Gen[0]));
	Hemisphere3Gen[2] = normalize(mul(Rot, Hemisphere3Gen[1]));

	oFORI(i, oHEMISPHERE3)
		oCHECK(equal_eps(Hemisphere3Gen[i], oHEMISPHERE3[i]), "Hemisphere vector %d wrong", i);
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

// http://en.wikipedia.org/wiki/HSV_color_space
// The original source lists H in degrees, but it's more useful to store it as a 
// unorm, so normalize it here
//#define DEGREES(x) x
#define DEGREES(x) (x / 360.0f)

struct HSV_TEST
{
	const char* Tag;
	float R, G, B, H, H2, C, C2, V, L, I, Y601, Shsv, Shsl, Shsi;
};

static const HSV_TEST kHSVTests[] = 
{
	{ "#FFFFFF", 1.000f, 1.000f, 1.000f, DEGREES(0.0f), DEGREES(0.0f), 0.000f, 0.000f, 1.000f, 1.000f, 1.000f, 1.000f, 0.000f, 0.000f, 0.000f, },
	{ "#808080", 0.500f, 0.500f, 0.500f, DEGREES(0.0f), DEGREES(0.0f), 0.000f, 0.000f, 0.500f, 0.500f, 0.500f, 0.500f, 0.000f, 0.000f, 0.000f, },
	{ "#000000", 0.000f, 0.000f, 0.000f, DEGREES(0.0f), DEGREES(0.0f), 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, },
	{ "#FF0000", 1.000f, 0.000f, 0.000f, DEGREES(0.0f), DEGREES(0.0f), 1.000f, 1.000f, 1.000f, 0.500f, 0.333f, 0.299f, 1.000f, 1.000f, 1.000f, },
	{ "#BFBF00", 0.750f, 0.750f, 0.000f, DEGREES(60.0f), DEGREES(60.0f), 0.750f, 0.750f, 0.750f, 0.375f, 0.500f, 0.664f, 1.000f, 1.000f, 1.000f, },
	{ "#008000", 0.000f, 0.500f, 0.000f, DEGREES(120.0f), DEGREES(120.0f), 0.500f, 0.500f, 0.500f, 0.250f, 0.167f, 0.293f, 1.000f, 1.000f, 1.000f, },
	{ "#80FFFF", 0.500f, 1.000f, 1.000f, DEGREES(180.0f), DEGREES(180.0f), 0.500f, 0.500f, 1.000f, 0.750f, 0.833f, 0.850f, 0.500f, 1.000f, 0.400f, },
	{ "#8080FF", 0.500f, 0.500f, 1.000f, DEGREES(240.0f), DEGREES(240.0f), 0.500f, 0.500f, 1.000f, 0.750f, 0.667f, 0.557f, 0.500f, 1.000f, 0.250f, },
	{ "#BF40BF", 0.750f, 0.250f, 0.750f, DEGREES(300.0f), DEGREES(300.0f), 0.500f, 0.500f, 0.750f, 0.500f, 0.583f, 0.457f, 0.667f, 0.500f, 0.571f, },
	{ "#A0A424", 0.628f, 0.643f, 0.142f, DEGREES(61.8f), DEGREES(61.5f), 0.501f, 0.494f, 0.643f, 0.393f, 0.471f, 0.581f, 0.779f, 0.638f, 0.699f, },
	{ "#411BEA", 0.255f, 0.104f, 0.918f, DEGREES(251.1f), DEGREES(250.0f), 0.814f, 0.750f, 0.918f, 0.511f, 0.426f, 0.242f, 0.887f, 0.832f, 0.756f, },
	{ "#1EAC41", 0.116f, 0.675f, 0.255f, DEGREES(134.9f), DEGREES(133.8f), 0.559f, 0.504f, 0.675f, 0.396f, 0.349f, 0.460f, 0.828f, 0.707f, 0.667f, },
	{ "#F0C80E", 0.941f, 0.785f, 0.053f, DEGREES(49.5f), DEGREES(50.5f), 0.888f, 0.821f, 0.941f, 0.497f, 0.593f, 0.748f, 0.944f, 0.893f, 0.911f, },
	{ "#B430E5", 0.704f, 0.187f, 0.897f, DEGREES(283.7f), DEGREES(284.8f), 0.710f, 0.636f, 0.897f, 0.542f, 0.596f, 0.423f, 0.792f, 0.775f, 0.686f, },
	{ "#ED7651", 0.931f, 0.463f, 0.316f, DEGREES(14.3f), DEGREES(13.2f), 0.615f, 0.556f, 0.931f, 0.624f, 0.570f, 0.586f, 0.661f, 0.817f, 0.446f, },
	{ "#FEF888", 0.998f, 0.974f, 0.532f, DEGREES(56.9f), DEGREES(57.4f), 0.466f, 0.454f, 0.998f, 0.765f, 0.835f, 0.931f, 0.467f, 0.991f, 0.363f, },
	{ "#19CB97", 0.099f, 0.795f, 0.591f, DEGREES(162.4f), DEGREES(163.4f), 0.696f, 0.620f, 0.795f, 0.447f, 0.495f, 0.564f, 0.875f, 0.779f, 0.800f, },
	{ "#362698", 0.211f, 0.149f, 0.597f, DEGREES(248.3f), DEGREES(247.3f), 0.448f, 0.420f, 0.597f, 0.373f, 0.319f, 0.219f, 0.750f, 0.601f, 0.533f, },
	{ "#7E7EB8", 0.495f, 0.493f, 0.721f, DEGREES(240.5f), DEGREES(240.4f), 0.228f, 0.227f, 0.721f, 0.607f, 0.570f, 0.520f, 0.316f, 0.290f, 0.135f, },
};

static void test_hsv()
{
	static const float HSV_EPS = 0.001f;

	oFORI(i, kHSVTests)
	{
		const HSV_TEST& c = kHSVTests[i];
		const float3 kRGB(c.R, c.G, c.B);
		const float3 kHSV(c.H, c.Shsv, c.V);

		float3 testRGB = hsvtorgb(kHSV);
		if (!equal_eps(testRGB, kRGB, HSV_EPS))
			oTHROW(protocol_error, "failed (%d \"%s\"): oHSVtoRGB(%.03f, %.03f, %.03f) expected (%.03f, %.03f, %.03f), got (%.03f, %.03f, %.03f)"
				, i, c.Tag
				, kHSV.x, kHSV.y, kHSV.z
				, kRGB.x, kRGB.y, kRGB.z
				, testRGB.x, testRGB.y, testRGB.z);

		float3 testHSV = rgbtohsv(kRGB);
		if (!equal_eps(testHSV, kHSV, HSV_EPS))
			oTHROW(protocol_error, "failed (%d \"%s\"): oRGBtoHSV(%.03f, %.03f, %.03f) expected (%.03f, %.03f, %.03f), got (%.03f, %.03f, %.03f)"
				, i, c.Tag
				, kRGB.x, kRGB.y, kRGB.z
				, kHSV.x, kHSV.y, kHSV.z
				, testHSV.x, testHSV.y, testHSV.z);
	}
}

void TESTcompute()
{
	test_hemisphere_vectors();
	test_morton_numbers();
	test_3dda_and_bresenham_rays();
	test_hsv();
}

	} // namespace tests
} // namespace ouro
