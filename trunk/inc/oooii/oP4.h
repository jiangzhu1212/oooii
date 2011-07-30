// $(header)
#pragma once
#ifndef oP4_h
#define oP4_h

#include <stdlib.h> // _MAX_PATH

namespace oP4
{
	enum SUBMIT_OPTIONS
	{
		SUBMIT_UNCHANGED,
		SUBMIT_UNCHANGED_REOPEN,
		REVERT_UNCHANGED,
		REVERT_UNCHANGED_REOPEN,
		LEAVE_UNCHANGED,
		LEAVE_UNCHANGED_REOPEN,
	};

	enum LINE_END
	{
		LOCAL,
		UNIX,
		MAC,
		WIN,
		SHARE,
	};

	struct CLIENT_SPEC
	{
		char Client[256];
		char Owner[256];
		char Host[256];
		char Description[2048];
		char Root[_MAX_PATH];
		char View[4096];
		time_t LastUpdated;
		time_t LastAccessed;
		SUBMIT_OPTIONS SubmitOptions;
		LINE_END LineEnd;
		bool Allwrite:1;
		bool Clobber:1;
		bool Compress:1;
		bool Locked:1;
		bool Modtime:1;
		bool Rmdir:1;
	};

	oP4::SUBMIT_OPTIONS GetSubmitOptions(const char* _P4String);
	oP4::LINE_END GetLineEnd(const char* _P4String);
	time_t GetTime(const char* _P4TimeString);
	bool GetClientSpec(CLIENT_SPEC* _pClientSpec, const char* _P4ClientSpecString);

	bool GetClientSpecString(char* _P4ClientSpecString, size_t _SizeofP4ClientSpecString);
	template<size_t size> inline bool GetClientSpecString(char (&_P4ClientSpecString)[size]) { return GetClientSpecString(_P4ClientSpecString, size); }

	inline bool GetClientSpec(CLIENT_SPEC* _pClientSpec)
	{
		char tmp[3*1024];
		if (!GetClientSpecString(tmp))
			return false;
		return GetClientSpec(_pClientSpec, tmp);
	}

	bool MarkForEdit(const char* _Path);
	bool MarkForAdd(const char* _Path);
	bool MarkForDelete(const char* _Path);
	bool Revert(const char* _Path);
};

#endif
