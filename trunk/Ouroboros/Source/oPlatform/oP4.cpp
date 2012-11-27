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
#include <oPlatform/oP4.h>
#include <oBasis/oRef.h>
#include <oBasis/oString.h>
#include <oPlatform/oReporting.h>
#include <oBasis/oError.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oProcess.h>
#include <time.h>

static const unsigned int kP4Timeout = 5000;

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

	for (size_t i = 0; i < oCOUNTOF(sErrStrings); i++)
	if (strstr(_P4ResponseString, sErrStrings[i]))
		return true;
	return false;
}

static bool oP4Execute(const char* _CommandLine, const char* _CheckValidString, char* _P4ResponseString, size_t _SizeofP4ResponseString)
{
	if (!oSystemExecute(_CommandLine, _P4ResponseString, _SizeofP4ResponseString, nullptr, kP4Timeout))
		return false; // pass through error
	if (!_P4ResponseString)
		return true;
	if (oP4IsExecutionError(_P4ResponseString) || (oSTRVALID(_CheckValidString) && !strstr(_P4ResponseString, _CheckValidString)))
		return oErrorSetLast(oERROR_NOT_FOUND, _P4ResponseString);
	return true;
}

template<typename T, size_t Capacity> inline bool oP4Execute(const char* _CommandLine, const char* _CheckValidString, oFixedString<T, Capacity>& _P4ResponseString) { return oP4Execute(_CommandLine, _CheckValidString, _P4ResponseString, _P4ResponseString.capacity()); }

const char* oAsString(const oP4_STATUS& _Value)
{
	switch (_Value)
	{
		case oP4_OPEN_FOR_EDIT: return "edit";
		case oP4_OPEN_FOR_ADD: return "add";
		case oP4_OPEN_FOR_DELETE: return "delete";
		case oP4_NEEDS_UPDATE: return "updating";
		case oP4_NEEDS_ADD: return "added";
		case oP4_NEEDS_DELETE: return "deleted";
		oNODEFAULT;
	}
}

bool oFromString(oP4_STATUS* _pValue, const char* _StrSource)
{
	for (size_t i = 0; i < oP4_NUM_STATUS_VALUES; i++)
	{
		if (!oStrcmp(_StrSource, oAsString((oP4_STATUS)i)))
		{
			*_pValue = (oP4_STATUS)i;
			return true;
		}
	}
	return false;
}

const char* oAsString(const oP4_SUBMIT_OPTIONS& _Value)
{
	switch (_Value)
	{
		case oP4_SUBMIT_UNCHANGED: return "submitunchanged";
		case oP4_SUBMIT_UNCHANGED_REOPEN: return "submitunchanged+reopen";
		case oP4_REVERT_UNCHANGED: return "revertunchanged";
		case oP4_REVERT_UNCHANGED_REOPEN: return "revertunchanged+reopen";
		case oP4_LEAVE_UNCHANGED: return "leaveunchanged";
		case oP4_LEAVE_UNCHANGED_REOPEN: return "leaveunchanged+reopen";
		oNODEFAULT;
	}
}

bool oFromString(oP4_SUBMIT_OPTIONS* _pValue, const char* _StrSource)
{
	for (size_t i = 0; i <= oP4_LEAVE_UNCHANGED_REOPEN; i++)
	{
		if (!oStrcmp(_StrSource, oAsString((oP4_SUBMIT_OPTIONS)i)))
		{
			*_pValue = (oP4_SUBMIT_OPTIONS)i;
			return true;
		}
	}
	return false;
}

const char* oAsString(const oP4_LINE_END& _Value)
{
	switch (_Value)
	{
		case oP4_LINE_END_LOCAL: return "local";
		case oP4_LINE_END_UNIX: return "unix";
		case oP4_LINE_END_MAC: return "mac";
		case oP4_LINE_END_WIN: return "win";
		case oP4_LINE_END_SHARE: return "share";
 		oNODEFAULT;
	}
}

bool oFromString(oP4_LINE_END* _pValue, const char* _StrSource)
{
	for (size_t i = 0; i <= oP4_LINE_END_SHARE; i++)
	{
		if (!oStrcmp(_StrSource, oAsString((oP4_LINE_END)i)))
		{
			*_pValue = (oP4_LINE_END)i;
			return true;
		}
	}
	return false;
}

bool oP4IsAvailable()
{
	oStringXL response;
	return oP4Execute("p4", "Perforce --", response);
}

