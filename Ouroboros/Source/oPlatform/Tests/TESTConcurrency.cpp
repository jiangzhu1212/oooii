// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oConcurrency/tests/oConcurrencyTests.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_CONCURRENCY_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oConcurrency_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_CONCURRENCY_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oConcurrency_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_CONCURRENCY_TEST_BUGGED0(_Name, _Bugged) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oConcurrency_, _Name), oCONCAT(TEST, _Name), _Bugged)
#define oTEST_REGISTER_CONCURRENCY_TEST_BUGGED(_Name, _Bugged) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oConcurrency_, _Name), oCONCAT(TEST, _Name), _Bugged)

oTEST_REGISTER_CONCURRENCY_TEST(concurrent_hash_map);
oTEST_REGISTER_CONCURRENCY_TEST(concurrent_queue);
oTEST_REGISTER_CONCURRENCY_TEST(concurrent_queue_concrt);
oTEST_REGISTER_CONCURRENCY_TEST(concurrent_queue_opt);
oTEST_REGISTER_CONCURRENCY_TEST(concurrent_queue_tbb);
oTEST_REGISTER_CONCURRENCY_TEST(concurrent_stack);
oTEST_REGISTER_CONCURRENCY_TEST(coroutine);
oTEST_REGISTER_CONCURRENCY_TEST(countdown_latch);
oTEST_REGISTER_CONCURRENCY_TEST(future);
oTEST_REGISTER_CONCURRENCY_TEST(parallel_for);
oTEST_REGISTER_CONCURRENCY_TEST(task_group);
oTEST_REGISTER_CONCURRENCY_TEST(threadpool);
oTEST_REGISTER_CONCURRENCY_TEST(threadpool_perf);

