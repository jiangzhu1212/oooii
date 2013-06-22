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
// This header is compiled both by HLSL and C++. It contains a robust set of 
// light-based parameters fit for use as a constant buffer in a 3D rendering 
// system, including forward, deferred and inferred setups. Use of oGPU_ does 
// not require that this be used, but for the vast majority of use cases this 
// provides a robust solution and thus has been factored out as utility code to 
// use, or be the basis of a new, more fitted solution.
#ifndef oHLSL
#pragma once
#endif
#ifndef oGfxLightConstants_h
#define oGfxLightConstants_h

#include <oGfx/oGfxHLSL.h>
//asdfsadf
static const uint oGPU_MAX_NUM_LIGHTS_PER_PASS = 256;

// Optionally redefine which constant buffer is used. This can be used as an 
// index in C++.

#ifndef oGFX_LIGHT_CONSTANTS_REGISTER
	#define oGFX_LIGHT_CONSTANTS_REGISTER 3
#endif

struct oGfxLight
{
	float3 WSPosition;
	float Intensity;
};

struct oGfxLightConstants
{
	// Constant buffer to represent light-dependent parameters used in most 
	// straightforward rendering.
	#ifndef oHLSL
		oGfxLightConstants(const oGfxLight* _pLights, uint _NumLights) { Set(_pLights, _NumLights); }

		oGfxLightConstants(const oGfxLight& _Light) { Set(&_Light, 1); }
		oGfxLightConstants() : NumLights(0) {}

		void Set(const oGfxLight* _pLights, uint _NumLights)
		{
			oASSERT(_NumLights <= oGPU_MAX_NUM_LIGHTS_PER_PASS, "_LightCount of %d exceeds %d", _NumLights, oGPU_MAX_NUM_LIGHTS_PER_PASS);
			if (_pLights)
				memcpy(Lights, _pLights, min(oGPU_MAX_NUM_LIGHTS_PER_PASS, _NumLights) * sizeof(oGfxLight));
			NumLights = _NumLights;
			Pad = int3(0,0,0);
		}

		void Add(const oGfxLight& _Light)
		{
			oASSERT(NumLights < oGPU_MAX_NUM_LIGHTS_PER_PASS, "AddLight has reached %d lights", oGPU_MAX_NUM_LIGHTS_PER_PASS);
			Lights[NumLights++] = _Light;
		}
protected:
	#endif

	oGfxLight Lights[oGPU_MAX_NUM_LIGHTS_PER_PASS];
	uint NumLights;
	uint3 Pad;
};

#ifdef oHLSL
cbuffer cbuffer_GfxLightConstants : register(oCONCAT(b, oGFX_LIGHT_CONSTANTS_REGISTER)) { oGfxLightConstants GfxLightConstants; }

uint oGfxGetNumLights()
{
	return min(GfxLightConstants.NumLights, oGPU_MAX_NUM_LIGHTS_PER_PASS);
}

oGfxLight oGfxGetLight(uint _Light)
{
	return GfxLightConstants.Lights[_Light];
}

#endif

#endif
