// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// This is an abstraction for the implementations required to run unit tests.
// Libraries in Ouroboros are broken up into dependencies on each other and on 
// system resources. For example oBase is dependent on C++11/compiler features 
// and oCore is dependent on non-standard operating system API. To be able to 
// test a library sometimes requires extra features not available directly to 
// the library so to keep each library isolated to the point it can be used in 
// a different library without other higher-level Ouroboros libraries and expose 
// an abstract interface for enabling the unit tests - it would be up to client 
// code to implement these interfaces.

#pragma once
#include <oMemory/allocate.h>
#include <oSurface/image.h>
#include <chrono>
#include <cstdarg>
#include <functional>
#include <memory>
#include <stdexcept>

#define oTEST_THROW(msg, ...) do { services.error(msg, ## __VA_ARGS__); } while(false)
#define oTEST(expr, msg, ...) do { if (!(expr)) services.error(msg, ## __VA_ARGS__); } while(false)
#define oTEST0(expr) do { if (!(expr)) services.error("\"" #expr "\" failed"); } while(false)

namespace ouro {

namespace surface { class buffer; }

class test_services
{
public:

	class finally
	{
	public:
		explicit finally(std::function<void()>&& f) : fin(std::move(f)) {}
		~finally() { fin(); }
	private:
		std::function<void()> fin;
	};

	// Detail some timings to TTY
	class timer
	{
	public:
		timer(test_services& services) : srv(services) { start = srv.now(); }
		void reset() { start = srv.now(); }
		double seconds() const { return srv.now() - start; }

	private:
		double start;
		test_services& srv;
	};

	// Detail some timings to TTY
	class scoped_timer
	{
	public:
		scoped_timer(test_services& services, const char* name) : srv(services), n(name) { start = srv.now(); }
		~scoped_timer() { trace(); }
	
		double seconds() const { return srv.now() - start; }

		inline void trace() { srv.report("%s took %.03f sec", n ? n : "(null)", seconds()); }

	private:
		const char* n;
		double start;
		test_services& srv;
	};

	// Generate a random number. The seed is often configurable from the test
	// infrastructure so behavior can be reproduced.
	virtual int rand() = 0;

	// Returns a timer reasonably suited for benchmarking performance in unit 
	// tests.
	inline double now() const { return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); }
	
	// Write to the test infrastructure's TTY
	virtual void vreport(const char* fmt, va_list args) = 0;
	inline void report(const char* fmt, ...) { va_list a; va_start(a, fmt); vreport(fmt, a); va_end(a); }

	// throws the specified message
	inline void verror(const char* fmt, va_list args) { char msg[1024]; vsnprintf(msg, 1024, fmt, args); throw std::logic_error(msg); }
	inline void error(const char* fmt, ...) { va_list a; va_start(a, fmt); verror(fmt, a); va_end(a); }

	inline void vskip(const char* fmt, va_list args) { char msg[1024]; vsnprintf(msg, 1024, fmt, args); throw std::system_error(std::make_error_code(std::errc::permission_denied), msg); }
	inline void skip(const char* fmt, ...) { va_list a; va_start(a, fmt); vskip(fmt, a); va_end(a); }

	// Abstracts vsnprintf and snprintf (since Visual Studio complains about it)
	virtual int vsnprintf(char* dst, size_t dst_size, const char* fmt, va_list args) = 0;
	inline int snprintf(char* dst, size_t dst_size, const char* fmt, ...) { va_list a; va_start(a, fmt); int x = vsnprintf(dst, dst_size, fmt, a); va_end(a); return x; }
	template<size_t size> int snprintf(char (&dst)[size], const char* fmt, ...) { va_list a; va_start(a, fmt); int x = vsnprintf(dst, size, fmt, a); va_end(a); return x; }

	virtual void begin_thread(const char* name) = 0;
	virtual void update_thread() = 0;
	virtual void end_thread() = 0;

	// Returns the root path from which any test data should be loaded.
	virtual char* test_root_path(char* dst, size_t dst_size) const = 0;
	template<size_t size> char* test_root_path(char (&dst)[size]) const { return test_root_path(dst, size); }

	// Load the entire contents of the specified file into a newly allocated 
	// buffer.
	virtual ouro::scoped_allocation load_buffer(const char* _Path) = 0;

	virtual bool is_debugger_attached() const = 0;

	virtual bool is_remote_session() const = 0;

	virtual size_t total_physical_memory() const = 0;

	// Returns the average and peek percent usage of the CPU by this process
	virtual void get_cpu_utilization(float* out_avg, float* out_peek) = 0;

	// Resets the frame of recording average and peek percentage CPU utilization
	virtual void reset_cpu_utilization() = 0;
	
	// This function compares the specified surface to a golden image named after
	// the test's name suffixed with nth. If nth is 0 then the golden 
	// image should not have a suffix. If max_rms_error is negative a default 
	// should be used. If the surfaces are not similar this throws an exception.
	virtual void check(const surface::image& img, int nth = 0, float max_rms_error = -1.0f) = 0;
};

}
