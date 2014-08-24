/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
// Treat D3DCompile and associated API more uniformly with FXC.
#pragma once
#ifndef oGPU_d3d_compile_h
#define oGPU_d3d_compile_h

#include <oBase/allocate.h>
#include <oBase/path.h>
#include <oBase/version.h>
#include <oGPU/shader.h>
#include "d3d_util.h"

enum D3D_FEATURE_LEVEL;

namespace ouro { namespace gpu { namespace d3d {

// Create a set of command line options that you would pass to fxc and the 
// loaded source to compile to returns the compiled bytecode allocated from
// the specified allocator.
scoped_allocation compile_shader(const char* cmdline_options, const path& shader_source_path, const char* shader_source, const allocator& alloc);

// Given a shader model (i.e. 4.0) return a feature level (i.e. D3D_FEATURE_LEVEL_10_1)
D3D_FEATURE_LEVEL feature_level(const version& shader_model);

// Returns the shader profile for the specified stage of the specified feature
// level. If the specified feature level does not support the specified stage,
// this will return nullptr.
const char* shader_profile(D3D_FEATURE_LEVEL level, const stage::value& stage);

}}}

#endif
