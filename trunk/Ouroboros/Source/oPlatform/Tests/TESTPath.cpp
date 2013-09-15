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
#include <oPlatform/oTest.h>
#include <oCore/filesystem.h>

using namespace oCore::filesystem;

struct PLATFORM_oSystemPaths : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oStd::path path = current_path();
		oTRACE("CWD: %s", path.c_str());
		path = app_path();
		oTRACE("APP: %s", path.c_str());
		path = temp_path();
		oTRACE("SYSTMP: %s", path.c_str());
		path = system_path();
		oTRACE("SYS: %s", path.c_str());
		path = os_path();
		oTRACE("OS: %s", path.c_str());
		path = dev_path();
		oTRACE("DEV: %s", path.c_str());
		path = desktop_path();
		oTRACE("DESKTOP: %s", path.c_str());
		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_oSystemPaths);
