// $(header)
// A dispatch queue that runs tasks on any thread at any time (no order 
// guarantees). Most likely client code should prefer oParallelFor over using 
// this type of dispatch queue directly. This is a custom implementation using
// one oConcurrentQueue to feed worker threads and exists to:
// A. hammer oConcurrentQueue hard in test cases so that if new queue 
//    implementations are used we have a small test usage case where contension 
//    is high.
// B. This serves as a benchmark. How much faster is TBB or Windows Threadpool? 
//    Well here's the point of reference.
// C. This serves as a starting point for learning. When teaching task-based 
//    systems to Concurrency newcomers, this is relatively simple to understand
//    compared to digging through TBB source. Also someone can modify this code
//    to see if something smarter can be done, or more often how doing something
//    thought to be smarter can easily result in performance degradation.
#pragma once
#ifndef oThreadpool_h
#define oThreadpool_h

#include <oBasis/oDispatchQueue.h>

interface oDispatchQueueConcurrent : oDispatchQueue
{
};

bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppDispatchQueue);

#endif
