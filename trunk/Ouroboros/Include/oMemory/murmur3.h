// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// murmur3 hash.

#pragma once
#include <oMemory/uint128.h>

namespace ouro {

uint128_t murmur3(const void* buf, size_t buf_size);

}
