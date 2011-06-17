// Functions to help in making sure files written are the same in debug and release builds. In particular unwritten parts of descriptions can have fixed values in debug, and random values in release.
#pragma once
#ifndef oSerialization_h
#define oSerialization_h

namespace oSerialization
{
	static const int MEMORY_INIT_VALUE = 0xBB; //so we have a different code other than the microsoft codes
		
	inline void oInitForSerialization(void *_arg, size_t _size)
	{
		memset(_arg, MEMORY_INIT_VALUE, _size);		
	}

	template<typename T>
	void oInitForSerialization(T &_arg)
	{
		oInitForSerialization((void*)(&_arg), sizeof(_arg));
		new(&_arg) T;
	}
	
	class DisableCRTMemoryInit //doesn't do anything in release builds. otherwise, it will keep secure crt functions from filling buffers with 0xFE while alive.
	{
	public:
		DisableCRTMemoryInit();
		~DisableCRTMemoryInit();
	};
}

#endif