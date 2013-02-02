/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
// Thin wrapper for Std::thread that provides threadsafe specification 
// to reduce thread_cast throughout the code. Prefer using these 
// wrappers over using std::thread directly.
#pragma once
#ifndef oThread_h
#define oThread_h

#include <oBasis/oGUID.h>
#include <oBasis/oStdThread.h>
#include <oBasis/oThreadsafe.h>

class oThread
{
public:
	typedef oStd::thread::id id;

	oThread() {}
	oDEFINE_CALLABLE_CTOR_WRAPPERS(explicit, oThread, initialize);

	~oThread() {}

	#ifdef oHAS_MOVE_CTOR
		oThread(oThread&& _That) { *this = std::move(_That); }
		oThread& operator=(oThread&& _That) { Thread = std::move(_That.Thread); return *this; }
	#else
		void move_ctor(oThread& _That) { Thread.move_ctor(_That.Thread); }
		oThread& move_operator_eq(oThread& _That) { Thread.move_operator_eq(_That.Thread); return *this; }
	#endif

	void swap(threadsafe oThread& _That) threadsafe { T().swap(_That.T()); }
	void join() threadsafe { T().join(); }
	void detach() threadsafe { T().detach(); }
	bool joinable() const threadsafe { return T().joinable(); }
	oStd::thread::id get_id() const threadsafe { return T().get_id(); }
	typedef oStd::thread::native_handle_type native_handle_type;
	native_handle_type native_handle() threadsafe { return T().native_handle(); }
	static unsigned int hardware_concurrency() { return oStd::thread::hardware_concurrency(); }

private:
	oStd::thread Thread;
	oStd::thread& T() threadsafe { return thread_cast<oStd::thread&>(Thread); }
	const oStd::thread& T() const threadsafe { return thread_cast<oStd::thread&>(Thread); }
	#ifdef oHAS_MOVE_CTOR
		oThread(const oThread&); /* = delete */
		const oThread& operator=(const oThread&); /* = delete */
	#endif
	inline void initialize(oCALLABLE _ThreadProc) { oStd::thread t(_ThreadProc); Thread = std::move(t); }
};

inline unsigned int oAsUint(const oStd::thread::id& _ID) { return *(unsigned int*)&_ID; }

// the standard is a bit too obtuse for sleeping a thread, so wrap it
inline void oSleep(unsigned int _Milliseconds) { oStd::this_thread::sleep_for(oStd::chrono::milliseconds(_Milliseconds)); }

// For allocating buffers assigned to a thread_local pointer. Because there can 
// be separate instances of a thread_local in different modules (DLLs), the GUID
// must be specified. This function allocates only once for a given GUID and 
// returns true, indicating that any first-time init/construction should take
// place. For any subsequent/concurrent allocation the GUID is resolved to that 
// same pointer so thread_local values across DLLs are all pointing to the same 
// memory, and this returns false indicating that no initialization of the 
// memory should occur. This function will report a link error because enforcing 
// these rules requires a platform-specific implementation. This will 
// automatically register a oThreadAtExit call to free the memory. It is further 
// recommended that a user oThreadAtExit be registered by client code to destroy
// or deinit the constructed memory, but only when oThreadlocalMalloc returns
// true.
bool oThreadlocalMalloc(const oGUID& _GUID, size_t _Size, void** _ppAllocation);
template<typename T> bool oThreadlocalMalloc(const oGUID& _GUID, T** _ppAllocation) { return oThreadlocalMalloc(_GUID, sizeof(T), (void**)_ppAllocation); }

// This function is not defined by this library as executing code at the end of 
// a thread requires platform-specific API, so implementation is left to client
// code. This function should register the specified oFUNCTION to be run just 
// before the current thread exits. More than one function can be registered.
void oThreadAtExit(std::function<void()> _AtExit);
oDEFINE_CALLABLE_WRAPPERS(oThreadAtExit,, oThreadAtExit);

// This should be called as the last line of a thread proc that will flush 
// oThreadlocalMalloc allocations and oThreadAtExit functions. This is not 
// defined by this library, but should be implemented by platform code to ensure
// user threads exit cleanly.
void oEndThread();

#endif
