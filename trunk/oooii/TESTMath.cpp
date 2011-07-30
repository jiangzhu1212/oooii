// $(header)
#include <oooii/oMath.h>
#include <oooii/oTest.h>

#define oTESTB_MATH(fn) do { RESULT r = fn; if (r != SUCCESS) return r; } while(false)

struct TESTMath : public oTest
{
	RESULT Test_oEqual(char* _StrStatus, size_t _SizeofStrStatus)
	{
		oTESTB(!oEqual(1.0f, 1.000001f), "oEqual() failed");
		oTESTB(oEqual(1.0f, 1.000001f, 8), "oEqual() failed");

		oTESTB(!oEqual(2.0, 1.99999999999997), "oEqual failed");
		oTESTB(oEqual(2.0, 1.99999999999997, 135), "oEqual failed");

		return SUCCESS;
	}

	RESULT Test_oAABoxf(char* _StrStatus, size_t _SizeofStrStatus)
	{
		oAABoxf box(float3(-1.0f), float3(1.0f));

		oTESTB(oEqual(box.GetMin(), float3(-1.0f)), "oAABoxf::GetMin() failed");
		oTESTB(oEqual(box.GetMax(), float3(1.0f)), "oAABoxf::GetMin() failed");

		box.SetMin(float3(-2.0f));
		box.SetMax(float3(2.0f));
		oTESTB(oEqual(box.GetMin(), float3(-2.0f)), "oAABoxf::SetMin() failed");	
		oTESTB(oEqual(box.GetMax(), float3(2.0f)), "oAABoxf::SetMin() failed");

		oTESTB(!box.IsEmpty(), "oAABoxf::IsEmpty() failed (1)");
		box.Clear();
		oTESTB(box.IsEmpty(), "oAABoxf::IsEmpty() failed (2)");

		box = oAABoxf(float3(0.0f), float3(1.0f, 2.0f, 3.0f));
		oTESTB(oEqual(box.GetCenter(), float3(0.5f, 1.0f, 1.5f)), "oAABoxf::GetCenter() failed");

		float3 dim = box.GetDimensions();
		oTESTB(oEqual(dim.x, 1.0f) && oEqual(dim.y, 2.0f) && oEqual(dim.z, 3.0f), "oAABoxf::GetDimensions() failed");

		float radius = box.GetBoundingRadius();
		oTESTB(oEqual(radius, 1.87083f, 15), "oAABoxf:GetBoundingRadius() failed");

		box.ExtendBy(float3(5.0f));
		box.ExtendBy(float3(-1.0f));
		dim = box.GetDimensions();
		oTESTB(oEqual(dim.x, 6.0f) && oEqual(dim.y, 6.0f) && oEqual(dim.z, 6.0f), "oAABoxf::ExtendBy() failed");

		return SUCCESS;
	}

	RESULT Test_decompose(char* _StrStatus, size_t _SizeofStrStatus)
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
		oTESTB(oEqual(testScale, scale), "Scales are not the same through assembly and decomposition");
		oTESTB(oEqual(testRotDegrees, rot), "Rotations are not the same through assembly and decomposition");
		oTESTB(oEqual(testTx, tx), "Translations are not the same through assembly and decomposition");

