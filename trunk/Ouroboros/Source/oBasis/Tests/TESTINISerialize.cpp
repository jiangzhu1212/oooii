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
#include <oBasis/oINISerialize.h>
#include "oBasisTestCommon.h"
#include <vector>

static const char* sINICompoundTest = 
	"[CompoundTest]\r\n"
	"Bool = true\r\n"
	"Int = 31337\r\n"
	"Int2 = 1 2\r\n"
	"Float = 255.\r\n"
	"Float4 = 0.500000 255.000000 1.000000 2.500000\r\n"
	"LongLong = 54491065317\r\n"
	"String = Some test text\r\n"
	"\r\n";

static const char* sINIArrayTest = 
	"[ArrayTest]\r\n"
	"IntValues = 0,1,2,3,4,5\r\n"
	"StringValues = 0,1,2,3,4,5\r\n"
	"Float2Values = 0.500000 1.000000,0.500000 1.000000,0.500000 1.000000,0.500000 1.000000,0.500000 1.000000,0.500000 1.000000\r\n"
	"\r\n";

struct oINICompoundTest
{
	bool Bool;
	int Int;
	int2 Int2;
	float Float;
	float4 Float4;
	long long LongLong;
	oStd::lstring String;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oINICompoundTest)

struct oINIArrayTest
{
	std::vector<int> IntValues;
	std::vector<oStd::sstring> StringValues;
	std::vector<float2> Float2Values;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oINIArrayTest)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oINICompoundTest)
	oRTTI_COMPOUND_ABSTRACT(oINICompoundTest)
	oRTTI_COMPOUND_VERSION(oINICompoundTest, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oINICompoundTest)
		oRTTI_COMPOUND_ATTR(oINICompoundTest, Bool, oRTTI_OF(bool), "Bool", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oINICompoundTest, Int, oRTTI_OF(int), "Int", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oINICompoundTest, Int2, oRTTI_OF(int2), "Int2", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oINICompoundTest, Float, oRTTI_OF(float), "Float", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oINICompoundTest, Float4, oRTTI_OF(float4), "Float4", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oINICompoundTest, LongLong, oRTTI_OF(llong), "LongLong", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oINICompoundTest, String, oRTTI_OF(ostd_lstring), "String", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oINICompoundTest)
oRTTI_COMPOUND_END_DESCRIPTION(oINICompoundTest)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oINIArrayTest)
	oRTTI_COMPOUND_ABSTRACT(oINIArrayTest)
	oRTTI_COMPOUND_VERSION(oINIArrayTest, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oINIArrayTest)
		oRTTI_COMPOUND_ATTR(oINIArrayTest, IntValues, oRTTI_OF(std_vector_int), "IntValues", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oINIArrayTest, StringValues, oRTTI_OF(std_vector_ostd_sstring), "StringValues", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oINIArrayTest, Float2Values, oRTTI_OF(std_vector_float2), "Float2Values", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oINIArrayTest)
oRTTI_COMPOUND_END_DESCRIPTION(oINIArrayTest)

static bool TestCompound()
{
	oStd::ini INI("Test INI Document", const_cast<char*>(sINICompoundTest), nullptr, 100);

	oINICompoundTest Test;
	oTESTB0(oINIReadCompound(&Test, oRTTI_OF(oINICompoundTest), INI, INI.find_section("CompoundTest"), true));
	oTESTB(Test.Bool, "TestCompound: Expected Bool=true");
	oTESTB(Test.Int == 31337, "TestCompound: Expected Int=31337");
	oTESTB(Test.Int2 == int2(1, 2), "TestCompound: Expected Int2=(1, 2)");
	oTESTB(Test.Float == 255.0f, "TestCompound: Expected Float=255");
	oTESTB(Test.Float4 == float4(0.5f, 255.f, 1.f, 2.5f), "TestCompound: Expected Float4=(0.5, 255., 1., 2.5)");
	oTESTB(Test.LongLong == 54491065317, "TestCompound: Expected LongLong=54491065317");
	oTESTB(strcmp(Test.String, "Some test text") == 0, "TestCompound: Expected String=\"Some test text\"");

	oStd::xlstring TestBuffer;
	oTESTB0(oINIWriteCompound(TestBuffer.c_str(), TestBuffer.capacity(), &Test, oRTTI_OF(oINICompoundTest), "CompoundTest"));
	oTESTB(0 == strcmp(TestBuffer.c_str(), sINICompoundTest), "TestCompound: oINIWriteCompound result not as expected");
	return true;
}

static bool TestArray()
{
	oStd::ini INI("Test INI Document", const_cast<char*>(sINIArrayTest), nullptr, 100);
	oINIArrayTest Test;
	oTESTB0(oINIReadCompound(&Test, oRTTI_OF(oINIArrayTest), INI, INI.find_section("ArrayTest"), true));
	oTESTB(Test.IntValues.size() == 6, "TestArray: Expected 6 values of Int");
	oTESTB(Test.StringValues.size() == 6, "TestArray: Expected 6 values of String");
	oTESTB(Test.Float2Values.size() == 6, "TestArray: Expected 6 values of Float2");
	for (int i = 0; i < oInt(Test.IntValues.size()); i++)
	{
		oTESTB(i == Test.IntValues[i], "TestArray: Expected value at index %d to be %d, but got %d",i,i,Test.IntValues[i]);
		oStd::sstring val;
		snprintf(val, "%d", i);
		oTESTB(0 == strcmp(val.c_str(), Test.StringValues[i].c_str()), "TestArray: Expected value at index %d to be %d, but got %d",i,i,Test.IntValues[i]);
		oTESTB(Test.Float2Values[i] == float2(0.5f, 1.0f), "TestArray: Expected value at index %d to be (0.5, 1.0), but got (%f, %f)", Test.Float2Values[i].x, Test.Float2Values[i].y);
	}
	oStd::xlstring TestBuffer;
	oTESTB0(oINIWriteCompound(TestBuffer.c_str(), TestBuffer.capacity(), &Test, oRTTI_OF(oINIArrayTest), "ArrayTest"));
	oTESTB(0 == strcmp(TestBuffer.c_str(), sINIArrayTest), "TestArray: oINIWriteCompound result not as expected");
	return true;
}

bool oBasisTest_oINISerialize()
{
	if (!TestCompound())
		return false; // Pass through error

	if (!TestArray())
		return false; // Pass through error

	oErrorSetLast(0, "");
	return true;
}
