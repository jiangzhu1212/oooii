// $(header)
#include <oBasis/oFixedString.h>
#include <oBasis/oPlatformFeatures.h>
#include <string.h>
#include <wchar.h>

template<typename T> T SS(T _String) { return _String ? _String : T(""); }

void oFixedStringCopy(char* _StrDestination, size_t _NumDestinationChars, const char* _StrSource)
{
	strcpy_s(_StrDestination, _NumDestinationChars, SS(_StrSource));
}

void oFixedStringCopy(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _StrSource)
{
	wcscpy_s(_StrDestination, _NumDestinationChars, SS(_StrSource));
}

void oFixedStringCopy(char* _StrDestination, size_t _NumDestinationChars, const wchar_t* _StrSource)
{
	size_t sz = 0;
	const wchar_t* src = SS(_StrSource);
	wcsrtombs_s(&sz, _StrDestination, _NumDestinationChars, &src, _NumDestinationChars, nullptr);
}

void oFixedStringCopy(wchar_t* _StrDestination, size_t _NumDestinationChars, const char* _StrSource)
{
	size_t sz = 0;
	const char* src = SS(_StrSource);
	mbsrtowcs_s(&sz, _StrDestination, _NumDestinationChars, &src, _NumDestinationChars, nullptr);
}

size_t oFixedStringLength(const char* _StrSource)
{
	return strlen(SS(_StrSource));
}

size_t oFixedStringLength(const wchar_t* _StrSource)
{
	return wcslen(SS(_StrSource));
}
