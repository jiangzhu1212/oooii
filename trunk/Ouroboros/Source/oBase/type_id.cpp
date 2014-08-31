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
#include <oBase/type_id.h>
#include <oString/stringize.h>

namespace ouro {

const char* as_string(const type::value& _TypeID)
{
	switch (_TypeID)
	{
		default:
		case type::unknown: return "unknown_type";
		case type::bool_: return "bool";
		case type::char_: return "char";
		case type::uchar_: return "unsigned char";
		case type::wchar_: return "wchar";
		case type::short_: return "short";
		case type::ushort_: return "unsigned short";
		case type::int_: return "int";
		case type::uint_: return "unsigned int";
		case type::long_: return "long";
		case type::ulong_: return "unsigned long";
		case type::llong_: return "long long";
		case type::ullong_: return "unsigned long long";
		case type::float_: return "float";
		case type::double_: return "double";
		case type::half_: return "half";
		case type::int2_: return "int2";
		case type::int3_: return "int3";
		case type::int4_: return "int4";
		case type::uint2_: return "uint2";
		case type::uint3_: return "uint3";
		case type::uint4_: return "uint4";
		case type::float2_: return "float2";
		case type::float3_: return "float3";
		case type::float4_: return "float4";
		case type::float4x4_: return "float4x4";
	}
}

oDEFINE_ENUM_TO_STRING(type::value);
oDEFINE_ENUM_FROM_STRING(type::value);

}
