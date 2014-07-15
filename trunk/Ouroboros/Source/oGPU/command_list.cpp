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
#include <oGPU/command_list.h>
#include <oCore/windows/win_error.h>
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);

DeviceContext* get_dc(command_list& cl)
{
	return *(DeviceContext**)&cl;
}

void command_list::initialize(const char* name, device& dev, uint _id)
{
	deinitialize();

	id = _id;

	Device* D3DDevice = get_device(dev);

	intrusive_ptr<DeviceContext> dc;

	HRESULT hr = D3DDevice->CreateDeferredContext(0, &dc);
	if (FAILED(hr))
	{
		mstring err;

		UINT DeviceCreationFlags = D3DDevice->GetCreationFlags();

		if (DeviceCreationFlags & D3D11_CREATE_DEVICE_SINGLETHREADED)
			snprintf(err, "command_lists cannot be created on a device created as single-threaded.");
		else
			snprintf(err, "Failed to create command_list %d: ", id);

		throw windows::error(hr, err);
	}

	debug_name(dc, name);
	dc->AddRef();
	context = dc;
}

void command_list::deinitialize()
{
	oSAFE_RELEASEV(context);
	id = 0;
}

void command_list::reset()
{
	((DeviceContext*)context)->ClearState();
}

void command_list::flush()
{
	DeviceContext* ctx = (DeviceContext*)context;
	intrusive_ptr<Device> dev;
	ctx->GetDevice(&dev);

	if (id == immediate)
		ctx->Flush();
	else
	{
		intrusive_ptr<CommandList> cmdlist;
		oV(ctx->FinishCommandList(false, &cmdlist));
		intrusive_ptr<DeviceContext> imm;
		dev->GetImmediateContext(&imm);
		imm->ExecuteCommandList(cmdlist, FALSE);
	}
}

}}
