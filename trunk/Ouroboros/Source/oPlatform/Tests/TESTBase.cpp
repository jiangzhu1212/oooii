// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/tests/oBaseTests.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_BASE_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_BASE_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))

#define oTEST_REGISTER_BASE_TEST_BUGGED0(_Name) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_BASE_TEST_BUGGED(_Name) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oBase_, _Name), oCONCAT(TEST, _Name))

oTEST_REGISTER_BASE_TEST0(aaboxf);
oTEST_REGISTER_BASE_TEST(compression);
oTEST_REGISTER_BASE_TEST0(concurrent_growable_object_pool);
oTEST_REGISTER_BASE_TEST(concurrent_hash_map);
oTEST_REGISTER_BASE_TEST0(concurrent_queue);
oTEST_REGISTER_BASE_TEST0(concurrent_queue_concrt);
oTEST_REGISTER_BASE_TEST0(concurrent_queue_opt);
oTEST_REGISTER_BASE_TEST0(concurrent_queue_tbb);
oTEST_REGISTER_BASE_TEST0(concurrent_stack);
oTEST_REGISTER_BASE_TEST0(coroutine);
oTEST_REGISTER_BASE_TEST0(countdown_latch);
oTEST_REGISTER_BASE_TEST(date);
oTEST_REGISTER_BASE_TEST0(equal);
oTEST_REGISTER_BASE_TEST0(filter_chain);
oTEST_REGISTER_BASE_TEST0(fourcc);
oTEST_REGISTER_BASE_TEST(future);
oTEST_REGISTER_BASE_TEST(hash_map);
oTEST_REGISTER_BASE_TEST0(osc);
oTEST_REGISTER_BASE_TEST0(parallel_for);
oTEST_REGISTER_BASE_TEST0(path);
oTEST_REGISTER_BASE_TEST0(task_group);
oTEST_REGISTER_BASE_TEST0(threadpool);
oTEST_REGISTER_BASE_TEST(threadpool_perf);
oTEST_REGISTER_BASE_TEST0(uri);


