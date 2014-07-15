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
