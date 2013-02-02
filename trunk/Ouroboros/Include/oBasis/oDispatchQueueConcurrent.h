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
// A dispatch queue that runs tasks on any thread at any time (no order 
// guarantees). Most likely client code should prefer oParallelFor over using 
// this type of dispatch queue directly. This is a custom implementation using
// one oConcurrentQueue to feed worker threads and exists to:
// A. hammer oConcurrentQueue hard in test cases so that if new queue 
//    implementations are used we have a small test usage case where contention 
//    is high.
// B. This serves as a benchmark. How much faster is TBB or Windows Threadpool? 
//    Well here's the point of reference.
// C. This serves as a starting point for learning. When teaching task-based 
//    systems to Concurrency newcomers, this is relatively simple to understand
//    compared to digging through TBB source. Also someone can modify this code
//    to see if something smarter can be done, or more often how doing something
//    thought to be smarter can easily result in performance degradation.
#pragma once
#ifndef oDispatchQueueConcurrent_h
#define oDispatchQueueConcurrent_h

#include <oBasis/oDispatchQueue.h>

interface oDispatchQueueConcurrent : oDispatchQueue
{
};

bool oDispatchQueueCreateConcurrentGeneric(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppDispatchQueue);

// Like oTask, this API is generally an abstraction for an underlying task-based
// scheduler. Such implementations are still platform-specific. (though since TBB 
// is so cross-platform and so mature and so radically better in performance 
// than other options, promoting it to oBasis is an increasingly likely option)
// To allow task-based code to be promoted to oBasis while still allowing 
// platform-specific implementations, this declaration is left dangling with no
// implementation. In a pinch, oDispatchQueueCreateConcurrentGeneric can be 
// used, but probably TBB or PPL or Grand Central or the like should be 
// implemented.
bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppDispatchQueue);

#endif
