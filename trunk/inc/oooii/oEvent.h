// $(header)

// Interface for a typical multi-threaded sync event. This also has supported
// for multi-process synchronization through the use of named events.
#pragma once
#ifndef oEvent_h
#define oEvent_h

#include <oooii/oNoncopyable.h>
#include <oooii/oStddef.h>

class oEvent : oNoncopyable
{
	void* hEvent;
	#ifdef _DEBUG
		char DebugName[64];
	#endif
public:

	// It is advised to almost always assign a debug name so that triggered events 
	// can be more well understood while debugging many-threaded applications.
	// Specify _InterProcessName to create a system-global event that different
	// processes can attach to by each creating an event of the same name.

	// Two separate ctors are required because compiler can decide to convert a 
	// char pointer to a bool causing unexpected behavior
	oEvent(bool _AutoReset, const char* _DebugName = 0, const char* _InterProcessName = 0);
	oEvent(const char* _DebugName = 0, const char* _InterProcessName = 0);
	~oEvent();
	inline void* GetNativeHandle() threadsafe { return hEvent; }
	inline const char* GetDebugName() const threadsafe
	{
		#ifdef _DEBUG
			return thread_cast<const char*>(DebugName);
		#else
			return "";
		#endif
	}

	void Set() threadsafe;
	void Reset() threadsafe;

	// Returns false if timed out
	bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe;

	// Returns TIMED_OUT if timed out, else the index of the event that broke the wait
	static size_t WaitMultiple(threadsafe oEvent** _ppEvents, size_t _NumEvents, bool _WaitAll = true, unsigned int _TimeoutMS = oINFINITE_WAIT);
private:
	void CommonConstructor(bool _AutoReset, const char* _DebugName, const char* _InterProcessName);
};

#endif
