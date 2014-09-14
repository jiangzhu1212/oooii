// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
	, const surface::image& _Buffer
	, surface::filter _Filter
	, const int3& _NewSize
	, int _NthImage)
{
	surface::info srcInfo = _Buffer.get_info();
	surface::info destInfo = srcInfo;
	destInfo.dimensions = _NewSize;
	surface::image dst(destInfo);
	{
		surface::shared_lock lock(_Buffer);
		surface::lock_guard lock2(dst);

		scoped_timer timer("resize time");
		surface::resize(srcInfo, lock.mapped, destInfo, lock2.mapped, _Filter);
	}

	_Services.check(dst, _NthImage);
}

static void TESTsurface_resize_test_filter(test_services& _Services
	, const surface::image& _Buffer
	, surface::filter _Filter
	, int _NthImage)
{
	TESTsurface_resize_test_size(_Services, _Buffer, _Filter, _Buffer.get_info().dimensions * int3(2,2,1), _NthImage);
	TESTsurface_resize_test_size(_Services, _Buffer, _Filter, _Buffer.get_info().dimensions / int3(2,2,1), _NthImage+1);
}

void TESTsurface_resize(test_services& _Services)
{
	scoped_allocation b = _Services.load_buffer("Test/Textures/lena_1.png");
	surface::image s = surface::decode(b, b.size());

	int NthImage = 0;
	for (const auto& f : enum_iterator<surface::filter>())
	{
		TESTsurface_resize_test_filter(_Services, s, f, NthImage);
		NthImage += 2;
	}
}

	}
}
