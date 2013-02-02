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
#include <oGPU/oGPUMaterialConstants.h>

static_assert((sizeof(oGPUMaterialConstants) % 16) == 0, "sizeof(oGPUMaterialConstants) must be 16-byte aligned");

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oGPUMaterialConstants)
	oRTTI_COMPOUND_CONCRETE(oGPUMaterialConstants)
	oRTTI_COMPOUND_VERSION(oGPUMaterialConstants, 0x1000)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oGPUMaterialConstants)
		// Basics
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, DiffuseColor,			oRTTI_OF(oRGBf),			"DiffuseColor",				oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, DiffuseAmount,			oRTTI_OF(float),			"DiffuseAmount",			oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, EmissiveColor,			oRTTI_OF(oRGBf),			"EmissiveColor",			oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, Opacity,					oRTTI_OF(float),			"Opacity",					oRTTI_COMPOUND_ATTR_REGULAR)
		// Reflection
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, ReflectionColor,			oRTTI_OF(oRGBf),			"ReflectionColor",			oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, ReflectionAmount,		oRTTI_OF(float),			"ReflectionAmount",			oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, SpecularColor,			oRTTI_OF(oRGBf),			"SpecularColor",			oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, Specularity,				oRTTI_OF(float),			"Specularity",				oRTTI_COMPOUND_ATTR_REGULAR)
		// Refraction
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, RefractionColor,			oRTTI_OF(oRGBf),			"RefractionColor",			oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, RefractionAmount,		oRTTI_OF(float),			"RefractionAmount",			oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, RefractiveSpecularColor,	oRTTI_OF(oRGBf),			"RefractiveSpecularColor",	oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, RefractiveSpecularity,	oRTTI_OF(float),			"RefractiveSpecularity",	oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, BRDFModel,				oRTTI_OF(oGPU_BRDF_MODEL),	"BRDFModel",				oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, IndexOfRefraction,		oRTTI_OF(float),			"IndexOfRefraction",		oRTTI_COMPOUND_ATTR_REGULAR)
		// Bump and Normal mapping
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, BumpScale,				oRTTI_OF(float),			"BumpScale",				oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, BumpDeltaScale,			oRTTI_OF(float),			"BumpDeltaScale",			oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, BumpSpace,				oRTTI_OF(oGPU_NORMAL_SPACE),"BumpSpace",				oRTTI_COMPOUND_ATTR_REGULAR)
		// Subsurface scattering
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, ForwardBackwardCoeff,	oRTTI_OF(float),			"ForwardBackwardCoeff",		oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, ScatterCoeff,			oRTTI_OF(float),			"ScatterCoeff",				oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, Thickness,				oRTTI_OF(float),			"Thickness",				oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, TransmissionColor,		oRTTI_OF(oRGBf),			"TransmissionColor",		oRTTI_COMPOUND_ATTR_REGULAR)

		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, LightShadingContribution,oRTTI_OF(float),			"LightShadingContribution",	oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, ParallaxHeightScale,		oRTTI_OF(float),			"ParallaxHeightScale",		oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, ParallaxHeightBias,		oRTTI_OF(float),			"ParallaxHeightBias",		oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oGPUMaterialConstants, AlphaTestReference,		oRTTI_OF(float),			"AlphaTestReference",		oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oGPUMaterialConstants)
oRTTI_COMPOUND_END_DESCRIPTION(oGPUMaterialConstants)

bool oGPUMaterialConstantsRead(const threadsafe oINI* _pINI, oINI::HSECTION _hSection, oGPUMaterialConstants* _pConstants)
{
	bool AtLeastOneMissing = false;
	oStringXL Missing("The following fields were not assigned: ");

	const oRTTI& rtti = oRTTI_OF(oGPUMaterialConstants);
	for (int i=0; i<rtti.GetNumAttrs(); ++i)
	{
		const oRTTI_ATTR* attr = rtti.GetAttr(i);
		if (!attr->RTTI->FromString(_pINI->GetValue(_hSection, attr->Name), attr->GetDestPtr(_pConstants)))
		{ 
			oStrAppendf(Missing, "%s%s", AtLeastOneMissing ? ", " : "", attr->Name); 
			AtLeastOneMissing = true; 
		}
		
	}
	_pConstants->Pad0 = 0.0f;

	if (AtLeastOneMissing)
		return oErrorSetLast(oERROR_IO, Missing);
	return true;
}

bool oGPUMaterialConstantsWrite(char* _StrDestination, size_t _SizeofStrDestination, const oGPUMaterialConstants& _Constants)
{
	bool AtLeastOneMissing = false;
	oStringXL Missing("The following fields were not written correctly: ");
	oStringL buf;

	const oRTTI& rtti = oRTTI_OF(oGPUMaterialConstants);
	for (int i=0; i<rtti.GetNumAttrs(); ++i)
	{
		const oRTTI_ATTR* attr = rtti.GetAttr(i);
		if (attr->RTTI->ToString(buf, attr->GetSrcPtr(&_Constants)))
			oStrAppendf(_StrDestination, _SizeofStrDestination, "%s=%s\n", attr->Name, buf.c_str());
		else
		{	oStrAppendf(Missing, "%s%s", AtLeastOneMissing ? ", " : "", attr->Name);
			AtLeastOneMissing = true;
		}
	}

	if (AtLeastOneMissing)
		return oErrorSetLast(oERROR_IO, Missing);
	return true;
}
