// $(header)

// To protect against bugs between 32-bit and 64-bit builds, create a type
// called oSizeXX that is fixed in bit-size for all builds. Since different
// APIs treat size differently (STL: size_t, DirectX, unsigned int), all the
// operators for oSizeXX are overloaded to ensure there is no overflow.
// Prefer constructing an oSize type rather than C-casting or static_cast.
#pragma once
#ifndef oSize_h
#define oSize_h

#include <oBasis/oCheckedAssignment.h>

namespace detail {

template<typename T> class oSizeT
{
	T Size;

	oDEFINE_RANGE_CHECKS(oSizeT, T, Size, oNumericLimits<T>::GetMax());
	oDEFINE_ALWAYS_VALID_CHECK(T);

public:
	typedef T value_type;
	oSizeT() : Size(0) {}
	template<typename U> oSizeT(const U& _Size) { CheckMax(_Size, "ctor"); Size = static_cast<T>(_Size); }
	oDEFINE_CHECKED_OPERATORS(oSizeT, T, Size);
};

} // namespace detail

typedef detail::oSizeT<unsigned short> oSize16;
typedef detail::oSizeT<unsigned int> oSize32;
typedef detail::oSizeT<unsigned long long> oSize64;

#endif
