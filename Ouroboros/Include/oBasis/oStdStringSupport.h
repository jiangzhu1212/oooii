// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// oStd::to_string/oStd::from_string/oStrcpy for std::string
#pragma once
#ifndef oStdStringSupport_h
#define oStdStringSupport_h

#include <string>

// Copy from wchar_t std::basic_string to char std::basic_string
inline void oStrcpy(std::string& _StrDestination, const std::wstring& _StrSource)
{
	_StrDestination.assign(std::begin(_StrSource), std::end(_StrSource));
}

// Copy from char std::basic_string to wchar_t std::basic_string
inline void oStrcpy(std::wstring& _StrDestination, const std::string& _StrSource)
{
	_StrDestination.assign(std::begin(_StrSource), std::end(_StrSource));
}

#endif
