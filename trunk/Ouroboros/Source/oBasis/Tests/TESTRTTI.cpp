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
#include <oBasis/oError.h>
#include <oBase/fixed_string.h>
#include <oBasis/oRTTI.h>
#include <oBasis/oInvalid.h>
#include "oBasisTestCommon.h"
#include <oCompute/linear_algebra.h>
#include <oCompute/oAABox.h>
#include <oCompute/oFrustum.h>
#include <oCompute/oSphere.h>

using namespace ouro;

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
	sstring Member4;
	mstring Member5;
	float2 Float2Member;
	float3 Float3Member;
	float4 Float4Member;
	oPlanef PlaneMember;
	oSpheref SphereMember;
	oAABoxf BoxMember;

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
	oRTTI_COMPOUND_VERSION(Test, 0,1,0,0)
	oRTTI_COMPOUND_ON_ATTR_CHANGED(Test, OnAttrChanged)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(Test)
		oRTTI_COMPOUND_ATTR_GROUP("Data")
		oRTTI_COMPOUND_ATTR(Test, Member1, oRTTI_OF(int), "Member1", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, Member2, oRTTI_OF(uint), "Member2", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, Member3, oRTTI_OF(float), "Member3", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, Member4, oRTTI_OF(ouro_sstring), "Member4", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, Member5, oRTTI_OF(ouro_mstring), "Member5", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, Float2Member, oRTTI_OF(float2), "Float2Member", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, Float3Member, oRTTI_OF(float3), "Float3Member", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, Float4Member, oRTTI_OF(float4), "Float4Member", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, PlaneMember, oRTTI_OF(oPlanef), "PlaneMember", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, SphereMember, oRTTI_OF(oSpheref), "SphereMember", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(Test, BoxMember, oRTTI_OF(oAABoxf), "BoxMember", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR_VIRTUAL(Test, oRTTI_OF(int), "Virtual", GetVirtual, SetVirtual, oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(Test)
oRTTI_COMPOUND_END_DESCRIPTION(Test)


bool oBasisTest_oRTTI()
{
	oGPU_API2 value = oGPU_API_OGL;
	lstring enumText;
	oTESTB(!strcmp(to_string(enumText, value), "OpenGL"), "Failed to_string(oGPU_API_OGL)");
	oTESTB(!strcmp(ouro::as_string(value), "OpenGL"), "Failed as_string(oGPU_API_OGL)");

	value = oGPU_API_UNKNOWN;
	from_string(&value, "OpenGL");
	oTESTB(oGPU_API_OGL== value, "Failed from_string(oGPU_API_OGL)");

	oTESTB(!strcmp(ouro::as_string(oGPU_HULL_SHADER), "oGPU_HULL_SHADER"), "Failed as_string(oGPU_HULL_SHADER)");

	Test test(1);
	oRTTI_Test.Constructor(oRTTI_OF(Test), &test);

	const oRTTI_ATTR* f = oRTTI_OF(Test).GetAttr(4);
	oTESTB(f->RTTI->FromString("012345678901234567890123456789012345678901234567890123456789012", f->GetDestPtr(&test), static_cast<int>(f->Size)), "Failed FromString(test.Member4)");
	// @oooii-jeffrey: Disabling this test as oStrcpy (strcpy_s) asserts, so we 
	// can't handle that
	//oTESTB(!f->RTTI->FromString("0123456789012345678901234567890123456789012345678901234567890123", f->GetDestPtr(&test), static_cast<int>(f->Size)), "Should have failed FromString(test.Member4)");

	sstring TestMember4;
	f->RTTI->ToString(TestMember4, f->GetDestPtr(&test));
	oTESTB(0==strcmp(TestMember4.c_str(), "012345678901234567890123456789012345678901234567890123456789012"), "Failed ToString(test.Member4)");

	f = oRTTI_OF(Test).GetAttr(6);
	f->RTTI->FromString("	 1.0 2.0", f->GetDestPtr(&test), static_cast<int>(f->Size));
	oTESTB(ouro::equal(test.Float2Member, float2(1.0f, 2.0f)), "Failed FromString(test.Float2Member)");

	sstring str;
	oTESTB(f->RTTI->ToString(str, f->GetDestPtr(&test)), "Failed ToString(test.Float2Member)");
	oTESTB(!strcmp("1.000000 2.000000", str), "Failed ToString(test.Float2Member)");

	f = oRTTI_OF(Test).GetAttr(7);
	f->RTTI->FromString("	 1.0 2.0  3.0", f->GetDestPtr(&test), static_cast<int>(f->Size));
	oTESTB(ouro::equal(test.Float3Member, float3(1.0f, 2.0f, 3.0f)), "Failed FromString(test.Float3Member)");

	str.clear();
	oTESTB(f->RTTI->ToString(str, f->GetDestPtr(&test)), "Failed ToString(test.Float3Member)");
	oTESTB(!strcmp("1.000000 2.000000 3.000000", str), "Failed ToString(test.Float3Member)");

	f = oRTTI_OF(Test).GetAttr(8);
	f->RTTI->FromString("	 1.0   2.0  3.0 4.0", f->GetDestPtr(&test), static_cast<int>(f->Size));
	oTESTB(ouro::equal(test.Float4Member, float4(1.0f, 2.0f, 3.0f, 4.0f)), "Failed FromString(test.Float4Member)");

	str.clear();
	oTESTB(f->RTTI->ToString(str, f->GetDestPtr(&test)), "Failed ToString(test.Float4Member)");
	oTESTB(!strcmp("1.000000 2.000000 3.000000 4.000000", str), "Failed ToString(test.Float4Member)");

	f = oRTTI_OF(Test).GetAttr(9);
	f->RTTI->FromString("4.0 3.0 2.0 1.0", f->GetDestPtr(&test), static_cast<int>(f->Size));
	oTESTB(ouro::equal(test.PlaneMember, oPlanef(float4(4.0f, 3.0f, 2.0f, 1.0f))), "Failed FromString(test.PlaneMember)");

	str.clear();
	oTESTB(f->RTTI->ToString(str, f->GetDestPtr(&test)), "Failed ToString(test.PlaneMember)");
	oTESTB(!strcmp("4.000000 3.000000 2.000000 1.000000", str), "Failed ToString(test.PlaneMember)");

	f = oRTTI_OF(Test).GetAttr(10);
	f->RTTI->FromString("1.1 2.2 3.3 4.4", f->GetDestPtr(&test), static_cast<int>(f->Size));
	oTESTB(all(test.SphereMember == oSpheref(float3(1.1f, 2.2f, 3.3f), 4.4f)), "Failed FromString(test.SphereMember)");

	str.clear();
	oTESTB(f->RTTI->ToString(str, f->GetDestPtr(&test)), "Failed ToString(test.SphereMember)");
	oTESTB(!strcmp("1.100000 2.200000 3.300000 4.400000", str), "Failed ToString(test.SphereMember)");

	f = oRTTI_OF(Test).GetAttr(11);
	f->RTTI->FromString("	 1.0   2.0  3.0 4.0 5.0   6.0", f->GetDestPtr(&test), static_cast<int>(f->Size));
	oTESTB(ouro::equal(test.BoxMember, oAABoxf(oAABoxf::min_max, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f))), "Failed FromString(test.BoxMember)");

	str.clear();
	oTESTB(f->RTTI->ToString(str, f->GetDestPtr(&test)), "Failed ToString(test.BoxMember)");
	oTESTB(!strcmp("1.000000 2.000000 3.000000 4.000000 5.000000 6.000000", str), "Failed ToString(test.BoxMember)");

	oErrorSetLast(0, "");
	return true;
}

