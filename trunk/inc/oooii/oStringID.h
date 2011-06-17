// $(header)

// Syntactic sugar for a hash
#pragma once
#ifndef oStringID_h
#define oStringID_h

#include <oooii/oHash.h>

class oStringID
{
	unsigned int id;
public:
	oStringID() : id(0) {}
	oStringID(int _id) : id(_id) {} // required so 0 can be assigned to a handle to initialize it
	oStringID(const char* _String) : id(oHash_stlpi(_String)) {}
	inline operator bool() const { return !!id; }
	inline operator unsigned int() const { return id; }
	inline bool operator==(const oStringID& other) const { return id == other.id; }
	inline bool operator!=(const oStringID& other) const { return !(*this == other); }
};

#endif
