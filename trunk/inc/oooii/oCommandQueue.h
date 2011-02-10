#pragma once
#ifndef oCommandQueue_h
#define oCommandQueue_h

#include "oThreading.h"
#include <list>

// Issues commands in order asynchronously 
class oCommandQueue
{
public:
	oCommandQueue() :
	  Enabled(true)
	  {
	  }

	  void AddCommand(oFUNCTION<void()> _Command) threadsafe
	  {
		  // Queue can be disabled during a drain event
		  if( !Enabled )
			  return;

		  // Add this command to the queue
		  ConsumerLock.Lock();
		  thread_cast<oCommandQueue*>(this)->Commands.push_back(_Command);
		  size_t CurrentCommandCount = thread_cast<oCommandQueue*>(this)->Commands.size();
		  ConsumerLock.Unlock();

		  // If this command is the only one in the queue kick of the execution
		  if( 1 == CurrentCommandCount )
		  {
			  LaunchExecute( _Command );
		  }
	  } 

	  void Drain( bool _ReEnable = false)
	  {
		  // This has the potential to deadlock but we need to make certain there are no commands in flight before tearing down
		  // higher level code should ensure against deadlock
		  Enabled = false;		 
		  while( !Commands.empty() )
		  {
			  oYield();
		  }
		  Enabled = _ReEnable;
	  }

	  ~oCommandQueue()
	  {
			Drain();
	  }

private:

	void LaunchExecute(oFUNCTION<void()> _Command) threadsafe
	{
		oIssueAsyncTask( oBIND(&oCommandQueue::Execute, this, _Command) );
	}
	void Execute(oFUNCTION<void()> _Command)
	{
		// Execute and clear the command
		_Command();
		_Command = NULL;

		// Push the command off the list and execute the next one if available
		ConsumerLock.Lock();
		Commands.pop_front();
	
		if( !Commands.empty() )
		{
			LaunchExecute( Commands.front() );
		}
		ConsumerLock.Unlock();
	}
	std::list<oFUNCTION<void()>> Commands;
	oRWMutex ConsumerLock;
	volatile bool Enabled;
};


#endif // oCommandQueue_h