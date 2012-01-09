// $(header)
// Syntactic sugar for a hash
#pragma once
#ifndef oStringID_h
#define oStringID_h

#include <oBasis/oHash.h>

// If defined, an oStringID created from a string will retain the string. This
// is for debugging purposes only as the string might be longer than the debug
// space allocated.
#ifndef oSTRINGID_STORE_STRING
	#define oSTRINGID_STORE_STRING 0
#endif

template<typename id_t, class hash = std::hash<const char*> >
class oStringID
{
	unsigned int id;
	#if oSTRINGID_STORE_STRING == 1
		char s[512];
	#endif
public:
	oStringID()
		: id(0)
	{
		#if oSTRINGID_STORE_STRING == 1
			*s = 0;
		#endif
	}
	
	// required so 0 can be assigned to a handle to initialize it
	oStringID(int _id)
		: id(_id) 
	{
		#if oSTRINGID_STORE_STRING == 1
			*s = 0;
		#endif
	} 

	oStringID(const char* _String) 
		: id(oHash_stlpi(_String))
	{
		#if oSTRINGID_STORE_STRING == 1
			strcpy_s(s, _String ? _String : "");
		#endif
	}
	
	inline operator bool() const { return !!id; }
	inline operator unsigned int() const { return id; }
	inline bool operator==(const oStringID& other) const { return id == other.id; }
	inline bool operator!=(const oStringID& other) const { return !(*this == other); }
};

#endif
