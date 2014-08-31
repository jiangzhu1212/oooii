// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oCompute/tests/oComputeTests.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_COMPUTE_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oCompute_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_COMPUTE_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oCompute_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_COMPUTE_TEST_BUGGED0(_Name, _Bugged) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oCompute_, _Name), oCONCAT(TEST, _Name), _Bugged)
#define oTEST_REGISTER_COMPUTE_TEST_BUGGED(_Name, _Bugged) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oCompute_, _Name), oCONCAT(TEST, _Name), _Bugged)

oTEST_REGISTER_COMPUTE_TEST0(compute)
oTEST_REGISTER_COMPUTE_TEST0(easing)
