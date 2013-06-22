/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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

int GPU_BufferAppendIndices[20] = 
{ 5, 6, 7, 18764, 2452, 2423, 52354, 344, -1542, 3434, 53, -4535, 3535, 88884747, 534535, 88474, -445, 4428855, -1235, 4661};

struct GPU_Buffer : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPUDevice::INIT init("GPU_Buffer");
		init.DriverDebugLevel = oGPU_DEBUG_NORMAL;
		oRef<oGPUDevice> Device;
		oTESTB0(oGPUDeviceCreate(init, &Device));

		oGPUBuffer::DESC BufferDesc;
		BufferDesc.StructByteSize = sizeof(int);
		BufferDesc.Type = oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND;
		BufferDesc.ArraySize = oCOUNTOF(GPU_BufferAppendIndices) * 2;

		oRef<oGPUBuffer> AppendBuffer;
		oTESTB0( Device->CreateBuffer("GPU_BufferAppend", BufferDesc, &AppendBuffer) );

		BufferDesc.Type = oGPU_BUFFER_READBACK;

		oRef<oGPUBuffer> AppendReadbackBuffer;
		oTESTB0( Device->CreateBuffer("GPU_BufferAppend", BufferDesc, &AppendReadbackBuffer) );

		BufferDesc.Type = oGPU_BUFFER_READBACK;
		BufferDesc.ArraySize = 1;

		oRef<oGPUBuffer> AppendBufferCount;
		oTESTB0( Device->CreateBuffer("GPU_BufferAppendCount", BufferDesc, &AppendBufferCount) );
		
		oGPUPipeline::DESC PipelineDesc;
		oTESTB0( oGPUTestGetPipeline(oGPU_TEST_BUFFER, &PipelineDesc) );

		oRef<oGPUPipeline> Pipeline;
		oTESTB0( Device->CreatePipeline("GPU_BufferPipeline", PipelineDesc, &Pipeline) );

		oRef<oGPUCommandList> CommandList;
		Device->GetImmediateCommandList(&CommandList);

		Device->BeginFrame();
		CommandList->Begin();

		CommandList->SetBlendState(oGPU_OPAQUE);
		CommandList->SetDepthStencilState(oGPU_DEPTH_STENCIL_NONE);
		CommandList->SetSurfaceState(oGPU_TWO_SIDED);

		CommandList->SetRenderTargetAndUnorderedResources(nullptr, 0, nullptr, false, 0, 1, &AppendBuffer);
		CommandList->SetPipeline(Pipeline);
		CommandList->Draw(nullptr, 0, 0, nullptr, 0, oCOUNTOF(GPU_BufferAppendIndices));
		CommandList->CopyCounter(AppendBufferCount, 0, AppendBuffer);
		CommandList->Copy(AppendReadbackBuffer, AppendBuffer);

		oSURFACE_MAPPED_SUBRESOURCE ReadBack;
		oTESTB0( Device->MapRead(AppendBufferCount, 0, &ReadBack, true) );
		oTESTB(oCOUNTOF( GPU_BufferAppendIndices) == *(int*)ReadBack.pData, "Append counter didn't reach %d", oCOUNTOF(GPU_BufferAppendIndices));
		Device->UnmapRead(AppendBufferCount, 0);

		oTESTB0( Device->MapRead(AppendReadbackBuffer, 0, &ReadBack, true) );

		const int* pBuffer = (const int*)ReadBack.pData;
		std::vector<int> Values;
		for(int i = 0; i < oCOUNTOF(GPU_BufferAppendIndices); ++i)
			Values.push_back(GPU_BufferAppendIndices[i]);

		for(int i = 0; i < oCOUNTOF(GPU_BufferAppendIndices); ++i)
			oTESTB( oStd::find_and_erase(Values, GPU_BufferAppendIndices[i]), "GPU Appended bad value");

		Device->UnmapRead(AppendReadbackBuffer, 0);

		CommandList->End();
		Device->EndFrame();

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_Buffer);
