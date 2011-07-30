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
#pragma once
#ifndef oOBJLoader_h
#define oOBJLoader_h

#include <oooii/oMath.h>
#include <oooii/oSTL.h>

struct oOBJ
{
	struct GROUP
	{
		std::string GroupName;
		std::string MaterialName;
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

		std::string Name;

		float3 AmbientColor;
		float4 DiffuseColor;
		float3 SpecularColor;
		float Specularity;
		float Transparency;
		float RefractionIndex;
		ILLUM Illum;

		MATERIAL()
			: AmbientColor(0.0f, 0.0f, 0.0f)
			, DiffuseColor(0.0f, 0.0f, 0.0f, 1.0f)
			, SpecularColor(0.0f, 0.0f, 0.0f)
			, Specularity(0.0f)
			, Transparency(0.0f)
			, RefractionIndex(1.0f)
			, Illum(COLOR_ON_AMBIENT_OFF)
		{}

		std::string AmbientTexturePath;
		std::string DiffuseTexturePath;
		std::string AlphaTexturePath;
		std::string SpecularTexturePath;
		std::string BumpTexturePath;
	};

	std::string OBJPath;
	std::string MaterialLibraryPath;
	std::vector<float3> Positions;
	std::vector<float3> Normals;
	std::vector<float2> Texcoords;
	std::vector<unsigned int> Indices;
	std::vector<GROUP> Groups;
};

bool oOBJLoad(const char* _OBJPath, const char* _OBJString, bool _FlipFaces, oOBJ* _pOBJ);
bool oOBJLoadMTL(const char* _MTLPath, const char* _MTLString, std::vector<oOBJ::MATERIAL>* _pMTLLibrary);

#endif
