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
#include <oBasis/oString.h>
#include <oPlatform/oTest.h>
#include <oGPU/oGPU.h>

struct GPU_Device : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPUDevice::INIT init("GPU_Device");
		init.Version = oStd::version(10,0); // for more compatibility when running on varied machines
		oStd::intrusive_ptr<oGPUDevice> Device;
		oTESTB0(oGPUDeviceCreate(init, &Device));

		oGPUDevice::DESC desc;
		Device->GetDesc(&desc);

		oTESTB(desc.AdapterIndex == 0, "Index is incorrect");
		oTESTB(desc.FeatureVersion >= oStd::version(9,0), "Invalid version retrieved");
		oStd::sstring VRAMSize, SharedSize, IVer, FVer, DVer;
		oStd::format_bytes(VRAMSize, desc.NativeMemory, 1);
		oStd::format_bytes(SharedSize, desc.SharedSystemMemory, 1);
		oPrintf(_StrStatus, _SizeofStrStatus, "%s %s %s feature level %s %s (%s shared) running on %s v%s drivers (%s)"
			, desc.DeviceDescription.c_str()
			, oStd::as_string(desc.API)
			, oStd::to_string2(IVer, desc.InterfaceVersion)
			, oStd::to_string2(FVer, desc.FeatureVersion)
			, VRAMSize.c_str()
			, SharedSize.c_str()
			, oStd::as_string(desc.Vendor)
			, oStd::to_string2(DVer, desc.DriverVersion)
			, desc.DriverDescription.c_str());

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_Device);
