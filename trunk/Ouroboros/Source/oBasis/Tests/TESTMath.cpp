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
#include <oStd/macros.h>
#include <oCompute/oComputeUtil.h>
#include <oBasis/oMath.h>
#include <oBasis/oError.h>
#include "oBasisTestCommon.h"

#define oTESTB2(test) do { if (!(test)) return false; } while(false)

bool Test_oEqual()
{
	oTESTB(!oStd::equal(1.0f, 1.000001f), "oStd::equal() failed");
	oTESTB(oStd::equal(1.0f, 1.000001f, 8), "oStd::equal() failed");
	oTESTB(!oStd::equal(2.0, 1.99999999999997), "oStd::equal failed");
	oTESTB(oStd::equal(2.0, 1.99999999999997, 135), "oStd::equal failed");
	oErrorSetLast(0, "");
	return true;
}

bool Test_oAABoxf()
{
	oAABoxf box(float3(-1.0f), float3(1.0f));

	oTESTB(oStd::equal(box.Min, float3(-1.0f)), "oAABoxf::Min failed");
	oTESTB(oStd::equal(box.Max, float3(1.0f)), "oAABoxf::Min failed");

	box.Min = float3(-2.0f);
	box.Max = float3(2.0f);
	oTESTB(oStd::equal(box.Min, float3(-2.0f)), "oAABoxf::SetMin() failed");	
	oTESTB(oStd::equal(box.Max, float3(2.0f)), "oAABoxf::SetMin() failed");

	oTESTB(!box.empty(), "oAABoxf::empty() failed (1)");
	box.clear();
	oTESTB(box.empty(), "oAABoxf::empty() failed (2)");

	box = oAABoxf(float3(0.0f), float3(1.0f, 2.0f, 3.0f));
	oTESTB(oStd::equal(box.center(), float3(0.5f, 1.0f, 1.5f)), "oAABoxf::GetCenter() failed");

	float3 dim = box.size();
	oTESTB(oStd::equal(dim.x, 1.0f) && oStd::equal(dim.y, 2.0f) && oStd::equal(dim.z, 3.0f), "oAABoxf::GetDimensions() failed");

	float radius = box.bounding_radius();
	oTESTB(oStd::equal(radius, 1.87083f, 15), "oAABoxf:bounding_radius() failed");

	oExtendBy(box, float3(5.0f));
	oExtendBy(box, float3(-1.0f));
	dim = box.size();
	oTESTB(oStd::equal(dim.x, 6.0f) && oStd::equal(dim.y, 6.0f) && oStd::equal(dim.z, 6.0f), "oAABoxf::ExtendBy() failed");

	oErrorSetLast(0, "");
	return true;
}

bool Test_decompose()
{
	float3 testRotDegrees = float3(45.0f, 32.0f, 90.0f);
	float3 testTx = float3(-14.0f, 70.0f, 32.0f);
	float3 testScale = float3(17.0f, 2.0f, 3.0f); // negative scale not supported

	// Remember assembly of the matrix must be in the canonical order for the 
	// same values to come back out through decomposition.
	float4x4 m = oCreateScale(testScale) * oCreateRotation(radians(testRotDegrees)) * oCreateTranslation(testTx);

	float shearXY, shearXZ, shearZY;
	float3 scale, rot, tx;
	float4 persp;
	oDecompose(m, &scale, &shearXY, &shearXZ, &shearZY, &rot, &tx, &persp);

	rot = degrees(rot);
	oTESTB(oStd::equal(testScale, scale), "Scales are not the same through assembly and decomposition");
	oTESTB(oStd::equal(testRotDegrees, rot), "Rotations are not the same through assembly and decomposition");
	oTESTB(oStd::equal(testTx, tx), "Translations are not the same through assembly and decomposition");

	oErrorSetLast(0, "");
	return true;
}

