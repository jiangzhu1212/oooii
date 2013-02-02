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
#include <oBasis/oString.h>
#include <oBasis/oTypeID.h>

const char* oAsString(oTYPE_ID _TypeID)
{
	switch (_TypeID)
	{
		default:
		case oTYPE_UNKNOWN: return "unknown_type";
		case oTYPE_BOOL: return "bool";
		case oTYPE_CHAR: return "char";
		case oTYPE_UCHAR: return "unsigned char";
		case oTYPE_WCHAR: return "wchar";
		case oTYPE_SHORT: return "short";
		case oTYPE_USHORT: return "unsigned short";
		case oTYPE_INT: return "int";
		case oTYPE_UINT: return "unsigned int";
		case oTYPE_LONG: return "long";
		case oTYPE_ULONG: return "unsigned long";
		case oTYPE_LLONG: return "long long";
		case oTYPE_ULLONG: return "unsigned long long";
		case oTYPE_FLOAT: return "float";
		case oTYPE_DOUBLE: return "double";
		case oTYPE_HALF: return "half";
		case oTYPE_INT2: return "int2";
		case oTYPE_INT3: return "int3";
		case oTYPE_INT4: return "int4";
		case oTYPE_UINT2: return "uint2";
		case oTYPE_UINT3: return "uint3";
		case oTYPE_UINT4: return "uint4";
		case oTYPE_FLOAT2: return "float2";
		case oTYPE_FLOAT3: return "float3";
		case oTYPE_FLOAT4: return "float4";
		case oTYPE_FLOAT4X4: return "float4x4";
	}
}

bool oFromString(oTYPE_ID* _pTypeID, const char* _StrSource)
{
	*_pTypeID = oTYPE_UNKNOWN;
	for (int i = 0; i < oNUM_TYPES; i++)
	{
		if (!oStrcmp(_StrSource, oAsString(oTYPE_ID(i))))
		{
			*_pTypeID = oTYPE_ID(i);
			return true;
		}
	}

	return false;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oTYPE_ID& _Value)
{
	return oStrcpy(_StrDestination, _SizeofStrDestination, oAsString(_Value));
}
