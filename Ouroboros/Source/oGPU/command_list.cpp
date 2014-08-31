// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
