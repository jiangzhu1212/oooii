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
#include <oPlatform/oP4.h>
#include <oBasis/oString.h>
#include <oPlatform/oReporting.h>
#include <oBasis/oError.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oProcess.h>
#include <time.h>

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oP4_STATUS)
	oRTTI_ENUM_BEGIN_VALUES(oP4_STATUS)
		oRTTI_VALUE_CUSTOM(oP4_OPEN_FOR_EDIT, "edit")
		oRTTI_VALUE_CUSTOM(oP4_OPEN_FOR_ADD, "add")
		oRTTI_VALUE_CUSTOM(oP4_OPEN_FOR_DELETE, "delete")
		oRTTI_VALUE_CUSTOM(oP4_NEEDS_UPDATE, "updating")
		oRTTI_VALUE_CUSTOM(oP4_NEEDS_ADD, "added")
		oRTTI_VALUE_CUSTOM(oP4_NEEDS_DELETE, "deleted")
		oRTTI_VALUE_CUSTOM(oP4_NEEDS_RESOLVE, "needs resolve")
	oRTTI_ENUM_END_VALUES(oP4_STATUS)
	oRTTI_ENUM_VALIDATE_COUNT(oP4_STATUS, oP4_STATUS_COUNT)
oRTTI_ENUM_END_DESCRIPTION(oP4_STATUS)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oP4_SUBMIT_OPTIONS)
	oRTTI_ENUM_BEGIN_VALUES(oP4_SUBMIT_OPTIONS)
		oRTTI_VALUE_CUSTOM(oP4_SUBMIT_UNCHANGED, "submitunchanged")
		oRTTI_VALUE_CUSTOM(oP4_SUBMIT_UNCHANGED_REOPEN, "submitunchanged+reopen")
		oRTTI_VALUE_CUSTOM(oP4_REVERT_UNCHANGED, "revertunchanged")
		oRTTI_VALUE_CUSTOM(oP4_REVERT_UNCHANGED_REOPEN, "revertunchanged+reopen")
		oRTTI_VALUE_CUSTOM(oP4_LEAVE_UNCHANGED, "leaveunchanged")
		oRTTI_VALUE_CUSTOM(oP4_LEAVE_UNCHANGED_REOPEN, "leaveunchanged+reopen")
	oRTTI_ENUM_END_VALUES(oP4_SUBMIT_OPTIONS)
	oRTTI_ENUM_VALIDATE_COUNT(oP4_SUBMIT_OPTIONS, oP4_SUBMIT_OPTIONS_COUNT)
oRTTI_ENUM_END_DESCRIPTION(oP4_SUBMIT_OPTIONS)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oP4_LINE_END)
	oRTTI_ENUM_BEGIN_VALUES(oP4_LINE_END)
		oRTTI_VALUE_CUSTOM(oP4_LINE_END_LOCAL, "local")
		oRTTI_VALUE_CUSTOM(oP4_LINE_END_UNIX, "unix")
		oRTTI_VALUE_CUSTOM(oP4_LINE_END_MAC, "mac")
		oRTTI_VALUE_CUSTOM(oP4_LINE_END_WIN, "win")
		oRTTI_VALUE_CUSTOM(oP4_LINE_END_SHARE, "share")
	oRTTI_ENUM_END_VALUES(oP4_LINE_END)
	oRTTI_ENUM_VALIDATE_COUNT(oP4_LINE_END, oP4_LINE_END_COUNT)
oRTTI_ENUM_END_DESCRIPTION(oP4_LINE_END)

static const unsigned int kP4TypicalTimeoutMS = 5000;
static const unsigned int kP4FileTimeoutMS = 60 * 1000;

