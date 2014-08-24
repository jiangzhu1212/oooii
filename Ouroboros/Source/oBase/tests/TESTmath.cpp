/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oBase/aabox.h>
#include <oBase/throw.h>
#include <oBase/types.h>

namespace ouro {
	namespace tests {

void TESTequal()
{
	oCHECK(!equal(1.0f, 1.000001f), "equal() failed");
	oCHECK(equal(1.0f, 1.000001f, 8), "equal() failed");
	oCHECK(!equal(2.0, 1.99999999999997), "equal failed");
	oCHECK(equal(2.0, 1.99999999999997, 135), "equal failed");
}

void TESTaaboxf()
{
	aaboxf box(aaboxf::min_max, float3(-1.0f), float3(1.0f));

	oCHECK(equal(box.Min, float3(-1.0f)), "aaboxf::Min failed");
	oCHECK(equal(box.Max, float3(1.0f)), "aaboxf::Min failed");

	box.Min = float3(-2.0f);
	box.Max = float3(2.0f);
	oCHECK(equal(box.Min, float3(-2.0f)), "aaboxf::SetMin() failed");	
	oCHECK(equal(box.Max, float3(2.0f)), "aaboxf::SetMin() failed");

	oCHECK(!box.empty(), "aaboxf::empty() failed (1)");
	box.clear();
	oCHECK(box.empty(), "aaboxf::empty() failed (2)");

	box = aaboxf(aaboxf::min_max, float3(0.0f), float3(1.0f, 2.0f, 3.0f));
	oCHECK(equal(box.center(), float3(0.5f, 1.0f, 1.5f)), "aaboxf::GetCenter() failed");

	float3 dim = box.size();
	oCHECK(equal(dim.x, 1.0f) && equal(dim.y, 2.0f) && equal(dim.z, 3.0f), "aaboxf::GetDimensions() failed");

	float radius = box.bounding_radius();
	oCHECK(equal(radius, 1.87083f, 15), "aaboxf:bounding_radius() failed");

	extend_by(box, float3(5.0f));
	extend_by(box, float3(-1.0f));
	dim = box.size();
	oCHECK(equal(dim.x, 6.0f) && equal(dim.y, 6.0f) && equal(dim.z, 6.0f), "aaboxf::ExtendBy() failed");
}

	} // namespace tests
} // namespace ouro
