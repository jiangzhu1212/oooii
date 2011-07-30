// $(header)

// Utility for querying the Central Processing Unit hardware
// of the current computer.
#pragma once
#ifndef oCPU_h
#define oCPU_h

namespace oCPU {

enum TYPE
{
	UNKNOWN,
	X86,
	X64,
	IA64,
};

enum CACHE_TYPE
{
	DATA,
	INSTRUCTION,
};

struct CACHE_DESC
{
	unsigned int Size;
	unsigned int LineSize;
	unsigned int Associativity;
};

struct DESC
{
	TYPE Type;
	unsigned int NumProcessors;
	unsigned int NumProcessorPackages;
	unsigned int NumNumaNodes;
	unsigned int NumHardwareThreads;
	unsigned int SSEVersion;
	CACHE_DESC DataCacheDescs[3];
	CACHE_DESC InstructionCacheDescs[3];
	bool HasX87FPU;
	bool Has8ByteAtomicSwap;
	bool HasHyperThreading;
	char String[32];
	char BrandString[64];
};

// Returns false if the specified CPU doesn't exist.
bool GetDesc(unsigned int _CPUIndex, DESC* _pDesc);

} // namespace oCPU

#endif
