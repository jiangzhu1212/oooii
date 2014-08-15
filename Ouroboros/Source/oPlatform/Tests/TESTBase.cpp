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
#include <oBase/tests/oBaseTests.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_BASE_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_BASE_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))

#define oTEST_REGISTER_BASE_TEST_BUGGED0(_Name) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_BASE_TEST_BUGGED(_Name) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))

oTEST_REGISTER_BASE_TEST0(aaboxf);
oTEST_REGISTER_BASE_TEST(atof);
oTEST_REGISTER_BASE_TEST(compression);
oTEST_REGISTER_BASE_TEST0(concurrent_growable_object_pool);
oTEST_REGISTER_BASE_TEST(concurrent_hash_map);
oTEST_REGISTER_BASE_TEST0(concurrent_linear_allocator);
oTEST_REGISTER_BASE_TEST0(concurrent_pool);
oTEST_REGISTER_BASE_TEST0(concurrent_queue);
oTEST_REGISTER_BASE_TEST0(concurrent_queue_concrt);
oTEST_REGISTER_BASE_TEST0(concurrent_queue_opt);
oTEST_REGISTER_BASE_TEST0(concurrent_queue_tbb);
oTEST_REGISTER_BASE_TEST0(concurrent_stack);
oTEST_REGISTER_BASE_TEST0(coroutine);
oTEST_REGISTER_BASE_TEST0(countdown_latch);
oTEST_REGISTER_BASE_TEST0(csv);
oTEST_REGISTER_BASE_TEST(date);
oTEST_REGISTER_BASE_TEST0(equal);
oTEST_REGISTER_BASE_TEST0(filter_chain);
oTEST_REGISTER_BASE_TEST0(fourcc);
oTEST_REGISTER_BASE_TEST(future);
oTEST_REGISTER_BASE_TEST(hash_map);
oTEST_REGISTER_BASE_TEST0(ini);
oTEST_REGISTER_BASE_TEST0(json);
oTEST_REGISTER_BASE_TEST0(osc);
oTEST_REGISTER_BASE_TEST0(parallel_for);
oTEST_REGISTER_BASE_TEST0(path);
oTEST_REGISTER_BASE_TEST0(pool);
oTEST_REGISTER_BASE_TEST(sbb);
oTEST_REGISTER_BASE_TEST0(task_group);
oTEST_REGISTER_BASE_TEST(tlsf_allocator);
oTEST_REGISTER_BASE_TEST0(threadpool);
oTEST_REGISTER_BASE_TEST(threadpool_perf);
oTEST_REGISTER_BASE_TEST0(uri);
oTEST_REGISTER_BASE_TEST0(xml);


