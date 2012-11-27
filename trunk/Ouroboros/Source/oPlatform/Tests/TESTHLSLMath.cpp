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
#include <oPlatform/oTest.h>
#include <oPlatform/oHLSL.h>

bool oEqualEPS(float a, float b, float eps = FLT_EPSILON)
{
	return abs(a - b) <= eps;
}

bool oEqualEPS(double a, double b, double eps = DBL_EPSILON)
{
	return abs(a - b) <= eps;
}

template<typename T> T oCalcEPSAtDepth(uint _TargetDepth)
{
	return pow(T(0.5), (T)(_TargetDepth));
}

template<typename T> bool oEqualAtDepth(T a, T b, uint _TargetDepth = oMORTON_OCTREE_MAX_TARGET_DEPTH)
{
	return oEqualEPS(a, b, oCalcEPSAtDepth<T>(_TargetDepth));
}

template<typename T> bool oEqualAtDepth(const TVEC2<T>& a, const TVEC2<T>& b, uint _TargetDepth = oMORTON_OCTREE_MAX_TARGET_DEPTH)
{
	const float EPS = oCalcEPSAtDepth<T>(_TargetDepth);
	return oEqualEPS(a.x, b.x, EPS) && oEqualEPS(a.y, b.y, EPS);
}

template<typename T> bool oEqualAtDepth(const TVEC3<T>& a, const TVEC3<T>& b, uint _TargetDepth = oMORTON_OCTREE_MAX_TARGET_DEPTH)
{
	const float EPS = oCalcEPSAtDepth<T>(_TargetDepth);
	return oEqualEPS(a.x, b.x, EPS) && oEqualEPS(a.y, b.y, EPS) && oEqualEPS(a.z, b.z, EPS);
}

template<typename T> bool oEqualAtDepth(const TVEC4<T>& a, const TVEC4<T>& b, uint _TargetDepth = oMORTON_OCTREE_MAX_TARGET_DEPTH)
{
	const float EPS = oCalcEPSAtDepth<T>(_TargetDepth);
	return oEqualEPS(a.x, b.x, EPS) && oEqualEPS(a.y, b.y, EPS) && oEqualEPS(a.z, b.z, EPS) && oEqualEPS(a.w, b.w, EPS);
}

struct TESTHLSLMath : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		// Test Morton numbers
		{
			auto Result = oMorton3D(uint3(325, 458, 999));
			oTESTB( 668861813 == Result, "oMorton3D failed returning %d", Result);

			// This is an easier case to debug if there's an issue
			float3 ToEncode(0.5f, 0.0f, 0.0f);
			auto Encoded = oMorton3DEncodeNormalizedPosition(ToEncode);
			auto Decoded = oMorton3DDecodeNormalizedPosition(Encoded);
			oTESTB( oEqualAtDepth(ToEncode, Decoded), "oMorton3DEncodeNormalizedPos/oMorton3DDecodeNormalizedPos 1 failed");

			// and then the more thorough case...
			ToEncode = float3(0.244f, 0.873f, 0.111f);
			Encoded = oMorton3DEncodeNormalizedPosition(ToEncode);
			Decoded = oMorton3DDecodeNormalizedPosition(Encoded);
			oTESTB( oEqualAtDepth(ToEncode, Decoded), "oMorton3DEncodeNormalizedPos/oMorton3DDecodeNormalizedPos 2 failed");
		}

		// Test Delta Rays
		{
			float3 TestVector = float3( -890.01f, 409.2f, 204.6f);
			float3 NrmVec = normalize(TestVector);
			oDeltaRay DRay = oDeltaRayFromNrmlRay(NrmVec);
			float3 Pos = 0.0f;
			for(int i = 0; i< 100; ++i)
			{
				oDeltaRayStep(DRay, Pos);
				
				if(i > 5)
				{
					float3 Diff = normalize(Pos) - NrmVec;
					oTESTB( max(abs(Diff)) < 0.15f, "oDeltaRayStep failed to find correct ray at step %d", i);
				}
			}
		}


		// Test Bresenham raycasts
		{
			float3 TestVector = float3( 890.01f, 409.2f, 204.6f);
			float3 BresenhamRay = oBresenhamRay(TestVector);
			float StepCount = 25.0f;
			float3 TestRay = BresenhamRay * StepCount;
			float3 NrmVec = normalize(TestVector);
			float3 TestRay2 = NrmVec * (StepCount / NrmVec.x);
			oTESTB( oEqual(TestRay, TestRay2), "oRayCastDeltaRay failed to find correct ray");
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTHLSLMath);