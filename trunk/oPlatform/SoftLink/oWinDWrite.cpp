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
#include "oWinDWrite.h"
#include <oBasis/oAssert.h>

static const char* dll_procs[] =
{
	"DWriteCreateFactory",
};

oWinDWrite::oWinDWrite()
{
	hModule = oModuleLinkSafe("dwrite.dll", dll_procs, (void**)&DWriteCreateFactory);
	// Create a single shared one of these and always use it
	if (S_OK != DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&SharedFactory))
		oASSERT(false, "Failed to create a DirectWrite Factory");
}

oWinDWrite::~oWinDWrite()
{
	// Need to ensure all freeing of resoures is done before the code disappears
	SharedFactory = nullptr;
	oModuleUnlink(hModule);
}

// {85BB30DC-2C0E-4244-A6EF-CE0DB10A2160}
const oGUID oWinDWrite::GUID = { 0x85bb30dc, 0x2c0e, 0x4244, { 0xa6, 0xef, 0xce, 0xd, 0xb1, 0xa, 0x21, 0x60 } };
