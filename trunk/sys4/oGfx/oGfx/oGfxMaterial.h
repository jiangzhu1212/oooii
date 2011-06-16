// $(header)
// NOTE: This header is compiled by both HLSL and C++
#pragma once
#ifndef oHLSLMaterial_h
#define oHLSLMaterial_h

#include <oooii/oHLSL.h>

#ifndef oHLSL
	enum oTEXTURE_CHANNEL
	{
		oTEXTURE_CHANNEL_EMISSIVE = 0,
		oTEXTURE_CHANNEL_DIFFUSE = 1,
		oTEXTURE_CHANNEL_SPECULAR = 2,
		oTEXTURE_CHANNEL_NORMAL = 3,
		oTEXTURE_CHANNEL_REFLECTION = 4,
		oTEXTURE_CHANNEL_REFRACTION = 5,
		oTEXTURE_CHANNEL_TRANSMISSION = 6,
		oTEXTURE_CHANNEL_COUNT = 7,
	};
#else
	#define oTEXTURE_CHANNEL_EMISSIVE 0
	#define oTEXTURE_CHANNEL_DIFFUSE 1
	#define oTEXTURE_CHANNEL_SPECULAR 2
	#define oTEXTURE_CHANNEL_NORMAL 3
	#define oTEXTURE_CHANNEL_REFLECTION 4
	#define oTEXTURE_CHANNEL_REFRACTION 5
	#define oTEXTURE_CHANNEL_TRANSMISSION 6
	#define oTEXTURE_CHANNEL_COUNT 7
#endif

struct oCBMaterial
{
	oHLSLColor Emissive;
	float AlphaTest; // [0,1]
	oHLSLColor Diffuse;
	float Opacity; // [0,1]
	oHLSLColor Specular;
	float Shininess; // [0,1]

	float LightingScale; // [0,1]
	float Unused0;
	float Unused1;
	float Unused2;

	#ifndef oHLSL
		oCBMaterial()
			: Emissive(std::Black)
			, AlphaTest(0.0f)
			, Diffuse(std::White)
			, Opacity(1.0f)
			, Specular(std::White)
			, Shininess(0.1f)
			, LightingScale(1.0f)
		{}
		oHLSLCheckSize(oCBMaterial);
	#endif
};

#endif
