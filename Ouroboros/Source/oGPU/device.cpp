// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/device.h>
#include "d3d_debug.h"
#include "d3d_device.h"
#include <d3d11.h>

#include <CSNoop.h>

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

DeviceContext* get_dc(command_list& cl);
Device* get_device(device& dev)
{
	return *(Device**)&dev;
}

compute_shader* get_noop_cs(device& dev)
{
	return &dev.noop;
}

void device::initialize(const device_init& init)
{
	deinitialize();

	intrusive_ptr<Device> D3DDevice = make_device(init);
	supports_deferred = supports_deferred_contexts(D3DDevice);
	is_sw = init.use_software_emulation;

	intrusive_ptr<DeviceContext> Immediate;
	D3DDevice->GetImmediateContext(&Immediate);

	struct cmdlist
	{
		void* context;
		uint id;
	};

	cmdlist initImm;
	initImm.context = Immediate;
	initImm.id = command_list::immediate;
	*(cmdlist*)&imm = initImm;
	Immediate->AddRef();
	
	D3DDevice->AddRef();
	dev = D3DDevice;

	// Set up a noop compute shader to flush for SetCounter()
	try
	{
		sstring CSName;
		debug_name(CSName, D3DDevice);
		sncatf(CSName, ".noop");
		noop.initialize(CSName, *this, CSNoop);
	}
	catch (std::exception&)
	{
		deinitialize();
		std::rethrow_exception(std::current_exception());
	}
}

void device::deinitialize()
{
	noop.deinitialize();
	imm.deinitialize();
	oSAFE_RELEASEV(dev);
	supports_deferred = false;
	is_sw = false;
}

device_info device::get_info() const
{
	return d3d::get_info((Device*)dev, is_sw);
}

void device::reset()
{
	get_dc(imm)->ClearState();
}

void device::flush()
{
	get_dc(imm)->Flush();
}

}}
