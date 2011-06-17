// $(header)

// An EightCC is much like a FourCC (http://en.wikipedia.org/wiki/FourCC) but 
// twice as long to avoid collisions.
#ifndef oEightCC_h
#define oEightCC_h

struct oEightCC
{
	oEightCC()
	{}
	oEightCC(long _FourCCA, long _FourCCB)
	{
		FourCC[0] = _FourCCA;
		FourCC[1] = _FourCCB;
	}

	operator long long() const
	{
		return EightCC;
	}

	union
	{
		long FourCC[2];
		long long EightCC;
	};

	bool operator!=(const oEightCC& _other) const { return _other.EightCC != EightCC; }
	bool operator==(const oEightCC& _other) const { return _other.EightCC == EightCC; }
};

#endif
