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
// This file contains definitions used in both C++ and shader languages authored
// for the oGPU system.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGPUShared_h
#define oGPUShared_h

#ifdef oHLSL
	#include <oPlatform/oHLSL.h>
#else
	#include <oBasis/oHLSLMath.h>
#endif

// Reproduced from <d3d11.h> to be used in shader
#define D3D11_MAX_NUM_SRVS (128)

// To emulate YUV formats both that are slated for HW support and for those 
// formats not yet on GPU roadmaps, provide an emulation system that auto-binds
// oGPUTextures of YUV-like formats like a regular RGB(A) texture for Y or AY, 
// but additionally binds a second texture for UV. This addition is back-indexed
// from the end of the t registers. Use this macro to abstract that. Example:
//
// texture2D MyYUVDiffuseAY : t0
// texture2D MyYUVDiffuseUV : oGPU_SECONDARY_SHADER_RESOURCE_REGISTER(0)
#define oGPU_SECONDARY_SHADER_RESOURCE_REGISTER(_PrimaryResourceIndex) oCONCAT(t,D3D11_MAX_NUM_SRVS-1-_PrimaryResourceIndex)

#endif
