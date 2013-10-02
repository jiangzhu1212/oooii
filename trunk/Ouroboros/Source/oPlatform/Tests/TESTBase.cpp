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
#include <oBase/tests/oBaseTests.h>
#include <oCore/filesystem.h>
#include "oTestIntegration.h"
#include <cstdlib>

namespace ouro {
	namespace tests {

struct requirements_implementation : requirements
{
	int rand() override { return ::rand(); }

	void vreport(const char* _Format, va_list _Args) override
	{
		oErrorSetLastV(0, _Format, _Args);
		oTRACEA("%s", oErrorGetLastString());
	}

	std::shared_ptr<char> load_buffer(const path& _Path, size_t* _pSize = nullptr)
	{
		path FullPath = filesystem::data_path() / _Path;

		return std::move(filesystem::load(FullPath, filesystem::load_option::binary_read, _pSize));
	}

	std::shared_ptr<surface::buffer> load_surface(const path& _Path)
	{
		path FullPath = filesystem::data_path() / _Path;
		intrusive_ptr<oImage> image;
		if (!oImageLoad(FullPath, &image))
			oThrowLastError();

		surface::info si = oImageGetSurfaceInfo(image);
		std::shared_ptr<surface::buffer> b = surface::buffer::make(si);
		surface::const_mapped_subresource msr = oImageGetMappedSubresource(image);
		b->update_subresource(0, msr);
		return b;
	}

	void check(const surface::info& _SourceInfo
		, const surface::const_mapped_subresource& _Source
		, int _NthTest = 0
		, float _MaxRMSError = -1.0f)
	{
		intrusive_ptr<oImage> image;
		if (!oImageCreate("check image", _SourceInfo, &image))
			oThrowLastError();
		
		image->CopyData(_Source.data, _Source.row_pitch);
		extern oTest* g_Test;
		if (!g_Test->TestImage(image, _NthTest, oDEFAULT, _MaxRMSError, oDEFAULT))
			oThrowLastError();
	}
};

	} // namespace tests
} // namespace oStd

using namespace ouro::tests;

#define oTEST_REGISTER_BASE_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_BASE_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))

#define oTEST_REGISTER_BASE_TEST_BUGGED0(_Name) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_BASE_TEST_BUGGED(_Name) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))

oTEST_REGISTER_BASE_TEST(atof);
oTEST_REGISTER_BASE_TEST0(csv);
oTEST_REGISTER_BASE_TEST(date);
oTEST_REGISTER_BASE_TEST0(fourcc);
oTEST_REGISTER_BASE_TEST0(ini);
oTEST_REGISTER_BASE_TEST0(path);
oTEST_REGISTER_BASE_TEST(surface_codec);
oTEST_REGISTER_BASE_TEST(surface_resize);
oTEST_REGISTER_BASE_TEST0(uri);
oTEST_REGISTER_BASE_TEST0(xml);
