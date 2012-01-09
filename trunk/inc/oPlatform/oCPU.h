// $(header)

// Utility for querying the Central Processing Unit hardware
// of the current computer.
#pragma once
#ifndef oCPU_h
#define oCPU_h

enum oCPU_TYPE
{
	oCPU_UNKNOWN,
	oCPU_X86,
	oCPU_X64,
	oCPU_IA64,
};

struct oCPU_CACHE_DESC
{
	unsigned int Size;
	unsigned int LineSize;
	unsigned int Associativity;
};

struct oCPU_DESC
{
	oCPU_TYPE Type;
	unsigned int NumProcessors;
	unsigned int NumProcessorPackages;
	unsigned int NumNumaNodes;
	unsigned int NumHardwareThreads;
	unsigned int SSEVersion;
	oCPU_CACHE_DESC DataCacheDescs[3];
	oCPU_CACHE_DESC InstructionCacheDescs[3];
	bool HasX87FPU;
	bool Has8ByteAtomicSwap;
	bool HasHyperThreading;
	char String[32];
	char BrandString[64];
};

// Returns false if the specified CPU doesn't exist.
bool oCPUEnum(unsigned int _Index, oCPU_DESC* _pDesc);

#endif
