// $(header)
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
