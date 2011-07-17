#pragma once

// Enqueues commands such that they are executed in the order they were enqueued,
// A single thread will be created, and all commands will execute on this new thread.
// Should almost always use oCommandQueue instead of this class. Use this class
// only if you have a really good reason to want commands to always be executed
// from the same thread.
#ifndef oCommandQueueSingleThread_h
#define oCommandQueueSingleThread_h

#include <oooii/oCommandQueue.h>
#include <oooii/oThread.h>
#include <oooii/oRef.h>
#include <oooii/oEvent.h>

class oCommandQueueProc;

struct oCommandQueueSingleThread : public oCommandQueue
{
	oCommandQueueSingleThread();
	~oCommandQueueSingleThread();

private:
	void IssueCommand() threadsafe override;
	oRef<oCommandQueueProc> ThreadProc;
	oRef<threadsafe oThread> Thread;
};

#endif
