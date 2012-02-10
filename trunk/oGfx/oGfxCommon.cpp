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
#include "oGfxCommon.h"
#if 0
const oGUID& oGetGUID(threadsafe const oGfxCommandList* threadsafe const *)
{
	// {272A9B3E-64BC-4D20-845E-D4EE3F0ED890}
	static const oGUID oIID_GfxCommandList = { 0x272a9b3e, 0x64bc, 0x4d20, { 0x84, 0x5e, 0xd4, 0xee, 0x3f, 0xe, 0xd8, 0x90 } };
	return oIID_GfxCommandList;
}

const oGUID& oGetGUID(threadsafe const oGfxDevice* threadsafe const *)
{
	// {B3B5D7BC-F0C5-48D4-A976-7D41308F7450}
	static const oGUID oIID_GfxDevice = { 0xb3b5d7bc, 0xf0c5, 0x48d4, { 0xa9, 0x76, 0x7d, 0x41, 0x30, 0x8f, 0x74, 0x50 } };
	return oIID_GfxDevice;
}

const oGUID& oGetGUID(threadsafe const oGfxDeviceChild* threadsafe const *)
{
	// {E4BEBD80-C7D1-4470-995E-041116E09BBE}
	static const oGUID oIID_GfxDeviceChild = { 0xe4bebd80, 0xc7d1, 0x4470, { 0x99, 0x5e, 0x4, 0x11, 0x16, 0xe0, 0x9b, 0xbe } };
	return oIID_GfxDeviceChild;
}

const oGUID& oGetGUID(threadsafe const oGfxResource* threadsafe const *)
{
	// {D5A0E41C-AB91-496E-8D5D-A335A92778A2}
	static const oGUID oIID_GfxResource = { 0xd5a0e41c, 0xab91, 0x496e, { 0x8d, 0x5d, 0xa3, 0x35, 0xa9, 0x27, 0x78, 0xa2 } };
	return oIID_GfxResource;
}

const oGUID& oGetGUID(threadsafe const oGfxInstanceList* threadsafe const *)
{
// {4738B5E2-FECE-4478-8349-6121906D6A2C}
	static const oGUID oIID_GfxInstanceList = { 0x4738b5e2, 0xfece, 0x4478, { 0x83, 0x49, 0x61, 0x21, 0x90, 0x6d, 0x6a, 0x2c } };
	return oIID_GfxInstanceList;
}

const oGUID& oGetGUID(threadsafe const oGfxLineList* threadsafe const *)
{
	// {2B15D13C-D0DD-4573-B64B-3F000287FB2E}
	static const oGUID oIID_GfxLineList = { 0x2b15d13c, 0xd0dd, 0x4573, { 0xb6, 0x4b, 0x3f, 0x0, 0x2, 0x87, 0xfb, 0x2e } };
	return oIID_GfxLineList;
}

const oGUID& oGetGUID(threadsafe const oGfxMaterial* threadsafe const *)
{
	// {E4B3CB37-2FD7-4BF5-8CBB-6923F0185A51}
	static const oGUID oIID_GfxMaterial = { 0xe4b3cb37, 0x2fd7, 0x4bf5, { 0x8c, 0xbb, 0x69, 0x23, 0xf0, 0x18, 0x5a, 0x51 } };
	return oIID_GfxMaterial;
}

const oGUID& oGetGUID(threadsafe const oGfxMesh* threadsafe const *)
{
	// {CDAA61DB-D52A-44C1-8643-D32F05B2326C}
	static const oGUID oIID_GfxMesh = { 0xcdaa61db, 0xd52a, 0x44c1, { 0x86, 0x43, 0xd3, 0x2f, 0x5, 0xb2, 0x32, 0x6c } };
	return oIID_GfxMesh;
}

const oGUID& oGetGUID(threadsafe const oGfxPipeline* threadsafe const *)
{
	// {2401B122-EB19-4CEF-B3BE-9543C003B896}
	static const oGUID oIID_GfxPipeline = { 0x2401b122, 0xeb19, 0x4cef, { 0xb3, 0xbe, 0x95, 0x43, 0xc0, 0x3, 0xb8, 0x96 } };
	return oIID_GfxPipeline;
}

const oGUID& oGetGUID(threadsafe const oGfxRenderTarget2* threadsafe const *)
{
	// {E7F8FD41-737A-4AC5-A3C0-EB04876C6071}
	static const oGUID oIID_GfxRenderTarget2 = { 0xe7f8fd41, 0x737a, 0x4ac5, { 0xa3, 0xc0, 0xeb, 0x4, 0x87, 0x6c, 0x60, 0x71 } };
	return oIID_GfxRenderTarget2;
}

const oGUID& oGetGUID(threadsafe const oGfxTexture* threadsafe const *)
{
	// {19374525-0CC8-445B-80ED-A6D2FF13362C}
	static const oGUID oIID_GfxTexture = { 0x19374525, 0xcc8, 0x445b, { 0x80, 0xed, 0xa6, 0xd2, 0xff, 0x13, 0x36, 0x2c } };
	return oIID_GfxTexture;
}
#endif