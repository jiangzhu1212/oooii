// $(header)
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
	oStd::thread Thread;
	oStd::thread& T() threadsafe { return thread_cast<oStd::thread&>(Thread); }
	const oStd::thread& T() const threadsafe { return thread_cast<oStd::thread&>(Thread); }
	#ifdef oHAS_MOVE_CTOR
		oThread(const oThread&); /* = delete */
		const oThread& operator=(const oThread&); /* = delete */
	#endif
public:
	oThread() {}
	#ifndef oHAS_VARIADIC_TEMPLATES
		oCALLABLE_TEMPLATE0 explicit oThread(oCALLABLE_PARAMS0) : Thread(oCALLABLE_BIND0) {}
		oCALLABLE_TEMPLATE1 explicit oThread(oCALLABLE_PARAMS1) : Thread(oCALLABLE_BIND1) {}
		oCALLABLE_TEMPLATE2 explicit oThread(oCALLABLE_PARAMS2) : Thread(oCALLABLE_BIND2) {}
		oCALLABLE_TEMPLATE3 explicit oThread(oCALLABLE_PARAMS3) : Thread(oCALLABLE_BIND3) {}
		oCALLABLE_TEMPLATE4 explicit oThread(oCALLABLE_PARAMS4) : Thread(oCALLABLE_BIND4) {}
		oCALLABLE_TEMPLATE5 explicit oThread(oCALLABLE_PARAMS5) : Thread(oCALLABLE_BIND5) {}
		oCALLABLE_TEMPLATE6 explicit oThread(oCALLABLE_PARAMS6) : Thread(oCALLABLE_BIND6) {}
		oCALLABLE_TEMPLATE7 explicit oThread(oCALLABLE_PARAMS7) : Thread(oCALLABLE_BIND7) {}
		oCALLABLE_TEMPLATE8 explicit oThread(oCALLABLE_PARAMS8) : Thread(oCALLABLE_BIND8) {}
		oCALLABLE_TEMPLATE9 explicit oThread(oCALLABLE_PARAMS9) : Thread(oCALLABLE_BIND9) {}
		oCALLABLE_TEMPLATE10 explicit oThread(oCALLABLE_PARAMS10) : Thread(oCALLABLE_BIND10) {}
	#endif
	~oThread() {}

	void move_ctor(oThread& _This, oThread& _That) { if (&_This != &_That) T() = std::move(_That.T()); }
	oThread& move_operator_eq(oThread& _That) { move_ctor(*this, _That); return *this; }
	#ifdef oHAS_MOVE_CTOR
		oThread(oThread&& _That) { move_ctor(*this, _That); }
		oThread& operator=(oThread&& _That) { return move_operator_eq(_That); }
	#endif

	void swap(threadsafe oThread& _That) threadsafe { T().swap(_That.T()); }
	void join() threadsafe { T().join(); }
	void detach() threadsafe { T().detach(); }
	bool joinable() const threadsafe { return T().joinable(); }
	oStd::thread::id get_id() const threadsafe { return T().get_id(); }
	typedef oStd::thread::native_handle_type native_handle_type;
	native_handle_type native_handle() threadsafe { return T().native_handle(); }
	static unsigned int hardware_concurrency() { return oStd::thread::hardware_concurrency(); }

	typedef oStd::thread::id id;
};

inline unsigned int oAsUint(const oStd::thread::id& _ID) { return *(unsigned int*)&_ID; }

// the standard is a bit too obtuse for sleeping a thread, so wrap it
inline void oSleep(unsigned int _Milliseconds) { oStd::this_thread::sleep_for(oStd::chrono::milliseconds(_Milliseconds)); }

// For allocating buffers assigned to a thread_local pointer. Because there can 
// be separate instances of a thread_local in different modules (DLLs), the GUID
// must be specified. This function should allocate only once for a given GUID
// and for any subsequent/concurrent allocation resolve to that same pointer so
// that thread_local values across DLLs are all pointing to the same memory.
// This function will report a link error because enforcing these rules requires
// a platform-specific implementation. It is also recommended that an 
// oThreadAtExit() call be registered to free that memory at the time of 
// allocation.
void* oThreadlocalMalloc(const oGUID& _GUID, size_t _Size);

// This function is not defined by this library as executing code at the end of 
// a thread requires platform-specific API, so implementation is left to client
// code. This function should register the specified oFUNCTION to be run just 
// before the current thread exits. More than one function can be registered.
void oThreadAtExit(std::function<void()> _AtExit);

// This should be called as the last line of a thread proct that will flush 
// oThreadlocalMalloc allocations and oThreadAtExit functions. This is not 
// defined by this library, but should be implemented by platform code to ensure
// user threads exit cleanly.
void oEndThread();

#endif
