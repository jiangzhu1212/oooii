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
#include <oStd/fourcc.h>
#include <oStd/stringize.h>
#include "oBasisTestCommon.h"
#include <cstring>

bool oBasisTest_oFourCC()
{
	oStd::fourcc fcc('TEST');

	char str[16];

	oTESTB(oStd::to_string(str, fcc), "oStd::to_string on oStd::fourcc failed 1");
	oTESTB(!strcmp("TEST", str), "oStd::to_string on oStd::fourcc failed 2");

	const char* fccStr = "RGBA";
	oTESTB(oStd::from_string(&fcc, fccStr), "oFromSTring on oStd::fourcc failed 1");
	oTESTB(oStd::fourcc('RGBA') == fcc, "oFromSTring on oStd::fourcc failed 2");

	oErrorSetLast(0, "");
	return true;
}
