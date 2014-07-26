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
#include <oBase/enum_iterator.h>
#include <oBase/throw.h>
#include <oBase/timer.h>
#include <vector>

#include "../../test_services.h"

namespace ouro { 
	namespace tests {

static void TESTsurface_resize_test_size(test_services& _Services
	, const surface::texel_buffer& _Buffer
	, surface::filter _Filter
	, const int3& _NewSize
	, int _NthImage)
{
	surface::info srcInfo = _Buffer.get_info();
	surface::info destInfo = srcInfo;
	destInfo.dimensions = _NewSize;
	surface::texel_buffer dst(destInfo);
	{
		surface::shared_lock lock(_Buffer);
		surface::lock_guard lock2(dst);

		scoped_timer timer("resize time");
		surface::resize(srcInfo, lock.mapped, destInfo, lock2.mapped, _Filter);
	}

	_Services.check(dst, _NthImage);
}

static void TESTsurface_resize_test_filter(test_services& _Services
	, const surface::texel_buffer& _Buffer
	, surface::filter _Filter
	, int _NthImage)
{
	TESTsurface_resize_test_size(_Services, _Buffer, _Filter, _Buffer.get_info().dimensions * int3(2,2,1), _NthImage);
	TESTsurface_resize_test_size(_Services, _Buffer, _Filter, _Buffer.get_info().dimensions / int3(2,2,1), _NthImage+1);
}

void TESTsurface_resize(test_services& _Services)
{
	scoped_allocation b = _Services.load_buffer("Test/Textures/lena_1.png");
	surface::texel_buffer s = surface::decode(b, b.size());

	int NthImage = 0;
	for (const auto& f : enum_iterator<surface::filter>())
	{
		TESTsurface_resize_test_filter(_Services, s, f, NthImage);
		NthImage += 2;
	}
}

	} // namespace tests
} // namespace ouro
