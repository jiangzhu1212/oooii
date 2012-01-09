// $(header)
// The benchmark code used on John Ratcliff's code supository codebase
// It's MIT, so safe to use.
// http://codesuppository.blogspot.com/
// http://code.google.com/p/codesuppository/
#pragma once
#ifndef RatcliffJobSwarmTest_h
#define RatcliffJobSwarmTest_h

#include <oBasis/oInterface.h>

interface oDispatchQueue;

namespace RatcliffJobSwarm {

bool RunDispatchQueueTest(const char* _Name, threadsafe oDispatchQueue* _pDispatchQueue);
bool RunParallelForTest(const char* _Name);

} // namespace RatcliffJobSwarm

#endif
