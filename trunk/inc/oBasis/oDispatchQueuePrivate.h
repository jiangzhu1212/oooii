// $(header)
// A dispatch queue that executes all enqueued tasks in order on a specific 
// thread. This is most useful when there are platform requirements such as 
// Window's window handling that must occur all in the same thread.
#ifndef oDispatchQueuePrivate_h
#define oDispatchQueuePrivate_h

#include <oBasis/oDispatchQueue.h>

oAPI bool oDispatchQueueCreatePrivate(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueue** _ppDispatchQueue);

#endif
