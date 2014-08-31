// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <half.h>
#include <oString/string.h>
#include <oString/stringize.h>

namespace ouro {

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const half& _Value) { if (to_string(_StrDestination, _SizeofStrDestination, static_cast<float>(_Value))) { trim_right(_StrDestination, _SizeofStrDestination, _StrDestination, "0"); return _StrDestination; } return nullptr; }
bool from_string(half* _pValue, const char* _StrSource) { float v; if (!from_string(&v, _StrSource)) return false; *_pValue = v; return true; }

} // namespace ouro
