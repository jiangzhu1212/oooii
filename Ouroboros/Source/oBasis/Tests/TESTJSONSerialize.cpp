// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBasis/oURI.h>
#include <oBasis/oError.h>
#include <oBasis/oJSONSerialize.h>
#include <oHLSL/oHLSLMath.h>
#include "oBasisTestCommon.h"

using namespace ouro;

enum oJSONTestEnum
{
	ENUM1,
	Enum2,
	enum3
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_NONE, oJSONTestEnum)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oJSONTestEnum)
	oRTTI_ENUM_BEGIN_VALUES(oJSONTestEnum)
		oRTTI_VALUE(ENUM1)
		oRTTI_VALUE(Enum2)
		oRTTI_VALUE(enum3)
	oRTTI_ENUM_END_VALUES(oJSONTestEnum)
oRTTI_ENUM_END_DESCRIPTION(oJSONTestEnum)

struct oJSONTestCompound
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
	oJSONTestEnum Enum;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oJSONTestCompound)
	
oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oJSONTestCompound)
	oRTTI_COMPOUND_ABSTRACT(oJSONTestCompound)
	oRTTI_COMPOUND_VERSION(oJSONTestCompound, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oJSONTestCompound)
		oRTTI_COMPOUND_ATTR(oJSONTestCompound, Bool, oRTTI_OF(bool), "Bool", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestCompound, Char, oRTTI_OF(char), "Char", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestCompound, Uchar, oRTTI_OF(uchar), "Uchar", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestCompound, Int, oRTTI_OF(int), "Int", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestCompound, Llong, oRTTI_OF(llong), "Llong", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestCompound, Float, oRTTI_OF(float), "Float", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestCompound, Double, oRTTI_OF(double), "Double", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestCompound, String, oRTTI_OF(ouro_lstring), "String", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestCompound, Int2, oRTTI_OF(int2), "Int2", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestCompound, Enum, oRTTI_OF(oJSONTestEnum), "Enum", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oJSONTestCompound)
oRTTI_COMPOUND_END_DESCRIPTION(oJSONTestCompound)

struct oJSONTestContainer
{
	int changelist;
	sstring user;
	int remainingms;
	float progress;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_ARRAY, oJSONTestContainer)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oJSONTestContainer)
	oRTTI_COMPOUND_ABSTRACT(oJSONTestContainer)
	oRTTI_COMPOUND_VERSION(oJSONTestContainer, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oJSONTestContainer)
		oRTTI_COMPOUND_ATTR(oJSONTestContainer, changelist, oRTTI_OF(int), "changelist", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestContainer, user, oRTTI_OF(ouro_sstring), "user", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestContainer, remainingms, oRTTI_OF(int), "remainingms", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oJSONTestContainer, progress, oRTTI_OF(float), "progress", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oJSONTestContainer)
oRTTI_COMPOUND_END_DESCRIPTION(oJSONTestContainer)


oJSONTestCompound sJSONTestCompound = {
	true,
	-126,
	254,
	0x1ee7c0de,
	54491065317,
	-1.5f,
	1.5,
	"Some test text for \"JSON\" with some\r\n\tcharacters:\f\b\t\\ that need to be escaped and/or turned into unicode format:\v\a\x1b",
	int2(10,20),
	enum3
};

static const char* sJSONTestCompoundReferenceResult = 
	"{"
		"\"Bool\":true,"
		"\"Char\":-126,"
		"\"Uchar\":254,"
		"\"Int\":518504670,"
		"\"Llong\":54491065317,"
		"\"Float\":-1.5,"
		"\"Double\":1.5,"
		"\"String\":\"Some test text for \\\"JSON\\\" with some\\r\\n\\tcharacters:\\f\\b\\t\\\\ that need to be escaped and/or turned into unicode format:\\u000b\\u0007\\u001b\","
		"\"Int2\":\"10 20\","
		"\"Enum\":\"enum3\""
	"}";

bool operator ==(const oJSONTestCompound& _LHS, const oJSONTestCompound& _RHS)
{
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

oJSONTestContainer sJSONTestContainer = 
{
	10,
	"jeffrey.exterkate",
	60000,
	95.0f,
};

bool operator ==(const oJSONTestContainer& _LHS, const oJSONTestContainer& _RHS)
{
	if (_LHS.changelist != _RHS.changelist) return false;
	if (0 != strcmp(_LHS.user.c_str(), _RHS.user.c_str())) return false;
	if (_LHS.remainingms != _RHS.remainingms) return false;
	if (_LHS.progress != _RHS.progress) return false;
	return true;
}

static const char* oJSONTestContainerReferenceResult = "[{\"changelist\":10,\"user\":\"jeffrey.exterkate\",\"remainingms\":60000,\"progress\":95.},{\"changelist\":10,\"user\":\"jeffrey.exterkate\",\"remainingms\":60000,\"progress\":95.}]";

bool oBasisTest_oJSONSerialize()
{
	xxlstring JSONWriteTestResult;
	oTESTB0(oJSONWriteCompound(JSONWriteTestResult.c_str(), JSONWriteTestResult.capacity(), &sJSONTestCompound, oRTTI_OF(oJSONTestCompound)));
	oTESTB(0 == strcmp(JSONWriteTestResult.c_str(), sJSONTestCompoundReferenceResult), "oJSONWriteCompound result doesn't match the expected one");

	json JSON("Test JSON", sJSONTestCompoundReferenceResult);

	oJSONTestCompound JSONReadTestResult;
	oTESTB0(oJSONReadCompound(&JSONReadTestResult, oRTTI_OF(oJSONTestCompound), JSON, JSON.root(), true));
	oTESTB(JSONReadTestResult == sJSONTestCompound, "oJSONReadCompound result doesn't match the expected one");

	std::vector<oJSONTestContainer> JSONWriteContainerTest;
	JSONWriteContainerTest.push_back(sJSONTestContainer);
	JSONWriteContainerTest.push_back(sJSONTestContainer);

	xxlstring PendingTasksJSON;
	oJSONWriteContainer(PendingTasksJSON.c_str(), PendingTasksJSON.capacity(), &JSONWriteContainerTest, sizeof(JSONWriteContainerTest), oRTTI_OF(std_vector_oJSONTestContainer));
	oTESTB(0 == strcmp(PendingTasksJSON.c_str(), oJSONTestContainerReferenceResult), "oJSONReadCompound result doesn't match the expected one");

	JSON = std::move(json("2nd Test JSON", oJSONTestContainerReferenceResult));
	
	std::vector<oJSONTestContainer> JSONReadContainerTestResult;
	oTESTB0(oJSONReadContainer(&JSONReadContainerTestResult, sizeof(JSONReadContainerTestResult), oRTTI_OF(std_vector_oJSONTestContainer), JSON, JSON.root(), true));
	oTESTB(JSONReadContainerTestResult.size() == 2, "oJSONReadContainer result doesn't match the expected size");
	oTESTB(JSONReadContainerTestResult[0] == sJSONTestContainer, "oJSONReadContainer result elem 0 doesn't match the expected values");
	oTESTB(JSONReadContainerTestResult[1] == sJSONTestContainer, "oJSONReadContainer result elem 1 doesn't match the expected values");

	oErrorSetLast(0, "");
	return true;
}