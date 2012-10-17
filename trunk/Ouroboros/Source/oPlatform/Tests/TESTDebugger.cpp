/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oPlatform/oDebugger.h>
#include <oPlatform/oTest.h>
#include <oBasis/oString.h>

struct TESTDebugger : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		size_t SizesToTest[] = {sizeof(int), oKB(4), oMB(4), oMB(2) - sizeof(int)};

		for(int i = 0; i < oCOUNTOF(SizesToTest); ++i)
		{
			void* pAlloc = oDebuggerGuardedAlloc(SizesToTest[i]);
			oTESTB(pAlloc, "oDebuggerGuardedAlloc failed on size %d", SizesToTest[i]);

			oDebuggerAllocationInfo AllocInfo;
			oTESTB( oDebuggerGuardedInfo(pAlloc, &AllocInfo) || oInvalid != AllocInfo.ThreadFreedOn, "oDebuggerGuardedInfo failed");
			oDebuggerGuardedFree(pAlloc);
		}

		// inlining results in validly differing stack traces
		#ifdef _DEBUG
			#ifdef _WIN64
				static const size_t OFFSET = 2;
				static const char* sExpectedStack[] = 
				{
					"TESTDebugger::Run",
					"oTestManager_Impl::RunTest",
					"oTestManager_Impl::RunTests",
					"main",
					"__tmainCRTStartup",
					"mainCRTStartup",
					"BaseThreadInitThunk",
					"RtlUserThreadStart",
				};
			#else
				static const size_t OFFSET = 1;
				static const char* sExpectedStack[] = 
				{
					"TESTDebugger::Run",
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
			static const size_t OFFSET = 0;
			#ifdef _WIN64
				static const char* sExpectedStack[] = 
				{
					"TESTDebugger::Run",
					"oTestManager_Impl::RunTest",
					"oTestManager_Impl::RunTests",
					"main",
					"__tmainCRTStartup",
					"BaseThreadInitThunk",
					"RtlUserThreadStart",
				};
			#else
				// @oooii-tony: There's nothing incorrect AFAICT, so I think this stuff
				// just doesn't work in 32-bit
				oPrintf(_StrStatus, _SizeofStrStatus, "No debug stack in release for WIN32");
				return SKIPPED;
			#endif

		#endif

		#if defined(_WIN64) || defined(_DEBUG)

				unsigned long long addresses[32];
				size_t nAddresses = oDebuggerGetCallstack(addresses, OFFSET);

				for (size_t i = 0; i < nAddresses; i++)
				{
					oDEBUGGER_SYMBOL sym;
					oTESTB(oDebuggerTranslateSymbol(&sym, addresses[i]), "TranslateSymbol failed on %u%s symbol", i, oOrdinal((int)i));
					//printf("%u: %s\n", i, sym.Name);
					oTESTB(!oStrcmp(sym.Name, sExpectedStack[i]), "Mismatch on stack trace at level %u", i);
				}

				return SUCCESS;

		#endif
	}
};

oTEST_REGISTER(TESTDebugger);
