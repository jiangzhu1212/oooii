// $(header)
// Implementation of a concurrent dispatch queue (tasks execute on any thread at
// any time, in any order) using Windows Threadpools. This is mostly used as a 
// benchmark for oDispatchQueueConcurrent.h and any other middleware such as TBB.
#pragma once
#ifndef oThreadpoolWTP_h
#define oThreadpoolWTP_h

#include <oBasis/oDispatchQueueConcurrent.h>

bool oDispatchQueueCreateConcurrentWTP(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppDispatchQueue);

#endif
