// $(header)
#pragma once
#ifndef oOBJLoader_h
#define oOBJLoader_h

#include <oBasis/oMath.h>
#include <oBasis/oFixedString.h>
#include <vector>

struct oOBJ
{
	struct GROUP
	{
		GROUP()
			: StartIndex(0)
			, NumIndices(0)
		{}

		oStringM GroupName;
		oStringM MaterialName;
		unsigned int StartIndex;
		unsigned int NumIndices;
	};

	struct MATERIAL
	{
		enum ILLUM
		{
			COLOR_ON_AMBIENT_OFF,
			COLOR_ON_AMBIENT_ON,
			HIGHLIGHT_ON,
			REFLECTION_ON_RAY_TRACE_ON,
			TRANSPARENCY_GLASS_ON_REFLECTION_RAY_TRACE_ON,
			REFLECTION_FRESNEL_ON_RAY_TRACE_ON,
			TRANSPARENCY_REFRACTION_ON_REFLECTION_FRESNEL_OFF_RAY_TRACE_ON,
			TRANSPARENCY_REFRACTION_ON_REFLECTION_FRENSEL_ON_RAY_TRACE_ON,
			REFLECTION_ON_RAY_TRACE_OFF,
			TRANSPARENCY_GLASS_ON_REFLECTION_RAY_TRACE_OFF,
			CASTS_SHADOWS_ONTO_INVISIBLE_SURFACES,
		};

		MATERIAL()
			: AmbientColor(0.0f, 0.0f, 0.0f)
			, DiffuseColor(0.0f, 0.0f, 0.0f, 1.0f)
			, SpecularColor(0.0f, 0.0f, 0.0f)
			, Specularity(0.0f)
			, Transparency(0.0f)
			, RefractionIndex(1.0f)
			, Illum(COLOR_ON_AMBIENT_OFF)
		{}

		float3 AmbientColor;
		float4 DiffuseColor;
		float3 SpecularColor;
		float Specularity;
		float Transparency;
		float RefractionIndex;
		ILLUM Illum;

		oStringPath Name;
		oStringPath AmbientTexturePath;
		oStringPath DiffuseTexturePath;
		oStringPath AlphaTexturePath;
		oStringPath SpecularTexturePath;
		oStringPath BumpTexturePath;
	};

	oStringPath OBJPath;
	oStringPath MaterialLibraryPath;
	std::vector<float3> Positions;
	std::vector<float3> Normals;
	std::vector<float2> Texcoords;
	std::vector<unsigned int> Indices;
	std::vector<GROUP> Groups;
};

// oOBJLoad hashes faces for sharing of vertices, but this can result in many
// small allocations internally that then need to be freed, which can take a 
// long time. As an optimization, client code can pass a hint in _InternalReserve
// to make internal memory management a lot faster.
bool oOBJLoad(const char* _OBJPath, const char* _OBJString, bool _FlipFaces, size_t _InternalReserve, oOBJ* _pOBJ);
bool oOBJLoadMTL(const char* _MTLPath, const char* _MTLString, std::vector<oOBJ::MATERIAL>* _pMTLLibrary);

#endif
