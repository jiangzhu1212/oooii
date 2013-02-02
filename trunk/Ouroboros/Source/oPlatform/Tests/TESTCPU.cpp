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
#include <oPlatform/oCPU.h>
#include <oBasis/oString.h>
#include <oPlatform/oTest.h>

struct PLATFORM_oCPU : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oCPU_DESC cpu;
		oCPUEnum(0, &cpu);
		bool HasHT = oCPUCheckFeatureSupport(0, "Hyperthreading") >= oCPU_FULL_SUPPORT;
		bool HasAVX = oCPUCheckFeatureSupport(0, "AVX1") >= oCPU_FULL_SUPPORT;
		oPrintf(_StrStatus, _SizeofStrStatus, "%s %s %s%s%s %d HWThreads", oAsString(cpu.Type), cpu.String, cpu.BrandString, HasHT ? " HT" : "", HasAVX ? " AVX" : "", cpu.NumHardwareThreads);
		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_oCPU);
