// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Integrate unit tests from modules lower-Level than oTest.

#pragma once
#ifndef oTestIntegration_h
#define oTestIntegration_h

#include <oBasis/oError.h>
#include <oPlatform/oTest.h>

#include "../../test_services.h"

#include <oBase/assert.h>
#include <oCore/process.h>
#include <oCore/process_stats_monitor.h>
#include <oCore/system.h>
#include <oCore/thread_traits.h>

namespace ouro {
	
class test_services_implementation : public test_services
{
public:
	int rand() override { return ::rand(); }
	
	void vreport(const char* _Format, va_list _Args) override
	{
		oErrorSetLastV(0, _Format, _Args);
		oTRACEA("%s", oErrorGetLastString());
	}

	int vsnprintf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, va_list _Args) override
	{
		return ouro::vsnprintf(_StrDestination, _SizeofStrDestination, _Format, _Args);
	}

	void begin_thread(const char* _Name) override
	{
		core_thread_traits::begin_thread(_Name);
	}

	void update_thread() override
	{
		core_thread_traits::update_thread();
	}

	void end_thread() override
	{
		core_thread_traits::end_thread();
	}

	char* test_root_path(char* _StrDestination, size_t _SizeofStrDestination) const override
	{
		return strlcpy(_StrDestination, filesystem::data_path().c_str(), _SizeofStrDestination) < _SizeofStrDestination ? _StrDestination : nullptr;
	}

	scoped_allocation load_buffer(const char* _Path) override
	{
		scoped_allocation b;
		try { b = filesystem::load(_Path); }
		catch (std::system_error& e)
		{
			if (e.code().value() != std::errc::no_such_file_or_directory)
				throw;

			path FullPath = filesystem::data_path() / _Path;
			b = filesystem::load(FullPath);
		}
		return b;
	}

	bool is_debugger_attached() const override { return this_process::has_debugger_attached(); }
	bool is_remote_session() const override { return system::is_remote_session(); }

	size_t total_physical_memory() const override
	{
		system::heap_info hi = system::get_heap_info();
		return static_cast<size_t>(hi.total_physical);
	}

	void get_cpu_utilization(float* _pAverage, float* _pPeek) override
	{
		ouro::process_stats_monitor::info stats = PSM.get_info();
		*_pAverage = stats.average_usage;
		*_pPeek = stats.high_usage;
	}

	void reset_cpu_utilization() override { PSM.reset(); }
	
	void check(const surface::image& _Buffer, int _NthTest = 0, float _MaxRMSError = -1.0f)
	{
		extern oTest* g_Test;
		if (!g_Test->TestImage(_Buffer, _NthTest, oDEFAULT, _MaxRMSError, oDEFAULT))
			oThrowLastError();
	}

private:
	ouro::process_stats_monitor PSM;
};

} // namespace ouro

// _____________________________________________________________________________
// oTest wrapper/integration

#define oTEST_BEGIN__ do \
	{	RESULT r = oTest::SUCCESS; \
		oErrorSetLast(0); \
		*_StrStatus = 0; \

#define oTEST_END__ \
		catch (std::exception& e) \
		{	r = oTest::FAILURE; \
			std::system_error* se = dynamic_cast<std::system_error*>(&e); \
			if (se && se->code().value() == std::errc::permission_denied) \
				r = oTest::SKIPPED; \
			snprintf(_StrStatus, _SizeofStrStatus, "%s", e.what()); \
			oTRACE("%s: %s", ouro::as_string(r), _StrStatus); \
		} \
		if (r == oTest::SUCCESS && oErrorGetLast() == 0) \
			snprintf(_StrStatus, _SizeofStrStatus, oErrorGetLastString()); \
		return r; \
	} while (false)

#define oTEST_THROWS0(fn) oTEST_BEGIN__ try { fn(); } oTEST_END__
#define oTEST_THROWS(fn) oTEST_BEGIN__ try { ouro::test_services_implementation S; fn(S); } oTEST_END__

#define oTEST_THROWS_WRAPPER__(_Macro, _NameInUnitTests, _ActualUnitTestName) \
	struct _NameInUnitTests : oTest \
	{	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override \
		{	_Macro(_ActualUnitTestName); } \
	};

#define oTEST_THROWS_WRAPPER0(_NameInUnitTests, _ActualUnitTestName) oTEST_THROWS_WRAPPER__(oTEST_THROWS0, _NameInUnitTests, _ActualUnitTestName)
#define oTEST_THROWS_WRAPPER(_NameInUnitTests, _ActualUnitTestName) oTEST_THROWS_WRAPPER__(oTEST_THROWS, _NameInUnitTests, _ActualUnitTestName)

#define oTEST_THROWS_REGISTER__(_Macro, _NameInUnitTests, _ActualUnitTestName) _Macro(_NameInUnitTests, _ActualUnitTestName) oTEST_REGISTER(_NameInUnitTests);
#define oTEST_THROWS_REGISTER_BUGGED__(_Macro, _NameInUnitTests, _ActualUnitTestName, _Bug) _Macro(_NameInUnitTests, _ActualUnitTestName) oTEST_REGISTER_BUGGED(_NameInUnitTests, _Bug)

// Registers a void param unit test function call
#define oTEST_THROWS_REGISTER0(_NameInUnitTests, _ActualUnitTestName) oTEST_THROWS_REGISTER__(oTEST_THROWS_WRAPPER0, _NameInUnitTests, _ActualUnitTestName)
#define oTEST_THROWS_REGISTER(_NameInUnitTests, _ActualUnitTestName) oTEST_THROWS_REGISTER__(oTEST_THROWS_WRAPPER, _NameInUnitTests, _ActualUnitTestName)

#define oTEST_THROWS_REGISTER_BUGGED0(_NameInUnitTests, _ActualUnitTestName, _Bug) oTEST_THROWS_REGISTER_BUGGED__(oTEST_THROWS_WRAPPER0, _NameInUnitTests, _ActualUnitTestName, _Bug)
#define oTEST_THROWS_REGISTER_BUGGED(_NameInUnitTests, _ActualUnitTestName, _Bug) oTEST_THROWS_REGISTER_BUGGED__(oTEST_THROWS_WRAPPER, _NameInUnitTests, _ActualUnitTestName, _Bug)

#endif