bool oP4GetWorkspaceString(char* _P4WorkspaceString, size_t _SizeofP4WorkspaceString)
{
	return oP4Execute("p4 client -o", "# A Perforce Client", _P4WorkspaceString, _SizeofP4WorkspaceString);
}

bool oP4Open(oP4_STATUS _Type, const char* _Path)
{
	if (_Type > oP4_OPEN_FOR_DELETE)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "invalid open type");

	oStringXL cmdline, validstring, response;
	oPrintf(cmdline, "p4 %s \"%s\"", oAsString(_Type), oSAFESTR(_Path));
	oPrintf(validstring, " - opened for %s", oAsString(_Type));
	return oP4Execute(cmdline, validstring, response);
}

bool oP4Revert(const char* _Path)
{
	oStringXL cmdline, response;
	oPrintf(cmdline, "p4 revert \"%s\"", _Path);
	return oP4Execute(cmdline, ", reverted", response);
}


oAPI bool oP4Sync(int _ChangeList, const char* _Path/*= nullptr*/)
{
	oStringXL cmdline, response;
	oPrintf(cmdline, "p4 sync \"%s\"@%d", _Path, _ChangeList);

	// Try twice.  Once to sync, and once to verify
	oP4Execute(cmdline, "", nullptr, 0);
	return oP4Execute(cmdline, "up-to-date", response);
}


static void oP4CreateLabelSpec(const oP4_LABEL_SPEC& _Label, std::string& _OutLabelSpec)
{
	oStringM Owner(_Label.Owner);
	if (Owner.empty())
		oSystemGetEnvironmentVariable(Owner, "P4USER");

	oStringL Desc(_Label.Description);
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
		oStringS StrRev;
		_OutLabelSpec.append("\nRevision:	");
		_OutLabelSpec.append(oToString(StrRev, _Label.Revision));
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
	oRef<threadsafe oProcess> process;
	if (!oProcessCreate(desc, &process))
		return false;
	process->Start();

	size_t sizeWritten = process->WriteToStdin(labelSpec.c_str(), labelSpec.size());
	if (sizeWritten != labelSpec.size())
		return oErrorSetLast(oERROR_IO, "Failed to write label spec to stdin of the p4 process.");

	if (!process->Wait(kP4Timeout))
		return oErrorSetLast(oERROR_TIMEOUT, "Executing \"%s\" timed out after %.01f seconds.", desc.CommandLine, static_cast<float>(kP4Timeout) / 1000.0f);

	oStringXL response;
	size_t sizeRead = process->ReadFromStdout(response, response.capacity());
	oASSERT(sizeRead < response.capacity(), "");
	response[sizeRead] = 0;
		
	if (oP4IsExecutionError(response) || !strstr(response, " saved."))
		return oErrorSetLast(oERROR_GENERIC, response);
		
	return true;
}

bool oP4GetUserOfChangelist(char* _StrDestination, size_t _SizeofStrDestination, int _ChangeList)
{
	oStringXL cmdline, response;
	oPrintf(cmdline, "p4 change -o %d",_ChangeList);
	if (!oP4Execute(cmdline, "A Perforce Change Specification", response))
		return false;

	const char UserStart[] = "User:";
	char* pUserLine = response.c_str();
	
	// UserStart shows up twice.  Once in the info and once for real
	for(int i = 0; i < 2; ++i)
	{
		pUserLine = strstr(pUserLine, UserStart);
		if (!pUserLine)
			return oErrorSetLast(oERROR_GENERIC, "No user found");

		pUserLine += oCOUNTOF(UserStart);
	}
	
	// Skip whitespace
	while (pUserLine[0] == ' ')
	{
		++pUserLine;
	}

	const char* pUserName = pUserLine;
	// Null terminate
	{
		while (pUserLine[0] != '\xD')
		{
			++pUserLine;
		}
		pUserLine[0] = 0;
	}


	oPrintf(_StrDestination, _SizeofStrDestination, pUserName);
	return true;
}


bool oP4GetClientPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _pDepotPath)
{
	oStringXL cmdline, response;
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
	oCleanPath(_StrDestination, _SizeofStrDestination, pFileName);
	return true;
}


