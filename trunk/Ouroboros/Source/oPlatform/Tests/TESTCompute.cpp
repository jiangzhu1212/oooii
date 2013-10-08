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
#include <oCompute/tests/oComputeTestRequirements.h>
#include <oCompute/tests/oComputeTests.h>
#include <oPlatform/oTest.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_COMPUTE_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oCompute_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_COMPUTE_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oCompute_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_COMPUTE_TEST_BUGGED0(_Name, _Bugged) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oCompute_, _Name), oCONCAT(TEST, _Name), _Bugged)
#define oTEST_REGISTER_COMPUTE_TEST_BUGGED(_Name, _Bugged) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oCompute_, _Name), oCONCAT(TEST, _Name), _Bugged)

oTEST_REGISTER_COMPUTE_TEST0(compute)
oTEST_REGISTER_COMPUTE_TEST0(easing)
