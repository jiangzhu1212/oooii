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
#include "pch.h"
#include <oooii/oP4.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oProcess.h>
#include <oooii/oRef.h>
#include <oooii/oStddef.h>
#include <oooii/oString.h>
#include <time.h>

template<> const char* oAsString(const oP4::SUBMIT_OPTIONS& _Value)
{
	switch (_Value)
	{
		case oP4::SUBMIT_UNCHANGED: return "submitunchanged";
		case oP4::SUBMIT_UNCHANGED_REOPEN: return "submitunchanged+reopen";
		case oP4::REVERT_UNCHANGED: return "revertunchanged";
		case oP4::REVERT_UNCHANGED_REOPEN: return "revertunchanged+reopen";
		case oP4::LEAVE_UNCHANGED: return "leaveunchanged";
		case oP4::LEAVE_UNCHANGED_REOPEN: return "leaveunchanged+reopen";
		default: oASSUME(0);
	}
}

template<> const char* oAsString(const oP4::LINE_END& _Value)
{
	switch (_Value)
	{
		case oP4::LOCAL: return "local";
		case oP4::UNIX: return "unix";
		case oP4::MAC: return "mac";
		case oP4::WIN: return "win";
		case oP4::SHARE: return "share";
 		default: oASSUME(0);
	}
}

namespace oP4 {

oP4::SUBMIT_OPTIONS GetSubmitOptions(const char* _P4String)
{
	for (size_t i = 0; i <= oP4::LEAVE_UNCHANGED_REOPEN; i++)
		if (!strcmp(_P4String, oAsString((oP4::SUBMIT_OPTIONS)i)))
			return (oP4::SUBMIT_OPTIONS)i;
	return oP4::SUBMIT_UNCHANGED;
}

oP4::LINE_END GetLineEnd(const char* _P4String)
{
	for (size_t i = 0; i <= oP4::SHARE; i++)
		if (!strcmp(_P4String, oAsString((oP4::LINE_END)i)))
			return (oP4::LINE_END)i;
	return oP4::LOCAL;
}

time_t GetTime(const char* _P4TimeString)
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

bool GetClientSpec(CLIENT_SPEC* _pClientSpec, const char* _P4ClientSpecString)
{
	// move past commented text
	const char* c = _P4ClientSpecString;
	c = strstr(c, "views and options.");
	if (!c) return false;
	c += strlen("views and options.");

	if (!oGetKeyValuePair(0, 0, _pClientSpec->Client, ':', oNEWLINE, c, &c))
		return false;

	char tmp[256];
	if (!oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pClientSpec->LastUpdated = GetTime(tmp);

	if (!oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pClientSpec->LastAccessed = GetTime(tmp);

	if (!oGetKeyValuePair(0, 0, _pClientSpec->Owner, ':', oNEWLINE, c, &c))
		return false;

	if (!oGetKeyValuePair(0, 0, _pClientSpec->Host, ':', oNEWLINE, c, &c))
		return false;

	// Description is multi-line...
	c += strspn(c, oWHITESPACE);
	c += strlen("Description:");
	c += strspn(c, oWHITESPACE);
	const char* end = strstr(c, oNEWLINE oNEWLINE "Root:");

	size_t descLen = end - c;
	memcpy_s(_pClientSpec->Description, oCOUNTOF(_pClientSpec->Description)-1, c, descLen);
	_pClientSpec->Description[__min(oCOUNTOF(_pClientSpec->Description)-1, descLen)] = 0;

	if (!oGetKeyValuePair(0, 0, _pClientSpec->Root, ':', oNEWLINE, end, &c))
		return false;

	if (!oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;

	_pClientSpec->Allwrite = !strstr(tmp, "noallwrite");
	_pClientSpec->Clobber = !strstr(tmp, "noclobber");
	_pClientSpec->Compress = !strstr(tmp, "nocompress");
	_pClientSpec->Locked = !strstr(tmp, "unlocked");
	_pClientSpec->Modtime = !strstr(tmp, "nomodtime");
	_pClientSpec->Rmdir = !strstr(tmp, "normdir");

	if (!oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pClientSpec->SubmitOptions = GetSubmitOptions(tmp);

	if (!oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pClientSpec->LineEnd = GetLineEnd(tmp);

	// View is multi-line...
	c += strspn(c, oWHITESPACE);
	c += strlen("View:");
	strcpy_s(_pClientSpec->View, c);

	return true;
}

static bool Execute(const char* _CommandLine, const char* _CheckValidString, char* _P4ResponseString, size_t _SizeofP4ResponseString)
{
	oProcess::DESC desc;
	desc.CommandLine = _CommandLine;
	desc.EnvironmentString = 0;
	desc.StdHandleBufferSize = 4096;
	threadsafe oRef<oProcess> process;
	if (!oProcess::Create(&desc, &process))
		return false;

	const unsigned int TIMEOUT = 20000;

	process->Start();
	if (!process->Wait(TIMEOUT))
		oASSERT(false, "Executing \"%s\" timed out after %.01f seconds.", _CommandLine, static_cast<float>(TIMEOUT) / 1000.0f);

	size_t sizeRead = process->ReadFromStdout(_P4ResponseString, _SizeofP4ResponseString);
	oASSERT(sizeRead < _SizeofP4ResponseString, "");
	_P4ResponseString[sizeRead] = 0;

	if (!strstr(_P4ResponseString, _CheckValidString))
	{
		oSetLastError(ENXIO, _P4ResponseString);
		return false;
	}

	return true;
}

template<size_t size> inline bool Execute(const char* _CommandLine, const char* _CheckValidString, char (&_P4ResponseString)[size]) { return Execute(_CommandLine, _CheckValidString, _P4ResponseString, size); }

bool GetClientSpecString(char* _P4ClientSpecString, size_t _SizeofP4ClientSpecString)
{
	return Execute("p4 client -o", "# A Perforce Client", _P4ClientSpecString, _SizeofP4ClientSpecString);
}

bool MarkForEdit(const char* _Path)
{
	char cmdline[1024];
	char response[1024];
	sprintf_s(cmdline, "p4 edit \"%s\"", _Path);
	return Execute(cmdline, " - opened for edit", response);
}

bool MarkForAdd(const char* _Path)
{
	char cmdline[1024];
	char response[1024];
	sprintf_s(cmdline, "p4 add \"%s\"", _Path);
	return Execute(cmdline, " - opened for add", response);
}

bool MarkForDelete(const char* _Path)
{
	char cmdline[1024];
	char response[1024];
	sprintf_s(cmdline, "p4 delete \"%s\"", _Path);
	return Execute(cmdline, " - opened for delete", response);
}

bool Revert(const char* _Path)
{
	char cmdline[1024];
	char response[1024];
	sprintf_s(cmdline, "p4 revert \"%s\"", _Path);
	return Execute(cmdline, ", reverted", response);
}

} // namespace oP4