bool Test_SplitRectOverlap()
{
	static const int NumberOfRects = 4;
	oRECT BrokenRects[NumberOfRects];
	float BrokenRectWeights[NumberOfRects];
	float fEvenWeight = 1.0f / (float)NumberOfRects;

	for(int i = 0; i < NumberOfRects; ++i)
		BrokenRectWeights[i] = fEvenWeight;

	for(int i = 0; i < NumberOfRects; ++i)
	{
		float fRand = (float)rand() / (float)RAND_MAX;
		int iRand = (int)(fRand * (NumberOfRects - 1));
		float& StolenValue =  BrokenRectWeights[iRand];
		float StolenWeight = StolenValue * fRand;
		StolenValue -= StolenWeight;
		BrokenRectWeights[i] += StolenWeight;
		// Randomly steal weight

	}
	float totalWeight = 0.0f;
	for(int i = 0; i < NumberOfRects; ++i)
		totalWeight += BrokenRectWeights[i];

	oTESTB(oStd::equal(totalWeight, 1.0f, 20), "Test failed to distribute random weight evenly (totalWeight = %f)", totalWeight);

	oRECT srcRect;
	srcRect.Min = 0;
	srcRect.Max = int2(320, 480);
	oTESTB(SplitRect(srcRect, NumberOfRects, BrokenRectWeights, 1, 1, BrokenRects), "Failed to split rectangles in an efficient way");

	oErrorSetLast(0, "");
	return true;
}

