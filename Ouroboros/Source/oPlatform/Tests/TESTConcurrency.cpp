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
#include <oConcurrency/tests/oConcurrencyTests.h>
#include "oTestIntegration.h"

using namespace oConcurrency::tests;

#define oTEST_REGISTER_CONCURRENCY_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oConcurrency_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_CONCURRENCY_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oConcurrency_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_CONCURRENCY_TEST_BUGGED0(_Name, _Bugged) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oConcurrency_, _Name), oCONCAT(TEST, _Name), _Bugged)
#define oTEST_REGISTER_CONCURRENCY_TEST_BUGGED(_Name, _Bugged) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oConcurrency_, _Name), oCONCAT(TEST, _Name), _Bugged)

oTEST_REGISTER_CONCURRENCY_TEST0(block_allocator);
oTEST_REGISTER_CONCURRENCY_TEST0(concurrent_queue);
oTEST_REGISTER_CONCURRENCY_TEST0(concurrent_queue_concrt);
oTEST_REGISTER_CONCURRENCY_TEST0(concurrent_queue_opt);
oTEST_REGISTER_CONCURRENCY_TEST0(concurrent_queue_tbb);
oTEST_REGISTER_CONCURRENCY_TEST0(concurrent_stack);
oTEST_REGISTER_CONCURRENCY_TEST0(coroutine);
oTEST_REGISTER_CONCURRENCY_TEST0(fixed_block_allocator);
oTEST_REGISTER_CONCURRENCY_TEST0(parallel_for);
oTEST_REGISTER_CONCURRENCY_TEST0(task_group);
oTEST_REGISTER_CONCURRENCY_TEST0(threadpool);
oTEST_REGISTER_CONCURRENCY_TEST(threadpool_perf);
