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
#include <oBasis/oGPUEnums.h>

// @oooii-tony: Currently defined in oD3D11 in oPlatform until I can unwind
// dependencies a bit more.

const char* oAsString(oGPU_API _API)
{
	switch (_API)
	{
		case oGPU_API_UNKNOWN: return "Unknown";
		case oGPU_API_D3D: return "Direct3D";
		case oGPU_API_OGL: return "OpenGL";
		case oGPU_API_OGLES: return "OpenGL ES";
		case oGPU_API_WEBGL: return "WebGL";
		case oGPU_API_CUSTOM: return "Custom";
		oNODEFAULT;
	}
};

const char* oAsString(oGPU_VENDOR _Vendor)
{
	switch (_Vendor)
	{
		case oGPU_VENDOR_UNKNOWN: return "Unknown";
		case oGPU_VENDOR_NVIDIA: return "NVIDIA";
		case oGPU_VENDOR_AMD: return "AMD";
		case oGPU_VENDOR_INTEL: return "Intel";
		case oGPU_VENDOR_ARM: return "ARM";
		case oGPU_VENDOR_CUSTOM: return "Custom";
		case oGPU_VENDOR_INTERNAL: return "Internal";
		oNODEFAULT;
	}
};

const char* oAsString(oGPU_DEBUG_LEVEL _DebugLevel)
{
	switch (_DebugLevel)
	{
		case oGPU_DEBUG_NONE: return "None";
		case oGPU_DEBUG_NORMAL: return "Normal";
		case oGPU_DEBUG_UNFILTERED: return "Unfiltered";
		oNODEFAULT;
	}
};

