// $(header)
// A dispatch queue that executes all enqueued tasks in order on a specific 
// thread. This is most useful when there are platform requirements such as 
// Window's window handling that must occur all in the same thread.
#ifndef oDispatchQueueGlobal_h
#define oDispatchQueueGlobal_h

#include <oBasis/oDispatchQueue.h>

interface oDispatchQueueGlobal : oDispatchQueue
{
};

oAPI bool oDispatchQueueCreateGlobal(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueGlobal** _ppDispatchQueue);

#endif