		return SUCCESS;
	}

	RESULT Test_SplitRectOverlap(char* _StrStatus, size_t _SizeofStrStatus)
	{
		static const int NumberOfRects = 4;
		oRECT BrokenRects[NumberOfRects];
		float BrokenRectWeights[NumberOfRects];
		float fEvenWeight = 1.0f / (float)NumberOfRects;

		for( int i = 0; i < NumberOfRects; ++i )
			BrokenRectWeights[i] = fEvenWeight;

		for( int i = 0; i < NumberOfRects; ++i )
		{
			float fRand = (float)rand() / (float)RAND_MAX;
			int iRand = (int)( fRand * ( NumberOfRects - 1 ));
			float& StolenValue =  BrokenRectWeights[iRand];
			float StolenWeight = StolenValue * fRand;
			StolenValue -= StolenWeight;
			BrokenRectWeights[i] += StolenWeight;
			// Randomly steal weight

		}
		float totalWeight = 0.0f;
		for( int i = 0; i < NumberOfRects; ++i )
			totalWeight += BrokenRectWeights[i];

		oTESTB( oEqual( totalWeight, 1.0f, 20 ), "Test failed to distribute random weight evenly (totalWeight = %f)", totalWeight );

		oRECT srcRect;
		srcRect.SetMin( 0 );
		srcRect.SetMax( int2( 320, 480 ) );
		oTESTB( SplitRect( srcRect, NumberOfRects, BrokenRectWeights, 1, 1, BrokenRects ), "Failed to split rectangles in an efficient way" );

		return SUCCESS;
	}

	RESULT Test_Frustum(char* _StrStatus, size_t _SizeofStrStatus)
	{
		static const float4 EXPECTED_PLANES_LH[6] =
		{
			float4(1.0f, 0.0f, 0.0f, -1.0f),
			float4(-1.0f, 0.0f, 0.0f, -1.0f),
			float4(0.0f, -1.0f, 0.0f,- 1.0f),
			float4(0.0f, 1.0f, 0.0f, -1.0f),
			float4(0.0f, 0.0f, 1.0f, -1.0f),
			float4(0.0f, 0.0f, -1.0f, -1.0f),
		};

		static const float4 EXPECTED_PLANES_RH[6] =
		{
			float4(1.0f, 0.0f, 0.0f, -1.0f),
			float4(-1.0f, 0.0f, 0.0f, -1.0f),
			float4(0.0f, -1.0f, 0.0f,- 1.0f),
			float4(0.0f, 1.0f, 0.0f, -1.0f),
			float4(0.0f, 0.0f, -1.0f, -1.0f),
			float4(0.0f, 0.0f, 1.0f, -1.0f),
		};

		static const float4 EXPECTED_PERSPECTIVE_PLANES_LH[6] =
		{
			float4(0.707f, 0.0f, 0.707f, 0.0f),
			float4(-0.707f, 0.0f, 0.707f, 0.0f),
			float4(0.0f, -0.707f, 0.707f, 0.0f),
			float4(0.0f, 0.707f, 0.707f, 0.0f),
			float4(0.0f, 0.0f, 1.0f, 0.1f),
			float4(0.0f, 0.0f, -1.0f, -100.0f),
		};

		static const float4 EXPECTED_PERSPECTIVE_PLANES_RH[6] =
		{
			float4(0.707f, 0.0f, -0.707f, 0.0f),
			float4(-0.707f, 0.0f, -0.707f, 0.0f),
			float4(0.0f, -0.707f, -0.707f, 0.0f),
			float4(0.0f, 0.707f, -0.707f, 0.0f),
			float4(0.0f, 0.0f, -1.0f, 0.1f),
			float4(0.0f, 0.0f, 1.0f, -100.0f),
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

		float4 planes[6];
		float4x4 P = oCreateOrthographicLH(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
		oExtractFrustumPlanes(planes, P, false);
		for (size_t i = 0; i < oCOUNTOF(EXPECTED_PLANES_LH); i++)
			oTESTB(oEqual(EXPECTED_PLANES_LH[i], planes[i], MAX_ULPS), "orthographic LH %s plane comparison failed", names[i]);

		P = oCreateOrthographicRH(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
		oExtractFrustumPlanes(planes, P, false);
		for (size_t i = 0; i < oCOUNTOF(EXPECTED_PLANES_RH); i++)
			oTESTB(oEqual(EXPECTED_PLANES_RH[i], planes[i], MAX_ULPS), "orthographic RH %s plane comparison failed", names[i]);

		P = oCreatePerspectiveLH(radians(90.0f), 1.0f, 0.1f, 100.0f);
		oExtractFrustumPlanes(planes, P, true);
		for (size_t i = 0; i < oCOUNTOF(EXPECTED_PERSPECTIVE_PLANES_LH); i++)
			oTESTB(oEqual(EXPECTED_PERSPECTIVE_PLANES_LH[i], planes[i], MAX_ULPS), "perspective LH %s plane comparison failed", names[i]);

		P = oCreatePerspectiveRH(radians(90.0f), 1.0f, 0.1f, 100.0f);
		oExtractFrustumPlanes(planes, P, true);
		for (size_t i = 0; i < oCOUNTOF(EXPECTED_PERSPECTIVE_PLANES_RH); i++)
			oTESTB(oEqual(EXPECTED_PERSPECTIVE_PLANES_RH[i], planes[i], MAX_ULPS), "perspective RH %s plane comparison failed", names[i]);

		return SUCCESS;
	}

	RESULT Test_FrustumCalcCorners(char* _StrStatus, size_t _SizeofStrStatus)
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
		f.ExtractCorners(corners);
		for (size_t i = 0; i < oCOUNTOF(EXPECTED_ORTHO_LH); i++)
			oTESTB(oEqual(EXPECTED_ORTHO_LH[i], corners[i], MAX_ULPS), "corner %s mismatch", oAsString((oFrustumf::CORNER)i));

		P = oCreatePerspectiveLH(radians(90.0f), 1.0f, 0.1f, 100.0f);
		float4x4 V = oCreateLookAtLH(float3(10.0f, 0.0f, -10.0f), float3(0.0f, 0.0f, -10.0f), float3(0.0f, 1.0f, 0.0f));
		float4x4 VP = V * P;
		oFrustumf wsf(VP);
		wsf.ExtractCorners(corners);

		float nearWidth = distance(corners[oFrustumf::RIGHT_TOP_NEAR], corners[oFrustumf::LEFT_TOP_NEAR]);
		float farWidth = distance(corners[oFrustumf::RIGHT_TOP_FAR], corners[oFrustumf::LEFT_TOP_FAR]);
		oTESTB(oEqual(0.2f, nearWidth, MAX_ULPS), "width of near plane incorrect");
		oTESTB(oEqual(200.0f, farWidth, MAX_ULPS), "width of far plane incorrect");
		for (size_t i = 0; i < oCOUNTOF(EXPECTED_VP_PROJ_LH); i++)
			oTESTB(oEqual(EXPECTED_VP_PROJ_LH[i], corners[i], MAX_ULPS), "corner %s mismatch", oAsString((oFrustumf::CORNER)i));

		// Verify world pos extraction math.  This is used in deferred rendering.
		const float3 TestPos = float3( -30, 25, 41 );
		float3 ComputePos;
		{
			oFrustumf psf(P);
			float3 EyePos = oExtractEye(V);
			float EyeZ = mul( V, float4( TestPos, 1.0f) ).z;
			float InverseFarPlaneDistance = 1.0f / distance( wsf.Far, EyePos );
			float Depth = EyeZ * InverseFarPlaneDistance;

			float4 ProjTest = mul( VP, float4( TestPos, 1.0f ) );
			ProjTest /= ProjTest.w;

			float2 LerpFact = ProjTest.XY() * 0.5f + 0.5f;
			LerpFact.y = 1.0f - LerpFact.y;

			oFrustumf Frust(P);
			Frust.ExtractCorners(corners);
			float3x3 FrustTrans = invert(V).GetUpper3x3();
			float3 LBF = mul( FrustTrans, corners[oFrustumf::LEFT_BOTTOM_FAR] );
			float3 RBF = mul( FrustTrans, corners[oFrustumf::RIGHT_BOTTOM_FAR] );
			float3 LTF = mul( FrustTrans, corners[oFrustumf::LEFT_TOP_FAR] );
			float3 RTF = mul( FrustTrans, corners[oFrustumf::RIGHT_TOP_FAR] );

			float3 FarPointTop = lerp( LTF,  RTF, LerpFact.x );
			float3 FarPointBottom = lerp( LBF, RBF, LerpFact.x );
			float3 FarPoint = lerp( FarPointTop, FarPointBottom, LerpFact.y );

			ComputePos = EyePos + (FarPoint * Depth);
		}
		oTESTB(oEqual(ComputePos, TestPos, MAX_ULPS), "Computed world pos is incorrect");

		return SUCCESS;
	}

	RESULT Test_ClipRect(char* _StrStatus, size_t _SizeofStrStatus)
	{
		oRECT a;
		oRECT b;
		oRECT ClipedRect;

		a.SetMin( int2( -512, 0 ) );
		a.SetMax( int2( 0, 512 ) );

		b.SetMin( int2( -510, 514 ) );
		b.SetMax( int2( 0, 516 ) );
		ClipedRect = oClip(a, b);
		oTESTB( ClipedRect.IsEmpty(), "Clipped rect should be empty");		

		a.SetMin( int2( 0, 0 ) );
		a.SetMax( int2( 2560, 1600 ) );

		b.SetMin( int2( 40, 40 ) );
		b.SetMax( int2( 640 + 40, 480 + 40 ) );

		ClipedRect = oClip(b, a);
		oTESTB( !ClipedRect.IsEmpty(), "Clipped rect should not be empty");
		int2 dim = ClipedRect.GetDimensions();
		oTESTB( 307200 == dim.x * dim.y, "Clipped rect is the wrong dimensions");

		ClipedRect = oClip(a, b);
		oTESTB( !ClipedRect.IsEmpty(), "Clipped rect should not be empty");
		dim = ClipedRect.GetDimensions();
		oTESTB( 307200 == dim.x * dim.y, "Clipped rect is the wrong dimensions");

		return SUCCESS;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oTESTB_MATH(Test_ClipRect(_StrStatus, _SizeofStrStatus));
		oTESTB_MATH(Test_oEqual(_StrStatus, _SizeofStrStatus));
		oTESTB_MATH(Test_oAABoxf(_StrStatus, _SizeofStrStatus));
		oTESTB_MATH(Test_decompose(_StrStatus, _SizeofStrStatus));
		oTESTB_MATH(Test_SplitRectOverlap(_StrStatus, _SizeofStrStatus));
		oTESTB_MATH(Test_Frustum(_StrStatus, _SizeofStrStatus));
		oTESTB_MATH(Test_FrustumCalcCorners(_StrStatus, _SizeofStrStatus));

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTMath);
