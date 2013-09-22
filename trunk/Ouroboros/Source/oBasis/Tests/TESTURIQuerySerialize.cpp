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
#include <oBasis/oURI.h>
#include <oBasis/oError.h>
#include <oBasis/oURIQuerySerialize.h>
#include "oBasisTestCommon.h"

using namespace ouro;

enum oURIQueryTestEnum
{
	ENUM1,
	Enum2,
	enum3
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_NONE, oURIQueryTestEnum)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oURIQueryTestEnum)
	oRTTI_ENUM_BEGIN_VALUES(oURIQueryTestEnum)
		oRTTI_VALUE(ENUM1)
		oRTTI_VALUE(Enum2)
		oRTTI_VALUE(enum3)
	oRTTI_ENUM_END_VALUES(oURIQueryTestEnum)
oRTTI_ENUM_END_DESCRIPTION(oURIQueryTestEnum)

struct oURIQueryTestCompoundBase
{
	int BaseInt;
	sstring BaseString;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oURIQueryTestCompoundBase)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oURIQueryTestCompoundBase)
	oRTTI_COMPOUND_ABSTRACT(oURIQueryTestCompoundBase)
	oRTTI_COMPOUND_VERSION(oURIQueryTestCompoundBase, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oURIQueryTestCompoundBase)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompoundBase, BaseInt, oRTTI_OF(int), "BaseInt", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompoundBase, BaseString, oRTTI_OF(ouro_sstring), "BaseString", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oURIQueryTestCompoundBase)
oRTTI_COMPOUND_END_DESCRIPTION(oURIQueryTestCompoundBase)

struct oURIQueryTestCompound : oURIQueryTestCompoundBase
{
	bool Bool;
	char Char;
	uchar Uchar;
	int Int;
	llong Llong;
	float Float;
	double Double;
	lstring String;
	int2 Int2;
	oURIQueryTestEnum Enum;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oURIQueryTestCompound)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oURIQueryTestCompound)
	oRTTI_COMPOUND_BASES_SINGLE_BASE(oURIQueryTestCompound, oURIQueryTestCompoundBase)
	oRTTI_COMPOUND_ABSTRACT(oURIQueryTestCompound)
	oRTTI_COMPOUND_VERSION(oURIQueryTestCompound, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oURIQueryTestCompound)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompound, Bool, oRTTI_OF(bool), "Bool", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompound, Char, oRTTI_OF(char), "Char", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompound, Uchar, oRTTI_OF(uchar), "Uchar", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompound, Int, oRTTI_OF(int), "Int", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompound, Llong, oRTTI_OF(llong), "Llong", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompound, Float, oRTTI_OF(float), "Float", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompound, Double, oRTTI_OF(double), "Double", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompound, String, oRTTI_OF(ouro_lstring), "String", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompound, Int2, oRTTI_OF(int2), "Int2", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oURIQueryTestCompound, Enum, oRTTI_OF(oURIQueryTestEnum), "Enum", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oURIQueryTestCompound)
oRTTI_COMPOUND_END_DESCRIPTION(oURIQueryTestCompound)

oURIQueryTestCompound sURIQueryTestCompound;

static void sInitURIQueryTestCompound()
{
	sURIQueryTestCompound.BaseInt = 11000;
	sURIQueryTestCompound.BaseString = "OOOii";
	sURIQueryTestCompound.Bool = true;
	sURIQueryTestCompound.Char = -126;
	sURIQueryTestCompound.Uchar = 254;
	sURIQueryTestCompound.Int = 0x1ee7c0de;
	sURIQueryTestCompound.Llong = 54491065317;
	sURIQueryTestCompound.Float = -1.5f;
	sURIQueryTestCompound.Double = 1.5;
	sURIQueryTestCompound.String = "Some test text";
	sURIQueryTestCompound.Int2 = int2(10,20);
	sURIQueryTestCompound.Enum = enum3;
}

static const char* sURIQueryTestCompoundReferenceResult = "BaseInt=11000&BaseString=OOOii&Bool=true&Char=-126&Uchar=254&Int=518504670&Llong=54491065317&Float=-1.5&Double=1.5&String=Some test text&Int2=10 20&Enum=enum3";

bool operator ==(const oURIQueryTestCompound& _LHS, const oURIQueryTestCompound& _RHS)
{
	if (_LHS.BaseInt != _RHS.BaseInt) return false;
	if (0 != strcmp(_LHS.BaseString.c_str(), _RHS.BaseString.c_str())) return false;

	if (_LHS.Bool != _RHS.Bool) return false;
	if (_LHS.Char != _RHS.Char) return false;
	if (_LHS.Uchar != _RHS.Uchar) return false;
	if (_LHS.Int != _RHS.Int) return false;
	if (_LHS.Llong != _RHS.Llong) return false;
	if (_LHS.Float != _RHS.Float) return false;
	if (_LHS.Double != _RHS.Double) return false;
	if (0 != strcmp(_LHS.String.c_str(), _RHS.String.c_str())) return false;
	if (any(_LHS.Int2 != _RHS.Int2)) return false;
	if (_LHS.Enum != _RHS.Enum) return false;
	return true;
}

bool oBasisTest_oURIQuerySerialize()
{
	sInitURIQueryTestCompound();

	xxlstring URIQueryWriteTestResult;
	oTESTB0(oURIQueryWriteCompound(URIQueryWriteTestResult.c_str(), URIQueryWriteTestResult.capacity(), &sURIQueryTestCompound, oRTTI_OF(oURIQueryTestCompound)));
	oTESTB(0 == strcmp(URIQueryWriteTestResult.c_str(), sURIQueryTestCompoundReferenceResult), "oURIQueryWriteCompound result doesn't match the expected one");

	oURIQueryTestCompound URIQueryReadTestResult;
	oURIQueryReadCompound(&URIQueryReadTestResult, oRTTI_OF(oURIQueryTestCompound), sURIQueryTestCompoundReferenceResult, true);
	oTESTB(URIQueryReadTestResult == sURIQueryTestCompound, "oURIQueryReadCompound result doesn't match the expected one");

	oErrorSetLast(0, "");
	return true;
}