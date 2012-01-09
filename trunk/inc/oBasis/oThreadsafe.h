// $(header)
// Policy addition/clarification to C++. The volatile keyword has no semantic on 
// class methods, so co-opt it into a limiter of when the method can be called. 
// Like non-const methods cannot be called by const pointers/references to a 
// class, non-volatile methods cannot be called by volatile pointers/references. 
// So marking a class pointer/reference as volatile marks the class as only 
// being allowed to call volatile methods without special handling. Rename 
// volatile 'threadsafe' and you have a well-documented mechanism to mark 
// classes that threadsafety is a consideration and make it clear which methods
// are safe to call and which are not.
#pragma once
#ifndef oThreadsafe_h
#define oThreadsafe_h

#ifndef threadsafe
	#define threadsafe volatile
#endif

// Differentiate thread_cast from const_cast so it's easier to grep the code 
// looking for situations where the threadsafe qualifier was cast away.
template<typename T, typename U> inline T thread_cast(const U& threadsafeObject) { return const_cast<T>(threadsafeObject); }

#endif
