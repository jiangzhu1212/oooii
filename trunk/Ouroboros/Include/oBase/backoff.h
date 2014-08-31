// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// A useful concept for minimizing locks from TBB source: this object 
// encapsulates a temporary spin lock.
#pragma once
#ifndef oBase_backoff_h
#define oBase_backoff_h

#include <thread>

namespace ouro {

class backoff
{
public:
	backoff();

	// A spin that degrades (spins longer) with each call until it makes more 
	// sense to yield.
	void pause();

	// A spin that degrades (spins longer) with each call (returning true) until 
	// it would yield, then it returns false without yielding.
	bool try_pause();

	// Resets the degradation of the spin loop.
	void reset();

private:
	static const size_t SpinThreshold = 16;
	size_t SpinCount;
	void spin(size_t _Count);
};

inline backoff::backoff()
	: SpinCount(1)
{}

#pragma optimize("", off)
inline void backoff::spin(size_t _Count)
{
	for (size_t i = 0; i < _Count; i++) {}
}
#pragma optimize("", on)

inline void backoff::pause()
{
	if (SpinCount <= SpinThreshold)
	{
		spin(SpinCount);
		SpinCount *= 2;
	}

	else
		std::this_thread::yield();
}

inline bool backoff::try_pause()
{
	if (SpinCount <= SpinThreshold)
	{
		spin(SpinCount);
		SpinCount *= 2;
		return true;
	}

	return false;
}

inline void backoff::reset()
{
	SpinCount = 1;
}

}

#endif