bool get_kvpair(char* _KeyDestination, size_t _SizeofKeyDestination, char* _ValueDestination, size_t _SizeofValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = nullptr)
{
	const char* k = _SourceString + strspn(_SourceString, oWHITESPACE); // move past whitespace
	const char strSep[2] = { _KeyValueSeparator, 0 };
	const char* sep = k + strcspn(k, strSep); // mark sep
	if (!sep) return false;
	const char* end = sep + 1 + strcspn(sep+1, _KeyValuePairSeparators); // make end of value

	if (_KeyDestination)
	{
		size_t keyLen = sep - k;
		memcpy_s(_KeyDestination, _SizeofKeyDestination-1, k, keyLen);
		_KeyDestination[__min(_SizeofKeyDestination-1, keyLen)] = 0;
	}
	
	if (_ValueDestination)
	{
		const char* v = sep + 1 + strspn(sep+1, oWHITESPACE);
		size_t valLen = end - v;
		memcpy_s(_ValueDestination, _SizeofValueDestination-1, v, valLen);
		_ValueDestination[__min(_SizeofValueDestination-1, valLen)] = 0;
	}

	if (_ppLeftOff)
		*_ppLeftOff = end;

	return true;
}
template<size_t size> bool get_kvpair(char (&_KeyDestination)[size], char* _ValueDestination, size_t _SizeofValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return get_kvpair(_KeyDestination, size, _ValueDestination, _SizeofValueDestination, _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t size> bool get_kvpair(char* _KeyDestination, size_t _SizeofKeyDestination, char (&_ValueDestination)[size], char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return get_kvpair(_KeyDestination, _SizeofKeyDestination, _ValueDestination, size, _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t key_size, size_t value_size> bool get_kvpair(char (&_KeyDestination)[key_size], char (&_ValueDestination)[value_size], const char* _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return get_kvpair(_KeyDestination, key_size, _ValueDestination, value_size, _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t capacity> bool get_kvpair(oStd::fixed_string<char, capacity>& _KeyDestination, char* _ValueDestination, size_t _SizeofValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return get_kvpair(_KeyDestination, _KeyDestination.capacity(), _ValueDestination, _SizeofValueDestination, _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t capacity> bool get_kvpair(char* _KeyDestination, size_t _SizeofKeyDestination, oStd::fixed_string<char, capacity>& _ValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return get_kvpair(_KeyDestination, _SizeofKeyDestination, _ValueDestination, _ValueDestination.capacity(), _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }
template<size_t KEY_capacity, size_t VALUE_capacity> bool get_kvpair(oStd::fixed_string<char, KEY_capacity>& _KeyDestination, oStd::fixed_string<char, VALUE_capacity>& _ValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff = 0) { return get_kvpair(_KeyDestination, _KeyDestination.capacity(), _ValueDestination, _ValueDestination.capacity(), _KeyValueSeparator, _KeyValuePairSeparators, _SourceString, _ppLeftOff); }

static bool oP4IsExecutionError(const char* _P4ResponseString)
{
	static const char* sErrStrings[] =
	{
		"host unknown",
		"use 'client' command to create it",
		"Invalid changelist/",
		"is not under client's root",
		"Perforce client error:",
		"not recognized as an internal",
		"has not been enabled by",
		"- file(s) not in client view.",
	};

	oFORI(i, sErrStrings)
		if (strstr(_P4ResponseString, sErrStrings[i]))
			return true;
	return false;
}

static bool oP4Execute(const char* _CommandLine, const char* _CheckValidString, char* _P4ResponseString, size_t _SizeofP4ResponseString, unsigned int _TimeoutMS = kP4TypicalTimeoutMS)
{
	if (_P4ResponseString)
		*_P4ResponseString = 0;
	if (!oSystemExecute(_CommandLine, [&](char* _Line) { if (_P4ResponseString) strlcat(_P4ResponseString, _Line, _SizeofP4ResponseString); }, nullptr, false, _TimeoutMS))
		return false; // pass through error
	if (!_P4ResponseString)
		return true;
	if (oP4IsExecutionError(_P4ResponseString) || (oSTRVALID(_CheckValidString) && !strstr(_P4ResponseString, _CheckValidString)))
		return oErrorSetLast(std::errc::protocol_error, _P4ResponseString);
	return true;
}

template<typename T, size_t Capacity> inline bool oP4Execute(const char* _CommandLine, const char* _CheckValidString, oStd::fixed_string<T, Capacity>& _P4ResponseString) { return oP4Execute(_CommandLine, _CheckValidString, _P4ResponseString, _P4ResponseString.capacity()); }

bool oP4IsAvailable()
{
	oStd::xlstring response;
	return oP4Execute("p4", "Perforce --", response);
}

bool oP4GetWorkspaceString(char* _P4WorkspaceString, size_t _SizeofP4WorkspaceString)
{
	return oP4Execute("p4 client -o", "# A Perforce Client", _P4WorkspaceString, _SizeofP4WorkspaceString);
}

bool oP4Open(oP4_STATUS _Type, const char* _Path)
{
	if (_Type > oP4_OPEN_FOR_DELETE)
		return oErrorSetLast(std::errc::invalid_argument, "invalid open type");

	oStd::xlstring cmdline, validstring, response;
	oPrintf(cmdline, "p4 %s \"%s\"", oStd::as_string(_Type), oSAFESTR(_Path));
	oPrintf(validstring, " - opened for %s", oStd::as_string(_Type));
	return oP4Execute(cmdline, validstring, response);
}

bool oP4Revert(const char* _Path)
{
	oStd::xlstring cmdline, response;
	oPrintf(cmdline, "p4 revert \"%s\"", _Path);
	return oP4Execute(cmdline, ", reverted", response);
}

oAPI bool oP4Sync(int _ChangeList, const char* _Path/*= nullptr*/, bool _Force/*= false*/)
{
	oStd::xlstring cmdline, response;
	oPrintf(cmdline, "p4 sync %s \"%s\"@%d", _Force ? "-f":"", _Path, _ChangeList);

	// Try twice.  Once to sync, and once to verify
	oP4Execute(cmdline, "", nullptr, 0, kP4FileTimeoutMS);
	oPrintf(cmdline, "p4 sync \"%s\"@%d", _Path, _ChangeList);
	return oP4Execute(cmdline, "up-to-date", response);
}


static void oP4CreateLabelSpec(const oP4_LABEL_SPEC& _Label, std::string& _OutLabelSpec)
{
	oStd::mstring Owner(_Label.Owner);
	if (Owner.empty())
		oSystemGetEnvironmentVariable(Owner, "P4USER");

	oStd::lstring Desc(_Label.Description);
	if (Desc.empty())
		oPrintf(Desc, "Created by %s", Owner.c_str());

	_OutLabelSpec = "Label:	";
	_OutLabelSpec.append(_Label.Label);
	_OutLabelSpec.append("\nOwner:	");
	_OutLabelSpec.append(Owner);
	_OutLabelSpec.append("\nDescription:\n	");
	_OutLabelSpec.append(Desc);
	_OutLabelSpec.append("\n\nOptions:	");
	_OutLabelSpec.append(_Label.Locked ? "locked" : "unlocked");

	if (_Label.Revision)
	{
		oStd::sstring StrRev;
		_OutLabelSpec.append("\nRevision:	");
		_OutLabelSpec.append(oStd::to_string(StrRev, _Label.Revision));
	}

	_OutLabelSpec.append("\nView:\n");
	_OutLabelSpec.append(_Label.View.empty() ? "\t//depot/..." : _Label.View);
	_OutLabelSpec.append("\n\n");
	char AsciiEOF[3];
	AsciiEOF[0] = 26; // ^Z
	AsciiEOF[1] = 26; // ^Z
	AsciiEOF[2] = 0;
	_OutLabelSpec.append(AsciiEOF);
}

bool oP4Label(const oP4_LABEL_SPEC& _Label)
{
	std::string labelSpec;
	oP4CreateLabelSpec(_Label, labelSpec);

	oProcess::DESC desc;
	desc.CommandLine = "p4 label -i";
	desc.EnvironmentString = 0;
	desc.StdHandleBufferSize = oKB(16);
	oStd::intrusive_ptr<threadsafe oProcess> process;
	if (!oProcessCreate(desc, &process))
		return false;

	size_t sizeWritten = process->WriteToStdin(labelSpec.c_str(), labelSpec.size());
	if (sizeWritten != labelSpec.size())
		return oErrorSetLast(std::errc::io_error, "Failed to write label spec to stdin of the p4 process.");

	if (!process->Wait(kP4TypicalTimeoutMS))
		return oErrorSetLast(std::errc::timed_out, "Executing \"%s\" timed out after %.01f seconds.", desc.CommandLine, static_cast<float>(kP4TypicalTimeoutMS) / 1000.0f);

	oStd::xlstring response;
	size_t sizeRead = process->ReadFromStdout(response, response.capacity());
	oASSERT(sizeRead < response.capacity(), "");
	response[sizeRead] = 0;
		
	if (oP4IsExecutionError(response) || !strstr(response, " saved."))
		return oErrorSetLast(std::errc::protocol_error, response);
		
	return true;
}

bool oP4GetChangelistShared(const char* _pSearch, int _ChangeList, oFUNCTION<void(char* _Result)> _Result)
{
	oStd::xlstring cmdline, response;
	oPrintf(cmdline, "p4 change -o %d",_ChangeList);
	if (!oP4Execute(cmdline, "A Perforce Change Specification", response))
		return false;

	char* pUserLine = response.c_str();

	// _pSearch shows up twice.  Once in the info and once for real
	for(int i = 0; i < 2; ++i)
	{
		pUserLine = strstr(pUserLine, _pSearch);
		if (!pUserLine)
			return oErrorSetLast(std::errc::protocol_error, "No %s found", _pSearch);

		pUserLine += strlen(_pSearch);
	}

	// Skip whitespace
	while (pUserLine[0] == ' ')
	{
		++pUserLine;
	}

	_Result(pUserLine);
	return true;
}
bool oP4GetChangelistDescription(char* _StrDestination, size_t _SizeofStrDestination, int _ChangeList)
{
	return oP4GetChangelistShared("Description:", _ChangeList, [&](char* _Result)
	{
		while (_Result[0] != 0 && (_Result[0] < 32) ) // Skip formatting
		{
			++_Result;
		}
		oStrcpy(_StrDestination, _SizeofStrDestination, _Result);
	});
}

bool oP4GetChangelistUser(char* _StrDestination, size_t _SizeofStrDestination, int _ChangeList)
{
	return oP4GetChangelistShared("User:", _ChangeList, [&](char* _Result)
		{
			const char* pUserName = _Result;
			// Null terminate
			{
				while (_Result[0] != '\xD')
				{
					++_Result;
				}
				_Result[0] = 0;
			}
			oStrcpy(_StrDestination, _SizeofStrDestination, pUserName);
		});
}

bool oP4GetChangelistDate(char* _StrDestination, size_t _SizeofStrDestination, int _ChangeList)
{
	return oP4GetChangelistShared("Date:", _ChangeList, [&](char* _Result)
	{
		const char* pDate = _Result;
		// Null terminate
		{
			while (_Result[0] != '\xD')
			{
				++_Result;
			}
			_Result[0] = 0;
		}
		oStrcpy(_StrDestination, _SizeofStrDestination, pDate);
	});
}

bool oP4GetClientPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _pDepotPath)
{
	oStd::xlstring cmdline, response;
	oPrintf(cmdline, "p4 fstat %s", _pDepotPath);

	const char ClientFile[] = "clientFile";
	if (!oP4Execute(cmdline, ClientFile, response))
		return false;
	
	char* pFileLine = strstr(response, ClientFile);
	pFileLine += oCOUNTOF(ClientFile);
	const char* pFileName = pFileLine;
	// Null terminate
	{
		while (pFileLine[0] != '\xD')
		{
			++pFileLine;
		}
		pFileLine[0] = 0;
	}
	oStd::clean_path(_StrDestination, _SizeofStrDestination, pFileName);
	return true;
}


bool oP4GetLabelSpecString(char* _P4LabelSpecString, size_t _SizeofP4LabelSpecString, const char* _Label)
{
	oStd::lstring cmdline;
	oPrintf(cmdline, "p4 label -o %s", oSAFESTRN(_Label));
	return oP4Execute(cmdline, "# A Perforce Label", _P4LabelSpecString, _SizeofP4LabelSpecString);
}

// Parses the pieces from the format returned by "p4 files" or "p4 opened"
// modifies _P4FilesLine for parsing.
// returns the pointer to the next entry/line
static char* oP4ParseFilesLine(oP4_FILE_DESC* _pFile, char* _P4FilesLine)
{
	char* c = _P4FilesLine;

	char* hash = c + strcspn(c, "#");
	char* dash = hash + strcspn(hash, "-");

	*hash = 0;
	_pFile->Path = c;

	*dash = 0;
	_pFile->Revision = atoi(hash+1);

	char* attribs = dash+1;
	c = attribs + strcspn(attribs, oNEWLINE);
	*c = 0;

	char* ctx = nullptr;
	char* tok = oStrTok(attribs, " ", &ctx);
	oStd::from_string(&_pFile->Status, tok);

	tok = oStrTok(nullptr, " ", &ctx);
	if (!oStrcmp("default", tok))
		_pFile->Changelist = oInvalid;

	tok = oStrTok(nullptr, " ", &ctx);
	if (oStrcmp("change", tok))
		_pFile->Changelist = atoi(tok);

	tok = oStrTok(nullptr, " ", &ctx);
	_pFile->IsText = !oStrcmp("(text)", tok);

	oStrTokClose(&ctx);

	return c + 1 + strspn(c + 1, oNEWLINE);
}

// This destroys _FileListString
static size_t oP4ParseOpenedList(oP4_FILE_DESC* _pOpenedFiles, size_t _MaxNumOpenedFiles, char* _FileListString)
{
	char* c = _FileListString;
	char* end = c + oStrlen(c);
	size_t i = 0;
	while (c < end)
	{
		c = oP4ParseFilesLine(&_pOpenedFiles[i], c);
		i++;
		if (i >= _MaxNumOpenedFiles)
			break;
	}

	return i;
}

size_t oP4ListOpened(oP4_FILE_DESC* _pOpenedFiles, size_t _MaxNumOpenedFiles, const char* _P4Base)
{
	oStd::xlstring cmdline;
	std::vector<char> response;
	response.resize(oKB(100));
	char* result = oStd::data(response);
	oPrintf(cmdline, "p4 opened -m %u %s", _MaxNumOpenedFiles, oSAFESTR(_P4Base));
	if (!oP4Execute(cmdline, nullptr, result, response.size()))
		return oInvalid; // pass through error

	if (!oStrncmp("File(s) not", oStd::data(response), 11))
		return 0;

	if (strstr(result, "file(s) not opened"))
		return 0;

	return oP4ParseOpenedList(_pOpenedFiles, _MaxNumOpenedFiles, oStd::data(response));
}

// Parses the pieces from the format returned by "p4 sync -n"
// modifies _P4SyncLine for parsing.
// returns the pointer to the next entry/line
// This hard-sets Changelist to oInvalid and IsText to false because that data
// is not included in this data.
static char* oP4ParseSyncLine(oP4_FILE_DESC* _pFile, char* _P4SyncLine)
{
	char* c = _P4SyncLine;

	char* hash = c + strcspn(c, "#");
	char* dash = hash + strcspn(hash, "-");

	*hash = 0;
	_pFile->Path = c;

	*dash = 0;
	_pFile->Revision = atoi(hash+1);

	char* attribs = dash+1;
	c = attribs + strcspn(attribs, oNEWLINE);
	*c = 0;

	if (!strstr(attribs, "can't be replaced"))
	{
		if (strstr(attribs, "not being changed"))
			_pFile->Status = oP4_NEEDS_RESOLVE;
		else
		{
			char* ctx = nullptr;
			char* tok = oStrTok(attribs, " ", &ctx);
			oVERIFY(oStd::from_string(&_pFile->Status, tok));

			tok = oStrTok(nullptr, " ", &ctx);
			if (!oStrcmp("as", tok))
				tok = oStrTok(nullptr, " ", &ctx);

			oStrTokClose(&ctx);
		}
	}

	// Changelist cannot be parsed from the source string, a new query must be 
	// made.
	_pFile->Changelist = oInvalid;

	// IsText cannot be parsed from the source string, a new query must be made.
	_pFile->IsText = false;

	return c + 1 + strspn(c + 1, oNEWLINE);
}

// This destroys _OutOfDateListString
static size_t oP4ParseOutOfDateList(oP4_FILE_DESC* _pOutOfDateFiles, size_t _MaxNumOutOfDateFiles, char* _OutOfDateListString)
{
	char* c = _OutOfDateListString;
	char* end = c + oStrlen(c);

	size_t i = 0;
	while (c < end)
	{
		c = oP4ParseSyncLine(&_pOutOfDateFiles[i], c);
		i++;
		if (i >= _MaxNumOutOfDateFiles)
			break;
	}

	return i;
}

size_t oP4ListOutOfDate(oP4_FILE_DESC* _pOutOfDateFiles, size_t _MaxNumOutOfDateFiles, const char* _P4Base, int _UpToChangelist)
{
	oStd::xlstring cmdline;
	std::vector<char> response;
	response.resize(oKB(100));
	char* result = oStd::data(response);

	if (!oSTRVALID(_P4Base) && _UpToChangelist != oInvalid)
		_P4Base = "//...";

	if (_UpToChangelist != oInvalid)
		oPrintf(cmdline, "p4 sync -n %s@%d", oSAFESTR(_P4Base), _UpToChangelist);
	else
		oPrintf(cmdline, "p4 sync -n %s", oSAFESTR(_P4Base));

	if (!oP4Execute(cmdline, nullptr, result, response.size()))
		return oInvalid; // pass through error

	if (strstr(result, "up-to-date"))
		return 0;

	size_t nFiles = oP4ParseOutOfDateList(_pOutOfDateFiles, _MaxNumOutOfDateFiles, result);
	if (nFiles == oInvalid)
		return oInvalid;

	// Now patch changelist and file type
	for (size_t i = 0; i < nFiles; i++)
	{
		if (_UpToChangelist != oInvalid)
			oPrintf(cmdline, "p4 files %s@%d", _pOutOfDateFiles[i].Path.c_str(), _UpToChangelist);
		else
			oPrintf(cmdline, "p4 files %s", _pOutOfDateFiles[i].Path.c_str());

		if (!oP4Execute(cmdline, nullptr, result, response.size()))
			return oInvalid; // pass through error

		// ignore other fields
		oP4_FILE_DESC fd;
		oP4ParseFilesLine(&fd, result);
		_pOutOfDateFiles[i].Changelist = fd.Changelist;
		_pOutOfDateFiles[i].IsText = fd.IsText;
	}

	return nFiles;
}

time_t oP4ParseTime(const char* _P4TimeString)
{
	int Y,M,D,h,m,s;
	sscanf_s(_P4TimeString, "%d/%d/%d %d:%d:%d", &Y,&M,&D,&h,&m,&s);
	tm t;
	t.tm_sec = s;
	t.tm_min = m;
	t.tm_hour = h;
	t.tm_mday = D;
	t.tm_mon = M;
	t.tm_year = Y;
	return mktime(&t);
}

// Determine if a given string is the next non-whitespace text in a string.
#define NEXT_STR_EXISTS(str1, str2, bExists) str1 += strspn(str1, oWHITESPACE); bExists = ((str1 - strstr(str1, str2)) == 0) ? true : false

bool oP4ParseWorkspace(oP4_WORKSPACE* _pWorkspace, const char* _P4WorkspaceString)
{
	// move past commented text
	const char* c = _P4WorkspaceString;
	c = strstr(c, "views and options.");
	if (!c) return false;
	c += oStrlen("views and options.");
	bool bNextStrExists = false;

	//Client
	NEXT_STR_EXISTS(c, "Client:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, _pWorkspace->Client, ':', oNEWLINE, c, &c))
		return false;

	oStd::mstring tmp;

	//Update
	NEXT_STR_EXISTS(c, "Update:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pWorkspace->LastUpdated = oP4ParseTime(tmp);

	//Access
	NEXT_STR_EXISTS(c, "Access:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pWorkspace->LastAccessed = oP4ParseTime(tmp);

	//Owner
	NEXT_STR_EXISTS(c,"Owner:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, _pWorkspace->Owner, ':', oNEWLINE, c, &c))
		return false;

	//Host
	NEXT_STR_EXISTS(c, "Host:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, _pWorkspace->Host, ':', oNEWLINE, c, &c))
		return false;

	// Description is multi-line...
	c += strspn(c, oWHITESPACE);
	c += oStrlen("Description:");
	c += strspn(c, oWHITESPACE);
	const char* end = strstr(c, oNEWLINE oNEWLINE "Root:");

	size_t descLen = end - c;
	memcpy_s(_pWorkspace->Description, _pWorkspace->Description.capacity()-1, c, descLen);
	_pWorkspace->Description[__min(_pWorkspace->Description.capacity()-1, descLen)] = 0;

	//Root
	NEXT_STR_EXISTS(end, "Root:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, _pWorkspace->Root, ':', oNEWLINE, end, &c))
		return false;

	//Options
	NEXT_STR_EXISTS(c, "Options:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;

	_pWorkspace->Allwrite = !strstr(tmp, "noallwrite");
	_pWorkspace->Clobber = !strstr(tmp, "noclobber");
	_pWorkspace->Compress = !strstr(tmp, "nocompress");
	_pWorkspace->Locked = !strstr(tmp, "unlocked");
	_pWorkspace->Modtime = !strstr(tmp, "nomodtime");
	_pWorkspace->Rmdir = !strstr(tmp, "normdir");

	//SubmitOptions
	NEXT_STR_EXISTS(c, "SubmitOptions:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	if (!oStd::from_string(&_pWorkspace->SubmitOptions, tmp))
		return false;

	//LineEnd
	NEXT_STR_EXISTS(c, "LineEnd:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	if (!oStd::from_string(&_pWorkspace->LineEnd, tmp))
		return false;

	// View is multi-line...
	c += strspn(c, oWHITESPACE);
	c += oStrlen("View:");
	oStrcpy(_pWorkspace->View, c);

	return true;
}

int oP4ParseChangesLine(const char* _ChangesLine)
{
	char* ctx = nullptr;
	const char* tok = oStrTok(_ChangesLine, " ", &ctx);
	oStd::finally OSEClose([&]{ oStrTokClose(&ctx); });

	if (tok)
	{
		tok = oStrTok(nullptr, "", &ctx);
		if (tok)
			return atoi(tok);
	}

	oErrorSetLast(std::errc::protocol_error);
	return oInvalid;
}

bool oP4ParseLabelSpec(oP4_LABEL_SPEC* _pLabelSpec, const char* _P4LabelSpecString)
{
	// move past commented text
	const char* c = _P4LabelSpecString;
	c = strstr(c, "about label views.");
	if (!c) return false;
	c += oStrlen("about label views.");
	bool bNextStrExists = false;

	//Label
	NEXT_STR_EXISTS(c, "Label:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, _pLabelSpec->Label, ':', oNEWLINE, c, &c))
		return false;

	oStd::mstring tmp;

	//Update
	NEXT_STR_EXISTS(c, "Update:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pLabelSpec->LastUpdated = oP4ParseTime(tmp);

	//Access
	NEXT_STR_EXISTS(c, "Access:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pLabelSpec->LastAccessed = oP4ParseTime(tmp);

	//Owner
	NEXT_STR_EXISTS(c,"Owner:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, _pLabelSpec->Owner, ':', oNEWLINE, c, &c))
		return false;

	// Description is multi-line...
	c += strspn(c, oWHITESPACE);
	c += oStrlen("Description:");
	c += strspn(c, oWHITESPACE);
	const char* end = strstr(c, oNEWLINE oNEWLINE "Options:");

	size_t descLen = end - c;
	memcpy_s(_pLabelSpec->Description, _pLabelSpec->Description.capacity()-1, c, descLen);
	_pLabelSpec->Description[__min(_pLabelSpec->Description.capacity()-1, descLen)] = 0;

	//Options
	NEXT_STR_EXISTS(end, "Options:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pLabelSpec->Locked = !!strstr(tmp, "unlocked");

	//Revision
	NEXT_STR_EXISTS(c, "Revision:", bNextStrExists);
	if (bNextStrExists && !get_kvpair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	if (!oStd::from_string(&_pLabelSpec->Revision, tmp))
		return false;

	//View is multi-line...
	c += strspn(c, oWHITESPACE);
	c += oStrlen("View:");
	oStrcpy(_pLabelSpec->View, c);

	return true;
}

static int oP4RunChangesCommand(const char* _P4Base, const char* _pCommand)
{
	oStd::xlstring cmdline;
	std::vector<char> response;
	response.resize(oKB(100));
	char* result = oStd::data(response);

	if (!oSTRVALID(_P4Base))
		_P4Base = "//...";

	oPrintf(cmdline, "p4 changes -m1 %s%s", _P4Base, _pCommand);

	if (!oP4Execute(cmdline, nullptr, result, response.size()))
		return oInvalid; // pass through error

	return oP4ParseChangesLine(result);
}

static int oP4GetTopOfTree(const char* _P4Base)
{
	return oP4RunChangesCommand(_P4Base, "");
}

static int oP4EstimateHaveChangelist(const char* _P4Base)
{
	return oP4RunChangesCommand(_P4Base, "#have");
}

#define oSAFEBASE(_P4Base) ((_P4Base) ? (_P4Base) : "//...")

int oP4GetCurrentChangelist(const char* _P4Base)
{
	std::vector<oP4_FILE_DESC> files;
	files.resize(1); // for speed, don't check every file, just see if anything is open or out-of-date.
	// Check all open files if they're under P4Base
	size_t nOpened = oP4ListOpened(oStd::data(files), files.size(), _P4Base);
	if (nOpened == oInvalid)
		return oInvalid; // pass through error

	int CL = oP4EstimateHaveChangelist(_P4Base);

	// Spew out the reasons for a non-pure estimate for dbgview
	for (size_t i = 0; i < nOpened; i++)
		oTRACEA("P4 Changelist is non-pure due to opened: %s", files[i].Path.c_str());

	// Now check for needed updates
	size_t nOutOfDate = oP4ListOutOfDate(oStd::data(files), files.size(), _P4Base, CL);
	if (nOutOfDate == oInvalid)
		return oInvalid; // pass through error
	
	else if (nOutOfDate)
	{
		for (size_t i = 0; i < nOutOfDate; i++)
			oTRACEA("P4 Changelist is non-pure due to out-of-date: %s", files[i].Path.c_str());

		if (nOpened)
			oErrorSetLast(std::errc::protocol_error, "opened and out-of-date files under %s mean changelist is only an estimate", oSAFEBASE(_P4Base));
		else
			oErrorSetLast(std::errc::protocol_error, "out-of-date files under %s mean changelist is only an estimate", oSAFEBASE(_P4Base));
	}

	if (nOpened)
		oErrorSetLast(std::errc::protocol_error, "open files under %s mean changelist is only an estimate", oSAFEBASE(_P4Base));
	else
		oErrorSetLast(0);

	return CL;
}

int oP4GetNextChangelist(int _CurrentCL, const char* _P4Base /*= nullptr*/)
{
	int TOT = oP4GetTopOfTree(_P4Base);
	if(oInvalid == TOT || _CurrentCL == TOT)
		return TOT;

	for(int CL = _CurrentCL + 1; CL < TOT; ++CL)
	{
		// Check CL in between to see if they have changes
		oStd::sstring cmd;
		oPrintf(cmd, "@%d", CL);
		int ChangesUpTo = oP4RunChangesCommand(_P4Base, cmd);

		if(oInvalid == ChangesUpTo || ChangesUpTo == CL)
			return CL;
	}

	return TOT;
}
