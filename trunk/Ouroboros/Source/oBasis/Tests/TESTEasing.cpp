/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasis/tests/oBasisTests.h>
#include <oBasis/oEasing.h>
#include "oBasisTestCommon.h"

bool oBasisTest_oEasing()
{
	float result;
	// linear
	result = oSimpleLinearTween(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.6f), "oSimpleLinearTween returned incorrect value");
	result = oSimpleLinearTween(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.6857142f), "oSimpleLinearTween returned incorrect value");
	result = oSimpleLinearTween(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oSimpleLinearTween returned incorrect value");

	// quadratic
	result = oQuadraticEaseIn(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.28f), "oQuadraticEaseIn returned incorrect value");
	result = oQuadraticEaseIn(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.117551f), "oQuadraticEaseIn returned incorrect value");
	result = oQuadraticEaseIn(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oQuadraticEaseIn returned incorrect value");

	result = oQuadraticEaseOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.92f), "oQuadraticEaseOut returned incorrect value");
	result = oQuadraticEaseOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 2.2538776f), "oQuadraticEaseOut returned incorrect value");
	result = oQuadraticEaseOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oQuadraticEaseOut returned incorrect value");

	result = oQuadraticEaseInOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.84f), "oQuadraticEaseInOut returned incorrect value");
	result = oQuadraticEaseInOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.235102f), "oQuadraticEaseInOut returned incorrect value");
	result = oQuadraticEaseInOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oQuadraticEaseInOut returned incorrect value");

	// cubic
	result = oCubicEaseIn(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.024f), "oCubicEaseIn returned incorrect value");
	result = oCubicEaseIn(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.0201516f), "oCubicEaseIn returned incorrect value");
	result = oCubicEaseIn(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oCubicEaseIn returned incorrect value");

	result = oCubicEaseOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.984f), "oCubicEaseOut returned incorrect value");
	result = oCubicEaseOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 2.7246414f), "oCubicEaseOut returned incorrect value");
	result = oCubicEaseOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oCubicEaseOut returned incorrect value");

	result = oCubicEaseInOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.936f), "oCubicEaseInOut returned incorrect value");
	result = oCubicEaseInOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.0806065f), "oCubicEaseInOut returned incorrect value");
	result = oCubicEaseInOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oCubicEaseInOut returned incorrect value");

	// quartic
	result = oQuarticEaseIn(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 0.8192f), "oQuarticEaseIn returned incorrect value");
	result = oQuarticEaseIn(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.0034546f), "oQuarticEaseIn returned incorrect value");
	result = oQuarticEaseIn(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oQuarticEaseIn returned incorrect value");

	result = oQuarticEaseOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.9967999f), "oQuarticEaseOut returned incorrect value");
	result = oQuarticEaseOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 3.1147029f), "oQuarticEaseOut returned incorrect value");
	result = oQuarticEaseOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oQuarticEaseOut returned incorrect value");

	result = oQuarticEaseInOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(result == 1.9744f, "oQuarticEaseInOut returned incorrect value");
	result = oQuarticEaseInOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(result == 1.0276365f, "oQuarticEaseInOut returned incorrect value");
	result = oQuarticEaseInOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(result == 5.f, "oQuarticEaseInOut returned incorrect value");

	// quintic
	result = oQuinticEaseIn(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 0.65535998f), "oQuinticEaseIn returned incorrect value");
	result = oQuinticEaseIn(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.0005922f), "oQuinticEaseIn returned incorrect value");
	result = oQuinticEaseIn(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oQuinticEaseIn returned incorrect value");

	result = oQuinticEaseOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.99936f), "oQuinticEaseOut returned incorrect value");
	result = oQuinticEaseOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 3.4378967f), "oQuinticEaseOut returned incorrect value");
	result = oQuinticEaseOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oQuinticEaseOut returned incorrect value");

	result = oQuinticEaseInOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.98976f), "oQuinticEaseInOut returned incorrect value");
	result = oQuinticEaseInOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.0094754f), "oQuinticEaseInOut returned incorrect value");
	result = oQuinticEaseInOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oQuinticEaseInOut returned incorrect value");

	// sin
	result = oSinEaseIn(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.3819660f), "oSinEaseIn returned incorrect value");
	result = oSinEaseIn(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.1441486f), "oSinEaseIn returned incorrect value");
	result = oSinEaseIn(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oSinEaseIn returned incorrect value");

	result = oSinEaseOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.9021131f), "oSinEaseOut returned incorrect value");
	result = oSinEaseOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 2.0641475f), "oSinEaseOut returned incorrect value");
	result = oSinEaseOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oSinEaseOut returned incorrect value");

	result = oSinEaseInOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.8090169f), "oSinEaseInOut returned incorrect value");
	result = oSinEaseInOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.2831024f), "oSinEaseInOut returned incorrect value");
	result = oSinEaseInOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oSinEaseInOut returned incorrect value");

	// exponential
	result = oExponentialEaseIn(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 0.5f), "oExponentialEaseIn returned incorrect value");
	result = oExponentialEaseIn(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.0128177f), "oExponentialEaseIn returned incorrect value");
	result = oExponentialEaseIn(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oExponentialEaseIn returned incorrect value");

	result = oExponentialEaseOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.9921875f), "oExponentialEaseOut returned incorrect value");
	result = oExponentialEaseOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 3.7809863f), "oExponentialEaseOut returned incorrect value");
	result = oExponentialEaseOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(result > 4.99 && result < 5.01, "oExponentialEaseOut returned incorrect value");

	result = oExponentialEaseInOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.984375f), "oExponentialEaseInOut returned incorrect value");
	result = oExponentialEaseInOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.0210297f), "oExponentialEaseInOut returned incorrect value");
	result = oExponentialEaseInOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(result > 4.99 && result < 5.01, "oExponentialEaseInOut returned incorrect value");

	// circular
	result = oCircularEaseIn(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 0.8f), "oCircularEaseIn returned incorrect value");
	result = oCircularEaseIn(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.0592138f), "oCircularEaseIn returned incorrect value");
	result = oCircularEaseIn(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oCircularEaseIn returned incorrect value");

	result = oCircularEaseOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.9595917f), "oCircularEaseOut returned incorrect value");
	result = oCircularEaseOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 3.2395335f), "oCircularEaseOut returned incorrect value");
	result = oCircularEaseOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oCircularEaseOut returned incorrect value");

	result = oCircularEaseInOut(0.8f, 0.f, 2.f, 1.0f);
	oTESTB(oStd::equal(result, 1.9165151f), "oCircularEaseInOut returned incorrect value");
	result = oCircularEaseInOut(0.3f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 1.1212249f), "oCircularEaseInOut returned incorrect value");
	result = oCircularEaseInOut(1.75f, 1.f, 4.f, 1.75f);
	oTESTB(oStd::equal(result, 5.f), "oCircularEaseInOut returned incorrect value");
	
	oErrorSetLast(0);
	return true;
}
