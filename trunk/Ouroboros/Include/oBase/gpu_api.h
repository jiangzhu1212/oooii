// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Enumeration of common gpu apis

#pragma once

namespace ouro {

enum class gpu_api 
{
	unknown,
	d3d11,
	d3d12,
	mantle,
	ogl,
	ogles,
	webgl,
	custom,

	count,
};

}
