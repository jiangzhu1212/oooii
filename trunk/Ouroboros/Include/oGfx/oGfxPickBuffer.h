// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// PickBuffer
#pragma once
#ifndef oGfxPickBuffer_h
#define oGfxPickBuffer_h

#include <oGPU/device.h>

static const uint oGPU_MAX_NUM_PICKS_PER_FRAME = 16;

// {B83197F4-6064-4098-8EDA-042D554AD146}
oDEFINE_GUID_S(oGfxPickBuffer, 0xb83197f4, 0x6064, 0x4098, 0x8e, 0xda, 0x4, 0x2d, 0x55, 0x4a, 0xd1, 0x46);
struct oGfxPickBuffer : oInterface
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGfxPickBuffer);

	oGfxPickBuffer(ouro::gpu::device* _pDevice, const void* _pComputeShader, bool* bSuccess);

	void PIMap(ouro::gpu::command_list* _pCommandList, int2** _Picks);
	void PIUnmap(ouro::gpu::command_list* _pCommandList);
	//void PDraw(ouro::gpu::command_list* _pCommandList, ouro::gpu::texture1* _pPickRenderTargetTexture);
	//Map outputs from the pick shader. This must be an immediate context. And if a deferred context was
	//	used for any other functions in this class, those must be submitted before mapping the outputs.
	void POMap(uint** _Picks);
	void POUnmap();

private:
	oRefCount RefCount;
	int2 PicksInputBuffer[oGPU_MAX_NUM_PICKS_PER_FRAME];
	//std::shared_ptr<ouro::gpu::texture1> PicksInput;
#if 0
	std::shared_ptr<ouro::gpu::buffer> PicksOutput;
	std::shared_ptr<ouro::gpu::buffer> PicksStaging;
	std::shared_ptr<ouro::gpu::compute_kernel> PickResourceShader;
#endif
};

bool oGfxPickBufferCreate(ouro::gpu::device* _pDevice, const void* _pComputeShader, oGfxPickBuffer** _ppPickBuffer);

#endif // oGfxPickBuffer_h
