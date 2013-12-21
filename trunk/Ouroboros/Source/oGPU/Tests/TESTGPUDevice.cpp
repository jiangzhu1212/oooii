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
#include <oPlatform/oTest.h>
#include <oGPU/oGPU.h>

using namespace ouro;

struct GPU_Device : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPUDevice::INIT init("GPU_Device");
		init.version = version(10,0); // for more compatibility when running on varied machines
		intrusive_ptr<oGPUDevice> Device;
		oTESTB0(oGPUDeviceCreate(init, &Device));

		oGPUDevice::DESC info;
		Device->GetDesc(&info);

		oTESTB(info.adapter_index == 0, "Index is incorrect");
		oTESTB(info.feature_version >= version(9,0), "Invalid version retrieved");
		sstring VRAMSize, SharedSize, IVer, FVer, DVer;
		format_bytes(VRAMSize, info.native_memory, 1);
		format_bytes(SharedSize, info.shared_system_memory, 1);
		snprintf(_StrStatus, _SizeofStrStatus, "%s %s %s %s (%s shared) running on %s v%s drivers (%s)"
			, info.device_description.c_str()
			, ouro::as_string(info.api)
			, to_string2(FVer, info.feature_version)
			, VRAMSize.c_str()
			, SharedSize.c_str()
			, ouro::as_string(info.vendor)
			, to_string2(DVer, info.driver_version)
			, info.driver_description.c_str());

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_Device);
