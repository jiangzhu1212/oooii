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
#include <oBasis/oError.h>
#include <oStd/fixed_string.h>
#include <oBasis/oPath.h>
#include "oBasisTestCommon.h"

static const char* sDoubleSlashPath = "c:/my//path";
static const char* sCleanDoubleSlashPath = "c:/my/path";
static const char* sUNCPath = "//c/my/path";
static const char* sCleanUNCPath = "//c/my/path";

static const char* sDirtyPath = "C:/Users/oooii/Desktop/AutoBuild/1.0.002.6398//../WebRoot/26417/oUnitTests.txt.stderr";
static const char* sCleanPath = "C:/Users/oooii/Desktop/AutoBuild/WebRoot/26417/oUnitTests.txt.stderr";

bool oBasisTest_oPath()
{
	oStd::path_string path;
	oCleanPath(path, sDoubleSlashPath, '/');
	oTESTB(!strcmp(path, sCleanDoubleSlashPath), "Failed to clean path \"%s\"", sDoubleSlashPath);
	oCleanPath(path, sUNCPath, '/');
	oTESTB(!strcmp(path, sCleanUNCPath), "Failed to clean path \"%s\"", sUNCPath);
	oCleanPath(path, sDirtyPath, '/');
	oTESTB(!strcmp(path, sCleanPath), "Failed to clean path \"%s\"", sDirtyPath);
	oCleanPath(path, sDirtyPath, '/');
	oTESTB(!strcmp(path, sCleanPath), "Failed to clean path \"%s\" the second time", sDirtyPath);
	oErrorSetLast(0, "");
	return true;
}

