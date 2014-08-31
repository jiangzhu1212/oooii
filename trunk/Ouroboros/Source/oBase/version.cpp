// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/version.h>
#include <oString/string.h>

#define CHK(x) do { _pType->x = static_cast<unsigned short>(x); if (_pType->x != x) return false; } while(false)

namespace ouro {

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const version& _Version)
{
	return to_string4(_StrDestination, _SizeofStrDestination, _Version);
}

bool from_string(version* _pType, const char* _StrSource)
{
	return from_string4(_pType, _StrSource);
}

char* to_string4(char* _StrDestination, size_t _SizeofStrDestination, const version& _Version)
{
	return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%u.%u.%u.%u", _Version.major, _Version.minor, _Version.build, _Version.revision) ? _StrDestination : nullptr;
}

bool from_string4(version* _pType, const char* _StrSource)
{
	unsigned int major=0, minor=0, build=0, revision=0;
	int nScanned = sscanf_s(_StrSource, "%u.%u.%u.%u", &major, &minor, &build, &revision);
	if (nScanned <= 0) return false;
	CHK(major); CHK(minor); CHK(build); CHK(revision);
	return true;
}

bool from_string3(version* _pType, const char* _StrSource)
{
	_pType->build = 0;
	unsigned int major=0, minor=0, revision=0;
	int nScanned = sscanf_s(_StrSource, "%u.%u.%u", &major, &minor, &revision);
	if (nScanned <= 0) return false;
	CHK(major); CHK(minor); CHK(revision);
	return true;
}

char* to_string3(char* _StrDestination, size_t _SizeofStrDestination, const version& _Version)
{
	return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%u.%u.%u", _Version.major, _Version.minor, _Version.revision) ? _StrDestination : nullptr;
}

char* to_string2(char* _StrDestination, size_t _SizeofStrDestination, const ouro::version& _Version)
{
	return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%u.%u", _Version.major, _Version.minor) ? _StrDestination : nullptr;
}

bool from_string2(version* _pType, const char* _StrSource)
{
	_pType->build = 0;
	_pType->revision = 0;
	unsigned int major=0, minor=0;
	int nScanned = sscanf_s(_StrSource, "%u.%u", &major, &minor);
	if (nScanned <= 0) return false;
	CHK(major); CHK(minor);
	return true;
}

}
