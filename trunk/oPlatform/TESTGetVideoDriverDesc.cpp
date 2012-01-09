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

struct TESTGetVideoDriverDesc : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		
		oWINDOWS_VIDEO_DRIVER_DESC desc;
		memset(&desc, 0, sizeof(oWINDOWS_VIDEO_DRIVER_DESC));
		bool bSuccess = oWinGetVideoDriverDesc(&desc);

		if (bSuccess)
		{
			sprintf_s(_StrStatus, _SizeofStrStatus, "Video Driver: %i.%i", desc.MajorVersion, desc.MinorVersion);
		}
		else
		{
			sprintf_s(_StrStatus, _SizeofStrStatus, "ATI NOT IMPLEMENTED");
			bSuccess = false;
		}
		return bSuccess ? oTest::SUCCESS : oTest::FAILURE;
	}
};

oTEST_REGISTER(TESTGetVideoDriverDesc);