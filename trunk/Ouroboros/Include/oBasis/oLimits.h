/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#pragma once
#ifndef oLimits_h
#define oLimits_h
// @oooii-tony: Using <limits> can conflict with macro definitions of min and
// max, so unfortunately, don't use it and use this lame renamed under-featured
// version.

// @oooii-tony: Another lame thing. std::numeric_limit<float>::min() is a 
// positive value while std::numeric_limit<int>::min() is a negative value, so
// you can't naively make a template with Min and Max members and set it with
// std::numeric_limit because it won't behave well with signed numbers in both
// float and int cases.

#include <cfloat>
#include <climits>

template<typename T> class oNumericLimits
{
public:
	static T GetMin() { return T(0); }
	static T GetMax() { return T(0); }
	static T GetEpsilon() { return T(0); }
	static T GetRoundError() { return T(0); }

	static T GetSignedMin() { return T(0); }
	static T GetSmallestNonZeroMagnitude() { return T(0); }

	static unsigned int GetNumMantissaBits() { return 0; }
	static unsigned int GetNumPrecisionDigits() { return 0; }
	static bool IsSigned() { return false; }
};

template<> class oNumericLimits<float>
{
public:
	typedef float T;
	static T GetMin() { return FLT_MIN; };
	static T GetMax() { return FLT_MAX; };
	static T GetEpsilon() { return FLT_EPSILON; }
	static T GetRoundError() { return 0.5f; }

	static T GetSignedMin() { return -GetMax(); }
	static T GetSmallestNonZeroMagnitude() { return GetMin(); }

	static unsigned int GetNumMantissaBits() { return FLT_MANT_DIG; }
	static unsigned int GetNumPrecisionDigits() { return FLT_DIG; }
	static bool IsSigned() { return true; }
	static float quiet_NaN() { int NaN = 0x7fc00000; return *(float*)&NaN; }
};

template<> class oNumericLimits<double>
{
public:
	typedef double T;
	static T GetMin() { return DBL_MIN; };
	static T GetMax() { return DBL_MAX; };
	static T GetEpsilon() { return DBL_EPSILON; }
	static T GetRoundError() { return 0.5; }

	static T GetSignedMin() { return -GetMax(); }
	static T GetSmallestNonZeroMagnitude() { return GetMin(); }

	static unsigned int GetNumMantissaBits() { return DBL_MANT_DIG; }
	static unsigned int GetNumPrecisionDigits() { return DBL_DIG; }
	static bool IsSigned() { return true; }
};

template<> class oNumericLimits<long double>
{
public:
	typedef double T;
	static T GetMin() { return LDBL_MIN; };
	static T GetMax() { return LDBL_MAX; };
	static T GetEpsilon() { return LDBL_EPSILON; }
	static T GetRoundError() { return 0.5; }

	static T GetSignedMin() { return -GetMax(); }
	static T GetSmallestNonZeroMagnitude() { return GetMin(); }

	static unsigned int GetNumMantissaBits() { return LDBL_MANT_DIG; }
	static unsigned int GetNumPrecisionDigits() { return LDBL_DIG; }
	static bool IsSigned() { return true; }
};

template<> class oNumericLimits<char>
{
public:
	typedef char T;
	static T GetMin() { return SCHAR_MIN; };
	static T GetMax() { return SCHAR_MAX; };
	static T GetEpsilon() { return 0; }
	static T GetRoundError() { return 0; }

	static T GetSignedMin() { return GetMin(); }
	static T GetSmallestNonZeroMagnitude() { return 1; }

	static unsigned int GetNumMantissaBits() { return 0; }
	static unsigned int GetNumPrecisionDigits() { return 0; }
	static bool IsSigned() { return true; }
};

template<> class oNumericLimits<unsigned char>
{
public:
	typedef unsigned char T;
	static T GetMin() { return 0; };
	static T GetMax() { return UCHAR_MAX; };
	static T GetEpsilon() { return 0; }
	static T GetRoundError() { return 0; }

	static T GetSignedMin() { return 0; }
	static T GetSmallestNonZeroMagnitude() { return 1; }

	static unsigned int GetNumMantissaBits() { return 0; }
	static unsigned int GetNumPrecisionDigits() { return 0; }
	static bool IsSigned() { return false; }
};

