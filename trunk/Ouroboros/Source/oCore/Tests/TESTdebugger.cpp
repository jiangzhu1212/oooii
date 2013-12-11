/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
