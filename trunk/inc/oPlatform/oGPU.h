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
#pragma once
#ifndef oGPU_h
#define oGPU_h

#include <oBasis/oVersion.h>

enum oGPU_VENDOR
{
	oGPU_VENDOR_UNKNOWN,
	oGPU_VENDOR_NVIDIA,
	oGPU_VENDOR_AMD,
};

struct oGPU_DESC
{
	char GPUDescription[128];
	char DriverDescription[128];
	size_t VRAM;
	size_t DedicatedSystemMemory;
	size_t SharedSystemMemory;
	unsigned int Index;
	oGPU_VENDOR Vendor;
	oVersion DriverVersion;
	oVersion D3DVersion;
};

// Returns false if the specified GPU doesn't exist.
bool oGPUEnum(unsigned int _Index, oGPU_DESC* _pDesc);

// Returns false if there isn't an nth GPU that supports the specified minimum 
// feature level.
bool oGPUFindD3DCapable(unsigned int _NthMatch, const oVersion& _MinimumFeatureLevel, oGPU_DESC* _pDesc);

#endif
