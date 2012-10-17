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
// This header defines some common constants for various rendering-related 
// resources (mostly max sizes). This cross-compiles between C++ and shader 
// languages.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGPUConstants_h
#define oGPUConstants_h

// @oooii-tony: I wanted these at 20, but because of sampler slot requirements
// I can't. NOTE: There are 4 real sampler states: point/linear clamp/wrap. The
// rest are aniso settings or mip bias settings. So we can map from several 
// settings to a 16-member subset... come back to this.
static const int oGPU_MAX_NUM_SAMPLERS = 16;
static const int oGPU_MAX_NUM_MATERIAL_TEXTURES = 16;
static const int oGPU_MAX_NUM_ENVIRONMENT_TEXTURES = 16;
static const int oGPU_MAX_NUM_MRTS = 8;
static const int oGPU_MAX_NUM_PICKS_PER_FRAME = 16;

#endif
