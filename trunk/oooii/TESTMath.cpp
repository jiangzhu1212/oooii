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
#include <oooii/oMath.h>
#include <oooii/oTest.h>

struct TESTMath : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		// Test oEqual

		oTESTB(!oEqual(1.0f, 1.000001f), "oEqual() failed");
		oTESTB(oEqual(1.0f, 1.000001f, 8), "oEqual() failed");

		oTESTB(!oEqual(2.0, 1.99999999999997), "oEqual failed");
		oTESTB(oEqual(2.0, 1.99999999999997, 135), "oEqual failed");

		// Test oAABoxf

		oAABoxf box(float3(-1.0f), float3(1.0f));

		oTESTB(oEqual(box.GetMin(), float3(-1.0f)), "oAABoxf::GetMin() failed");
		oTESTB(oEqual(box.GetMax(), float3(1.0f)), "oAABoxf::GetMin() failed");

		box.SetMin(float3(-2.0f));
		box.SetMax(float3(2.0f));
		oTESTB(oEqual(box.GetMin(), float3(-2.0f)), "oAABoxf::SetMin() failed");	
		oTESTB(oEqual(box.GetMax(), float3(2.0f)), "oAABoxf::SetMin() failed");

		oTESTB(!box.IsEmpty(), "oAABoxf::IsEmpty() failed (1)");
		box.Empty();
		oTESTB(box.IsEmpty(), "oAABoxf::IsEmpty() failed (2)");

		box = oAABoxf(float3(0.0f), float3(1.0f, 2.0f, 3.0f));
		oTESTB(oEqual(box.GetCenter(), float3(0.5f, 1.0f, 1.5f)), "oAABoxf::GetCenter() failed");

		float w,h,d;
		box.GetDimensions(&w, &h, &d);
		oTESTB(oEqual(w, 1.0f) && oEqual(h, 2.0f) && oEqual(d, 3.0f), "oAABoxf::GetDimensions() failed");

		float radius = box.GetBoundingRadius();
		oTESTB(oEqual(radius, 1.87083f, 15), "oAABoxf:GetBoundingRadius() failed");

		box.ExtendBy(float3(5.0f));
		box.ExtendBy(float3(-1.0f));
		box.GetDimensions(&w, &h, &d);
		oTESTB(oEqual(w, 6.0f) && oEqual(h, 6.0f) && oEqual(d, 6.0f), "oAABoxf::ExtendBy() failed");


		// Test SplitRectOverlap
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

			oTESTB( oEqual( totalWeight, 1.0f ), "Test failed to distribute random wheight evenly" );

			oRECT srcRect;
			srcRect.SetMin( 0 );
			srcRect.SetMax( TVECTOR2<int>( 320, 480 ) );
			oTESTB( SplitRect( srcRect, NumberOfRects, BrokenRectWeights, 1, 1, BrokenRects ), "Failed to split rectangles in an efficient way" );
		}

		return SUCCESS;
	}
};

TESTMath TestMath;
