/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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

// @oooii-tony: Refactor note... for DLL export ease, this should be turned into
// an oP4Device interface.

#include <oStd/fixed_string.h>
#include <oBasis/oVersion.h>
#include <oBasis/oRTTI.h>

enum oP4_STATUS
{
	oP4_OPEN_FOR_EDIT,
	oP4_OPEN_FOR_ADD,
	oP4_OPEN_FOR_DELETE,
	oP4_NEEDS_UPDATE,
	oP4_NEEDS_ADD,
	oP4_NEEDS_DELETE,
	oP4_NEEDS_RESOLVE,

	oP4_STATUS_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oP4_STATUS)

enum oP4_SUBMIT_OPTIONS
{
	oP4_SUBMIT_UNCHANGED,
	oP4_SUBMIT_UNCHANGED_REOPEN,
	oP4_REVERT_UNCHANGED,
	oP4_REVERT_UNCHANGED_REOPEN,
	oP4_LEAVE_UNCHANGED,
	oP4_LEAVE_UNCHANGED_REOPEN,

	oP4_SUBMIT_OPTIONS_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oP4_SUBMIT_OPTIONS)

enum oP4_LINE_END
{
	oP4_LINE_END_LOCAL,
	oP4_LINE_END_UNIX,
	oP4_LINE_END_MAC,
	oP4_LINE_END_WIN,
	oP4_LINE_END_SHARE,

	oP4_LINE_END_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oP4_LINE_END)

struct oP4_FILE_DESC
{
	oP4_FILE_DESC()
		: Status(oP4_OPEN_FOR_EDIT)
		, Revision(0)
		, Changelist(0)
		, IsText(true)
	{}

	oStd::uri_string Path;
	oP4_STATUS Status;
	int Revision;
	int Changelist; // oInvalid is the default changelist
	bool IsText;
};

struct oP4_WORKSPACE
{
	oStd::mstring Client;
	oStd::mstring Owner;
	oStd::mstring Host;
	oStd::xlstring Description;
	oStd::path_string Root;
	oStd::xxlstring View;
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

struct oP4_LABEL_SPEC
{
	oP4_LABEL_SPEC()
		: LastUpdated(0)
		, LastAccessed(0)
		, Revision(0)
		, Locked(false)
	{}

	oStd::mstring Label;
	time_t LastUpdated;
	time_t LastAccessed;
	oStd::mstring Owner;
	oStd::xlstring Description;
	oStd::xxlstring View;
	unsigned int Revision;
	bool Locked;
};

// _____________________________________________________________________________

oAPI bool oP4IsAvailable();

// It is often useful to imprint the actual source-control in the version 
// number, but our version number is ushorts (as Window's is), so it's not 
// straightforward.
inline unsigned int oP4GetRevision(const oVersion& _Version) { return _Version.Build * 10000 + _Version.Revision; }
inline void oP4SetRevision(unsigned int _Revision, oVersion* _pVersion) { _pVersion->Build = static_cast<unsigned short>(_Revision / 10000); _pVersion->Revision = _Revision % 10000; }

// _____________________________________________________________________________
// Direct communication API with P4... mostly these wrap a spawn-command as if 
// typing the P4 command line at a prompt. The output is captured and returned.

oAPI bool oP4GetWorkspaceString(char* _StrDestination, size_t _SizeofStrDestination);
template<size_t size> inline bool oP4GetWorkspaceString(char (&_StrDestination)[size]) { return oP4GetWorkspaceString(_StrDestination, size); }
template<size_t capacity> inline bool oP4GetWorkspaceString(oStd::fixed_string<char, capacity>& _StrDestination) { return oP4GetWorkspaceString(_StrDestination, _StrDestination.capacity()); }

oAPI bool oP4GetLabelSpecString(char* _P4LabelSpecString, size_t _SizeofP4LabelSpecString, const char* _Label);
template<size_t size> inline bool oP4GetLabelSpecString(char (&_StrDestination)[size], const char* _Label) { return oP4GetLabelSpecString(_StrDestination, size, _Label); }
template<size_t capacity> inline bool oP4GetLabelSpecString(oStd::fixed_string<char, capacity>& _StrDestination, const char* _Label) { return oP4GetLabelSpecString(_StrDestination, _StrDestination.capacity(), _Label); }

// OpenType can only be one of the first 3 status values or this will return 
// false.
oAPI bool oP4Open(oP4_STATUS _OpenType, const char* _Path);
oAPI bool oP4Revert(const char* _Path);

// Syncs the tree or specified path if the entire tree is not needed.
// If _Force is true this will run p4 sync with -f
oAPI bool oP4Sync(int _ChangeList, const char* _Path = nullptr, bool _Force = false);

// Sets a label based on the specification.
oAPI bool oP4Label(const oP4_LABEL_SPEC& _Label);

// Returns the user supplied description associated with a given changelist
oAPI bool oP4GetChangelistDescription(char* _StrDestination, size_t _SizeofStrDestination, int _ChangeList);
template<size_t size> bool oP4GetChangelistDescription(char (&_StrDestination)[size], int _ChangeList) { return oP4GetChangelistDescription(_StrDestination, size, _ChangeList); }
template<size_t capacity> bool oP4GetChangelistDescription(oStd::fixed_string<char, capacity>& _StrDestination, int _ChangeList){ return oP4GetChangelistDescription(_StrDestination, _StrDestination.capacity(), _ChangeList); }

// Returns the username associated with a specific changelist
oAPI bool oP4GetChangelistUser(char* _StrDestination, size_t _SizeofStrDestination, int _ChangeList);
template<size_t size> bool oP4GetChangelistUser(char (&_StrDestination)[size], int _ChangeList) { return oP4GetChangelistUser(_StrDestination, size, _ChangeList); }
template<size_t capacity> bool oP4GetChangelistUser(oStd::fixed_string<char, capacity>& _StrDestination, int _ChangeList){ return oP4GetChangelistUser(_StrDestination, _StrDestination.capacity(), _ChangeList); }

// Returns the date associated with a specific changelist
oAPI bool oP4GetChangelistDate(char* _StrDestination, size_t _SizeofStrDestination, int _ChangeList);
template<size_t size> bool oP4GetChangelistDate(char (&_StrDestination)[size], int _ChangeList) { return oP4GetChangelistDate(_StrDestination, size, _ChangeList); }
template<size_t capacity> bool oP4GetChangelistDate(oStd::fixed_string<char, capacity>& _StrDestination, int _ChangeList){ return oP4GetChangelistDate(_StrDestination, _StrDestination.capacity(), _ChangeList); }

// Takes a p4 depot path and converts it into a client file path
oAPI bool oP4GetClientPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _pDepotPath);
template<size_t size> bool oP4GetClientPath(char (&_StrDestination)[size], const char* _pDepotPath) { return oP4GetClientPath(_StrDestination, size, _pDepotPath); }
template<size_t capacity> bool oP4GetClientPath(oStd::fixed_string<char, capacity>& _StrDestination, const char* _pDepotPath) { return oP4GetClientPath(_StrDestination, _StrDestination.capacity(), _pDepotPath); }

