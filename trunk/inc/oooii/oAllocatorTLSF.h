// $(header)

// oAllocator implementation using the two-level segregated fit
// allocation algorithm. Read more at http://tlsf.baisoku.org/ 
#pragma once
#ifndef oAllocatorTLSF_h
#define oAllocatorTLSF_h

#include <oooii/oAllocator.h>

interface oAllocatorTLSF : public oAllocator
{
	static bool Create(const char* _DebugName, const DESC* _pDesc, oAllocator** _ppAllocator);
};

#endif
