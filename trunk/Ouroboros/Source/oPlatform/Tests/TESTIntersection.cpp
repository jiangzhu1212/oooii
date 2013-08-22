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
#include <oPlatform/oTest.h>

// @oooii-tony: This should be moved to oBasis, or deleted. I dropped bring-up
// of a robust frust cull solution for a project, and have not gotten back to
// it, so this looks like I was setting up the test, but right now this doesn't
// do anything, so just plain removing this is as valid as any other option.

#define SIMPLE_TEST

struct PLATFORM_Intersection : public oTest
{
	struct PERSPECTIVE
	{
		float FovY;
		float AspectRatio;
		float ZNear;
	};

	struct EYE
	{
		float3 Position;
		float3 LookAtPoint;
	};

	bool InitializeAGridOfBoxes(std::vector<oAABoxf>* _pBoxes, size_t _W, size_t _H, size_t _D, float _Size, float _Spacing)
	{
		if ((_W * _H * _D) != _pBoxes->size())
			return false;

		const float GRID_W = _W * _Size + (_W-1) * _Spacing;
		const float GRID_H = _H * _Size + (_H-1) * _Spacing;
		const float GRID_D = _D * _Size + (_D-1) * _Spacing;
		const float3 CENTERING(-GRID_W / 2.0f, -GRID_H / 2.0f, -GRID_D / 2.0f);
		const float stride = _Size + _Spacing;
		for (size_t d = 0; d < _D; d++)
			for (size_t h = 0; h < _H; h++)
				for (size_t w = 0; w < _W; w++)
				{
					oAABoxf& b = (*_pBoxes)[(d * _W * _H) + (h * _W) + w];
					b.Min = float3(w * stride, h * stride, d * stride) + CENTERING;
					b.Max = b.Min + float3(_Size);
				}

		return true;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		static const size_t W = 2;
		static const size_t H = 2;
		static const size_t D = 2;

		static const float SIZE = 1.0f;
		static const float SPACING = 10.0f;

		std::vector<oAABoxf> Boxes(W * H * D);
		InitializeAGridOfBoxes(&Boxes, W, H, D, SIZE, SPACING);

		std::vector<int> Results(W * H * D);
		oStd::fill(Results, 0xdeaddead);

		static const EYE sEyes[] =
		{
			#ifdef SIMPLE_TEST
				{ float3(0.0f, 0.0f, 6.0f), float3(0.0f, 0.0f, -1.0f) },
			#else
				{ float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, -1.0f) },
				//{ float3(0.0f, 0.0f, 10.0f), float3(0.0f, 0.0f, -1.0f) },
				//{ float3(0.0f, 0.0f, -10.0f), float3(0.0f, 0.0f, 1.0f) },

				//{ float3(0.0f, 4.0f, 0.0f), float3(0.0f, 0.0f, 1.0f) },
				//{ float3(0.0f, -4.0f, 0.0f), float3(0.0f, 0.0f, 1.0f) },

				//{ float3(8.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 1.0f) },
				//{ float3(-8.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 1.0f) },
			#endif
		};

		static const PERSPECTIVE sPerspectives[] =
		{
			{ radians(45.0f), 1.3333f, 0.0001f },
		};

		for (size_t p = 0; p < oCOUNTOF(sPerspectives); p++)
		{
			float4x4 projection = oCreatePerspectiveLH(sPerspectives[p].FovY, sPerspectives[p].AspectRatio, sPerspectives[p].ZNear, -1.0f);

			for (size_t e = 0; e < oCOUNTOF(sEyes); e++)
			{
				float4x4 view = oCreateLookAtRH(sEyes[e].Position, sEyes[e].LookAtPoint, float3(0.0f, 1.0f, 0.0f));
				float4x4 projection = oCreateOrthographicRH(-100.0f, 100.0f, -100.0f, 100.0f, 0.01f, 1500.0f);
				float4x4 vp = view * projection;
				oFrustumf frustum(vp);

				// @oooii-tony: Assume the scalar frustcull works to get 'correct' answers
				for (size_t b = 0; b < Boxes.size(); b++)
					Results[b] = oContains(frustum, Boxes[b]);

				for (size_t b = 0; b < Boxes.size(); b++)
					oTRACE("Box %u: %s contained (0x%08x)", b, (Results[b] == -1) ? "partially" : (Results[b] == 1 ? "wholly" : "not"), Results[b]);
			}
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_Intersection);
