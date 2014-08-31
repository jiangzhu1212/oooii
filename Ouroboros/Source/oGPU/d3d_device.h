// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_d3d_device_h
#define oGPU_d3d_device_h

#include <oBase/intrusive_ptr.h>
#include <oGPU/device.h>
#include "d3d_types.h"

namespace ouro { namespace gpu { namespace d3d {

intrusive_ptr<Device> make_device(const gpu::device_init& init);

// returns info about dev. (there's no way to determine if the device is software, so pass that through)
gpu::device_info get_info(Device* dev, bool is_software_emulation);

// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476486(v=vs.85).aspx
// This is not cheap enough to reevaluate for each call to update_subresource, so
// call this once and cache the result per device and pass it to update_subresource
// as appropriate.
bool supports_deferred_contexts(Device* dev);

}}}

#endif
