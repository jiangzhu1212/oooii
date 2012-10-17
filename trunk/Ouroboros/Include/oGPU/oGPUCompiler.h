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
// Code that handles compiling shader source code.
#pragma once
#ifndef oGPUCompiler_h
#define oGPUCompiler_h

#include <oGPU/oGPU.h>

// Compiles a shader to its driver-specific bytecode. To make parsing easy, 
// there are two lists of defines that can be specified. These are concatenated
// internally.
bool oGPUCompileShader(
	const char* _IncludePaths // semi-colon delimited list of paths to look in. Use %DEV% for oSYSPATH_DEV
	, const char* _CommonDefines // semi-colon delimited list of symbols (= value) 
	, const char* _SpecificDefines // semi-colon delimited list of symbols (= value)
	, const oVersion& _TargetShaderModel // shader model version to compile against
	, oGPU_PIPELINE_STAGE _Stage // type of shader to compile
	, const char* _ShaderPath // full path to shader - mostly for error reporting
	, const char* _EntryPoint // name of the top-level shader function to use
	, const char* _ShaderBody // a string of the loaded shader file (NOTE: This still goes out to includes, so will still touch the file system
	, oBuffer** _ppByteCode // if successful, this is a buffer filled with byte code
	, oBuffer** _ppErrors); // if failure, this is filled with a string of compile errors

#endif
