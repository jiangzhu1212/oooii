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
#include <oPlatform/oTest.h>
#include <oBasis/oGPUConcepts.h>


struct GPU_Traits : public oTest
{
	RESULT TestTextureEnum(char* _StrStatus, size_t _SizeofStrStatus, oGPU_TEXTURE_TYPE _Type)
	{
		bool Is1D = false;
		bool Is2D = false;
		bool Is3D = false;
		bool IsCube = false;
		bool HasMips = false;
		bool IsRenderTarget = false;
		bool IsUnordered = false;
		bool IsReadBack = false;

		switch(_Type)
		{
		case oGPU_TEXTURE_1D_MAP:
			Is1D = true;
			break;
		case oGPU_TEXTURE_1D_MAP_MIPS:
			Is1D = true;
			HasMips = true;
			break;
		case oGPU_TEXTURE_1D_RENDER_TARGET:
			Is1D = true;
			IsRenderTarget = true;
			break;
		case oGPU_TEXTURE_1D_RENDER_TARGET_MIPS:
			Is1D = true;
			IsRenderTarget = true;
			HasMips = true;
			break;
		case oGPU_TEXTURE_1D_READBACK:
			Is1D = true;
			IsReadBack = true;
			break;
		case oGPU_TEXTURE_1D_READBACK_MIPS:
			Is1D = true;
			HasMips = true;
			IsReadBack = true;
			break;
		case oGPU_TEXTURE_2D_MAP:
			Is2D = true;
			break;
		case oGPU_TEXTURE_2D_MAP_MIPS:
			Is2D = true;
			HasMips = true;
			break;
		case oGPU_TEXTURE_2D_RENDER_TARGET:
			Is2D = true;
			IsRenderTarget = true;
			break;
		case oGPU_TEXTURE_2D_RENDER_TARGET_MIPS:
			Is2D = true;
			IsRenderTarget = true;
			HasMips = true;
			break;
		case oGPU_TEXTURE_2D_READBACK:
			Is2D = true;
			IsReadBack = true;
			break;
		case oGPU_TEXTURE_2D_READBACK_MIPS:
			Is2D = true;
			HasMips = true;
			IsReadBack = true;
			break;
		case oGPU_TEXTURE_2D_MAP_UNORDERED:
			Is2D = true;
			IsUnordered = true;
			break;
		case oGPU_TEXTURE_CUBE_MAP:
			IsCube = true;
			break;
		case oGPU_TEXTURE_CUBE_MAP_MIPS:
			IsCube = true;
			HasMips = true;
			break;
		case oGPU_TEXTURE_CUBE_RENDER_TARGET:
			IsCube = true;
			IsRenderTarget = true;
			break;
		case oGPU_TEXTURE_CUBE_RENDER_TARGET_MIPS:
			IsCube = true;
			IsRenderTarget = true;
			HasMips = true;
			break;
		case oGPU_TEXTURE_CUBE_READBACK:
			IsCube = true;
			IsReadBack = true;
			break;
		case oGPU_TEXTURE_CUBE_READBACK_MIPS:
			IsCube = true;
			HasMips = true;
			IsReadBack = true;
			break;
		case oGPU_TEXTURE_3D_MAP:
			Is3D = true;
			break;
		case oGPU_TEXTURE_3D_MAP_MIPS:
			Is3D = true;
			HasMips = true;
			break;
		case oGPU_TEXTURE_3D_RENDER_TARGET:
			Is3D = true;
			IsRenderTarget = true;
			break;
		case oGPU_TEXTURE_3D_RENDER_TARGET_MIPS:
			Is3D = true;
			IsRenderTarget = true;
			HasMips = true;
			break;
		case oGPU_TEXTURE_3D_READBACK:
			Is3D = true;
			IsReadBack = true;
			break;
		case oGPU_TEXTURE_3D_READBACK_MIPS:
			Is3D = true;
			HasMips = true;
			IsReadBack = true;
			break;
		default:
			oTESTB(false, "Unknown texture type %s", oAsString(_Type));
		}

		oTESTB(oGPUTextureTypeHasMips(_Type) == HasMips, "oGPUTextureTypeHasMips incorrectly returning %s for %s", HasMips ? "false" : "true", oAsString(_Type));
		oTESTB(oGPUTextureTypeIsReadback(_Type) == IsReadBack, "oGPUTextureTypeIsReadback incorrectly returning %s for %s", IsReadBack ? "false" : "true", oAsString(_Type));
		oTESTB(oGPUTextureTypeIsRenderTarget(_Type) == IsRenderTarget, "oGPUTextureTypeIsRenderTarget incorrectly returning %s for %s", IsRenderTarget ? "false" : "true", oAsString(_Type));
		oTESTB(oGPUTextureTypeIs1DMap(_Type) == Is1D, "oGPUTextureTypeIs1DMap incorrectly returning %s for %s", Is1D ? "false" : "true", oAsString(_Type));
		oTESTB(oGPUTextureTypeIs2DMap(_Type) == Is2D, "oGPUTextureTypeIs2DMap incorrectly returning %s for %s", Is2D ? "false" : "true", oAsString(_Type));
		oTESTB(oGPUTextureTypeIsCubeMap(_Type) == IsCube, "oGPUTextureTypeIsCubeMap incorrectly returning %s for %s", IsCube ? "false" : "true", oAsString(_Type));
		oTESTB(oGPUTextureTypeIs3DMap(_Type) == Is3D, "oGPUTextureTypeIs3DMap incorrectly returning %s for %s", Is3D ? "false" : "true", oAsString(_Type));
		oTESTB(oGPUTextureTypeIsUnordered(_Type) == IsUnordered, "oGPUTextureTypeIsUnordered incorrectly returning %s for %s", IsUnordered ? "false" : "true", oAsString(_Type));

		return SUCCESS;
	}
	bool TestTextureEnum(RESULT* _pResult, char* _StrStatus, size_t _SizeofStrStatus, oGPU_TEXTURE_TYPE _Type)
	{
		RESULT Result = TestTextureEnum( _StrStatus, _SizeofStrStatus, _Type);
		*_pResult = Result;
		return SUCCESS == Result;
	}
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		RESULT Result = SUCCESS;

