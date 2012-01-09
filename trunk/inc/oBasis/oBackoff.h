// $(header)
// One of the key values of Intel's TBB is its ability to avoid expensive 
// threading situations like false sharing, oversubscription and context 
// switches. One of the key detriments to TBB is that they don't expose enough
// of their awesomeness for public consumption or support enough platforms to
// truly be considered cross-platform (where's my Sony PS3 version?)

#pragma once
#ifndef oBackoff_h
#define oBackoff_h

#include <oBasis/oStdThread.h>

class oBackoff
{
	static const size_t SpinThreshold = 16;
	size_t SpinCount;

	#pragma optimize("", off)
	inline void spin(size_t _Count)
	{
		for (size_t i = 0; i < _Count; i++) {}
	}
	#pragma optimize("", on)

public:
	oBackoff()
		: SpinCount(1)
	{}

	inline void Pause()
	{
		if (SpinCount <= SpinThreshold)
		{
				spin(SpinCount);
				SpinCount *= 2;
		}

		else
			oStd::this_thread::yield();
	}

	inline bool TryPause()
	{
		if (SpinCount <= SpinThreshold)
		{
			spin(SpinCount);
			SpinCount *= 2;
			return true;
		}

		return false;
	}

	inline void Reset() { SpinCount = 1; }
};

#endif
