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
// An EightCC is much like a FourCC (http://en.wikipedia.org/wiki/FourCC) but 
// twice as long to support more robustly labeled types.
#ifndef oEightCC_h
#define oEightCC_h

#include <oStd/operators.h>
#include <oBasis/oPlatformFeatures.h>
#include <oStd/endian.h>

struct oEightCC : oComparable<oEightCC, unsigned long long, long long>
{
	inline oEightCC() {}
	inline oEightCC(int _EightCC) : EightCC((unsigned long long)_EightCC) {}
	inline oEightCC(unsigned int _EightCC) : EightCC(_EightCC) {}
	inline oEightCC(long long _EightCC) : EightCC(*(unsigned long long*)&_EightCC) {}
	inline oEightCC(unsigned long long _EightCC) : EightCC(_EightCC) {}
	inline oEightCC(const char* _EightCCString) : EightCC(oStd::to_big_endian(*(unsigned long long*)_EightCCString)) {}

	inline operator long long() const { return *(long long*)&EightCC; }
	inline operator unsigned long long() const { return EightCC; }

	inline bool operator==(const oEightCC& _That) const { return EightCC == _That.EightCC; }
	inline bool operator<(const oEightCC& _That) const { return EightCC < _That.EightCC; }
	inline bool operator==(unsigned long long _That) const { return EightCC == _That; }
	inline bool operator<(unsigned long long _That) const { return EightCC < _That; }
	inline bool operator==(long long _That) const { return *(long long*)&EightCC == _That; }
	inline bool operator<(long long _That) const { return *(long long*)&EightCC < _That; }

protected:
	unsigned long long EightCC;
};

// oLITTLEENDIAN has been deprecated in favor of a more consistent static const 
// bool, however that doesn't help in this case... so define in here until the 
// dependent code can be refactored to a different usage pattern.

#define oLITTLEENDIAN 1

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
