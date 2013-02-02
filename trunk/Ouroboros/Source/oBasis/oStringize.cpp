/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasis/oStringize.h>
#include <oBasis/oAtof.h>
#include <oBasis/oMacros.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oString.h>
#include <oBasis/oTypes.h>
#include <string>

const char* oAsString(bool _Bool)
{
	return _Bool ? "true" : "false";
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const char* const & _Value)
{
	return oStrcpy(_StrDestination, _SizeofStrDestination, oSAFESTRN(_Value));
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const char& _Value)
{
	if (_SizeofStrDestination < 2) return nullptr;
	_StrDestination[0] = _Value;
	_StrDestination[1] = 0;
	return _StrDestination;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const unsigned char& _Value)
{
	if (_SizeofStrDestination < 2) return nullptr;
	_StrDestination[0] = *(signed char*)&_Value;
	_StrDestination[1] = 0;
	return _StrDestination;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const bool& _Value) { return oStrcpy(_StrDestination, _SizeofStrDestination, _Value ? "true" : "false"); }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const short & _Value) { return 0 == _itoa_s(_Value, _StrDestination, _SizeofStrDestination, 10) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const unsigned short& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%hu", _Value) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const int& _Value) { return 0 == _itoa_s(_Value, _StrDestination, _SizeofStrDestination, 10) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const unsigned int& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%u", _Value) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const long& _Value) { return 0 == _itoa_s(_Value, _StrDestination, _SizeofStrDestination, 10) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const unsigned long& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%u", _Value) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const long long& _Value) { return 0 == _i64toa_s(_Value, _StrDestination, _SizeofStrDestination, 10) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const unsigned long long& _Value) { return 0 == _ui64toa_s(unsigned long long(_Value), _StrDestination, _SizeofStrDestination, 10) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const float& _Value) { if (-1 != oPrintf(_StrDestination, _SizeofStrDestination, "%f", _Value)) { oTrimRight(_StrDestination, _SizeofStrDestination, _StrDestination, "0"); return _StrDestination; } return nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const double& _Value) { if (-1 != oPrintf(_StrDestination, _SizeofStrDestination, "%lf", _Value)) { oTrimRight(_StrDestination, _SizeofStrDestination, _StrDestination, "0"); return _StrDestination; } return nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const half& _Value) { if (oToString(_StrDestination, _SizeofStrDestination, static_cast<float>(_Value))) { oTrimRight(_StrDestination, _SizeofStrDestination, _StrDestination, "0"); return _StrDestination; } return nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const std::string& _Value) { return oStrcpy(_StrDestination, _SizeofStrDestination, _Value.c_str()); }

#define oCHK do { if (!_pValue || !oSTRVALID(_StrSource)) return false; } while(false)
bool oFromString(bool* _pValue, const char* _StrSource)
{
	oCHK;
	if (!oStricmp("true", _StrSource) || !oStricmp("t", _StrSource) || !oStricmp("yes", _StrSource) || !oStricmp("y", _StrSource))
		*_pValue = true;
	else 
		*_pValue = atoi(_StrSource) != 0;
	return true;
}

bool oFromString(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource)
{
	return !!oStrcpy(_StrDestination, _SizeofStrDestination, _StrSource);
}

bool oFromString(char** _pValue, const char* _StrSource) { oCHK; return !!oStrcpy(*_pValue, SIZE_MAX, _StrSource); }
bool oFromString(char* _pValue, const char* _StrSource) { oCHK; *_pValue = *_StrSource; return true; }
bool oFromString(unsigned char* _pValue, const char* _StrSource) { oCHK; *_pValue = *(const unsigned char*)_StrSource; return true; }
template<typename T> inline bool _FromString(T* _pValue, const char* _Format, const char* _StrSource) { oCHK; return 1 == sscanf_s(_StrSource, _Format, _pValue); }
bool oFromString(short* _pValue, const char* _StrSource) { return _FromString(_pValue, "%hd", _StrSource); }
bool oFromString(unsigned short* _pValue, const char* _StrSource) { return _FromString(_pValue, "%hu", _StrSource); }
bool oFromString(int* _pValue, const char* _StrSource) { return _FromString(_pValue, "%d", _StrSource); }
bool oFromString(unsigned int* _pValue, const char* _StrSource) { return _FromString(_pValue, "%u", _StrSource); }
bool oFromString(long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%d", _StrSource); }
bool oFromString(unsigned long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%u", _StrSource); }
bool oFromString(long long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%lld", _StrSource); }
bool oFromString(unsigned long long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%llu", _StrSource); }
bool oFromString(float* _pValue, const char* _StrSource) { oCHK; return oAtof(_StrSource, _pValue); }
bool oFromString(double* _pValue, const char* _StrSource) { return _FromString(_pValue, "%lf", _StrSource); }
bool oFromString(half* _pValue, const char* _StrSource) { float v; if (!oFromString(&v, _StrSource)) return false; *_pValue = v; return true; }

char* oAsStringFlags(char* _StrDestination, size_t _SizeofStrDestination, unsigned int _Flags, const char* _AllZerosValue, oFUNCTION<const char*(unsigned int _SingleFlag)> _AsString)
{
	*_StrDestination = 0;
	unsigned int Mask = 0x80000000;

	if (_Flags == 0)
		oPrintf(_StrDestination, _SizeofStrDestination, oSAFESTRN(_AllZerosValue));
	else
	{
		while (Mask)
		{
			if (_Flags & Mask)
				oStrAppendf(_StrDestination, _SizeofStrDestination, "%s%s", *_StrDestination == 0 ? "" : "|", _AsString(_Flags & Mask));
			Mask >>= 1;
		}
	}

	return _StrDestination;
}

bool oIndexFromString(int* _pValue, const char* _StrTestSource, const char** _StrSourceArray, size_t _NumStrSources)
{
	for (size_t i = 0; i < _NumStrSources; i++)
	{
		if (0 == oStricmp(_StrSourceArray[i], _StrTestSource))
		{
			*_pValue = static_cast<int>(i);
			return true;
		}
	}
	return false;
}

char* oIndexToString(char* _StrDestination, size_t _SizeofStrDestination, int _Index, const char** _StrSourceArray, size_t _NumStrSources)
{
	if (_Index < 0 || _Index >= static_cast<int>(_NumStrSources))
		return nullptr;
	oStrcpy(_StrDestination, _SizeofStrDestination, _StrSourceArray[_Index]);
	return _StrDestination;
}
