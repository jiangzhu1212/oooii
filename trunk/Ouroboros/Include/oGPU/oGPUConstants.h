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
// This header defines some common constants for various rendering-related 
// resources (mostly max sizes). This cross-compiles between C++ and shader 
// languages.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGPUConstants_h
#define oGPUConstants_h

static const uint oGPU_MAX_NUM_PICKS_PER_FRAME = 16;
static const uint oGPU_MAX_NUM_THREAD_GROUPS_PER_DIMENSION = 65535;
static const uint oGPU_MAX_NUM_THREAD_GROUPS_PER_DIMENSION_MASK = 0xffff;
static const uint oGPU_MAX_NUM_THREAD_GROUPS_PER_DIMENSION_SHIFT = 16;

// @oooii-tony: These should be phased out...
static const uint oGPU_MAX_NUM_ENVIRONMENT_TEXTURES = 16;
static const uint oGPU_MAX_NUM_MATERIAL_TEXTURES = 16;

#endif
