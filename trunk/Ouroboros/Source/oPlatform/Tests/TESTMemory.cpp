// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oMemory/tests/oMemoryTests.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_MEMORY_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oMemory_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_MEMORY_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oMemory_, _Name), oCONCAT(TEST, _Name))

#define oTEST_REGISTER_MEMORY_TEST_BUGGED0(_Name) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oMemory_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_MEMORY_TEST_BUGGED(_Name) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oMemory_, _Name), oCONCAT(TEST, _Name))

oTEST_REGISTER_MEMORY_TEST(concurrent_linear_allocator);
oTEST_REGISTER_MEMORY_TEST(concurrent_pool);
oTEST_REGISTER_MEMORY_TEST(pool);
oTEST_REGISTER_MEMORY_TEST(sbb);
oTEST_REGISTER_MEMORY_TEST(tlsf_allocator);

