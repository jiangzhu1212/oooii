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
#include <oStd/assert.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oTest.h>

struct PLATFORM_oSystemPaths : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		char path[_MAX_PATH];
		oTESTB(oSystemGetPath(path, oSYSPATH_CWD), "Failed to get CWD");
		oTRACE("CWD: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_APP), "Failed to get APP");
		oTRACE("APP: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_SYSTMP), "Failed to get SYSTMP");
		oTRACE("SYSTMP: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_SYS), "Failed to get SYS");
		oTRACE("SYS: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_OS), "Failed to get OS");
		oTRACE("OS: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_DEV), "Failed to get DEV");
		oTRACE("DEV: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_TESTTMP), "Failed to get TESTTMP");
		oTRACE("TESTTMP: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_DESKTOP), "Failed to get DESKTOP");
		oTRACE("DESKTOP: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_DESKTOP_ALLUSERS), "Failed to get DESKTOP_ALLUSERS");
		oTRACE("DESKTOP_ALLUSERS: %s", path);
		oTESTB(oSystemGetPath(path, oSYSPATH_P4ROOT), "%s: %s", oErrorAsString(oErrorGetLast()), oErrorGetLastString());
		oTRACE("P4ROOT: %s", path);
		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_oSystemPaths);
