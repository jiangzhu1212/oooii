// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oSurface/tests/oSurfaceTests.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_SURFACE_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oSurface_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_SURFACE_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oSurface_, _Name), oCONCAT(TEST, _Name))

#define oTEST_REGISTER_SURFACE_TEST_BUGGED0(_Name) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oSurface_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_SURFACE_TEST_BUGGED(_Name) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oSurface_, _Name), oCONCAT(TEST, _Name))

oTEST_REGISTER_SURFACE_TEST0(surface);
oTEST_REGISTER_SURFACE_TEST(surface_bccodec);
oTEST_REGISTER_SURFACE_TEST(surface_codec);
oTEST_REGISTER_SURFACE_TEST(surface_fill);
oTEST_REGISTER_SURFACE_TEST(surface_generate_mips);
oTEST_REGISTER_SURFACE_TEST(surface_resize);
