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
#include <oSurface/resize.h>
#include <oSurface/codec.h>
#include <oSurface/tests/oSurfaceTestRequirements.h>
#include <oBase/throw.h>
#include <oBase/timer.h>
#include <vector>

namespace ouro { 
	namespace tests {

static void TESTsurface_resize_test_size(requirements& _Requirements
	, const surface::buffer* _pBuffer
	, surface::filter::value _Filter
	, const int3& _NewSize
	, int _NthImage)
{
	surface::info srcInfo = _pBuffer->get_info();
	surface::info destInfo = srcInfo;
	destInfo.dimensions = _NewSize;
	std::shared_ptr<surface::buffer> dst = surface::buffer::make(destInfo);
	{
		surface::shared_lock lock(_pBuffer);
		surface::lock_guard lock2(dst);

		scoped_timer timer("resize time");
		surface::resize(srcInfo, lock.mapped, destInfo, &lock2.mapped, _Filter);
	}

	_Requirements.check(dst, _NthImage);
}

static void TESTsurface_resize_test_filter(requirements& _Requirements
	, const surface::buffer* _pBuffer
	, surface::filter::value _Filter
	, int _NthImage)
{
	TESTsurface_resize_test_size(_Requirements, _pBuffer, _Filter, _pBuffer->get_info().dimensions * int3(2,2,1), _NthImage);
	TESTsurface_resize_test_size(_Requirements, _pBuffer, _Filter, _pBuffer->get_info().dimensions / int3(2,2,1), _NthImage+1);
}

void TESTsurface_resize(requirements& _Requirements)
{
	size_t Size = 0;
	std::shared_ptr<char> b = _Requirements.load_buffer(path("Test/Textures/lena_1.png"), &Size);
	std::shared_ptr<surface::buffer> s = surface::decode(b.get(), Size);

	int NthImage = 0;
	for (int i = 0; i < surface::filter::filter_count; i++, NthImage += 2)
	{
		TESTsurface_resize_test_filter(_Requirements, s.get(), surface::filter::value(i), NthImage);
	}
}

	} // namespace tests
} // namespace ouro
