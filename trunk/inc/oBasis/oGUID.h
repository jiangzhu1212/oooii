// $(header)
// http://en.wikipedia.org/wiki/Universally_Unique_Identifier
// Using <MSVSInstallDir>/Common7/Tools/guidgen.exe is the easiest
// way to generate these.
#pragma once
#ifndef oGUID_h
#define oGUID_h

struct oGUID
{
	unsigned int Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];

	bool operator==(const oGUID& other) const;
	inline bool operator!=(const oGUID& other) const { return !(*this == other); }
	bool operator<(const oGUID& other) const;
};

// Instead of polluting headers with oGUIDs we use oGetGUID to return
// a type's oGUID.  Every interface that is query-able by a oGUID should
// implement this prototype
template<typename T> const oGUID& oGetGUID(volatile const T* volatile const* = 0);

#endif
