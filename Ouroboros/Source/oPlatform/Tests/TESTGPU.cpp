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
