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
// Convenience "all headers" header for precompiled header files. Do NOT use 
// this to be lazy when including headers in .cpp files. Be explicit.
#pragma once
#ifndef oGPU_all_h
#define oGPU_all_h
#include <oGPU/blend_state.h>
#include <oGPU/color_target.h>
#include <oGPU/command_list.h>
#include <oGPU/compute_target.h>
#include <oGPU/constant_buffer.h>
#include <oGPU/depth_stencil_state.h>
#include <oGPU/depth_target.h>
#include <oGPU/device.h>
#include <oGPU/index_buffer.h>
#include <oGPU/primary_target.h>
#include <oGPU/rasterizer_state.h>
#include <oGPU/raw_buffer.h>
#include <oGPU/readback_buffer.h>
#include <oGPU/resource.h>
#include <oGPU/rwstructured_buffer.h>
#include <oGPU/sampler_state.h>
#include <oGPU/shader.h>
#include <oGPU/texture1d.h>
#include <oGPU/texture2d.h>
#include <oGPU/texture3d.h>
#include <oGPU/texturecube.h>
#include <oGPU/timer_query.h>
#include <oGPU/vertex_buffer.h>
#include <oGPU/vertex_layout.h>
#endif
