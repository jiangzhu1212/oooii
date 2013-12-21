/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
	RESULT TestTextureEnum(char* _StrStatus, size_t _SizeofStrStatus, ouro::gpu::texture_type::value _Type)
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
		case ouro::gpu::texture_type::default_1d:
			Is1D = true;
			break;
		case ouro::gpu::texture_type::mipped_1d:
			Is1D = true;
			HasMips = true;
			break;
		case ouro::gpu::texture_type::render_target_1d:
			Is1D = true;
			IsRenderTarget = true;
			break;
		case ouro::gpu::texture_type::mipped_render_target_1d:
			Is1D = true;
			IsRenderTarget = true;
			HasMips = true;
			break;
		case ouro::gpu::texture_type::readback_1d:
			Is1D = true;
			IsReadBack = true;
			break;
		case ouro::gpu::texture_type::mipped_readback_1d:
			Is1D = true;
			HasMips = true;
			IsReadBack = true;
			break;
		case ouro::gpu::texture_type::default_2d:
			Is2D = true;
			break;
		case ouro::gpu::texture_type::mipped_2d:
			Is2D = true;
			HasMips = true;
			break;
		case ouro::gpu::texture_type::render_target_2d:
			Is2D = true;
			IsRenderTarget = true;
			break;
		case ouro::gpu::texture_type::mipped_render_target_2d:
			Is2D = true;
			IsRenderTarget = true;
			HasMips = true;
			break;
		case ouro::gpu::texture_type::readback_2d:
			Is2D = true;
			IsReadBack = true;
			break;
		case ouro::gpu::texture_type::mipped_readback_2d:
			Is2D = true;
			HasMips = true;
			IsReadBack = true;
			break;
		case ouro::gpu::texture_type::unordered_2d:
			Is2D = true;
			IsUnordered = true;
			break;
		case ouro::gpu::texture_type::default_cube:
			IsCube = true;
			break;
		case ouro::gpu::texture_type::mipped_cube:
			IsCube = true;
			HasMips = true;
			break;
		case ouro::gpu::texture_type::render_target_cube:
			IsCube = true;
			IsRenderTarget = true;
			break;
		case ouro::gpu::texture_type::mipped_render_target_cube:
			IsCube = true;
			IsRenderTarget = true;
			HasMips = true;
			break;
		case ouro::gpu::texture_type::readback_cube:
			IsCube = true;
			IsReadBack = true;
			break;
		case ouro::gpu::texture_type::mipped_readback_cube:
			IsCube = true;
			HasMips = true;
			IsReadBack = true;
			break;
		case ouro::gpu::texture_type::default_3d:
			Is3D = true;
			break;
		case ouro::gpu::texture_type::mipped_3d:
			Is3D = true;
			HasMips = true;
			break;
		case ouro::gpu::texture_type::render_target_3d:
			Is3D = true;
			IsRenderTarget = true;
			break;
		case ouro::gpu::texture_type::mipped_render_target_3d:
			Is3D = true;
			IsRenderTarget = true;
			HasMips = true;
			break;
		case ouro::gpu::texture_type::readback_3d:
			Is3D = true;
			IsReadBack = true;
			break;
		case ouro::gpu::texture_type::mipped_readback_3d:
			Is3D = true;
			HasMips = true;
			IsReadBack = true;
			break;
		default:
			oTESTB(false, "Unknown texture type %s", ouro::as_string(_Type));
		}

		oTESTB(ouro::gpu::is_mipped(_Type) == HasMips, "gpu::is_mipped incorrectly returning %s for %s", HasMips ? "false" : "true", ouro::as_string(_Type));
		oTESTB(ouro::gpu::is_readback(_Type) == IsReadBack, "gpu::is_readback incorrectly returning %s for %s", IsReadBack ? "false" : "true", ouro::as_string(_Type));
		oTESTB(ouro::gpu::is_render_target(_Type) == IsRenderTarget, "gpu::is_render_target incorrectly returning %s for %s", IsRenderTarget ? "false" : "true", ouro::as_string(_Type));
		oTESTB(ouro::gpu::is_1d(_Type) == Is1D, "gpu::is_1d incorrectly returning %s for %s", Is1D ? "false" : "true", ouro::as_string(_Type));
		oTESTB(ouro::gpu::is_2d(_Type) == Is2D, "gpu::is_2d incorrectly returning %s for %s", Is2D ? "false" : "true", ouro::as_string(_Type));
		oTESTB(ouro::gpu::is_cube(_Type) == IsCube, "gpu::is_cube incorrectly returning %s for %s", IsCube ? "false" : "true", ouro::as_string(_Type));
		oTESTB(ouro::gpu::is_3d(_Type) == Is3D, "gpu::is_3d incorrectly returning %s for %s", Is3D ? "false" : "true", ouro::as_string(_Type));
		oTESTB(ouro::gpu::is_unordered(_Type) == IsUnordered, "gpu::is_unordered incorrectly returning %s for %s", IsUnordered ? "false" : "true", ouro::as_string(_Type));

		return SUCCESS;
	}
	bool TestTextureEnum(RESULT* _pResult, char* _StrStatus, size_t _SizeofStrStatus, ouro::gpu::texture_type::value _Type)
	{
		RESULT Result = TestTextureEnum( _StrStatus, _SizeofStrStatus, _Type);
		*_pResult = Result;
		return SUCCESS == Result;
	}
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		RESULT Result = SUCCESS;

		std::function<bool(ouro::gpu::texture_type::value _Type)> TestTextureTypeFn = 
			std::bind(&GPU_Traits::TestTextureEnum, this, &Result, _StrStatus, _SizeofStrStatus, std::placeholders::_1);
		
#define TEST_TEXTURE(_TextureType) \
	if(!TestTextureTypeFn(ouro::gpu::texture_type::_TextureType)) \
		return Result;

		TEST_TEXTURE(default_1d);
		TEST_TEXTURE(mipped_1d);
		TEST_TEXTURE(render_target_1d);
		TEST_TEXTURE(mipped_render_target_1d);
		TEST_TEXTURE(readback_1d);
		TEST_TEXTURE(mipped_readback_1d);

		TEST_TEXTURE(default_2d);
		TEST_TEXTURE(mipped_2d);
		TEST_TEXTURE(render_target_2d);
		TEST_TEXTURE(mipped_render_target_2d);
		TEST_TEXTURE(readback_2d);
		TEST_TEXTURE(mipped_readback_2d);

		TEST_TEXTURE(default_cube);
		TEST_TEXTURE(mipped_cube);
		TEST_TEXTURE(render_target_cube);
		TEST_TEXTURE(mipped_render_target_cube);
		TEST_TEXTURE(readback_cube);
		TEST_TEXTURE(mipped_readback_cube);

		TEST_TEXTURE(default_3d);
		TEST_TEXTURE(mipped_3d);
		TEST_TEXTURE(render_target_3d);
		TEST_TEXTURE(mipped_render_target_3d);
		TEST_TEXTURE(readback_3d);
		TEST_TEXTURE(mipped_readback_3d);

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_Traits);
