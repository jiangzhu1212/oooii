// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/gpu_api.h>

namespace ouro {

const char* as_string(const gpu_api& value)
{
	switch (value)
	{
		case gpu_api::unknown: return "unknown";
		case gpu_api::d3d11: return "d3d11";
		case gpu_api::ogl: return "ogl";
		case gpu_api::ogles: return "ogles";
		case gpu_api::webgl: return "webgl";
		case gpu_api::custom: return "custom";
		default: break;
	}
	return "?";
}

}
