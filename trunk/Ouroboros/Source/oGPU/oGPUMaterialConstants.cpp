/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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

#define oVALS() \
	oVAL(DiffuseColor); \
	oVAL(DiffuseAmount); \
	oVAL(EmissiveColor); \
	oVAL(Opacity); \
	oVAL(DiffuseColor); \
	oVAL(ReflectionColor); \
	oVAL(ReflectionAmount); \
	oVAL(SpecularColor); \
	oVAL(Specularity); \
	oVAL(RefractionColor); \
	oVAL(RefractionAmount); \
	oVAL(RefractiveSpecularColor); \
	oVAL(RefractiveSpecularity); \
	oVAL(BRDFModel); \
	oVAL(IndexOfRefraction); \
	oVAL(BumpScale); \
	oVAL(BumpDeltaScale); \
	oVAL(BumpSpace); \
	oVAL(ForwardBackwardCoeff); \
	oVAL(ScatterCoeff); \
	oVAL(Thickness); \
	oVAL(TransmissionColor); \
	oVAL(LightShadingContribution); \
	oVAL(ParallaxHeightScale); \
	oVAL(ParallaxHeightBias); \
	oVAL(AlphaTestReference); \

bool oGPUMaterialConstantsRead(const threadsafe oINI* _pINI, oINI::HSECTION _hSection, oGPUMaterialConstants* _pConstants)
{
	bool AtLeastOneMissing = false;
	oStringXL Missing("The following fields were not assigned: ");

#define oVAL(_FieldName) do { if (!_pINI->GetValue(_hSection, #_FieldName, &_pConstants->_FieldName)) { oStrAppendf(Missing, "%s%s", AtLeastOneMissing ? ", " : "", #_FieldName); AtLeastOneMissing = true; } } while(0)
	oVALS();
	_pConstants->Pad0 = 0.0f;
	#undef oVAL

	if (AtLeastOneMissing)
		return oErrorSetLast(oERROR_IO, Missing);
	return true;
}

bool oGPUMaterialConstantsWrite(char* _StrDestination, size_t _SizeofStrDestination, const oGPUMaterialConstants& _Constants)
{
	bool AtLeastOneMissing = false;
	oStringXL Missing("The following fields were not written correctly: ");
	oStringL buf;

	#define oVAL(_FieldName) do \
	{	if (oToString(buf, _Constants._FieldName)) \
			oStrAppendf(_StrDestination, _SizeofStrDestination, #_FieldName "=%s\n", buf.c_str()); \
		else \
		{	oStrAppendf(Missing, "%s%s", AtLeastOneMissing ? ", " : "", #_FieldName); \
			AtLeastOneMissing = true; \
		} \
	} while (0)

	oVALS();

	if (AtLeastOneMissing)
		return oErrorSetLast(oERROR_IO, Missing);
	return true;
}
