// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Encapsulation of a fourcc code: http://en.wikipedia.org/wiki/code

#pragma once
#include <oMemory/endian.h>
#include <oBase/operators.h>
#include <cstdint>

// Prefer using the class fourcc, but here's a macro for those cases where 
// static composition is required.
#define oFOURCC(a, b, c, d) ((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((c)&0xff)<<8)|((d)&0xff))

// "Readable" fourcc like 'fcc1' are not the right order on little-endian 
// machines, so either use '1ccf' or the oFCC macro
#define oFCC(x) (ouro::is_little_endian ? oFOURCC((x),((x)>>8),((x)>>16),((x)>>24)) : (x))

namespace ouro {

struct fourcc : oComparable<fourcc, uint32_t, int32_t>
{
	inline fourcc() {}
	inline fourcc(int32_t fcc) : code(*(uint32_t*)&fcc) {}
	inline fourcc(uint32_t fcc) : code(fcc) {}
	inline fourcc(const char* fcc_string) { code = to_big_endian(*(uint32_t*)fcc_string); }
	inline fourcc(char a, char b, char c, char d) { code = oFOURCC(a, b, c, d); }

	inline operator int32_t() const { return *(int32_t*)&code; }
	inline operator uint32_t() const { return code; }

	inline bool operator==(const fourcc& that) const { return code == that.code; }
	inline bool operator<(const fourcc& that) const { return code < that.code; }
	
	inline bool operator==(uint32_t that) const { return code == that; }
	inline bool operator<(uint32_t that) const { return code < that; }

	inline bool operator==(int32_t that) const { return code == *(uint32_t*)&that; }
	inline bool operator<(int32_t that) const { return code == *(uint32_t*)&that; }

protected:
	uint32_t code;
};

}
