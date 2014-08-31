// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/tests/oGPUTests.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_GPU_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oGPU_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_GPU_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oGPU_, _Name), oCONCAT(TEST, _Name))

#define oTEST_REGISTER_GPU_TEST_BUGGED0(_Name) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oGPU_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_GPU_TEST_BUGGED(_Name) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oGPU_, _Name), oCONCAT(TEST, _Name))

oTEST_REGISTER_GPU_TEST0(buffer);
oTEST_REGISTER_GPU_TEST(clear);
oTEST_REGISTER_GPU_TEST(device);
oTEST_REGISTER_GPU_TEST(instanced_triangle);
oTEST_REGISTER_GPU_TEST(lines);
oTEST_REGISTER_GPU_TEST0(query);
oTEST_REGISTER_GPU_TEST(render_target);
oTEST_REGISTER_GPU_TEST(spinning_triangle);
oTEST_REGISTER_GPU_TEST(texture1d);
oTEST_REGISTER_GPU_TEST(texture1dmip);
oTEST_REGISTER_GPU_TEST(texture2d);
oTEST_REGISTER_GPU_TEST(texture2dmip);
oTEST_REGISTER_GPU_TEST(texture3d);
oTEST_REGISTER_GPU_TEST(texture3dmip);
oTEST_REGISTER_GPU_TEST(texturecube);
oTEST_REGISTER_GPU_TEST(texturecubemip);
oTEST_REGISTER_GPU_TEST(triangle);
oTEST_REGISTER_GPU_TEST(window_in_window);