bool Test_Frustum()
{
	static const oPlanef EXPECTED_PLANES_LH[6] =
	{
		oPlanef(1.0f, 0.0f, 0.0f,  1.0f),
		oPlanef(-1.0f, 0.0f, 0.0f, 1.0f),
		oPlanef(0.0f, -1.0f, 0.0f, 1.0f),
		oPlanef(0.0f, 1.0f, 0.0f,  1.0f),
		oPlanef(0.0f, 0.0f, 1.0f,  1.0f),
		oPlanef(0.0f, 0.0f, -1.0f,  1.0f),
	};

	static const oPlanef EXPECTED_PLANES_RH[6] =
	{
		oPlanef(1.0f, 0.0f, 0.0f,  1.0f),
		oPlanef(-1.0f, 0.0f, 0.0f, 1.0f),
		oPlanef(0.0f, -1.0f, 0.0f, 1.0f),
		oPlanef(0.0f, 1.0f, 0.0f,  1.0f),
		oPlanef(0.0f, 0.0f, -1.0f, 1.0f),
		oPlanef(0.0f, 0.0f, 1.0f,  1.0f),
	};

	static const oPlanef EXPECTED_PERSPECTIVE_PLANES_LH[6] =
	{
		oPlanef(0.707f, 0.0f, 0.707f, 0.0f),
		oPlanef(-0.707f, 0.0f, 0.707f, 0.0f),
		oPlanef(0.0f, -0.707f, 0.707f, 0.0f),
		oPlanef(0.0f, 0.707f, 0.707f, 0.0f),
		oPlanef(0.0f, 0.0f, 1.0f,    -0.1f),
		oPlanef(0.0f, 0.0f, -1.0f,  100.0f),
	};

	static const oPlanef EXPECTED_PERSPECTIVE_PLANES_RH[6] =
	{
		oPlanef(0.707f, 0.0f, -0.707f, 0.0f),
		oPlanef(-0.707f, 0.0f, -0.707f, 0.0f),
		oPlanef(0.0f, -0.707f, -0.707f, 0.0f),
		oPlanef(0.0f, 0.707f, -0.707f, 0.0f),
		oPlanef(0.0f, 0.0f, -1.0f, -0.1f),
		oPlanef(0.0f, 0.0f, 1.0f, 100.0f),
	};

	static const char* names[6] =
	{
		"left",
		"right",
		"top",
		"bottom",
		"near",
		"far",
	};
                                                   
	const int MAX_ULPS = 1800;

	oPlanef planes[6];
	float4x4 P = oCreateOrthographicLH(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	oExtractFrustumPlanes(planes, P, false);
	oFORI(i, EXPECTED_PLANES_LH)
		oTESTB(oStd::equal(EXPECTED_PLANES_LH[i], planes[i], MAX_ULPS), "orthographic LH %s plane comparison failed", names[i]);

	P = oCreateOrthographicRH(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	oExtractFrustumPlanes(planes, P, false);
	oFORI(i, EXPECTED_PLANES_RH)
		oTESTB(oStd::equal(EXPECTED_PLANES_RH[i], planes[i], MAX_ULPS), "orthographic RH %s plane comparison failed", names[i]);

	P = oCreatePerspectiveLH(radians(90.0f), 1.0f, 0.1f, 100.0f);
	oExtractFrustumPlanes(planes, P, true);
	oFORI(i, EXPECTED_PERSPECTIVE_PLANES_LH)
		oTESTB(oStd::equal(EXPECTED_PERSPECTIVE_PLANES_LH[i], planes[i], MAX_ULPS), "perspective LH %s plane comparison failed", names[i]);

	P = oCreatePerspectiveRH(radians(90.0f), 1.0f, 0.1f, 100.0f);
	oExtractFrustumPlanes(planes, P, true);
	oFORI(i, EXPECTED_PERSPECTIVE_PLANES_RH)
		oTESTB(oStd::equal(EXPECTED_PERSPECTIVE_PLANES_RH[i], planes[i], MAX_ULPS), "perspective RH %s plane comparison failed", names[i]);

	oErrorSetLast(0, "");
	return true;
}

bool Test_FrustumCalcCorners()
{
	static const float3 EXPECTED_ORTHO_LH[8] =
	{
		float3(-1.0f, 1.0f, -1.0f),
		float3(-1.0f, 1.0f, 1.0f),
		float3(-1.0f, -1.0f, -1.0f),
		float3(-1.0f, -1.0f, 1.0f),
		float3(1.0f, 1.0f, -1.0f),
		float3(1.0f, 1.0f, 1.0f),
		float3(1.0f, -1.0f, -1.0f),
		float3(1.0f, -1.0f, 1.0f),
	};

	static const float3 EXPECTED_VP_PROJ_LH[8] =
	{
		float3(9.9f, 0.1f, -10.1f),
		float3(-90.0f, 100.0f, -110.0f),
		float3(9.9f, -0.1f, -10.1f),
		float3(-90.0f, -100.0f, -110.0f),
		float3(9.9f, 0.1f, -9.9f),
		float3(-90.0f, 100.0f, 90.0f),
		float3(9.9f, -0.1f, -9.9f),
		float3(-90.0f, -100.0f, 90.0f),
	};

	const int MAX_ULPS = 1800;

	float4x4 P = oCreateOrthographicLH(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	oTESTB(!oHasPerspective(P), "Ortho matrix says it has perspective when it doesn't");
	oFrustumf f(P);
	float3 corners[8];
	oTESTB(oExtractFrustumCorners(f, corners), "oExtractFrustumCorners failed (f)");
	oFORI(i, EXPECTED_ORTHO_LH)
		oTESTB(oStd::equal(EXPECTED_ORTHO_LH[i], corners[i], MAX_ULPS), "corner %s mismatch", oStd::as_string((oFRUSTUM_CORNER)i));

	P = oCreatePerspectiveLH(radians(90.0f), 1.0f, 0.1f, 100.0f);
	float4x4 V = oCreateLookAtLH(float3(10.0f, 0.0f, -10.0f), float3(0.0f, 0.0f, -10.0f), float3(0.0f, 1.0f, 0.0f));
	float4x4 VP = V * P;
	oFrustumf wsf(VP);
	oTESTB(oExtractFrustumCorners(wsf, corners), "oExtractFrustumCorners failed (wsf)");

	float nearWidth = distance(corners[oFRUSTUM_RIGHT_TOP_NEAR], corners[oFRUSTUM_LEFT_TOP_NEAR]);
	float farWidth = distance(corners[oFRUSTUM_RIGHT_TOP_FAR], corners[oFRUSTUM_LEFT_TOP_FAR]);
	oTESTB(oStd::equal(0.2f, nearWidth, MAX_ULPS), "width of near plane incorrect");
	oTESTB(oStd::equal(200.0f, farWidth, MAX_ULPS), "width of far plane incorrect");
	oFORI(i, EXPECTED_VP_PROJ_LH)
		oTESTB(oStd::equal(EXPECTED_VP_PROJ_LH[i], corners[i], MAX_ULPS), "corner %s mismatch", oStd::as_string((oFRUSTUM_CORNER)i));

	// Verify world pos extraction math.  This is used in deferred rendering.
	const float3 TestPos = float3(-30, 25, 41);
	float3 ComputePos;
	{
		oFrustumf psf(P);
		float3 EyePos = oExtractEye(V);
		float EyeZ = mul(V, float4(TestPos, 1.0f)).z;
		float InverseFarPlaneDistance = 1.0f / distance(wsf.Far, EyePos);
		float Depth = EyeZ * InverseFarPlaneDistance;

		float4 ProjTest = mul(VP, float4(TestPos, 1.0f));
		ProjTest /= ProjTest.w;

		float2 LerpFact = ProjTest.xy() * 0.5f + 0.5f;
		LerpFact.y = 1.0f - LerpFact.y;

		oFrustumf Frust(P);
		oTESTB(oExtractFrustumCorners(Frust, corners), "oExtractFrustumCorners failed (Frust)");
		float3x3 FrustTrans = (float3x3)invert(V);
		float3 LBF = mul(FrustTrans, corners[oFRUSTUM_LEFT_BOTTOM_FAR]);
		float3 RBF = mul(FrustTrans, corners[oFRUSTUM_RIGHT_BOTTOM_FAR]);
		float3 LTF = mul(FrustTrans, corners[oFRUSTUM_LEFT_TOP_FAR]);
		float3 RTF = mul(FrustTrans, corners[oFRUSTUM_RIGHT_TOP_FAR]);

		float3 FarPointTop = lerp(LTF,  RTF, LerpFact.x);
		float3 FarPointBottom = lerp(LBF, RBF, LerpFact.x);
		float3 FarPoint = lerp(FarPointTop, FarPointBottom, LerpFact.y);

		ComputePos = EyePos + (FarPoint * Depth);
	}
	oTESTB(oStd::equal(ComputePos, TestPos, MAX_ULPS), "Computed world pos is incorrect");

	oErrorSetLast(0, "");
	return true;
}

bool Test_MatrixMath()
{
	const int MAX_ULPS = 1800;
	float3 TestVec = normalize(float3(-54, 884, 32145));
	float3 TargetVecs [] =
	{
		TestVec, // Identity
		-TestVec, // Flip
		normalize(float3(TestVec.y, TestVec.z, TestVec.x)), // Axis swap
		normalize(float3(-242, 1234, 3)) // Other
	};

	oFORI(i, TargetVecs)
	{
		auto Target = TargetVecs[i];
		float4x4 Rotation = oCreateRotation(TestVec, Target);
		float3 RotatedVec = mul(Rotation, TestVec);
		oTESTB(oStd::equal(RotatedVec, Target, MAX_ULPS), "oCreateRotation failed with #%d (%f %f %f)", i, Target.x, Target.y, Target.z);
	}
	
	return true;
}


bool oBasisTest_oMath()
{
	oTESTB2(Test_MatrixMath());
	oTESTB2(Test_oEqual());
	oTESTB2(Test_oAABoxf());
	oTESTB2(Test_decompose());
	oTESTB2(Test_SplitRectOverlap());
	oTESTB2(Test_Frustum());
	oTESTB2(Test_FrustumCalcCorners());
	return true;
}
    