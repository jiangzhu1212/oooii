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

#include <oooii/oGPU.h>
#include <oooii/oString.h>
#include <oooii/oTest.h>
#include <oooii/oWindows.h>

struct TESTGPU : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPU::DESC desc;
		oGPU::GetDesc(0, &desc);

		if (oGetWindowsVersion() <= oWINDOWS_VISTA)
			oTESTB(desc.Index == 0, "Running on an OS earlier than Vista, so this won't work.");
		else
			oTESTB(desc.Index == 0, "Index is incorrect");

		char VRAMSize[64];
		oFormatMemorySize(VRAMSize, desc.VRAM, 1);
		char SharedSize[64];
		oFormatMemorySize(SharedSize, desc.SharedSystemMemory, 1);
		sprintf_s(_StrStatus, _SizeofStrStatus, "%s D3D%.01f %s (%s shared)", desc.Description, desc.D3DVersion, VRAMSize, SharedSize);

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTGPU);
