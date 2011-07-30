// $(header)

// Atomic operations (CAS, SWAP return OLD value; INC, DEC return NEW value)
#pragma once
#ifndef oAtomic_h
#define oAtomic_h

#include <oooii/oIntrinsic.h>
#include <oooii/oStddef.h>

#if defined(_WIN32) || defined(_WIN64)

oFORCEINLINE void oReadBarrier() { _ReadBarrier(); }
oFORCEINLINE void oReadWriteBarrier() { _ReadWriteBarrier(); }

inline void* oCAS(void* volatile* ptr, void* newVal, void* oldVal) { return _InterlockedCompareExchangePointer(ptr, newVal, oldVal); }
template<typename T> T oCAS(T volatile* ptr, T newVal, T oldVal) { return (T)_InterlockedCompareExchangePointer((void* volatile*)ptr, static_cast<void*>(newVal), static_cast<void*>(oldVal)); }

inline void* oSWAP(void* volatile* ptr, void* newVal) { return _InterlockedExchangePointer(ptr, newVal); }
template<typename T> T oSWAP(T volatile* ptr, T newVal) { return (T)_InterlockedExchangePointer((void* volatile*)ptr, static_cast<void*>(newVal)); }

inline int oINC(volatile int* ptr) { return _InterlockedIncrement((long*)ptr); }
inline int oDEC(volatile int* ptr) { return _InterlockedDecrement((long*)ptr); }
inline int oCAS(volatile int* ptr, int newVal, int oldVal) { return _InterlockedCompareExchange((long*)ptr, newVal, oldVal); }
inline int oSWAP(volatile int* ptr, int newVal) { return _InterlockedExchange((long*)ptr, newVal); }
inline unsigned int oINC(volatile unsigned int* ptr) { return (unsigned int)oINC((int*)ptr); }
inline unsigned int oDEC(volatile unsigned int* ptr) { return (unsigned int)oDEC((int*)ptr); }
inline unsigned int oCAS(volatile unsigned int* ptr, unsigned int newVal, unsigned int oldVal) { return (unsigned int)oCAS((int*)ptr, *(int*)&newVal, *(int*)&oldVal); }
inline unsigned int oSWAP(volatile unsigned int* ptr, unsigned int newVal) { return (unsigned int)oSWAP((int*)ptr, newVal); }

#ifdef _WIN64
	inline long long oINC(volatile long long* ptr) { return _InterlockedIncrement64((__int64*)ptr); }
	inline long long oDEC(volatile long long* ptr) { return _InterlockedDecrement64((__int64*)ptr); }
	inline long long oCAS(volatile long long* ptr, long long newVal, long long oldVal) { return (unsigned int)_InterlockedCompareExchange64((long long*)ptr, newVal, oldVal); }
	inline long long oSWAP(volatile long long* ptr, long long newVal) { return _InterlockedExchange64((long long*)ptr, newVal); }
	inline unsigned long long oINC(volatile unsigned long long* ptr) { return (unsigned long long)_InterlockedIncrement64((__int64*)ptr); }
	inline unsigned long long oDEC(volatile unsigned long long* ptr) { return (unsigned long long)_InterlockedDecrement64((__int64*)ptr); }
	inline unsigned long long oCAS(volatile unsigned long long* ptr, unsigned long long newVal, unsigned long long oldVal) { return _InterlockedCompareExchange64((long long*)ptr, newVal, oldVal); }
	inline unsigned long long oSWAP(volatile unsigned long long * ptr, unsigned long long newVal) { return (unsigned long long)_InterlockedExchange64((long long*)ptr, newVal); }
#endif
#else
	#error Unsupported platform (atomics)
#endif

inline bool oREF_RELEASE(volatile int* _refcount)
{
	// Start with classic atomic ref then test for 0, but then mark the count as 
	// garbage to prevent any quick inc/dec (ABA issue) in the body of the calling 
	// if() that is testing this release's result.
	static const int sFarFromZero = (1<<((sizeof(int)*8)-1)) / 2;
	int newRef = oDEC(_refcount);
	return (newRef == 0 && oCAS(_refcount, sFarFromZero, 0) == 0);
}

#endif