		oFUNCTION<bool(oGPU_TEXTURE_TYPE _Type)> TestTextureTypeFn = 
			oBIND(&GPU_Traits::TestTextureEnum, this, &Result, _StrStatus, _SizeofStrStatus, oBIND1);
		
#define TEST_TEXTURE(_TextureType) \
	if(!TestTextureTypeFn(_TextureType)) \
		return Result;

		TEST_TEXTURE(oGPU_TEXTURE_1D_MAP);
		TEST_TEXTURE(oGPU_TEXTURE_1D_MAP_MIPS);
		TEST_TEXTURE(oGPU_TEXTURE_1D_RENDER_TARGET);
		TEST_TEXTURE(oGPU_TEXTURE_1D_RENDER_TARGET_MIPS);
		TEST_TEXTURE(oGPU_TEXTURE_1D_READBACK);
		TEST_TEXTURE(oGPU_TEXTURE_1D_READBACK_MIPS);

		TEST_TEXTURE(oGPU_TEXTURE_2D_MAP);
		TEST_TEXTURE(oGPU_TEXTURE_2D_MAP_MIPS);
		TEST_TEXTURE(oGPU_TEXTURE_2D_RENDER_TARGET);
		TEST_TEXTURE(oGPU_TEXTURE_2D_RENDER_TARGET_MIPS);
		TEST_TEXTURE(oGPU_TEXTURE_2D_READBACK);
		TEST_TEXTURE(oGPU_TEXTURE_2D_READBACK_MIPS);
		TEST_TEXTURE(oGPU_TEXTURE_2D_MAP_UNORDERED);

		TEST_TEXTURE(oGPU_TEXTURE_CUBE_MAP);
		TEST_TEXTURE(oGPU_TEXTURE_CUBE_MAP_MIPS);
		TEST_TEXTURE(oGPU_TEXTURE_CUBE_RENDER_TARGET);
		TEST_TEXTURE(oGPU_TEXTURE_CUBE_RENDER_TARGET_MIPS);
		TEST_TEXTURE(oGPU_TEXTURE_CUBE_READBACK);
		TEST_TEXTURE(oGPU_TEXTURE_CUBE_READBACK_MIPS);

		TEST_TEXTURE(oGPU_TEXTURE_3D_MAP);
		TEST_TEXTURE(oGPU_TEXTURE_3D_MAP_MIPS);
		TEST_TEXTURE(oGPU_TEXTURE_3D_RENDER_TARGET);
		TEST_TEXTURE(oGPU_TEXTURE_3D_RENDER_TARGET_MIPS);
		TEST_TEXTURE(oGPU_TEXTURE_3D_READBACK);
		TEST_TEXTURE(oGPU_TEXTURE_3D_READBACK_MIPS);

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_Traits);
