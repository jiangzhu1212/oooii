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
// Abstraction for source code control.
#pragma once
#ifndef oStd_scc_h
#define oStd_scc_h

#include <oStd/date.h>
#include <oStd/fixed_string.h>
#include <oStd/function.h>

namespace oStd {

namespace scc_protocol
{	enum value {

	perforce,
	svn,
	git,

};}

namespace scc_status
{	enum value {

	unknown,
	unchanged,
	unversioned,
	ignored,
	modified,
	missing,
	added,
	removed,
	replaced,
	copied,
	conflict,
	merged,
	obstructed,
	out_of_date,

};}

struct scc_file
{
	scc_file()
		: status(scc_status::unknown)
		, revision(~0u)
		, text(true)
	{}

	path_string path;
	scc_status::value status;
	unsigned int revision;
	bool text;
};

struct scc_revision
{
	scc_revision()
		: revision(~0u)
	{}

	unsigned int revision;
	mstring who;
	ntp_date when;
	xlstring what;
};

typedef oFUNCTION<bool(const char* _Commandline
	, char* _StrStdout
	, size_t _SizeofStrStdOut
	, int* _pExitCode
	, unsigned int _TimeoutMS)> scc_spawn;

class scc
{
public:
	// Returns the current protocol in use by this instances
	virtual scc_protocol::value protocol() const = 0;

	// Returns true if the client executable for the specified protocol can be 
	// executed by the scc_spawn function.
	virtual bool available() const = 0;

	// Returns the root of the current tree
	virtual char* root(const char* _Path, char* _StrDestination, size_t _SizeofStrDestination) const = 0;
	template<size_t size> char* root(const char* _Path, char (&_StrDestination)[size]) const { return root(_Path, _StrDestination, size); }
	template<size_t capacity> char* root(const char* _Path, oStd::fixed_string<char, capacity>& _StrDestination) const { return root(_Path, _StrDestination, _StrDestination.capacity()); }

	// Returns the basic version that the path would be at if there were no 
	// modified files and all were up-to-date. If there is a failure this returns 
	// 0 (zero).
	virtual unsigned int revision(const char* _Path) const = 0;

	// Returns all files that are not at the latest or specifed revision up to the
	// max number of entries specified. This will return the actual number of 
	// entries populated. For checkout-style paradigms such as perforce, this will
	// also list opened files
	virtual size_t modifications(const char* _Path, unsigned int _UpToRevision, scc_file* _pFiles, size_t _MaxNumFiles) const = 0;
	template<size_t size> size_t modifications(const char* _Path, unsigned int _UpToRevision, scc_file (&_pFiles)[size]) const { return modifications(_Path, _UpToRevision, _pFiles, size); }

	// Returns information about the specified change. If 0, this will return the
	// latest/head revision.
	virtual scc_revision change(const char* _Path, unsigned int _Revision) const = 0;

	// Sync the specified path to the specified revision or date. Revision 0 will 
	// sync to the head revision.
	virtual void sync(const char* _Path, unsigned int _Revision, bool _Force = false) = 0;
	virtual void sync(const char* _Path, const ntp_date& _Date, bool _Force = false) = 0;

	virtual void add(const char* _Path) = 0;

	virtual void remove(const char* _Path, bool _Force = false) = 0;
	
	// for checkout-style scc like perforce. This noops on sandbox-style scc.
	virtual void edit(const char* _Path) = 0;

	virtual void revert(const char* _Path) = 0;
};

std::shared_ptr<scc> make_scc(scc_protocol::value _Protocol, const scc_spawn& _Spawn);

} // namespace oStd

#endif
