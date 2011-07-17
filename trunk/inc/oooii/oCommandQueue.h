// $(header)
// Enqueues commands such that they are executed in the order they were enqueued,
// but a command could execute at any time on any thread.
#pragma once
#ifndef oCommandQueue_h
#define oCommandQueue_h

#include <oooii/oMutex.h>
#include <list>

struct oCommandQueue
{
	oCommandQueue();
	~oCommandQueue();

	// Enqueue a command to be executed
	void Enqueue(oFUNCTION<void()> _Command) threadsafe;

	// Block until all commands enqueued are executed. If _AllowEnqueues is true,
	// then Enqueue() calls will append to the command list and thus this Flush
	// might never return. If _AllowEnqueues is false, then Enqueue calls are 
	// ignored.
	void Flush(bool _AllowEnqueues = false) threadsafe;
protected:
	void ExecuteNext() threadsafe;
	bool Empty();
	void Disable() {Enabled = false;}
private:
	virtual void IssueCommand() threadsafe;
	std::list<oFUNCTION<void()>> Commands;
	oRWMutex ConsumerLock;
	volatile bool Enabled;
};

#endif
