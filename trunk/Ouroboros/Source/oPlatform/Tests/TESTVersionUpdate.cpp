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
#include <oPlatform/oTest.h>
#include <oPlatform/oVersionUpdate.h>

static ouro::uri_string sUpdateURI = "oTestD-1.0.2.2234.exe";

struct PLATFORM_oVersionUpdate : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oVU_URI_PARTS Parts;
		oTESTB(oVUDecompose(sUpdateURI, true, &Parts), "oVUDecompose failed");
		oTESTB(0 == strcmp(Parts.Filebase, "oTest"), "UpdateURI decomposed filebase doesn't match");
		oTESTB(Parts.IsDebugBuild, "UpdateURI should have been decomposed as a debug build");
		oTESTB(0 == strcmp(Parts.Extension, ".exe"), "UpdateURI decomposed extension doesn't match");
		oTESTB(Parts.Version == ouro::version(1,0,2,2234), "UpdateURI decomposed version doesn't match");

		ouro::uri_string Result;
		oTESTB(nullptr != oVURecompose(Result, Parts), "oVURecompose failed");
		oTESTB(0==strcmp(Result.c_str(), sUpdateURI.c_str()), "oVURecompose result doesn't match");

		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_oVersionUpdate);