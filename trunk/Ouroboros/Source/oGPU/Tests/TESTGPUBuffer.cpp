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

using namespace ouro;

int GPU_BufferAppendIndices[20] = 
{ 5, 6, 7, 18764, 2452, 2423, 52354, 344, -1542, 3434, 53, -4535, 3535, 88884747, 534535, 88474, -445, 4428855, -1235, 4661};

struct GPU_Buffer : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPUDevice::INIT init("GPU_Buffer");
		init.driver_debug_level = gpu::debug_level::normal;
		intrusive_ptr<oGPUDevice> Device;
		oTESTB0(oGPUDeviceCreate(init, &Device));

		oGPUBuffer::DESC BufferDesc;
		BufferDesc.struct_byte_size = sizeof(int);
		BufferDesc.type = gpu::buffer_type::unordered_structured_append;
		BufferDesc.array_size = oCOUNTOF(GPU_BufferAppendIndices) * 2;

		intrusive_ptr<oGPUBuffer> AppendBuffer;
		oTESTB0( Device->CreateBuffer("GPU_BufferAppend", BufferDesc, &AppendBuffer) );

		BufferDesc.type = gpu::buffer_type::readback;

		intrusive_ptr<oGPUBuffer> AppendReadbackBuffer;
		oTESTB0( Device->CreateBuffer("GPU_BufferAppend", BufferDesc, &AppendReadbackBuffer) );

		BufferDesc.type = gpu::buffer_type::readback;
		BufferDesc.array_size = 1;

		intrusive_ptr<oGPUBuffer> AppendBufferCount;
		oTESTB0( Device->CreateBuffer("GPU_BufferAppendCount", BufferDesc, &AppendBufferCount) );
		
		oGPUPipeline::DESC PipelineDesc;
		oTESTB0( oGPUTestGetPipeline(oGPU_TEST_BUFFER, &PipelineDesc) );

		intrusive_ptr<oGPUPipeline> Pipeline;
		oTESTB0( Device->CreatePipeline("GPU_BufferPipeline", PipelineDesc, &Pipeline) );

		intrusive_ptr<oGPUCommandList> CommandList;
		Device->GetImmediateCommandList(&CommandList);

		Device->BeginFrame();
		CommandList->Begin();

		CommandList->SetBlendState(ouro::gpu::blend_state::opaque);
		CommandList->SetDepthStencilState(ouro::gpu::depth_stencil_state::none);
		CommandList->SetSurfaceState(ouro::gpu::surface_state::two_sided);

		CommandList->SetRenderTargetAndUnorderedResources(nullptr, 0, nullptr, false, 0, 1, &AppendBuffer);
		CommandList->SetPipeline(Pipeline);
		CommandList->Draw(nullptr, 0, 0, nullptr, 0, oCOUNTOF(GPU_BufferAppendIndices));
		CommandList->CopyCounter(AppendBufferCount, 0, AppendBuffer);
		CommandList->Copy(AppendReadbackBuffer, AppendBuffer);

		ouro::surface::mapped_subresource ReadBack;
		oTESTB0( Device->MapRead(AppendBufferCount, 0, &ReadBack, true) );
		oTESTB(oCOUNTOF( GPU_BufferAppendIndices) == *(int*)ReadBack.data, "Append counter didn't reach %d", oCOUNTOF(GPU_BufferAppendIndices));
		Device->UnmapRead(AppendBufferCount, 0);

		oTESTB0( Device->MapRead(AppendReadbackBuffer, 0, &ReadBack, true) );

		const int* pBuffer = (const int*)ReadBack.data;
		std::vector<int> Values;
		for(int i = 0; i < oCOUNTOF(GPU_BufferAppendIndices); ++i)
			Values.push_back(GPU_BufferAppendIndices[i]);

		for(int i = 0; i < oCOUNTOF(GPU_BufferAppendIndices); ++i)
			oTESTB( find_and_erase(Values, GPU_BufferAppendIndices[i]), "GPU Appended bad value");

		Device->UnmapRead(AppendReadbackBuffer, 0);

		CommandList->End();
		Device->EndFrame();

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_Buffer);
