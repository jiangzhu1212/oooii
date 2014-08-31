// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// murmur3 hash.
#pragma once
#ifndef oBase_murmur3_h
#define oBase_murmur3_h

#include <oBase/assert.h>
#include <oBase/uint128.h>

// Prefer ouro::murmur3 for codebase consistency
void MurmurHash3_x64_128(const void* key, const int len, unsigned int seed, void* out);

namespace ouro {

inline uint128 murmur3(const void* buf, size_t buf_size)
{
	oASSERT(size_t(int(buf_size)) == buf_size, "size is capped at signed int");
	uint128 h;
	MurmurHash3_x64_128(buf, static_cast<int>(buf_size), 0, &h);
	return h;
}

}

#endif
