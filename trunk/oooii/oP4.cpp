// $(header)
#include <oooii/oP4.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oRef.h>
#include <oooii/oStddef.h>
#include <oooii/oStdio.h>
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

//Determine if a given string is the next non-whitespace text in a string.
#define NEXT_STR_EXISTS(str1, str2, bExists) str1 += strspn(str1, oWHITESPACE); bExists = ((str1 - strstr(str1, str2)) == 0) ? true : false

bool GetClientSpec(CLIENT_SPEC* _pClientSpec, const char* _P4ClientSpecString)
{
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
	_pClientSpec->LastUpdated = GetTime(tmp);

	//Access
	NEXT_STR_EXISTS(c, "Access:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
		return false;
	_pClientSpec->LastAccessed = GetTime(tmp);

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
	_pClientSpec->SubmitOptions = GetSubmitOptions(tmp);

	//LineEnd
	NEXT_STR_EXISTS(c, "LineEnd:", bNextStrExists);
	if (bNextStrExists && !oGetKeyValuePair(0, 0, tmp, ':', oNEWLINE, c, &c))
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
	if (!oExecute(_CommandLine, _P4ResponseString, _SizeofP4ResponseString, 0, 10000))
		return false;

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
