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

#include <oPlatform/oGPU.h>
#include <oBasis/oString.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oWindows.h>

struct TESTGPU : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPU_DESC desc;
		oTESTB(oGPUEnum(0, &desc), "%s: %s", oAsString(oErrorGetLast()), oErrorGetLastString());
		oTESTB(desc.Index == 0, "Index is incorrect");
		oTESTB(desc.FeatureVersion >= oVersion(9,0), "Invalid version retrieved");

		oStringS VRAMSize, SharedSize;
		oFormatMemorySize(VRAMSize, desc.VRAM, 1);
		oFormatMemorySize(SharedSize, desc.SharedSystemMemory, 1);
		sprintf_s(_StrStatus, _SizeofStrStatus, "%s %s %d.%d feature level %d.%d %s (%s shared) running on %s v%d.%d drivers (%s)", desc.GPUDescription.c_str(), oAsString(desc.API), desc.InterfaceVersion.Major, desc.InterfaceVersion.Minor, desc.FeatureVersion.Major, desc.FeatureVersion.Minor, VRAMSize.c_str(), SharedSize.c_str(), oAsString(desc.Vendor), desc.DriverVersion.Major, desc.DriverVersion.Minor, desc.DriverDescription.c_str());

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTGPU);
