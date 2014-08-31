// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Encapsulation of a fourcc code: http://en.wikipedia.org/wiki/FourCC
#ifndef oBase_fourcc_h
#define oBase_fourcc_h

#include <oMemory/endian.h>
#include <oBase/operators.h>

// Prefer using the class fourcc, but here's a macro for those cases where 
// static composition is required.
#define oFOURCC(_A, _B, _C, _D) ((((_A)&0xff)<<24)|(((_B)&0xff)<<16)|(((_C)&0xff)<<8)|((_D)&0xff))

// "Readable" fourcc like 'fcc1' are not the right order on little-endian 
// machines, so either use '1ccf' or the oFCC macro
#define oFCC(x) (ouro::is_little_endian ? oFOURCC((x),((x)>>8),((x)>>16),((x)>>24)) : (x))

namespace ouro {

struct fourcc : oComparable<fourcc, unsigned int, int>
{
	inline fourcc() {}
	inline fourcc(int _FourCC) : FourCC(*(unsigned int*)&_FourCC) {}
	inline fourcc(unsigned int _FourCC) : FourCC(_FourCC) {}
	inline fourcc(const char* _FourCCString) { FourCC = to_big_endian(*(unsigned int*)_FourCCString); }
	inline fourcc(char _A, char _B, char _C, char _D) { FourCC = oFOURCC(_A, _B, _C, _D); }

	inline operator int() const { return *(int*)&FourCC; }
	inline operator unsigned int() const { return FourCC; }

	inline bool operator==(const fourcc& _That) const { return FourCC == _That.FourCC; }
	inline bool operator<(const fourcc& _That) const { return FourCC < _That.FourCC; }
	
	inline bool operator==(unsigned int _That) const { return FourCC == _That; }
	inline bool operator<(unsigned int _That) const { return FourCC < _That; }

	inline bool operator==(int _That) const { return FourCC == *(unsigned int*)&_That; }
	inline bool operator<(int _That) const { return FourCC == *(unsigned int*)&_That; }

protected:
	unsigned int FourCC;
};

} // namespace oStd

#endif
