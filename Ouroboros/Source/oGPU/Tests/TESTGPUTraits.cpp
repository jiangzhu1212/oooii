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
#include <oGPU/oGPU.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static void test_trait(texture_type::value _Type)
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
		case texture_type::default_1d: Is1D = true; break;
		case texture_type::mipped_1d: Is1D = HasMips = true; break;
		case texture_type::render_target_1d: Is1D = IsRenderTarget = true; break;
		case texture_type::mipped_render_target_1d: Is1D = IsRenderTarget = HasMips = true; break;
		case texture_type::readback_1d: Is1D = IsReadBack = true; break;
		case texture_type::mipped_readback_1d: Is1D = HasMips = IsReadBack = true; break;
		case texture_type::default_2d: Is2D = true; break;
		case texture_type::mipped_2d: Is2D = HasMips = true; break;
		case texture_type::render_target_2d: Is2D = IsRenderTarget = true; break;
		case texture_type::mipped_render_target_2d: Is2D = IsRenderTarget = HasMips = true; break;
		case texture_type::readback_2d: Is2D = IsReadBack = true; break;
		case texture_type::mipped_readback_2d: Is2D = HasMips = IsReadBack = true; break;
		case texture_type::unordered_2d: Is2D = IsUnordered = true; break;
		case texture_type::default_cube: IsCube = true; break;
		case texture_type::mipped_cube: IsCube = HasMips = true; break;
		case texture_type::render_target_cube: IsCube = IsRenderTarget = true; break;
		case texture_type::mipped_render_target_cube: IsCube = IsRenderTarget = HasMips = true; break;
		case texture_type::readback_cube: IsCube = IsReadBack = true; break;
		case texture_type::mipped_readback_cube: IsCube = HasMips = IsReadBack = true; break;
		case texture_type::default_3d: Is3D = true; break;
		case texture_type::mipped_3d: Is3D = HasMips = true; break;
		case texture_type::render_target_3d: Is3D = IsRenderTarget = true; break;
		case texture_type::mipped_render_target_3d: Is3D = IsRenderTarget = HasMips = true; break;
		case texture_type::readback_3d: Is3D = IsReadBack = true; break;
		case texture_type::mipped_readback_3d: Is3D = HasMips = IsReadBack = true; break;
		default: oTHROW_INVARG("Unknown texture type %s", as_string(_Type));
	}

	oCHECK(is_mipped(_Type) == HasMips, "is_mipped incorrectly returning %s for %s", HasMips ? "false" : "true", as_string(_Type));
	oCHECK(is_readback(_Type) == IsReadBack, "is_readback incorrectly returning %s for %s", IsReadBack ? "false" : "true", as_string(_Type));
	oCHECK(is_render_target(_Type) == IsRenderTarget, "is_render_target incorrectly returning %s for %s", IsRenderTarget ? "false" : "true", as_string(_Type));
	oCHECK(is_1d(_Type) == Is1D, "is_1d incorrectly returning %s for %s", Is1D ? "false" : "true", as_string(_Type));
	oCHECK(is_2d(_Type) == Is2D, "is_2d incorrectly returning %s for %s", Is2D ? "false" : "true", as_string(_Type));
	oCHECK(is_cube(_Type) == IsCube, "is_cube incorrectly returning %s for %s", IsCube ? "false" : "true", as_string(_Type));
	oCHECK(is_3d(_Type) == Is3D, "is_3d incorrectly returning %s for %s", Is3D ? "false" : "true", as_string(_Type));
	oCHECK(is_unordered(_Type) == IsUnordered, "is_unordered incorrectly returning %s for %s", IsUnordered ? "false" : "true", as_string(_Type));
}

void TESTtexture_traits()
{
	test_trait(texture_type::default_1d);
	test_trait(texture_type::mipped_1d);
	test_trait(texture_type::render_target_1d);
	test_trait(texture_type::mipped_render_target_1d);
	test_trait(texture_type::readback_1d);
	test_trait(texture_type::mipped_readback_1d);

	test_trait(texture_type::default_2d);
	test_trait(texture_type::mipped_2d);
	test_trait(texture_type::render_target_2d);
	test_trait(texture_type::mipped_render_target_2d);
	test_trait(texture_type::readback_2d);
	test_trait(texture_type::mipped_readback_2d);

	test_trait(texture_type::default_cube);
	test_trait(texture_type::mipped_cube);
	test_trait(texture_type::render_target_cube);
	test_trait(texture_type::mipped_render_target_cube);
	test_trait(texture_type::readback_cube);
	test_trait(texture_type::mipped_readback_cube);

	test_trait(texture_type::default_3d);
	test_trait(texture_type::mipped_3d);
	test_trait(texture_type::render_target_3d);
	test_trait(texture_type::mipped_render_target_3d);
	test_trait(texture_type::readback_3d);
	test_trait(texture_type::mipped_readback_3d);
}

	} // namespace tests	
} // namespace ouro