const char* oAsString(oGPU_PIPELINE_STAGE _Stage)
{
	switch (_Stage)
	{
		case oGPU_VERTEX_SHADER: return "oGPU_VERTEX_SHADER";
		case oGPU_HULL_SHADER: return "oGPU_HULL_SHADER";
		case oGPU_DOMAIN_SHADER: return "oGPU_DOMAIN_SHADER";
		case oGPU_GEOMETRY_SHADER: return "oGPU_GEOMETRY_SHADER";
		case oGPU_PIXEL_SHADER: return "oGPU_PIXEL_SHADER";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_RESOURCE_TYPE _Type)
{
	switch (_Type)
	{
		case oGPU_BUFFER: return "oGPU_BUFFER";
		case oGPU_INSTANCE_LIST: return "oGPU_INSTANCE_LIST";
		case oGPU_LINE_LIST: return "oGPU_LINE_LIST";
		case oGPU_MATERIAL: return "oGPU_MATERIAL";
		case oGPU_MATERIAL_SET: return "oGPU_MATERIAL_SET";
		case oGPU_MESH: return "oGPU_MESH";
		case oGPU_OUTPUT: return "oGPU_OUTPUT";
		case oGPU_SCENE: return "oGPU_SCENE";
		case oGPU_SKELETON: return "oGPU_SKELETON";
		case oGPU_SKELETON_POSE: return "oGPU_SKELETON_POSE";
		case oGPU_TEXTURE_RGB: return "oGPU_TEXTURE_RGB";
		case oGPU_TEXTURE_YUV: return "oGPU_TEXTURE_YUV";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_RESIDENCY _Residency)
{
	switch (_Residency)
	{
		case oGPU_UNINITIALIZED: return "oGPU_UNINITIALIZED";
		case oGPU_LOADING: return "oGPU_LOADING";
		case oGPU_LOAD_FAILED: return "oGPU_LOAD_FAILED";
		case oGPU_CONDENSING: return "oGPU_CONDENSING";
		case oGPU_CONDENSE_FAILED: return "oGPU_CONDENSE_FAILED";
		case oGPU_MAPPED: return "oGPU_MAPPED";
		case oGPU_RESIDENT: return "oGPU_RESIDENT";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_MESH_SUBRESOURCE _MeshSubresource)
{
	switch (_MeshSubresource)
	{
		case oGPU_MESH_RANGES: return "oGPU_MESH_RANGES";
		case oGPU_MESH_INDICES: return "oGPU_MESH_INDICES";
		case oGPU_MESH_VERTICES0: return "oGPU_MESH_VERTICES0";
		case oGPU_MESH_VERTICES1: return "oGPU_MESH_VERTICES1";
		case oGPU_MESH_VERTICES2: return "oGPU_MESH_VERTICES2";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_BUFFER_TYPE _Type)
{
	switch (_Type)
	{
		case oGPU_BUFFER_DEFAULT: return "oGPU_BUFFER_DEFAULT";
		case oGPU_BUFFER_READBACK: return "oGPU_BUFFER_READBACK";
		case oGPU_BUFFER_UNORDERED_RAW: return "oGPU_BUFFER_UNORDERED_RAW";
		case oGPU_BUFFER_UNORDERED_UNSTRUCTURED: return "oGPU_BUFFER_UNORDERED_UNSTRUCTURED";
		case oGPU_BUFFER_UNORDERED_STRUCTURED: return "oGPU_BUFFER_UNORDERED_STRUCTURED";
		case oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND: return "oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND";
		case oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER: return "oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_TEXTURE_TYPE _Type)
{
	switch (_Type)
	{
		case oGPU_TEXTURE_2D_MAP: return "oGPU_TEXTURE_2D_MAP";
		case oGPU_TEXTURE_2D_MAP_MIPS: return "oGPU_TEXTURE_2D_MAP_MIPS";
		case oGPU_TEXTURE_2D_RENDER_TARGET: return "oGPU_TEXTURE_2D_RENDER_TARGET";
		case oGPU_TEXTURE_2D_RENDER_TARGET_MIPS: return "oGPU_TEXTURE_2D_RENDER_TARGET_MIPS";
		case oGPU_TEXTURE_2D_READBACK: return "oGPU_TEXTURE_2D_READBACK";
		case oGPU_TEXTURE_2D_READBACK_MIPS: return "oGPU_TEXTURE_2D_READBACK_MIPS";
		case oGPU_TEXTURE_CUBE_MAP: return "oGPU_TEXTURE_CUBE_MAP";
		case oGPU_TEXTURE_CUBE_MAP_MIPS: return "oGPU_TEXTURE_CUBE_MAP_MIPS";
		case oGPU_TEXTURE_CUBE_RENDER_TARGET: return "oGPU_TEXTURE_CUBE_RENDER_TARGET";
		case oGPU_TEXTURE_CUBE_RENDER_TARGET_MIPS: return "oGPU_TEXTURE_CUBE_RENDER_TARGET_MIPS";
		case oGPU_TEXTURE_CUBE_READBACK: return "oGPU_TEXTURE_CUBE_READBACK";
		case oGPU_TEXTURE_CUBE_READBACK_MIPS: return "oGPU_TEXTURE_CUBE_READBACK_MIPS";
		case oGPU_TEXTURE_3D_MAP: return "oGPU_TEXTURE_3D_MAP";
		case oGPU_TEXTURE_3D_MAP_MIPS: return "oGPU_TEXTURE_3D_MAP_MIPS";
		case oGPU_TEXTURE_3D_RENDER_TARGET: return "oGPU_TEXTURE_3D_RENDER_TARGET";
		case oGPU_TEXTURE_3D_RENDER_TARGET_MIPS: return "oGPU_TEXTURE_3D_RENDER_TARGET_MIPS";
		case oGPU_TEXTURE_3D_READBACK: return "oGPU_TEXTURE_3D_READBACK";
		case oGPU_TEXTURE_3D_READBACK_MIPS: return "oGPU_TEXTURE_3D_READBACK_MIPS";
		case oGPU_TEXTURE_2D_MAP_UNORDERED: return "oGPU_TEXTURE_2D_MAP_UNORDERED";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_CUBE_FACE _Face)
{
	switch (_Face)
	{
		case oGPU_CUBE_POS_X: return "oGPU_CUBE_POS_X";
		case oGPU_CUBE_NEG_X: return "oGPU_CUBE_NEG_X";
		case oGPU_CUBE_POS_Y: return "oGPU_CUBE_POS_Y";
		case oGPU_CUBE_NEG_Y: return "oGPU_CUBE_NEG_Y";
		case oGPU_CUBE_POS_Z: return "oGPU_CUBE_POS_Z";
		case oGPU_CUBE_NEG_Z: return "oGPU_CUBE_NEG_Z";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_SURFACE_STATE _State)
{
	switch (_State)
	{
		case oGPU_FRONT_FACE: return "oGPU_FRONT_FACE";
		case oGPU_BACK_FACE: return "oGPU_BACK_FACE";
		case oGPU_TWO_SIDED: return "oGPU_TWO_SIDED";
		case oGPU_FRONT_WIREFRAME: return "oGPU_FRONT_WIREFRAME";
		case oGPU_BACK_WIREFRAME: return "oGPU_BACK_WIREFRAME";
		case oGPU_TWO_SIDED_WIREFRAME: return "oGPU_TWO_SIDED_WIREFRAME";
		case oGPU_FRONT_POINTS: return "oGPU_FRONT_POINTS";
		case oGPU_BACK_POINTS: return "oGPU_BACK_POINTS";
		case oGPU_TWO_SIDED_POINTS: return "oGPU_TWO_SIDED_POINTS";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_DEPTH_STENCIL_STATE _State)
{
	switch (_State)
	{
		case oGPU_DEPTH_STENCIL_NONE: return "oGPU_DEPTH_STENCIL_NONE";
		case oGPU_DEPTH_TEST_AND_WRITE: return "oGPU_DEPTH_STEST_AND_WRITE";
		case oGPU_DEPTH_TEST: return "oGPU_DEPTH_TEST";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_BLEND_STATE _State)
{
	switch (_State)
	{
		case oGPU_OPAQUE: return "oGPU_OPAQUE";
		case oGPU_ALPHA_TEST: return "oGPU_ALPHA_TEST";
		case oGPU_ACCUMULATE: return "oGPU_ACCUMULATE";
		case oGPU_MULTIPLY: return "oGPU_MULTIPLY";
		case oGPU_SCREEN: return "oGPU_SCREEN";
		case oGPU_ADDITIVE: return "oGPU_ADDITIVE";
		case oGPU_TRANSLUCENT: return "oGPU_TRANSLUCENT";
		case oGPU_MIN: return "oGPU_MIN";
		case oGPU_MAX: return "oGPU_MAX";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_SAMPLER_STATE _State)
{
	switch (_State)
	{
		case oGPU_POINT_CLAMP: return "oGPU_POINT_CLAMP";
		case oGPU_POINT_WRAP: return "oGPU_POINT_WRAP";
		case oGPU_LINEAR_CLAMP: return "oGPU_LINEAR_CLAMP";
		case oGPU_LINEAR_WRAP: return "oGPU_LINEAR_WRAP";
		case oGPU_ANISO_CLAMP: return "oGPU_ANISO_CLAMP";
		case oGPU_ANISO_WRAP: return "oGPU_ANISO_WRAP";

		case oGPU_POINT_CLAMP_BIAS_UP1: return "oGPU_POINT_CLAMP_BIAS_UP1";
		case oGPU_POINT_WRAP_BIAS_UP1: return "oGPU_POINT_WRAP_BIAS_UP1";
		case oGPU_LINEAR_CLAMP_BIAS_UP1: return "oGPU_LINEAR_CLAMP_BIAS_UP1";
		case oGPU_LINEAR_WRAP_BIAS_UP1: return "oGPU_LINEAR_WRAP_BIAS_UP1";
		case oGPU_ANISO_CLAMP_BIAS_UP1: return "oGPU_ANISO_CLAMP_BIAS_UP1";
		case oGPU_ANISO_WRAP_BIAS_UP1: return "oGPU_ANISO_WRAP_BIAS_UP1";

		case oGPU_POINT_CLAMP_BIAS_DOWN1: return "oGPU_POINT_CLAMP_BIAS_DOWN1";
		case oGPU_POINT_WRAP_BIAS_DOWN1: return "oGPU_POINT_WRAP_BIAS_DOWN1";
		case oGPU_LINEAR_CLAMP_BIAS_DOWN1: return "oGPU_LINEAR_CLAMP_BIAS_DOWN1";
		case oGPU_LINEAR_WRAP_BIAS_DOWN1: return "oGPU_LINEAR_WRAP_BIAS_DOWN1";
		case oGPU_ANISO_CLAMP_BIAS_DOWN1: return "oGPU_ANISO_CLAMP_BIAS_DOWN1";
		case oGPU_ANISO_WRAP_BIAS_DOWN1: return "oGPU_ANISO_WRAP_BIAS_DOWN1";

		case oGPU_POINT_CLAMP_BIAS_UP2: return "oGPU_POINT_CLAMP_BIAS_UP2";
		case oGPU_POINT_WRAP_BIAS_UP2: return "oGPU_POINT_WRAP_BIAS_UP2";
		case oGPU_LINEAR_CLAMP_BIAS_UP2: return "oGPU_LINEAR_CLAMP_BIAS_UP2";
		case oGPU_LINEAR_WRAP_BIAS_UP2: return "oGPU_LINEAR_WRAP_BIAS_UP2";
		case oGPU_ANISO_CLAMP_BIAS_UP2: return "oGPU_ANISO_CLAMP_BIAS_UP2";
		case oGPU_ANISO_WRAP_BIAS_UP2: return "oGPU_ANISO_WRAP_BIAS_UP2";

		case oGPU_POINT_CLAMP_BIAS_DOWN2: return "oGPU_POINT_CLAMP_BIAS_DOWN2";
		case oGPU_POINT_WRAP_BIAS_DOWN2: return "oGPU_POINT_WRAP_BIAS_DOWN2";
		case oGPU_LINEAR_CLAMP_BIAS_DOWN2: return "oGPU_LINEAR_CLAMP_BIAS_DOWN2";
		case oGPU_LINEAR_WRAP_BIAS_DOWN2: return "oGPU_LINEAR_WRAP_BIAS_DOWN2";
		case oGPU_ANISO_CLAMP_BIAS_DOWN2: return "oGPU_ANISO_CLAMP_BIAS_DOWN2";
		case oGPU_ANISO_WRAP_BIAS_DOWN2: return "oGPU_ANISO_WRAP_BIAS_DOWN2";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_CLEAR _Clear)
{
	switch (_Clear)
	{
		case oGPU_CLEAR_DEPTH: return "oGPU_CLEAR_DEPTH";
		case oGPU_CLEAR_STENCIL: return "oGPU_CLEAR_STENCIL";
		case oGPU_CLEAR_DEPTH_STENCIL: return "oGPU_CLEAR_DEPTH_STENCIL";
		case oGPU_CLEAR_COLOR: return "oGPU_CLEAR_COLOR";
		case oGPU_CLEAR_COLOR_DEPTH: return "oGPU_CLEAR_COLOR_DEPTH";
		case oGPU_CLEAR_COLOR_STENCIL: return "oGPU_CLEAR_COLOR_STENCIL";
		case oGPU_CLEAR_COLOR_DEPTH_STENCIL: return "oGPU_CLEAR_COLOR_DEPTH_STENCIL";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_NORMAL_SPACE _Space)
{
	switch (_Space)
	{
		case oGPU_NORMAL_LOCAL_SPACE: return "oGPU_NORMAL_LOCAL_SPACE";
		case oGPU_NORMAL_TANGENT_SPACE: return "oGPU_NORMAL_TANGENT_SPACE";
		case oGPU_NORMAL_BUMP_LOCAL_SPACE: return "oGPU_NORMAL_BUMP_LOCAL_SPACE";
		case oGPU_NORMAL_BUMP_TANGENT_SPACE: return "oGPU_NORMAL_BUMP_TANGENT_SPACE";
		oNODEFAULT;
	}
}

const char* oAsString(oGPU_BRDF_MODEL _Model)
{
	switch (_Model)
	{
		case oGPU_BRDF_PHONG: return "oGPU_BRDF_PHONG";
		case oGPU_BRDF_GOOCH: return "oGPU_BRDF_GOOCH";
		case oGPU_BRDF_MINNAERT: return "oGPU_BRDF_MINNAERT";
		case oGPU_BRDF_GAUSSIAN: return "oGPU_BRDF_GAUSSIAN";
		case oGPU_BRDF_BECKMANN: return "oGPU_BRDF_BECKMANN";
		case oGPU_BRDF_HEIDRICH_SEIDEL_ANISO: return "oGPU_BRDF_HEIDRICH_SEIDEL_ANISO"; 
		case oGPU_BRDF_WARD_ANISO: return "oGPU_BRDF_WARD_ANISO";
		case oGPU_BRDF_COOK_TORRANCE: return "oGPU_BRDF_COOK_TORRANCE";
		oNODEFAULT;
	}
}

#define oDEFINE_FROMSTRING(_EnumType, _NumEnums) bool oFromString(_EnumType* _pValue, const char* _StrSource) { return oEnumFromString<_NumEnums>(_pValue, _StrSource); }
oDEFINE_FROMSTRING(oGPU_PIPELINE_STAGE, oGPU_PIPELINE_NUM_STAGES);
oDEFINE_FROMSTRING(oGPU_RESOURCE_TYPE, oGPU_RESOURCE_NUM_TYPES);
oDEFINE_FROMSTRING(oGPU_RESIDENCY, oGPU_NUM_RESIDENCIES);
oDEFINE_FROMSTRING(oGPU_MESH_SUBRESOURCE, oGPU_MESH_NUM_SUBRESOURCES);
oDEFINE_FROMSTRING(oGPU_CUBE_FACE, oGPU_CUBE_NUM_FACES);
oDEFINE_FROMSTRING(oGPU_SURFACE_STATE, oGPU_SURFACE_NUM_STATES);
oDEFINE_FROMSTRING(oGPU_DEPTH_STENCIL_STATE, oGPU_DEPTH_STENCIL_NUM_STATES);
oDEFINE_FROMSTRING(oGPU_BLEND_STATE, oGPU_BLEND_NUM_STATES);
oDEFINE_FROMSTRING(oGPU_SAMPLER_STATE, oGPU_SAMPLER_NUM_STATES);
oDEFINE_FROMSTRING(oGPU_CLEAR, oGPU_NUM_CLEARS);
oDEFINE_FROMSTRING(oGPU_NORMAL_SPACE, oGPU_NORMAL_NUM_SPACES);
oDEFINE_FROMSTRING(oGPU_BRDF_MODEL, oGPU_BRDF_NUM_MODELS);

#define oDEFINE_TOSTRING(_EnumType) char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const _EnumType& _Value) { return oStrcpy(_StrDestination, _SizeofStrDestination, oAsString(_Value)); }
oDEFINE_TOSTRING(oGPU_PIPELINE_STAGE);
oDEFINE_TOSTRING(oGPU_RESOURCE_TYPE);
oDEFINE_TOSTRING(oGPU_RESIDENCY);
oDEFINE_TOSTRING(oGPU_MESH_SUBRESOURCE);
oDEFINE_TOSTRING(oGPU_TEXTURE_TYPE);
oDEFINE_TOSTRING(oGPU_CUBE_FACE);
oDEFINE_TOSTRING(oGPU_SURFACE_STATE);
oDEFINE_TOSTRING(oGPU_DEPTH_STENCIL_STATE);
oDEFINE_TOSTRING(oGPU_BLEND_STATE);
oDEFINE_TOSTRING(oGPU_SAMPLER_STATE);
oDEFINE_TOSTRING(oGPU_CLEAR);
oDEFINE_TOSTRING(oGPU_NORMAL_SPACE);
oDEFINE_TOSTRING(oGPU_BRDF_MODEL);

int oGPUGetSemanticIndex(const oFourCC& _FourCC)
{
	int a,b,c,d;
	oFourCCDecompose(_FourCC, &a, &b, &c, &d);
	if (d >= '0' && d <= '9')
		return d - '0';
	return 0;
}
