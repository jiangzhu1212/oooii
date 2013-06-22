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
#include "oD3DX11.h"

static const char* sExportedAPIs[] = 
{
	"D3DX11CreateTextureFromFileA",
	"D3DX11CreateTextureFromMemory",
	"D3DX11LoadTextureFromTexture",
	"D3DX11SaveTextureToFileA",
	"D3DX11SaveTextureToMemory",
};

oDEFINE_DLL_SINGLETON_CTOR(oD3DX11, "d3dx11_43.dll", D3DX11CreateTextureFromFileA)

// {11C910DA-CD93-4008-BD17-385E5E3555C3}
const oGUID oD3DX11::GUID = { 0x11c910da, 0xcd93, 0x4008, { 0xbd, 0x17, 0x38, 0x5e, 0x5e, 0x35, 0x55, 0xc3 } };
