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
#include <oGPU/readback_buffer.h>
#include <oGPU/vertex_buffer.h>
#include <oGPU/rwstructured_buffer.h>
#include "oGPUTestPipelines.h"

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

void TESTbuffer()
{
	static int GPU_BufferAppendIndices[20] = { 5, 6, 7, 18764, 2452, 2423, 52354, 344, -1542, 3434, 53, -4535, 3535, 88884747, 534535, 88474, -445, 4428855, -1235, 4661};
	
	device_init init("GPU Buffer test");
	init.enable_driver_reporting = true;
	std::shared_ptr<device> Device = device::make(init);

	rwstructured_buffer AppendBuffer;
	AppendBuffer.initialize_append("BufferAppend", Device.get(), sizeof(int), oCOUNTOF(GPU_BufferAppendIndices) * 2);

	readback_buffer AppendReadbackBuffer;
	AppendReadbackBuffer.initialize("BufferAppend", Device.get(), sizeof(int), oCOUNTOF(GPU_BufferAppendIndices) * 2);

	readback_buffer AppendBufferCount;
	AppendBufferCount.initialize("BufferAppendCount", Device.get(), sizeof(int));

	std::shared_ptr<pipeline1> Pipeline = Device->make_pipeline1(oGPUTestGetPipeline(oGPU_TEST_BUFFER));
	std::shared_ptr<command_list> CommandList = Device->get_immediate_command_list();

	scoped_device_frame DevFrame(Device.get());
	scoped_command_line_frame CommandListFrame(CommandList.get());

	CommandList->set_blend_state(blend_state::opaque);
	CommandList->set_depth_stencil_state(depth_stencil_state::none);
	CommandList->set_rasterizer_state(rasterizer_state::two_sided);

	AppendBuffer.set_draw_target(CommandList.get(), 0);

	CommandList->set_pipeline(Pipeline);
	vertex_buffer::draw_unindexed(CommandList.get(), oCOUNTOF(GPU_BufferAppendIndices), 0);

	AppendBuffer.copy_counter_to(CommandList.get(), AppendBufferCount, 0);

	int count = 0;
	AppendBufferCount.copy_to(&count, sizeof(int));
	oCHECK(oCOUNTOF(GPU_BufferAppendIndices) == count, "Append counter didn't reach %d", oCOUNTOF(GPU_BufferAppendIndices));

	AppendReadbackBuffer.copy_from(CommandList.get(), AppendBuffer);

	std::vector<int> Values(20);
	oCHECK(AppendReadbackBuffer.copy_to(Values.data(), Values.size() * sizeof(int)), "Copy out of readback buffer failed");

	for(int i = 0; i < oCOUNTOF(GPU_BufferAppendIndices); ++i)
		oCHECK(find_and_erase(Values, GPU_BufferAppendIndices[i]), "GPU Appended bad value");
}

	} // namespace tests
} // namespace ouro
