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
// Integrate unit tests from modules lower-Level than oTest.

#pragma once
#ifndef oTestIntegration_h
#define oTestIntegration_h

#include <oPlatform/oTest.h>

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
			oPrintf(_StrStatus, _SizeofStrStatus, "%s", e.what()); \
			oTRACE("%s: %s", oStd::as_string(r), _StrStatus); \
		} \
		if (r == oTest::SUCCESS && oErrorGetLast() == 0) \
			oPrintf(_StrStatus, _SizeofStrStatus, oErrorGetLastString()); \
		return r; \
	} while (false)

#define oTEST_THROWS0(fn) oTEST_BEGIN__ try { fn(); } oTEST_END__
#define oTEST_THROWS(fn) oTEST_BEGIN__ try { requirements_implementation R; fn(R); } oTEST_END__

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