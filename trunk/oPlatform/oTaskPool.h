// $(header)
// It is common in threadpool implementations that the queue needs to 
// retain the specified oFUNCTION, but the underlying system API still
// takes a raw function pointer. To support oFUNCTION in these 
// interfaces, a new instance must be allocated, so here's a cache of
// those to speed up the allocation process.
#pragma once
#ifndef oTaskPool_h
#define oTaskPool_h

#include <oBasis/oFunction.h>

class oTask* oTaskPoolAllocate(oTASK _Task);
void oTaskPoolDeallocate(class oTask* _pTask);

class oTask
{
	oTASK Task;
	oDECLARE_NEW_DELETE();
	oTask(oTASK _Task) : Task(_Task) {}
	~oTask() {}

	friend oTask* oTaskPoolAllocate(oTASK _Task);
	friend void oTaskPoolDeallocate(oTask* _pTask);

public:
	inline void operator()() { Task(); oTaskPoolDeallocate(this); }
};

#endif
