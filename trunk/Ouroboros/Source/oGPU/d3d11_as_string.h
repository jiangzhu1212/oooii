// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// as_string for d3d11 enums
#pragma once
#ifndef oGPU_d3d11_as_string_h
#define oGPU_d3d11_as_string_h

#include <d3d11.h>

namespace ouro {

const char* as_string(const D3D11_BIND_FLAG& _Flag);
const char* as_string(const D3D11_CPU_ACCESS_FLAG& _Flag);
const char* as_string(const D3D11_RESOURCE_MISC_FLAG& _Flag);
const char* as_string(const D3D11_RESOURCE_DIMENSION& _Type);
const char* as_string(const D3D11_UAV_DIMENSION& _Type);
const char* as_string(const D3D11_USAGE& _Usage);

}

#endif
