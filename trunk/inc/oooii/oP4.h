/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
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