bool oP4GetLabelSpecString(char* _P4LabelSpecString, size_t _SizeofP4LabelSpecString, const char* _Label)
{
	oStringL cmdline;
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
	oFromString(&_pFile->Status, tok);

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
	char* end = c + strlen(c);
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
	oStringXL cmdline;
	std::vector<char> response;
	response.resize(oKB(100));
	char* result = oGetData(response);
	oPrintf(cmdline, "p4 opened -m %u %s", _MaxNumOpenedFiles, oSAFESTR(_P4Base));
	if (!oP4Execute(cmdline, nullptr, result, response.size()))
		return oInvalid; // pass through error

	if (!oStrncmp("File(s) not", oGetData(response), 11))
	{
		oErrorSetLast(oERROR_END_OF_FILE, "%s", result);
		return 0;
	}

	if (strstr(result, "file(s) not opened"))
		return 0;

	return oP4ParseOpenedList(_pOpenedFiles, _MaxNumOpenedFiles, oGetData(response));
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

	char* ctx = nullptr;
	char* tok = oStrTok(attribs, " ", &ctx);
	oVERIFY(oFromString(&_pFile->Status, tok));

	tok = oStrTok(nullptr, " ", &ctx);
	if (!oStrcmp("as", tok))
		tok = oStrTok(nullptr, " ", &ctx);

	// Changelist cannot be parsed from the source string, a new query must be 
	// made.
	_pFile->Changelist = oInvalid;

	// IsText cannot be parsed from the source string, a new query must be made.
	_pFile->IsText = false;

	oStrTokClose(&ctx);

	return c + 1 + strspn(c + 1, oNEWLINE);
}

