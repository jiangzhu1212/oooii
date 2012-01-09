// $(header)
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
