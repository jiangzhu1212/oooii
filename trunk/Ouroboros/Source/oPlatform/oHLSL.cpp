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
#include <oPlatform/oHLSL.h>
#include <oHLSL_PS4_0_SAMPLE_FULLSCREEN_I420ByteCode.h>
#include <oHLSL_PS4_0_SAMPLE_FULLSCREEN_I420_ALPHAByteCode.h>
#include <oHLSL_PS4_0_SAMPLE_FULLSCREEN_NV12ByteCode.h>
#include <oHLSL_PS4_0_SAMPLE_FULLSCREEN_NV12_ALPHAByteCode.h>
#include <oHLSL_VS4_0_QUAD_FULLSCREENByteCode.h>
#include <oHLSL_VS5_0_QUAD_FULLSCREENByteCode.h>
#include <oHLSL_VS4_0_QUAD_PASSTHROUGHByteCode.h>

const void* oHLSLGetByteCode(oHLSL_SHADER _Shader)
{
	static const void* sShaders[] = 
	{
		oHLSL_PS4_0_SAMPLE_FULLSCREEN_I420ByteCode,
		oHLSL_PS4_0_SAMPLE_FULLSCREEN_I420_ALPHAByteCode,
		oHLSL_PS4_0_SAMPLE_FULLSCREEN_NV12ByteCode,
		oHLSL_PS4_0_SAMPLE_FULLSCREEN_NV12_ALPHAByteCode,
		oHLSL_VS4_0_QUAD_FULLSCREENByteCode,
		oHLSL_VS5_0_QUAD_FULLSCREENByteCode,
		oHLSL_VS4_0_QUAD_PASSTHROUGHByteCode,
	};
	static_assert(oCOUNTOF(sShaders) == oHLSL_NUM_SHADERS, "oHLSL shader count mismatch");
	return sShaders[_Shader];
}

size_t oHLSLGetByteCodeSize(const void* _pByteCode)
{
	// Discovered empirically
	return _pByteCode ? ((const unsigned int*)_pByteCode)[6] : 0;
}

