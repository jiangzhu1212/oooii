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
#include <oBasis/oError.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oRTTI.h>
#include "oBasisTestCommon.h"

enum oGPU_API2
{
	oGPU_API_UNKNOWN,
	oGPU_API_D3D,
	oGPU_API_OGL,
	oGPU_API_OGLES,
	oGPU_API_WEBGL,
	oGPU_API_CUSTOM,
	
	oGPU_API_COUNT,
};

oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGPU_API2)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPU_API2)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_API2)
		oRTTI_VALUE_CUSTOM(oGPU_API_UNKNOWN,	"Unknown")
		oRTTI_VALUE_CUSTOM(oGPU_API_D3D,		"Direct3D")
		oRTTI_VALUE_CUSTOM(oGPU_API_OGL,		"OpenGL")
		oRTTI_VALUE_CUSTOM(oGPU_API_OGLES,		"OpenGL ES")
		oRTTI_VALUE_CUSTOM(oGPU_API_WEBGL,		"WebGL")
		oRTTI_VALUE_CUSTOM(oGPU_API_CUSTOM,		"Custom")
	oRTTI_ENUM_END_VALUES(oGPU_API2)
oRTTI_ENUM_END_DESCRIPTION(oGPU_API2)


enum oGPU_PIPELINE_STAGE2
{
	oGPU_VERTEX_SHADER,
	oGPU_HULL_SHADER,
	oGPU_DOMAIN_SHADER,
	oGPU_GEOMETRY_SHADER,
	oGPU_PIXEL_SHADER,

	oGPU_PIPELINE_STAGE_COUNT,
};

oRTTI_ENUM_DECLARATION(oRTTI_CAPS_NONE, oGPU_PIPELINE_STAGE2)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oGPU_PIPELINE_STAGE2)
	oRTTI_ENUM_BEGIN_VALUES(oGPU_PIPELINE_STAGE2)
		oRTTI_VALUE(oGPU_VERTEX_SHADER)
		oRTTI_VALUE(oGPU_HULL_SHADER)
		oRTTI_VALUE(oGPU_DOMAIN_SHADER)
		oRTTI_VALUE(oGPU_GEOMETRY_SHADER)
		oRTTI_VALUE(oGPU_PIXEL_SHADER)
	oRTTI_ENUM_END_VALUES(oGPU_PIPELINE_STAGE2)
oRTTI_ENUM_END_DESCRIPTION(oGPU_PIPELINE_STAGE2)


class BaseResource
{
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_ARRAY, BaseResource)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, BaseResource)
	oRTTI_COMPOUND_CONCRETE(BaseResource)
oRTTI_COMPOUND_END_DESCRIPTION(BaseResource)


class Test : public BaseResource
{
public:
	Test(int val) : Member1(val), Member2(val), Member3((float)val) {}
	Test() : Member1(0), Member2(0), Member3(0) {}

	int Member1;
	uint Member2;
	float Member3;

	void OnAttrChanged(const oRTTI_ATTR_BASE* _pAttr)
	{
	}
	void GetVirtual(int*) {}
	void SetVirtual(const int*) {}
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_ARRAY, Test)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, Test)
	oRTTI_COMPOUND_BASES_SINGLE_BASE(Test, BaseResource)
	oRTTI_COMPOUND_CONCRETE(Test)
	oRTTI_COMPOUND_VERSION(Test, 0x1001)
	oRTTI_COMPOUND_ON_ATTR_CHANGED(Test, OnAttrChanged)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(Test)
		oRTTI_COMPOUND_ATTR_GROUP("Data")
		oRTTI_COMPOUND_ATTR(Test, Member1,	oRTTI_OF(int),	"Member1",							oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, Member2,	oRTTI_OF(uint),	"Member2",							oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, Member3,	oRTTI_OF(float),"Member3",							oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR_VIRTUAL(Test,	oRTTI_OF(int),	"Virtual",	GetVirtual,	SetVirtual,	oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(Test)
oRTTI_COMPOUND_END_DESCRIPTION(Test)


bool oBasisTest_oRTTI()
{
	oGPU_API2 value = oGPU_API_OGL;
	oStringL enumText;
	oTESTB(!oStrcmp(oToString(enumText, value), "OpenGL"), "Failed oToString(oGPU_API_OGL)");
	oTESTB(!oStrcmp(oAsString(value), "OpenGL"), "Failed oAsString(oGPU_API_OGL)");

	value = oGPU_API_UNKNOWN;
	oFromString(&value, "OpenGL");
	oTESTB(oGPU_API_OGL== value, "Failed oFromString(oGPU_API_OGL)");

	oTESTB(!oStrcmp(oAsString(oGPU_HULL_SHADER), "oGPU_HULL_SHADER"), "Failed oAsString(oGPU_HULL_SHADER)");

	Test test(1);
	oRTTI_Test.Constructor(oRTTI_OF(Test), &test);

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

