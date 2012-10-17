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
// This header is compiled both by HLSL and C++. It contains a robust set of 
// material-based parameters fit for use as a constant buffer in a 3D rendering 
// system. This set of constants is loosely based on VRay's Super-Shader, so it
// should provide a robust set of parameters for photo-real rendering. There are 
// some additional parameters for performance optimizations such as parallax
// mapping and alpha test.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGPUMaterialConstants_h
#define oGPUMaterialConstants_h

#include <oPlatform/oHLSL.h>

#ifdef oHLSL
	#define oGPU_BRDF_MODEL int
	#define oGPU_NORMAL_SPACE int

	#ifndef oGPU_MATERIAL_CONSTANT_BUFFER
		#define oGPU_MATERIAL_CONSTANT_BUFFER GPUMaterialConstants
	#endif
#else
	#include <oBasis/oGPUEnums.h>
	#include <oBasis/oINI.h>
#endif

// Optionally redefine which constant buffer is used. This can be used as an 
// index in C++.
#ifndef oGPU_MATERIAL_CONSTANT_BUFFER_REGISTER
	#define oGPU_MATERIAL_CONSTANT_BUFFER_REGISTER 2
#endif

// Light color inputs (expected RGB)
#define oGPU_MATERIAL_TEXTURE_DIFFUSE 0
#define oGPU_MATERIAL_TEXTURE_EMISSIVE 1
#define oGPU_MATERIAL_TEXTURE_SPECULAR 2
#define oGPU_MATERIAL_TEXTURE_SPECULARITY 3
#define oGPU_MATERIAL_TEXTURE_REFLECTION 4
#define oGPU_MATERIAL_TEXTURE_REFRACTION 5
#define oGPU_MATERIAL_TEXTURE_REFRACTION_SPECULAR 6
#define oGPU_MATERIAL_TEXTURE_REFRACTION_SPECULARITY 7
#define oGPU_MATERIAL_TEXTURE_TRANSMISSION 8

// Pixel geometry inputs (intensity/normal maps)
#define oGPU_MATERIAL_OPACITY 9
#define oGPU_MATERIAL_NORMAL 10
#define oGPU_MATERIAL_HEIGHT 11

// Adjustment maps (expected intensity)
#define oGPU_MATERIAL_TEXTURE_DIFFUSE_AMOUNT 12
#define oGPU_MATERIAL_TEXTURE_REFLECTION_AMOUNT 13
#define oGPU_MATERIAL_TEXTURE_REFRACTION_AMOUNT 14

struct oGPUMaterialConstants
{
	// Based off VRay's Super-Shader.
	// Probably want to add incidence? Or specialize separately for carpaint/ocean shaders?

	// All oRGBf values can be overridden with an RGB texture
	// All floats can be replaced with a 1-channel intensity texture
#ifndef oHLSL
	oGPUMaterialConstants()
		: DiffuseColor(std::White)
		, DiffuseAmount(1.0f)
		, EmissiveColor(std::Black)
		, Opacity(1.0f)
		, ReflectionColor(std::Black)
		, ReflectionAmount(1.0f)
		, SpecularColor(std::Black)
		, Specularity(0.1f)
		, RefractionColor(std::Black)
		, RefractionAmount(1.0f)
		, RefractiveSpecularColor(std::Black)
		, RefractiveSpecularity(0.1f)
		, BRDFModel(oGPU_BRDF_PHONG)
		, IndexOfRefraction(0.0f)
		, BumpScale(1.0f)
		, BumpDeltaScale(1.0f)
		, BumpSpace(oGPU_NORMAL_LOCAL_SPACE)
		, ForwardBackwardCoeff(0.0f)
		, ScatterCoeff(0.0f)
		, TransmissionColor(std::Black)
		, Thickness(0.0f)
		, LightShadingContribution(1.0f)
		, ParallaxHeightScale(1.0f)
		, ParallaxHeightBias(0.0f)
		, AlphaTestReference(0.0f)
		, Pad0(0.0f)
	{}

#endif

	// Basics
	oRGBf DiffuseColor;
	float DiffuseAmount;
	oRGBf EmissiveColor;
	float Opacity;

	// Reflection
	oRGBf ReflectionColor;
	float ReflectionAmount;
	oRGBf SpecularColor;
	float Specularity; // shininess

	// Refraction
	oRGBf RefractionColor;
	float RefractionAmount;
	oRGBf RefractiveSpecularColor;
	float RefractiveSpecularity;
	oGPU_BRDF_MODEL BRDFModel;
	float IndexOfRefraction;

	// Bump and Normal mapping
	float BumpScale;
	float BumpDeltaScale;
	oGPU_NORMAL_SPACE BumpSpace; // world, tangent, bump_world, bump_tangent

	// Subsurface scattering
	float ForwardBackwardCoeff;
	float ScatterCoeff;
	float Thickness;
	oRGBf TransmissionColor;

	float LightShadingContribution;
	float ParallaxHeightScale;
	float ParallaxHeightBias;
	float AlphaTestReference;
	float Pad0;
};

#ifdef oHLSL
	cbuffer cbuffer_GPUMaterialConstants : register(oCONCAT(b, oGPU_MATERIAL_CONSTANT_BUFFER_REGISTER)) { oGPUMaterialConstants oGPU_MATERIAL_CONSTANT_BUFFER; }
#else

// Reads values from the specified INI section. The exact name of the field is
// used as the key. Values are read using oFromString. Any field not read is
// left unwritten, so any initialization to default values must occur before 
// passing the constants struct to this function. This will return true only 
// if all fields are read correctly, so a return value of false may not 
// indicate a fatal error.
oAPI bool oGPUMaterialConstantsRead(const threadsafe oINI* _pINI, oINI::HSECTION _hSection, oGPUMaterialConstants* _pConstants);

// Write to the specified string buffer an INI section's body (no header), 
// which is a list of key/value pairs of field values written using oToString.
oAPI bool oGPUMaterialConstantsWrite(char* _StrDestination, size_t _SizeofStrDestination, const oGPUMaterialConstants& _Constants);

#endif
#endif