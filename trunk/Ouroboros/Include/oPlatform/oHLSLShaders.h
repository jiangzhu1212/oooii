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
// NOTE: This header is compiled both by HLSL and C++
#pragma once
#ifndef oHLSLSHaders_h
#define oHLSLSHaders_h
#include <oBasis/oMath.h>

#define oHLSL_REQUIRED_STRUCT_ALIGNMENT 16
#define oHLSLCheckSize(_Struct) oSTATICASSERT(sizeof(_Struct) % oHLSL_REQUIRED_STRUCT_ALIGNMENT) == 0);

// Precompiled utility shaders.  Corresponding HLSL
// is located in the file with the same name.
enum oHLSL_SHADER
{
	oHLSL_PS4_0_SAMPLE_FULLSCREEN_NV12,
	oHLSL_PS4_0_SAMPLE_FULLSCREEN_NV12_ALPHA,
	oHLSL_VS4_0_QUAD_FULLSCREEN,
	oHLSL_VS5_0_QUAD_FULLSCREEN,
	oHLSL_VS4_0_QUAD_PASSTHROUGH,
	oHLSL_NUM_SHADERS,
};

// Use this to obtain a const pointer to the byte code for the specified shader.
// Use oHLSLGetSize() to determine the size of the buffer.
oAPI const void* oHLSLGetByteCode(oHLSL_SHADER _Shader);

// Returns the size of the buffer in bytes. HLSL embeds this information, 
// there is not extra data added.
oAPI size_t oHLSLGetByteCodeSize(const void* _pByteCode);

#endif //oHLSLSHaders_h