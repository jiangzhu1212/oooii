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
#include <oCore/tests/oCoreTests.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_CORE_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oCore_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_CORE_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oCore_, _Name), oCONCAT(TEST, _Name))

#define oTEST_REGISTER_CORE_TEST_BUGGED0(_Name) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oCore_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_CORE_TEST_BUGGED(_Name) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oCore_, _Name), oCONCAT(TEST, _Name))

oTEST_REGISTER_CORE_TEST(adapter);
oTEST_REGISTER_CORE_TEST(camera);
oTEST_REGISTER_CORE_TEST(cpu);
oTEST_REGISTER_CORE_TEST(debugger);
oTEST_REGISTER_CORE_TEST0(filesystem);
oTEST_REGISTER_CORE_TEST0(filesystem_monitor);
oTEST_REGISTER_CORE_TEST0(process_heap);
#if defined(_WIN32) || defined(_WIN64)
	oTEST_REGISTER_CORE_TEST(win_crt_leak_tracker);
	oTEST_REGISTER_CORE_TEST0(win_registry);
#endif