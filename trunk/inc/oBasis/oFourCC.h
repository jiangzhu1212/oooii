// $(header)
// Encapsulation of a FourCC code mostly for better self-documenting code
// http://en.wikipedia.org/wiki/FourCC
#ifndef oFourCC_h
#define oFourCC_h

#include <oBasis/oOperators.h>

struct oFourCC : oCompareable<oFourCC>
{
	oFourCC() {}

	oFourCC(unsigned int _FourCC)
		: FourCC(_FourCC)
	{}

	operator int() const { return *(int*)&FourCC; }
	operator unsigned int() const { return FourCC; }

	bool operator==(const oFourCC& _That) const { return FourCC == _That.FourCC; }
	bool operator<(const oFourCC& _That) const { return FourCC < _That.FourCC; }

protected:
	unsigned int FourCC;
};

#endif
