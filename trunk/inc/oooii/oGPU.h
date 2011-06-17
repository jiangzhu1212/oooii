// $(header)
#pragma once
#ifndef oGPU_h
#define oGPU_h

namespace oGPU {

struct DESC
{
	char Description[128];
	size_t VRAM;
	size_t DedicatedSystemMemory;
	size_t SharedSystemMemory;
	unsigned int Index;
	float D3DVersion;
};

// Returns false if the specified GPU doesn't exist.
bool GetDesc(unsigned int _GPUIndex, DESC* _pDesc);

} // namespace oGPU

#endif
