// $(header)
// 
#pragma once
#ifndef oUint128_h
#define oUint128_h

struct uint128
{
	unsigned long long Data[2];
	bool operator==(const uint128& _That) const { return Data[0] == _That.Data[0] && Data[1] == _That.Data[1]; }
	bool operator!=(const uint128& _That) const { return !(*this == _That); }
};

#endif
