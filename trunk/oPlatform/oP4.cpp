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
#include <oPlatform/oStddef.h>
#include <oPlatform/oSystem.h>
#include <time.h>

static bool oP4IsExecutionError(const char* _P4ResponseString)
{
	static const char* sErrStrings[] =
	{
		"host unknown",
		"use 'client' command to create it",
	};

	for (size_t i = 0; i < oCOUNTOF(sErrStrings); i++)
	if (strstr(_P4ResponseString, sErrStrings[i]))
		return true;
	return false;
}

static bool oP4Execute(const char* _CommandLine, const char* _CheckValidString, char* _P4ResponseString, size_t _SizeofP4ResponseString)
{
	if (!oSystemExecute(_CommandLine, _P4ResponseString, _SizeofP4ResponseString, 0, 5000))
		return false; // pass through error
	if (oP4IsExecutionError(_P4ResponseString) || (oSTRVALID(_CheckValidString) && !strstr(_P4ResponseString, _CheckValidString)))
		return oErrorSetLast(oERROR_NOT_FOUND, _P4ResponseString);
	return true;
}

template<typename T, size_t Capacity> inline bool oP4Execute(const char* _CommandLine, const char* _CheckValidString, oFixedString<T, Capacity>& _P4ResponseString) { return oP4Execute(_CommandLine, _CheckValidString, _P4ResponseString, _P4ResponseString.capacity()); }

const char* oAsString(const oP4_OPEN_TYPE& _Value)
{
	switch (_Value)
	{
		case oP4_OPEN_FOR_EDIT: return "edit";
		case oP4_OPEN_FOR_ADD: return "add";
		case oP4_OPEN_FOR_DELETE: return "delete";
		oNODEFAULT;
	}
}

