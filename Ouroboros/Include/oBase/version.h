/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#ifndef oBase_version_h
#define oBase_version_h

#include <oBase/fixed_string.h>
#include <oBase/operators.h>

namespace ouro {

struct version : oComparable<version>
{
	version() : major(0), minor(0), build(0), revision(0) {}

	/*constexpr*/ version(unsigned short _Major, unsigned short _Minor) : major(_Major), minor(_Minor), build(0), revision(0) {}
	/*constexpr*/ version(unsigned short _Major, unsigned short _Minor, unsigned short _Build, unsigned short _Revision) : major(_Major), minor(_Minor), build(_Build), revision(_Revision) {}

	// _SCCRevision is split in a semi-user-readable way acress build and revision.
	/*constexpr*/ version(unsigned short _Major, unsigned short _Minor, unsigned int _SCCRevision)
		: major(_Major)
		, minor(_Minor)
		, build(static_cast<unsigned short>(_SCCRevision / 10000))
		, revision(static_cast<unsigned short>(_SCCRevision % 10000))
	{}

	inline bool valid() const { return major || minor || build || revision; }

	// Converts a version built with an SCCRevision back to that SCCRevision
	inline unsigned int scc_revision() const { return build * 10000 + revision; }

	inline bool operator<(const version& _That) const { return valid() && _That.valid() && ((major < _That.major) || (major == _That.major && minor < _That.minor) || (major == _That.major && minor == _That.minor && build < _That.build) || (major == _That.major && minor == _That.minor && build == _That.build) && revision < _That.revision); }
	inline bool operator==(const version& _That) const { return valid() && _That.valid() && major == _That.major && minor == _That.minor && build == _That.build && revision == _That.revision; }
	
	unsigned short major;
	unsigned short minor;
	unsigned short build;
	unsigned short revision;
};

// 4 part maj.min.build.rev (default to|from_string does all 4 parts)
char* to_string4(char* _StrDestination, size_t _SizeofStrDestination, const ouro::version& _Version);
template<size_t size> char* to_string4(char (&_StrDestination)[size], const version& _Version) { return to_string4(_StrDestination, size, _Version); }
template<size_t capacity> char* to_string4(fixed_string<char, capacity>& _StrDestination, const version& _Version) { return to_string4(_StrDestination, _StrDestination.capacity(), _Version); }
bool from_string4(version* _pType, const char* _StrSource);

// 3 part maj.min.rev (build is ignored)
char* to_string3(char* _StrDestination, size_t _SizeofStrDestination, const version& _Version);
template<size_t size> char* to_string3(char (&_StrDestination)[size], const version& _Version) { return to_string3(_StrDestination, size, _Version); }
template<size_t capacity> char* to_string3(fixed_string<char, capacity>& _StrDestination, const version& _Version) { return to_string3(_StrDestination, _StrDestination.capacity(), _Version); }
bool from_string3(version* _pType, const char* _StrSource);

// 2 part maj.min (build and revision are ignored)
char* to_string2(char* _StrDestination, size_t _SizeofStrDestination, const ouro::version& _Version);
template<size_t size> char* to_string2(char (&_StrDestination)[size], const version& _Version) { return to_string2(_StrDestination, size, _Version); }
template<size_t capacity> char* to_string2(fixed_string<char, capacity>& _StrDestination, const version& _Version) { return to_string2(_StrDestination, _StrDestination.capacity(), _Version); }
bool from_string2(version* _pType, const char* _StrSource);

}

#endif