// This destroys _OutOfDateListString
static size_t oP4ParseOutOfDateList(oP4_FILE_DESC* _pOutOfDateFiles, size_t _MaxNumOutOfDateFiles, char* _OutOfDateListString)
{
	char* c = _OutOfDateListString;
	char* end = c + strlen(c);

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
	oStringXL cmdline;
	std::vector<char> response;
	response.resize(oKB(100));
	char* result = oGetData(response);

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
	c += strlen("views and options.");
	bool bNextStrExists = false;

	//Client
	NEXT_STR_EXISTS(c, "Client:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, _pWorkspace->Client, ':', oNEWLINE, c, &c))
		return false;

	oStringM tmp;

	//Update
	NEXT_STR_EXISTS(c, "Update:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pWorkspace->LastUpdated = oP4ParseTime(tmp);

	//Access
	NEXT_STR_EXISTS(c, "Access:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pWorkspace->LastAccessed = oP4ParseTime(tmp);

	//Owner
	NEXT_STR_EXISTS(c,"Owner:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, _pWorkspace->Owner, ':', oNEWLINE, c, &c))
		return false;

	//Host
	NEXT_STR_EXISTS(c, "Host:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, _pWorkspace->Host, ':', oNEWLINE, c, &c))
		return false;

	// Description is multi-line...
	c += strspn(c, oWHITESPACE);
	c += strlen("Description:");
	c += strspn(c, oWHITESPACE);
	const char* end = strstr(c, oNEWLINE oNEWLINE "Root:");

	size_t descLen = end - c;
	memcpy_s(_pWorkspace->Description, _pWorkspace->Description.capacity()-1, c, descLen);
	_pWorkspace->Description[__min(_pWorkspace->Description.capacity()-1, descLen)] = 0;

	//Root
	NEXT_STR_EXISTS(end, "Root:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, _pWorkspace->Root, ':', oNEWLINE, end, &c))
		return false;

	//Options
	NEXT_STR_EXISTS(c, "Options:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;

	_pWorkspace->Allwrite = !strstr(tmp, "noallwrite");
	_pWorkspace->Clobber = !strstr(tmp, "noclobber");
	_pWorkspace->Compress = !strstr(tmp, "nocompress");
	_pWorkspace->Locked = !strstr(tmp, "unlocked");
	_pWorkspace->Modtime = !strstr(tmp, "nomodtime");
	_pWorkspace->Rmdir = !strstr(tmp, "normdir");

	//SubmitOptions
	NEXT_STR_EXISTS(c, "SubmitOptions:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	if (!oFromString(&_pWorkspace->SubmitOptions, tmp))
		return false;

	//LineEnd
	NEXT_STR_EXISTS(c, "LineEnd:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	if (!oFromString(&_pWorkspace->LineEnd, tmp))
		return false;

	// View is multi-line...
	c += strspn(c, oWHITESPACE);
	c += strlen("View:");
	oStrcpy(_pWorkspace->View, c);

	return true;
}

int oP4ParseChangesLine(const char* _ChangesLine)
{
	char* ctx = nullptr;
	const char* tok = oStrTok(_ChangesLine, " ", &ctx);
	oOnScopeExit OSEClose([&]{ oStrTokClose(&ctx); });

	if (tok)
	{
		tok = oStrTok(nullptr, "", &ctx);
		if (tok)
			return atoi(tok);
	}

	oErrorSetLast(oERROR_CORRUPT);
	return oInvalid;
}

bool oP4ParseLabelSpec(oP4_LABEL_SPEC* _pLabelSpec, const char* _P4LabelSpecString)
{
	// move past commented text
	const char* c = _P4LabelSpecString;
	c = strstr(c, "about label views.");
	if (!c) return false;
	c += strlen("about label views.");
	bool bNextStrExists = false;

	//Label
	NEXT_STR_EXISTS(c, "Label:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, _pLabelSpec->Label, ':', oNEWLINE, c, &c))
		return false;

	oStringM tmp;

	//Update
	NEXT_STR_EXISTS(c, "Update:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pLabelSpec->LastUpdated = oP4ParseTime(tmp);

	//Access
	NEXT_STR_EXISTS(c, "Access:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pLabelSpec->LastAccessed = oP4ParseTime(tmp);

	//Owner
	NEXT_STR_EXISTS(c,"Owner:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, _pLabelSpec->Owner, ':', oNEWLINE, c, &c))
		return false;

	// Description is multi-line...
	c += strspn(c, oWHITESPACE);
	c += strlen("Description:");
	c += strspn(c, oWHITESPACE);
	const char* end = strstr(c, oNEWLINE oNEWLINE "Options:");

	size_t descLen = end - c;
	memcpy_s(_pLabelSpec->Description, _pLabelSpec->Description.capacity()-1, c, descLen);
	_pLabelSpec->Description[__min(_pLabelSpec->Description.capacity()-1, descLen)] = 0;

	//Options
	NEXT_STR_EXISTS(end, "Options:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pLabelSpec->Locked = !!strstr(tmp, "unlocked");

	//Revision
	NEXT_STR_EXISTS(c, "Revision:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	if (!oFromString(&_pLabelSpec->Revision, tmp))
		return false;

	//View is multi-line...
	c += strspn(c, oWHITESPACE);
	c += strlen("View:");
	oStrcpy(_pLabelSpec->View, c);

	return true;
}

static int oP4RunChangesCommand(const char* _P4Base, const char* _pCommand)
{
	oStringXL cmdline;
	std::vector<char> response;
	response.resize(oKB(100));
	char* result = oGetData(response);

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
	size_t nOpened = oP4ListOpened(oGetData(files), files.size(), _P4Base);
	if (nOpened == oInvalid)
		return oInvalid; // pass through error

	int CL = oP4EstimateHaveChangelist(_P4Base);

	// Spew out the reasons for a non-pure estimate for dbgview
	for (size_t i = 0; i < nOpened; i++)
		oTRACEA("P4 Changelist is non-pure due to opened: %s", files[i].Path.c_str());

	// Now check for needed updates
	size_t nOutOfDate = oP4ListOutOfDate(oGetData(files), files.size(), _P4Base, CL);
	if (nOutOfDate == oInvalid)
		return oInvalid; // pass through error
	
	else if (nOutOfDate)
	{
		for (size_t i = 0; i < nOutOfDate; i++)
			oTRACEA("P4 Changelist is non-pure due to out-of-date: %s", files[i].Path.c_str());

		if (nOpened)
			oErrorSetLast(oERROR_CORRUPT, "opened and out-of-date files under %s mean changelist is only an estimate", oSAFEBASE(_P4Base));
		else
			oErrorSetLast(oERROR_CORRUPT, "out-of-date files under %s mean changelist is only an estimate", oSAFEBASE(_P4Base));
	}

	if (nOpened)
		oErrorSetLast(oERROR_CORRUPT, "open files under %s mean changelist is only an estimate", oSAFEBASE(_P4Base));
	else
		oErrorSetLast(oERROR_NONE);

	return CL;
}

int oP4GetNextChangelist(int _CurrentCL, const char* _P4Base /*= nullptr*/)
{
	int TOT = oP4GetTopOfTree(_P4Base);
	if (_CurrentCL == TOT)
		return TOT;

	for(int CL = _CurrentCL + 1; CL < TOT; ++CL)
	{
		// Check CL in between to see if they have changes
		oStringS cmd;
		oPrintf(cmd, "@%d", CL);
		int ChangesUpTo = oP4RunChangesCommand(_P4Base, cmd);
		if (ChangesUpTo == CL)
			return CL;
	}

	return TOT;
}
