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
#include "oGPUTestPipelines.h"

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

void TESTbuffer()
{
	static int GPU_BufferAppendIndices[20] = { 5, 6, 7, 18764, 2452, 2423, 52354, 344, -1542, 3434, 53, -4535, 3535, 88884747, 534535, 88474, -445, 4428855, -1235, 4661};
	
	device_init init("GPU Buffer test");
	init.driver_debug_level = debug_level::normal;
	std::shared_ptr<device> Device = device::make(init);

	buffer_info i;
	i.struct_byte_size = sizeof(int);
	i.type = buffer_type::unordered_structured_append;
	i.array_size = oCOUNTOF(GPU_BufferAppendIndices) * 2;
	std::shared_ptr<buffer> AppendBuffer = Device->make_buffer("BufferAppend", i);
	i.type = buffer_type::readback;

	std::shared_ptr<buffer> AppendReadbackBuffer = Device->make_buffer("BufferAppend", i);

	i.type = buffer_type::readback;
	i.array_size = 1;

	std::shared_ptr<buffer> AppendBufferCount = Device->make_buffer("BufferAppendCount", i);
	std::shared_ptr<pipeline> Pipeline = Device->make_pipeline(oGPUTestGetPipeline(oGPU_TEST_BUFFER));
	std::shared_ptr<command_list> CommandList = Device->get_immediate_command_list();

	Device->begin_frame();
	CommandList->begin();

	CommandList->set_blend_state(blend_state::opaque);
	CommandList->set_depth_stencil_state(depth_stencil_state::none);
	CommandList->set_surface_state(surface_state::two_sided);

	buffer* b = AppendBuffer.get();
	CommandList->set_render_target_and_unordered_resources(nullptr, 0, nullptr, false, 0, 1, &b);
	CommandList->set_pipeline(Pipeline);
	CommandList->draw(nullptr, 0, 0, nullptr, 0, oCOUNTOF(GPU_BufferAppendIndices));
	CommandList->copy_counter(AppendBufferCount, 0, AppendBuffer);
	CommandList->copy(AppendReadbackBuffer, AppendBuffer);

	surface::mapped_subresource ReadBack;
	oCHECK0(Device->map_read(AppendBufferCount, 0, &ReadBack, true));
	oCHECK(oCOUNTOF(GPU_BufferAppendIndices) == *(int*)ReadBack.data, "Append counter didn't reach %d", oCOUNTOF(GPU_BufferAppendIndices));
	Device->unmap_read(AppendBufferCount, 0);

	oCHECK0(Device->map_read(AppendReadbackBuffer, 0, &ReadBack, true));

	const int* pBuffer = (const int*)ReadBack.data;
	std::vector<int> Values;
	for(int i = 0; i < oCOUNTOF(GPU_BufferAppendIndices); ++i)
		Values.push_back(GPU_BufferAppendIndices[i]);

	for(int i = 0; i < oCOUNTOF(GPU_BufferAppendIndices); ++i)
		oCHECK(find_and_erase(Values, GPU_BufferAppendIndices[i]), "GPU Appended bad value");

	Device->unmap_read(AppendReadbackBuffer, 0);

	CommandList->end();
	Device->end_frame();
}

	} // namespace tests
} // namespace ouro
