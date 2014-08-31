// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include "oBasisTestCommon.h"
#include <oBase/algorithm.h>
#include <oBasis/oStdStringSupport.h>
#include <oBasis/oMath.h>
#include <vector>

using namespace ouro;

bool oBasisTest_oString()
{
	std::vector<ouro::rect> testRect;

	const char* rectString = "-6 -7 8 4, 0 0 1 1, -4 5 3 34";
	from_string(&testRect, rectString);
	oTESTB(testRect.size() == 3, "Should have converted 3 rects but instead converted %i", testRect.size());

	int2 mins[] = 
	{
		int2(-6, -7),
		int2(0, 0),
		int2(-4,  5)
	};

	int2 maxs[] = 
	{
		int2(8, 4),
		int2(1, 1),
		int2(3, 34)
	};

	for(int i = 0; i < 3; ++i)
	{
		oTESTB(all(testRect[i].Min == mins[i]), "Incorrect min: %i %i should be %i %i", testRect[i].Min.x, testRect[i].Min.y, mins[i].x, mins[i].y);
		oTESTB(all(testRect[i].Max == maxs[i]), "Incorrect max: %i %i should be %i %i", testRect[i].Max.x, testRect[i].Max.y, maxs[i].x, maxs[i].y);
	}

	oErrorSetLast(0, "");
	return true;
}
