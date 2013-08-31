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
#include "scc_svn.h"
#include <oStd/assert.h>
#include <oStd/macros.h>
#include <oStd/stringize.h>
#include <oStd/throw.h>

namespace oStd {
	namespace detail {

#define SVN_CMDf(_Format, ...) \
	lstring cmd; \
	if (-1 == snprintf(cmd, _Format, ## __VA_ARGS__)) \
		throw scc_exception(scc_error::command_string_too_long)

#define SVNf(_Format, ...) \
	SVN_CMDf(_Format, ## __VA_ARGS__); \
	xlstring StdOut; \
	spawn(cmd, StdOut)

#define SVN_THROWE(_ExitCode) throw scc_exception(scc_error::scc_exe_error, oStd::formatf("svn exited with code %d", _ExitCode))
#define SVN_THROWO(_ExitCode, _StdOut) throw scc_exception(scc_error::scc_exe_error, oStd::formatf("svn exited with code %d\n  stdout: %s", _ExitCode, _StdOut.c_str()))
#define SVN_THROWF(_SCCError, _Format, ...) throw scc_exception(scc_error::_SCCError, oStd::vformatf(_Format, ## __VA_ARGS__ ))

std::shared_ptr<scc> make_scc_svn(const scc_spawn& _Spawn)
{
	return std::move(std::make_shared<scc_svn>(_Spawn));
}

static bool svn_is_error(const char* _StdOut)
{
	const char* sErrors[] =
	{
		"not recognized",
		"not a working copy",
		"was not found",
		"Could not display info",
		"has no committed revision",
		"does not exist",
		"Error resolving",
		"E731001",
	};

	for(size_t i = 0; i < oCOUNTOF(sErrors); i++)
		if (strstr(_StdOut, sErrors[i]))
			return true;
	return false;
}

void scc_svn::spawn(const char* _Command, const oFUNCTION<void(char* _Line)>& _GetLine) const
{
	static const unsigned int kTimeout = 5000;
	int ec = 0;
	if (!Spawn(_Command, _GetLine, &ec, kTimeout))
		SVN_THROWE(ec);
}

void scc_svn::spawn(const char* _Command, oStd::xlstring& _StdOut) const
{
	static const unsigned int kTimeout = 5000;
	int ec = 0;
	_StdOut.clear();
	oFUNCTION<void(char* _Line)> GetLine = [&](char* _Line)
	{
		strlcat(_StdOut, _Line, _StdOut.capacity());
		strlcat(_StdOut, "\n", _StdOut.capacity());
	};

	if (!Spawn(_Command, GetLine, &ec, kTimeout) || svn_is_error(_StdOut))
		SVN_THROWO(ec, _StdOut);
}

bool scc_svn::available() const
{
	int ec = 0;
	oStd::xlstring StdOut;
	oFUNCTION<void(char* _Line)> GetLine = [&](char* _Line) { strlcat(StdOut, _Line, StdOut.capacity()); strlcat(StdOut, "\n", StdOut.capacity()); };
	if (!Spawn("svn", GetLine, &ec, 1000))
		SVN_THROWE(ec);
	return !!strstr(StdOut, "Type 'svn help'");
}

char* scc_svn::root(const char* _Path, char* _StrDestination, size_t _SizeofStrDestination) const
{
	SVNf("svn info \"%s\"", _Path);

	static const size_t PathKeyLen = 24;
	char* path = strstr(StdOut, "Working Copy Root Path: ");
	if (!path)
		SVN_THROWF(entry_not_found, "%s", "no working copy root path entry found");

	path += PathKeyLen;
	size_t len = strcspn(path, oNEWLINE); // move to end of line

	if (len >= _SizeofStrDestination)
		throw std::invalid_argument("destination buffer to receive scc root is too small");

	memcpy(_StrDestination, path, len);
	_StrDestination[len] = 0;

	if (!clean_path(_StrDestination, _SizeofStrDestination, _StrDestination))
		throw std::invalid_argument("destination buffer to receive scc root is too small");
	return _StrDestination;
}

unsigned int scc_svn::revision(const char* _Path) const
{
	unsigned int r = 0;

	SVNf("svn info \"%s\"", _Path);
	static const size_t RevKeyLen = 10;
	char* rev = strstr(StdOut, "Revision: ");
	if (!rev)
		return 0;

	rev += RevKeyLen;
	size_t len = strcspn(rev, oNEWLINE); // move to end of line
	
	oStd::sstring s;
	if (len >= s.capacity())
		return r;

	memcpy(s, rev, len);
	s[len] = 0;

	oStd::from_string(&r, s);

	return r;
}

static scc_status::value get_status(char _Status)
{
	switch (_Status)
	{
		case ' ': return scc_status::unchanged;
		case 'A': return scc_status::added;
		case 'C': return scc_status::conflict;
		case 'D': return scc_status::removed;
		case 'I': return scc_status::ignored;
		case 'M': return scc_status::modified;
		case 'R': return scc_status::replaced;
		case 'X': return scc_status::unversioned;
		case '?': return scc_status::unversioned;
		case '!': return scc_status::missing;
		case '~': return scc_status::obstructed;
		default: break;
	}
	return scc_status::unknown;
}

// returns new beginning into the status buffer. The contents of _StatusBuffer
// are altered by this call.
static char* svn_parse_status_line(char* _StatusBuffer, unsigned int _UpToRevision, scc_file& _File)
{
	if (!*_StatusBuffer) return nullptr;
	if (!strncmp(_StatusBuffer, "Status against", 14)) return nullptr;
	_File.status = get_status(_StatusBuffer[0]);
	oASSERT(_StatusBuffer[1] == ' ', "haven't thought about this yet");
	if (_StatusBuffer[8] == '*') _File.status = scc_status::out_of_date;
	_StatusBuffer += 9;
	_StatusBuffer += strspn(_StatusBuffer, oWHITESPACE);
	char* rev = _StatusBuffer;
	_StatusBuffer += strcspn(_StatusBuffer, oWHITESPACE);
	*_StatusBuffer++ = 0;
	_File.revision = 0;
	oStd::from_string(&_File.revision, rev);
	if (_UpToRevision != 0 && _File.revision > _UpToRevision) _File.status = scc_status::out_of_date;
	_StatusBuffer += strspn(_StatusBuffer, oWHITESPACE);
	char* p = _StatusBuffer;
	_StatusBuffer += strcspn(_StatusBuffer, oNEWLINE);
	*_StatusBuffer++ = 0;
	clean_path(_File.path, p);
	_StatusBuffer += strspn(_StatusBuffer, oWHITESPACE oNEWLINE);
	if (!*_StatusBuffer) return nullptr;
	return _StatusBuffer;
}

void scc_svn::status(const char* _Path, unsigned int _UpToRevision, scc_visit_option::value _Option, const scc_file_visitor& _Visitor) const
{
	oStd::lstring cmd;
	
	const char* opt = "";
	switch (_Option)
	{
		case scc_visit_option::visit_all: opt = " -v"; break;
		case scc_visit_option::skip_unversioned: opt = " -v -q"; break;
		case scc_visit_option::skip_unmodified: opt = ""; break;
		case scc_visit_option::modified_only: opt = " -q"; break;
		default: break;
	}

	if (-1 == snprintf(cmd, "svn status -u%s \"%s\"", opt, _Path))
		throw scc_exception(scc_error::command_string_too_long);

	oStd::xlstring line;
	scc_error::value errc = scc_error::none;
	oStd::mstring errline;
	spawn(cmd, [&](char* _Line)
	{
		if (errc == scc_error::none)
		{
			if (svn_is_error(_Line))
			{
				errc = scc_error::scc_exe_error;
				errline = _Line;
			}
			else
			{
				scc_file file;
				svn_parse_status_line(_Line, _UpToRevision, file);
				_Visitor(file);
			}
		}
	});

	if (errc != scc_error::none)
		throw scc_exception(errc, errline);
}

static bool svn_from_string(ntp_date* _pDate, const char* _String)
{
	int timezone = 0;
	date d;
	if (7 != sscanf_s(_String, "%d-%d-%d %d:%d:%d %d", &d.year, &d.month, &d.day, &d.hour, &d.minute, &d.second, &timezone))
		return false;
	timezone = (timezone / 100) * 60 * 60;

	*_pDate = date_cast<ntp_date>(d);
	_pDate->DataMS -= timezone;
	return true;
}

static char* svn_to_string(char* _StrDestination, size_t _SizeofStrDestination, const ntp_date& _Date)
{
	return (strftime(_StrDestination, _SizeofStrDestination, oStd::sortable_date_format, _Date) < _SizeofStrDestination)
		? _StrDestination : nullptr;
}
template<size_t size> char* svn_to_string(char (&_StrDestination)[size], const ntp_date& _Date) { return svn_to_string(_StrDestination, size, _Date); }
template<size_t capacity> char* svn_to_string(oStd::fixed_string<char, capacity>& _StrDestination, const ntp_date& _Date) { return svn_to_string(_StrDestination, _StrDestination.capacity(), _Date); }

scc_revision scc_svn::change(const char* _Path, unsigned int _Revision) const
{
	oStd::lstring cmd;
	if (_Revision)
	{
		if (-1 == snprintf(cmd, "svn log -r %d \"%s\"", _Revision, _Path))
			throw scc_exception(scc_error::command_string_too_long);
	}

	else if (-1 == snprintf(cmd, "svn log \"%s\"", _Path))
		throw scc_exception(scc_error::command_string_too_long);

	oStd::xlstring StdOut;
	spawn(cmd, StdOut);

	char* rev = strchr(StdOut, 'r');
	if (!rev)
		throw scc_exception(scc_error::entry_not_found);
	rev++;
	char* strWho = strchr(rev, '|');
	*(strWho-1) = 0;
	strWho += 2;
	char* strWhen = strchr(strWho, '|');
	*(strWhen-1) = 0;
	strWhen += 2;
	char* strWhat = strchr(strWhen, '(');
	*(strWhat-1) = 0;
	strWhat += strcspn(strWhat, oNEWLINE);
	strWhat += strspn(strWhat, oNEWLINE oWHITESPACE);
	char* strEnd = strWhat + strcspn(strWhat, oNEWLINE);
	*strEnd = 0;

	scc_revision r;
	oStd::from_string(&r.revision, rev);
	r.who = strWho;
	svn_from_string(&r.when, strWhen);
	r.what = strWhat;

	return r;
}

void scc_svn::sync(const char* _Path, unsigned int _Revision, bool _Force)
{
	oStd::sstring StrForce(_Force ? " --force" : "");
	oStd::lstring cmd;
	if (_Revision)
	{
		if (-1 == snprintf(cmd, "svn update%s -r %d \"%s\"", StrForce.c_str(), _Revision, _Path))
			throw scc_exception(scc_error::command_string_too_long);
	}

	else if (-1 == snprintf(cmd, "svn update \"%s\"", _Path))
		throw scc_exception(scc_error::command_string_too_long);

	oStd::xlstring StdOut;
	spawn(cmd, StdOut);
}

void scc_svn::sync(const char* _Path, const ntp_date& _Date, bool _Force)
{
	const char* StrForce = _Force ? " --force" : "";
	oStd::sstring StrDate;
	oStd::lstring cmd;
	if (-1 == snprintf(cmd, "svn update%s -r {%d} \"%s\"", StrForce, svn_to_string(StrDate, _Date), _Path))
		throw scc_exception(scc_error::command_string_too_long);
	oStd::xlstring StdOut;
	spawn(cmd, StdOut);
}

void scc_svn::add(const char* _Path)
{
	SVNf("svn add \"%s\"", _Path);
}

void scc_svn::remove(const char* _Path, bool _Force)
{
	SVNf("svn delete \"%s\"", _Path);
}

void scc_svn::edit(const char* _Path)
{
	// noop (svn is sandbox, always-writable)
}

void scc_svn::revert(const char* _Path)
{
	SVNf("snv revert \"%s\"", _Path);
}

	} // namespace detail
} // namespace oStd
