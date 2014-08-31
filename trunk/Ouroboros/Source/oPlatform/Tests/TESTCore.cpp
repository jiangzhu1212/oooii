// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
oTEST_REGISTER_CORE_TEST(filesystem);
oTEST_REGISTER_CORE_TEST0(filesystem_monitor);
oTEST_REGISTER_CORE_TEST0(process_heap);
#if defined(_WIN32) || defined(_WIN64)
	oTEST_REGISTER_CORE_TEST(win_crt_leak_tracker);
	oTEST_REGISTER_CORE_TEST0(win_registry);
#endif