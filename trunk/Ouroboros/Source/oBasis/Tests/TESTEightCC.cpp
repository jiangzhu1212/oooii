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
#include <oBasis/oEightCC.h>
#include <oBasis/oStringize.h>
#include "oBasisTestCommon.h"
#include <cstring>

bool oBasisTest_oEightCC()
{
	oEightCC fcc('TEXT', 'URE ');

	char str[16];

	oTESTB(oToString(str, fcc), "oToString on oEightCC failed 1");
	oTESTB(!oStrcmp("TEXTURE ", str), "oToString on oEightCC failed 2");

	const char* fccStr = "GEOMETRY";
	oTESTB(oFromString(&fcc, fccStr), "oFromSTring on oEightCC failed 1");
	oTESTB(oEightCC('GEOM', 'ETRY') == fcc, "oFromSTring on oEightCC failed 2");

	oErrorSetLast(oERROR_NONE, "");
	return true;
}
