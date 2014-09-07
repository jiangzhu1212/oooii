// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oString/tests/oStringTests.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_STRING_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oString_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_STRING_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oString_, _Name), oCONCAT(TEST, _Name))

#define oTEST_REGISTER_STRING_TEST_BUGGED0(_Name) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oString_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_STRING_TEST_BUGGED(_Name) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oString_, _Name), oCONCAT(TEST, _Name))

oTEST_REGISTER_STRING_TEST0(csv);
oTEST_REGISTER_STRING_TEST0(ini);
oTEST_REGISTER_STRING_TEST0(json);
oTEST_REGISTER_STRING_TEST0(xml);

