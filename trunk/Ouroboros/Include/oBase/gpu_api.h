// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Enumeration of common gpu apis
#pragma once
#ifndef oBase_gpu_api_h
#define oBase_gpu_api_h

namespace ouro {

/* enum class */ namespace gpu_api
{ enum value {

	unknown,
	d3d11,
	d3d12,
	mantle,
	ogl,
	ogles,
	webgl,
	custom,

	count,

};}

}

#endif
