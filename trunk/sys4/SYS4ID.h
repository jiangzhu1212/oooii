// $(header)
// String ID. Basically this is syntactic sugar for a hash.
#pragma once
#ifndef SYS4ID_h
#define SYS4ID_h

#include <oooii/oHash.h>

struct oID_DefaultStringHash { unsigned int operator()(const char* _s) const { return HASHstlpi(_s); } };
template<typename TStringHash = ooID_DefaultStringHash>
class oID
{
	unsigned int id;
public:
	oID() : id(0) {}
	oID(int _ID = 0) : id(_ID) {} // required so 0 can be assigned to a handle to initialize it
	oID(const char* _String): id(TStringHash()(_String)) {}
	inline operator bool() const { return !!id; }
	inline operator unsigned int() const { return id; }
	inline bool operator==(const oID& other) const { return id == other.id; }
	inline bool operator!=(const oID& other) const { return !(*this == other); }
};

#endif
