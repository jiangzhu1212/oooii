/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
// An EightCC is much like a FourCC (http://en.wikipedia.org/wiki/FourCC) but 
// twice as long to support more robustly labeled types.
#ifndef oEightCC_h
#define oEightCC_h

#include <oBasis/oOperators.h>
#include <oBasis/oByteSwizzle.h>
#include <oBasis/oPlatformFeatures.h>

struct oEightCC : oCompareable<oEightCC>
{
	oEightCC() {}
	oEightCC(unsigned int _FourCCA, unsigned int _FourCCB)
	{
		#ifdef oLITTLEENDIAN
			EightCC.AsUnsignedInt[0] = _FourCCB;
			EightCC.AsUnsignedInt[1] = _FourCCA;
		#else
			EightCC.AsUnsignedInt[0] = _FourCCA;
			EightCC.AsUnsignedInt[1] = _FourCCB;
		#endif
	}

	oEightCC(unsigned long long _EightCC)
	{
		EightCC.AsUnsignedLongLong = _EightCC;
	}

	operator long long() const { return EightCC.AsLongLong; }
	operator unsigned long long() const { return EightCC.AsUnsignedLongLong; }

	bool operator==(const oEightCC& _That) const { return EightCC.AsUnsignedLongLong == _That.EightCC.AsUnsignedLongLong; }
	bool operator<(const oEightCC& _That) const { return EightCC.AsUnsignedLongLong < _That.EightCC.AsUnsignedLongLong; }
	bool operator==(unsigned long long _That) const { return EightCC.AsUnsignedLongLong == _That; }
	bool operator<(unsigned long long _That) const { return EightCC.AsUnsignedLongLong < _That; }

protected:
	oByteSwizzle64 EightCC;
};

// If only constexpr were supported...
template<long _FourCCA, long _FourCCB>
struct oConstEightCC
{
	#ifdef oLITTLEENDIAN
		static const unsigned long long Value = ((unsigned long long)_FourCCB << 32) | _FourCCA;
	#else
		static const unsigned long long Value = ((unsigned long long)_FourCCA << 32) | _FourCCB;
	#endif

	operator unsigned long long() const { return Value; }
	operator oEightCC() const { return Value; }
	bool operator==(const oEightCC& _That) const { return Value == _That; }
	bool operator!=(const oEightCC& _That) const { return !(Value == _That); }
};

#endif
