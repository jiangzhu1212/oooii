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

#include <oBasis/oFixedString.h>

enum oP4_OPEN_TYPE
{
	oP4_OPEN_FOR_EDIT,
	oP4_OPEN_FOR_ADD,
	oP4_OPEN_FOR_DELETE,
};

enum oP4_SUBMIT_OPTIONS
{
	oP4_SUBMIT_UNCHANGED,
	oP4_SUBMIT_UNCHANGED_REOPEN,
	oP4_REVERT_UNCHANGED,
	oP4_REVERT_UNCHANGED_REOPEN,
	oP4_LEAVE_UNCHANGED,
	oP4_LEAVE_UNCHANGED_REOPEN,
};

enum oP4_LINE_END
{
	oP4_LINE_END_LOCAL,
	oP4_LINE_END_UNIX,
	oP4_LINE_END_MAC,
	oP4_LINE_END_WIN,
	oP4_LINE_END_SHARE,
};

struct oP4_FILE_DESC
{
	oP4_FILE_DESC()
		: OpenType(oP4_OPEN_FOR_EDIT)
		, Revision(0)
		, Changelist(0)
		, IsText(true)
	{}

	oStringURI Path;
	oP4_OPEN_TYPE OpenType;
	int Revision;
	int Changelist; // oInvalid is the default changelist
	bool IsText;
};

struct oP4_CLIENT_SPEC
{
	oStringM Client;
	oStringM Owner;
	oStringM Host;
	oStringXL Description;
	oStringPath Root;
	oStringXXL View;
	time_t LastUpdated;
	time_t LastAccessed;
	oP4_SUBMIT_OPTIONS SubmitOptions;
	oP4_LINE_END LineEnd;
	bool Allwrite;
	bool Clobber;
	bool Compress;
	bool Locked;
	bool Modtime;
	bool Rmdir;
};

// _____________________________________________________________________________
// Direct communication API with P4... mostly these wrap a spawn-command as if 
// typing the P4 command line at a prompt. The output is captured and returned.

oAPI bool oP4GetClientSpecString(char* _StrDestination, size_t _SizeofStrDestination);
template<size_t size> inline bool oP4GetClientSpecString(char (&_StrDestination)[size]) { return oP4GetClientSpecString(_StrDestination, size); }
template<size_t CAPACITY> inline bool oP4GetClientSpecString(oFixedString<char, CAPACITY>& _StrDestination) { return oP4GetClientSpecString(_StrDestination, _StrDestination.capacity()); }

oAPI bool oP4Open(oP4_OPEN_TYPE _Type, const char* _Path);
oAPI bool oP4Revert(const char* _Path);

// Returns the number of validly poulated _pOpenedFiles, or oInvalid in case of 
// error.
size_t oP4ListOpened(oP4_FILE_DESC* _pOpenedFiles, size_t _MaxNumOpenedFiles);
template<size_t size> size_t oP4ListOpened(oP4_FILE_DESC (&_pOpenedFiles)[size]) { return oP4ListOpened(_pOpenedFiles, size); }

// _____________________________________________________________________________
// P4 String Parsing: convert the raw string as returned by one of the above
// calls and turn it into a more manageable struct.

oAPI time_t oP4ParseTime(const char* _P4TimeString);
oAPI bool oP4ParseClientSpec(oP4_CLIENT_SPEC* _pClientSpec, const char* _P4ClientSpecString);

// _____________________________________________________________________________
// Higher-level/convenience API

inline bool oP4GetClientSpec(oP4_CLIENT_SPEC* _pClientSpec)
{
	oStringXXL tmp;
	if (!oP4GetClientSpecString(tmp))
		return false; // pass through error
	return oP4ParseClientSpec(_pClientSpec, tmp);
}

#endif
