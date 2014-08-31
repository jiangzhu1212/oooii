// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// http://en.wikipedia.org/wiki/Universally_Unique_Identifier
// Using <MSVSInstallDir>/Common7/Tools/guidgen.exe is the easiest
// way to generate these.
#pragma once
#ifndef oBase_guid_h
#define oBase_guid_h

#include <oBase/operators.h>

namespace ouro {

struct guid
{
private:
	typedef const unsigned long long* ptr;

public:
	unsigned int Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];

	bool operator<(const guid& _That) const { return ptr(this)[0] < ptr(&_That)[0] || (ptr(this)[0] == ptr(&_That)[0] && ptr(this)[1] < ptr(&_That)[1]); }
	inline bool operator==(const guid& _That) const { return ptr(this)[0] == ptr(&_That)[0] && ptr(this)[1] == ptr(&_That)[1]; }
	oOPERATORS_COMPARABLE(guid);
};

static const guid null_guid = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

}

#endif
