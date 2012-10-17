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
#include <oBasis/oFourCC.h>
#include <oBasis/oStringize.h>
#include "oBasisTestCommon.h"
#include <cstring>

bool oBasisTest_oFourCC()
{
	oFourCC fcc('TEST');

	char str[16];

	oTESTB(oToString(str, fcc), "oToString on oFourCC failed 1");
	oTESTB(!oStrcmp("TEST", str), "oToString on oFourCC failed 2");

	const char* fccStr = "RGBA";
	oTESTB(oFromString(&fcc, fccStr), "oFromSTring on oFourCC failed 1");
	oTESTB(oFourCC('RGBA') == fcc, "oFromSTring on oFourCC failed 2");

	oErrorSetLast(oERROR_NONE, "");
	return true;
}
