// $(header)
#pragma once
#ifndef oGPU_h
#define oGPU_h

struct oGPU_DESC
{
	char Description[128];
	size_t VRAM;
	size_t DedicatedSystemMemory;
	size_t SharedSystemMemory;
	unsigned int Index;
	float D3DVersion;
};

// Returns false if the specified GPU doesn't exist.
bool oGPUEnum(unsigned int _Index, oGPU_DESC* _pDesc);

#endif
