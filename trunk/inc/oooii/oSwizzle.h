// $(header)
#pragma once
#ifndef oSwizzle_h
#define oSwizzle_h

struct oByteSwizzle16
{
	union
	{
		short AsShort;
		unsigned short AsUnsignedShort;
		char AsChar[2];
		unsigned char AsUnsignedChar[2];
	};
};

struct oByteSwizzle32
{
	union
	{
		float AsFloat;
		int AsInt;
		unsigned int AsUnsignedInt;
		short AsShort[2];
		unsigned short AsUnsignedShort[2];
		char AsChar[4];
		unsigned char AsUnsignedChar[4];
	};
};

struct oByteSwizzle64
{
	union
	{
		double AsDouble;
		long long AsLongLong;
		unsigned long long AsUnsignedLongLong;
		float AsFloat[2];
		int AsInt[2];
		unsigned int AsUnsignedInt[2];
		short AsShort[4];
		unsigned short AsUnsignedShort[4];
		char AsChar[8];
		unsigned char AsUnsignedChar[8];
	};
};

#endif
