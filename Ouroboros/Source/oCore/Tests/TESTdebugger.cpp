// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oCore/debugger.h>

#include "../../test_services.h"

namespace ouro {
	namespace tests {

void TESTdebugger(test_services& _Services)
{
	// inlining results in validly differing stack traces
	#ifdef _DEBUG
		#ifdef _WIN64
			static const char* sExpectedStack[] = 
			{
				"ouro::tests::TESTdebugger",
				"oCore_debugger::Run",
				"oTestManager_Impl::RunTest",
				"oTestManager_Impl::RunTests",
				"main",
				"__tmainCRTStartup",
				"mainCRTStartup",
				"BaseThreadInitThunk",
				"RtlUserThreadStart",
			};
		#else
			static const char* sExpectedStack[] = 
			{
				"ouro::tests::TESTdebugger",
				"oCore_debugger::Run",
				"oTestManager_Impl::RunTest",
				"oTestManager_Impl::RunTests",
				"main",
				"__tmainCRTStartup",
				"mainCRTStartup",
				"BaseThreadInitThunk",
				"RtlInitializeExceptionChain",
				"RtlInitializeExceptionChain",
			};
		#endif
	#else
		#ifdef _WIN64
			static const char* sExpectedStack[] = 
			{
				"ouro::tests::TESTdebugger",
				"oCore_debugger::Run",
				"oTestManager_Impl::RunTest",
				"oTestManager_Impl::RunTests",
				"main",
				"__tmainCRTStartup",
				"BaseThreadInitThunk",
				"RtlUserThreadStart",
			};
		#else
			static const char* sExpectedStack[] = 
			{
				"ouro::tests::TESTdebugger",
				"oCore_debugger::Run",
				"oTestManager_Impl::RunTest",
				"oTestManager_Impl::RunTests",
				"main",
				"__tmainCRTStartup",
				"BaseThreadInitThunk",
				"RtlInitializeExceptionChain",
				"RtlInitializeExceptionChain",
			};
		#endif

	#endif

	debugger::symbol addresses[32];
	size_t nAddresses = debugger::callstack(addresses, 0);

	for (size_t i = 0; i < nAddresses; i++)
	{
		debugger::symbol_info sym = debugger::translate(addresses[i]);
		//printf("%u: %s\n", i, sym.Name);
		if (strcmp(sym.name, sExpectedStack[i]))
			throw std::exception(ouro::formatf("Mismatch on stack trace at level %u", i).c_str());
	}
}

	} // namespace debugger
} // namespace ouro
