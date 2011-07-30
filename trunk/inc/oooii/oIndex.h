// $(header)

// Often it is useful to have a counting integer value that can also be in an 
// invalid state. To ensure consistency and provide better self-documentation
// use oIndex because it has explicit validity test rather than > < with signed
// numbers or assumed reserved values.
#pragma once
#ifndef oIndex_h
#define oIndex_h

#include <oooii/oAssert.h>
#include <oooii/oLimits.h>
#include <oooii/oCheckedAssignment.h>

// @oooii-eric: Still some usability problems remaining. Tried to replace StartingFrame in OVideoCodec::Desc with this, and still getting some compile errors.
namespace detail {

template<typename T> class oIndexT
{
	T Index;

	oDEFINE_RANGE_CHECKS(oIndexT, T, Index, oNumericLimits<T>::GetMax() - 1);
	oDEFINE_ALWAYS_VALID_CHECK(T);

public:
	typedef T value_type;
	// @oooii-eric: Doesn't compile, would need the new c++0x feature constexpr to make it compile. That should be available in Visual Studio 2011.
	//static const T max_size = oNumericLimits<T>::GetMax() - 1;
	//static const T invalid = oNumericLimits<T>::GetMax();

	static T MaxSize() {return oNumericLimits<T>::GetMax() - 1;}
	static T Invalid() {return oNumericLimits<T>::GetMax();}

	oIndexT() : Index(Invalid()) {}
	oIndexT(unsigned int _Size) { CheckMax(_Size, "ctor"); Index = static_cast<T>(_Size); }
	oIndexT(unsigned long long _Size) { CheckMax(_Size, "ctor"); Index = static_cast<T>(_Size); }
	bool IsValid() const { return Index != invalid; }

	oDEFINE_CHECKED_OPERATORS(oIndexT, T, Index);
};

} // namespace detail

typedef detail::oIndexT<unsigned short> oIndex16;
typedef detail::oIndexT<unsigned int> oIndex32;
typedef detail::oIndexT<unsigned long long> oIndex64;

#endif
