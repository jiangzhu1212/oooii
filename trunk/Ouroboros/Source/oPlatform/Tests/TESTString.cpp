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
#include <oPlatform/oTest.h>
#include <oBasis/oMath.h>
#include <oBasis/oString.h>

struct TESTString : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		std::vector<oRECT> testRect;

		const char* rectString = "-6 -7 8 4, 0 0 1 1, -4 5 3 34";
		oFromString(&testRect, rectString);
		oTESTB( testRect.size() == 3, "Should have converted 3 rects but instead converted %i", testRect.size() );

		int2 mins[] = {
			int2(-6, -7 ),
			int2(0,   0 ),
			int2(-4,  5 )};

		int2 maxs[] = {
			int2( 8,  4 ),
			int2( 1,  1 ),
			int2( 3, 34 )};
		
		for( int i = 0; i < 3; ++i )
		{
			oTESTB( testRect[i].GetMin() == mins[i], "Incorrect min: %i %i should be %i %i", testRect[i].GetMin().x, testRect[i].GetMin().y, mins[i].x, mins[i].y );
			oTESTB( testRect[i].GetMax() == maxs[i], "Incorrect max: %i %i should be %i %i", testRect[i].GetMax().x, testRect[i].GetMax().y, maxs[i].x, maxs[i].y );
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTString);
