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
#include "oWinDWMAPI.h"
#include <oStd/assert.h>

static const char* sExportedAPIs[] = 
{
	"DwmDefWindowProc",
	"DwmEnableBlurBehindWindow",
	"DwmEnableComposition",
	"DwmEnableMMCSS",
	"DwmExtendFrameIntoClientArea",
	"DwmFlush",
	"DwmGetColorizationColor",
	"DwmGetCompositionTimingInfo",
	"DwmGetTransportAttributes",
	"DwmGetWindowAttribute",
	"DwmInvalidateIconicBitmaps",
	"DwmIsCompositionEnabled",
	"DwmModifyPreviousDxFrameDuration",
	"DwmQueryThumbnailSourceSize",
	"DwmRegisterThumbnail",
	"DwmSetDxFrameDuration",
	"DwmSetIconicLivePreviewBitmap",
	"DwmSetIconicThumbnail",
	"DwmSetPresentParameters",
	"DwmSetWindowAttribute",
	"DwmUnregisterThumbnail",
	"DwmUpdateThumbnailProperties",
};

oDEFINE_DLL_SINGLETON_CTOR(oWinDWMAPI, "Dwmapi.dll", DwmDefWindowProc)

// {D04363D3-F5BF-454B-92D0-5CAF9D0A4C09}
const oGUID oWinDWMAPI::GUID = { 0xd04363d3, 0xf5bf, 0x454b, { 0x92, 0xd0, 0x5c, 0xaf, 0x9d, 0xa, 0x4c, 0x9 } };