template<> class oNumericLimits<short>
{
public:
	typedef short T;
	static T GetMin() { return SHRT_MIN; };
	static T GetMax() { return SHRT_MAX; };
	static T GetEpsilon() { return 0; }
	static T GetRoundError() { return 0; }

	static T GetSignedMin() { return GetMin(); }
	static T GetSmallestNonZeroMagnitude() { return 1; }

	static unsigned int GetNumMantissaBits() { return 0; }
	static unsigned int GetNumPrecisionDigits() { return 0; }
	static bool IsSigned() { return true; }
};

template<> class oNumericLimits<unsigned short>
{
public:
	typedef unsigned short T;
	static T GetMin() { return 0; };
	static T GetMax() { return USHRT_MAX; };
	static T GetEpsilon() { return 0; }
	static T GetRoundError() { return 0; }

	static T GetSignedMin() { return 0; }
	static T GetSmallestNonZeroMagnitude() { return 1; }

	static unsigned int GetNumMantissaBits() { return 0; }
	static unsigned int GetNumPrecisionDigits() { return 0; }
	static bool IsSigned() { return false; }
};

template<> class oNumericLimits<int>
{
public:
	typedef int T;
	static T GetMin() { return INT_MIN; };
	static T GetMax() { return INT_MAX; };
	static T GetEpsilon() { return 0; }
	static T GetRoundError() { return 0; }

	static T GetSignedMin() { return GetMin(); }
	static T GetSmallestNonZeroMagnitude() { return 1; }

	static unsigned int GetNumMantissaBits() { return 0; }
	static unsigned int GetNumPrecisionDigits() { return 0; }
	static bool IsSigned() { return true; }
};

template<> class oNumericLimits<unsigned int>
{
public:
	typedef unsigned int T;
	static T GetMin() { return 0; };
	static T GetMax() { return UINT_MAX; };
	static T GetEpsilon() { return 0; }
	static T GetRoundError() { return 0; }

	static T GetSignedMin() { return 0; }
	static T GetSmallestNonZeroMagnitude() { return 1; }

	static unsigned int GetNumMantissaBits() { return 0; }
	static unsigned int GetNumPrecisionDigits() { return 0; }
	static bool IsSigned() { return false; }
};

template<> class oNumericLimits<long>
{
public:
	typedef long T;
	static T GetMin() { return LONG_MIN; };
	static T GetMax() { return LONG_MAX; };
	static T GetEpsilon() { return 0; }
	static T GetRoundError() { return 0; }

	static T GetSignedMin() { return GetMin(); }
	static T GetSmallestNonZeroMagnitude() { return 1; }

	static unsigned int GetNumMantissaBits() { return 0; }
	static unsigned int GetNumPrecisionDigits() { return 0; }
	static bool IsSigned() { return true; }
};

template<> class oNumericLimits<unsigned long>
{
public:
	typedef unsigned long T;
	static T GetMin() { return 0; };
	static T GetMax() { return ULONG_MAX; };
	static T GetEpsilon() { return 0; }
	static T GetRoundError() { return 0; }

	static T GetSignedMin() { return 0; }
	static T GetSmallestNonZeroMagnitude() { return 1; }

	static unsigned int GetNumMantissaBits() { return 0; }
	static unsigned int GetNumPrecisionDigits() { return 0; }
	static bool IsSigned() { return false; }
};

template<> class oNumericLimits<long long>
{
public:
	typedef long long T;
	static T GetMin() { return LLONG_MIN; };
	static T GetMax() { return LLONG_MAX; };
	static T GetEpsilon() { return 0; }
	static T GetRoundError() { return 0; }

	static T GetSignedMin() { return GetMin(); }
	static T GetSmallestNonZeroMagnitude() { return 1; }

	static unsigned int GetNumMantissaBits() { return 0; }
	static unsigned int GetNumPrecisionDigits() { return 0; }
	static bool IsSigned() { return true; }
};

template<> class oNumericLimits<unsigned long long>
{
public:
	typedef unsigned long long T;
	static T GetMin() { return 0; };
	static T GetMax() { return ULLONG_MAX; };
	static T GetEpsilon() { return 0; }
	static T GetRoundError() { return 0; }

	static T GetSignedMin() { return 0; }
	static T GetSmallestNonZeroMagnitude() { return 1; }

	static unsigned int GetNumMantissaBits() { return 0; }
	static unsigned int GetNumPrecisionDigits() { return 0; }
	static bool IsSigned() { return false; }
};

#endif