bool oFromString(oP4_OPEN_TYPE* _pValue, const char* _StrSource)
{
	for (size_t i = 0; i <= oP4_OPEN_FOR_DELETE; i++)
	{
		if (!strcmp(_StrSource, oAsString((oP4_OPEN_TYPE)i)))
		{
			*_pValue = (oP4_OPEN_TYPE)i;
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
		if (!strcmp(_StrSource, oAsString((oP4_SUBMIT_OPTIONS)i)))
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
		if (!strcmp(_StrSource, oAsString((oP4_LINE_END)i)))
		{
			*_pValue = (oP4_LINE_END)i;
			return true;
		}
	}
	return false;
}

bool oP4GetClientSpecString(char* _P4ClientSpecString, size_t _SizeofP4ClientSpecString)
{
	return oP4Execute("p4 client -o", "# A Perforce Client", _P4ClientSpecString, _SizeofP4ClientSpecString);
}

bool oP4Open(oP4_OPEN_TYPE _Type, const char* _Path)
{
	oStringXL cmdline, validstring, response;
	sprintf_s(cmdline, "p4 %s \"%s\"", oAsString(_Type), oSAFESTR(_Path));
	sprintf_s(validstring, " - opened for %s", oAsString(_Type));
	return oP4Execute(cmdline, validstring, response);
}

bool oP4Revert(const char* _Path)
{
	oStringXL cmdline, response;
	sprintf_s(cmdline, "p4 revert \"%s\"", _Path);
	return oP4Execute(cmdline, ", reverted", response);
}

size_t oP4ListOpened(oP4_FILE_DESC* _pOpenedFiles, size_t _MaxNumOpenedFiles)
{
	oStringXL cmdline;
	std::vector<char> response;
	response.resize(oKB(100));
	sprintf_s(cmdline, "p4 opened -m %u", _MaxNumOpenedFiles);
	if (!oP4Execute(cmdline, nullptr, oGetData(response), response.size()))
		return oInvalid; // pass through error

	if (!strncmp("File(s) not", oGetData(response), 11))
	{
		oErrorSetLast(oERROR_END_OF_FILE, "%s", oGetData(response));
		return 0;
	}

	char* c = oGetData(response);
	char* end = c + strlen(c);

	size_t i = 0;
	while (c < end)
	{
		char* hash = c + strcspn(c, "#");
		char* dash = hash + strcspn(hash, "-");

		*hash = 0;
		_pOpenedFiles[i].Path = c;

		*dash = 0;
		_pOpenedFiles[i].Revision = atoi(hash+1);

		char* attribs = dash+1;
		c = attribs + strcspn(attribs, oNEWLINE);
		*c = 0;

		char* ctx = nullptr;
		char* tok = oStrTok(attribs, " ", &ctx);
		oFromString(&_pOpenedFiles[i].OpenType, tok);

		tok = oStrTok(nullptr, " ", &ctx);
		if (!strcmp("default", tok))
			_pOpenedFiles[i].Changelist = oInvalid;

		tok = oStrTok(nullptr, " ", &ctx);
		if (strcmp("change", tok))
			_pOpenedFiles[i].Changelist = atoi(tok);

		tok = oStrTok(nullptr, " ", &ctx);
		_pOpenedFiles[i].IsText = !strcmp("(text)", tok);

		oStrTokClose(&ctx);

		c = c + 1 + strspn(c + 1, oNEWLINE);
		i++;
		if (i >= _MaxNumOpenedFiles)
			break;
	}

	return i;
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

bool oP4ParseClientSpec(oP4_CLIENT_SPEC* _pClientSpec, const char* _P4ClientSpecString)
{
	// Determine if a given string is the next non-whitespace text in a string.
	#define NEXT_STR_EXISTS(str1, str2, bExists) str1 += strspn(str1, oWHITESPACE); bExists = ((str1 - strstr(str1, str2)) == 0) ? true : false

	// move past commented text
	const char* c = _P4ClientSpecString;
	c = strstr(c, "views and options.");
	if (!c) return false;
	c += strlen("views and options.");
	bool bNextStrExists;

	//Client
	NEXT_STR_EXISTS(c, "Client:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, _pClientSpec->Client, ':', oNEWLINE, c, &c))
		return false;

	char tmp[256];
	//Update
	NEXT_STR_EXISTS(c, "Update:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pClientSpec->LastUpdated = oP4ParseTime(tmp);

	//Access
	NEXT_STR_EXISTS(c, "Access:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pClientSpec->LastAccessed = oP4ParseTime(tmp);

	//Owner
	NEXT_STR_EXISTS(c,"Owner:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, _pClientSpec->Owner, ':', oNEWLINE, c, &c))
		return false;

	//Host
	NEXT_STR_EXISTS(c, "Host:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, _pClientSpec->Host, ':', oNEWLINE, c, &c))
		return false;

	// Description is multi-line...
	c += strspn(c, oWHITESPACE);
	c += strlen("Description:");
	c += strspn(c, oWHITESPACE);
	const char* end = strstr(c, oNEWLINE oNEWLINE "Root:");

	size_t descLen = end - c;
	memcpy_s(_pClientSpec->Description, oCOUNTOF(_pClientSpec->Description)-1, c, descLen);
	_pClientSpec->Description[__min(oCOUNTOF(_pClientSpec->Description)-1, descLen)] = 0;

	//Root
	NEXT_STR_EXISTS(end, "Root:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, _pClientSpec->Root, ':', oNEWLINE, end, &c))
		return false;

	//Options
	NEXT_STR_EXISTS(c, "Options:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;

	_pClientSpec->Allwrite = !strstr(tmp, "noallwrite");
	_pClientSpec->Clobber = !strstr(tmp, "noclobber");
	_pClientSpec->Compress = !strstr(tmp, "nocompress");
	_pClientSpec->Locked = !strstr(tmp, "unlocked");
	_pClientSpec->Modtime = !strstr(tmp, "nomodtime");
	_pClientSpec->Rmdir = !strstr(tmp, "normdir");

	//SubmitOptions
	NEXT_STR_EXISTS(c, "SubmitOptions:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	if (!oFromString(&_pClientSpec->SubmitOptions, tmp))
		return false;

	//LineEnd
	NEXT_STR_EXISTS(c, "LineEnd:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	if (!oFromString(&_pClientSpec->LineEnd, tmp))
		return false;

	// View is multi-line...
	c += strspn(c, oWHITESPACE);
	c += strlen("View:");
	strcpy_s(_pClientSpec->View, c);

	return true;
	#undef NEXT_STR_EXISTS
}
