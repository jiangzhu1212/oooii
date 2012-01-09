// $(header)

// oAllocator implementation using the two-level segregated fit allocation 
// algorithm. Read more at http://tlsf.baisoku.org/ 
#pragma once
#ifndef oAllocatorTLSF_h
#define oAllocatorTLSF_h

#include <oBasis/oAllocator.h>

bool oAllocatorCreateTLSF(const char* _DebugName, const oAllocator::DESC& _Desc, oAllocator** _ppAllocator);

#endif