// Fill the specified array with file descs for all files that are opened in a 
// pending changelist (oInvalid is the default changelist) under the specified 
// P4Base.
// Returns the number of validly populated _pOpenedFiles or oInvalid in case of 
// error.
size_t oP4ListOpened(oP4_FILE_DESC* _pOpenedFiles, size_t _MaxNumOpenedFiles, const char* _P4Base = nullptr);
template<size_t size> size_t oP4ListOpened(oP4_FILE_DESC (&_pOpenedFiles)[size], const char* _P4Base = nullptr) { return oP4ListOpened(_pOpenedFiles, size, _P4Base); }

// Fill the specified array with file descs for all files that are out-of-date.
// NOTE: For these the Revision and Changelist are the TARGET values to which 
// a Get Latest will update them to. To limit the check to a specific 
// changelist, specify that changelist as the _UpToChangelist value.
// Returns the number of validly populated _pOutOfDateFiles or oInvalid in case 
// of error.
size_t oP4ListOutOfDate(oP4_FILE_DESC* _pOutOfDateFiles, size_t _MaxNumOutOfDateFiles, const char* _P4Base = nullptr, int _UpToChangelist = oInvalid);
template<size_t size> size_t oP4ListOutOfDate(oP4_FILE_DESC (&_pOutOfDateFiles)[size], const char* _P4Base = nullptr, int _UpToChangelist = oInvalid) { return oP4ListOutOfDate(_pOutOfDateFiles, size, _P4Base, _UpToChangelist); }

// _____________________________________________________________________________
// P4 String Parsing: convert the raw string as returned by one of the above
// calls and turn it into a more manageable struct.

oAPI time_t oP4ParseTime(const char* _P4TimeString);
oAPI bool oP4ParseWorkspace(oP4_WORKSPACE* _pWorkspace, const char* _P4WorkspaceString);
oAPI bool oP4ParseLabelSpec(oP4_LABEL_SPEC* _pLabelSpec, const char* _P4LabelSpecString);

// Returns the changelist from a line from the resulting from p4 changes
oAPI int oP4ParseChangesLine(const char* _ChangesLine);

inline bool oP4GetLabelSpec(const char* _Label, oP4_LABEL_SPEC* _pLabelSpec)
{
	oStd::xxlstring lstr;
	if (!oP4GetLabelSpecString(lstr, _Label))
		return false;
	return oP4ParseLabelSpec(_pLabelSpec, lstr);
}

// _____________________________________________________________________________
// Higher-level/convenience API

inline bool oP4GetWorkspace(oP4_WORKSPACE* _pWorkspace)
{
	oStd::xxlstring tmp;
	if (!oP4GetWorkspaceString(tmp))
		return false; // pass through error
	return oP4ParseWorkspace(_pWorkspace, tmp);
}

// This function estimates what revision the current code being built. If there 
// are any out-of-date or open-for-edit files under the specified base then this 
// will set std::errc::protocol_error as the last error, indicating that the returned 
// changelist is only an estimate. If there are no open files and everything is
// up-to-date, then 0 is explicitly set.
// NOTE: This uses oTRACEA for the first several files that cause the changelist
// to be corrupt. When using tools that call this, dbgview.exe can be run to see
// why a changelist is labeled as corrupt.
oAPI int oP4GetCurrentChangelist(const char* _P4Base = nullptr);

// Gets the next change based on a specified _CurrentCL (which can be estimated
// from oP4GetCurrentChangelist)
oAPI int oP4GetNextChangelist(int _CurrentChangelist, const char* _P4Base = nullptr);

#endif
