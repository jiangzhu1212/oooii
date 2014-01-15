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
// This cpp contains implemenations of to_string and from_string for intrinsic
// types as well as ouro types.

#include <oBase/stringize.h>
#include <oBase/atof.h>
#include <oBase/fourcc.h>
#include <oBase/guid.h>
#include <oBase/string.h>
#include <oBase/date.h>
#include <half.h>

oDEFINE_WHITESPACE_PARSING();
namespace ouro {

const char* as_string(const bool& _Value) { return _Value ? "true" : "false"; }

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const char* const & _Value)
{
	return strlcpy(_StrDestination, oSAFESTRN(_Value), _SizeofStrDestination) < _SizeofStrDestination ? _StrDestination : nullptr;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const char& _Value)
{
	if (_SizeofStrDestination < 2) return nullptr;
	_StrDestination[0] = _Value;
	_StrDestination[1] = 0;
	return _StrDestination;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const unsigned char& _Value)
{
	if (_SizeofStrDestination < 2) return nullptr;
	_StrDestination[0] = *(signed char*)&_Value;
	_StrDestination[1] = 0;
	return _StrDestination;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const bool& _Value) { return strlcpy(_StrDestination, _Value ? "true" : "false", _SizeofStrDestination) < _SizeofStrDestination ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const short & _Value) { return 0 == _itoa_s(_Value, _StrDestination, _SizeofStrDestination, 10) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const unsigned short& _Value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%hu", _Value) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const int& _Value) { return 0 == _itoa_s(_Value, _StrDestination, _SizeofStrDestination, 10) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const unsigned int& _Value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%u", _Value) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const long& _Value) { return 0 == _itoa_s(_Value, _StrDestination, _SizeofStrDestination, 10) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const unsigned long& _Value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%u", _Value) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const long long& _Value) { return 0 == _i64toa_s(_Value, _StrDestination, _SizeofStrDestination, 10) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const unsigned long long& _Value) { return 0 == _ui64toa_s(unsigned long long(_Value), _StrDestination, _SizeofStrDestination, 10) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const float& _Value) { if (-1 != snprintf(_StrDestination, _SizeofStrDestination, "%f", _Value)) { trim_right(_StrDestination, _SizeofStrDestination, _StrDestination, "0"); return _StrDestination; } return nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const double& _Value) { if (-1 != snprintf(_StrDestination, _SizeofStrDestination, "%lf", _Value)) { trim_right(_StrDestination, _SizeofStrDestination, _StrDestination, "0"); return _StrDestination; } return nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const half& _Value) { if (to_string(_StrDestination, _SizeofStrDestination, static_cast<float>(_Value))) { trim_right(_StrDestination, _SizeofStrDestination, _StrDestination, "0"); return _StrDestination; } return nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const std::string& _Value) { return strlcpy(_StrDestination, _Value.c_str(), _SizeofStrDestination) < _SizeofStrDestination ? _StrDestination : nullptr; }

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const fourcc& _Value)
{
	if (_SizeofStrDestination < 5) return nullptr;
	unsigned int fcc = from_big_endian((unsigned int)_Value);
	memcpy(_StrDestination, &fcc, sizeof(unsigned int));
	_StrDestination[4] = 0;
	return _StrDestination;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const guid& _Value)
{
	if (_SizeofStrDestination <= 38) return nullptr;
	return -1 != snprintf(_StrDestination, _SizeofStrDestination, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", _Value.Data1, _Value.Data2, _Value.Data3, _Value.Data4[0], _Value.Data4[1], _Value.Data4[2], _Value.Data4[3], _Value.Data4[4], _Value.Data4[5], _Value.Data4[6], _Value.Data4[7]) ? _StrDestination : nullptr;
}

#define oCHK do { if (!_pValue || !_StrSource) return false; } while(false)
bool from_string(bool* _pValue, const char* _StrSource)
{
	oCHK;
	if (!_stricmp("true", _StrSource) || !_stricmp("t", _StrSource) || !_stricmp("yes", _StrSource) || !_stricmp("y", _StrSource))
		*_pValue = true;
	else 
		*_pValue = atoi(_StrSource) != 0;
	return true;
}

bool from_string(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource)
{
	return strlcpy(_StrDestination, _StrSource, _SizeofStrDestination) < _SizeofStrDestination;
}

bool from_string(char** _pValue, const char* _StrSource) { oCHK; return strlcpy(*_pValue, _StrSource, SIZE_MAX) < SIZE_MAX; }
bool from_string(char* _pValue, const char* _StrSource) { oCHK; *_pValue = *_StrSource; return true; }
bool from_string(unsigned char* _pValue, const char* _StrSource) { oCHK; *_pValue = *(const unsigned char*)_StrSource; return true; }
template<typename T> inline bool _FromString(T* _pValue, const char* _Format, const char* _StrSource) { oCHK; return 1 == sscanf_s(_StrSource, _Format, _pValue); }
bool from_string(short* _pValue, const char* _StrSource) { return _FromString(_pValue, "%hd", _StrSource); }
bool from_string(unsigned short* _pValue, const char* _StrSource) { return _FromString(_pValue, "%hu", _StrSource); }
bool from_string(int* _pValue, const char* _StrSource) { return _FromString(_pValue, "%d", _StrSource); }
bool from_string(unsigned int* _pValue, const char* _StrSource) { return _FromString(_pValue, "%u", _StrSource); }
bool from_string(long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%d", _StrSource); }
bool from_string(unsigned long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%u", _StrSource); }
bool from_string(long long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%lld", _StrSource); }
bool from_string(unsigned long long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%llu", _StrSource); }
bool from_string(float* _pValue, const char* _StrSource) { oCHK; return atof(_StrSource, _pValue); }
bool from_string(double* _pValue, const char* _StrSource) { return _FromString(_pValue, "%lf", _StrSource); }
bool from_string(half* _pValue, const char* _StrSource) { float v; if (!from_string(&v, _StrSource)) return false; *_pValue = v; return true; }
bool from_string(fourcc* _pValue, const char* _StrSource) { oCHK; *_pValue = fourcc(_StrSource); return true; }
bool from_string(guid* _pValue, const char* _StrSource) { oCHK; return 11 == sscanf_s(_StrSource, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", &_pValue->Data1, &_pValue->Data2, &_pValue->Data3, &_pValue->Data4[0], &_pValue->Data4[1], &_pValue->Data4[2], &_pValue->Data4[3], &_pValue->Data4[4], &_pValue->Data4[5], &_pValue->Data4[6], &_pValue->Data4[7]); }

bool from_string_float_array(float* _pValue, size_t _NumValues, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return false;
	move_past_line_whitespace(&_StrSource);
	while (_NumValues--)
	{
		if (!*_StrSource) return false;
		if (!atof(&_StrSource, _pValue++)) return false;
	}
	return true;
}

bool from_string_double_array(double* _pValue, size_t _NumValues, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return false;
	while (_NumValues--)
	{
		move_past_line_whitespace(&_StrSource);
		if (!*_StrSource) return false;
		if (1 != sscanf_s(_StrSource, "%f", _pValue)) return false;
		move_to_whitespace(&_StrSource);
	}
	return true;
}

} // namespace ouro
