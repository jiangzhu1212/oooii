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
// Declarations of oGPU unit tests. These throw on failure.
#pragma once
#ifndef oGPUTests_h
#define oGPUTests_h

namespace ouro {

	class test_services;

	namespace tests {

		void TESTbuffer();
		void TESTclear(test_services& _Services);
		void TESTdevice(test_services& _Services);
		void TESTinstanced_triangle(test_services& _Services);
		void TESTlines(test_services& _Services);
		void TESTquery();
		void TESTrender_target(test_services& _Services);
		void TESTspinning_triangle(test_services& _Services);
		void TESTtexture1d(test_services& _Services);
		void TESTtexture1dmip(test_services& _Services);
		void TESTtexture2d(test_services& _Services);
		void TESTtexture2dmip(test_services& _Services);
		void TESTtexture3d(test_services& _Services);
		void TESTtexture3dmip(test_services& _Services);
		void TESTtexturecube(test_services& _Services);
		void TESTtexturecubemip(test_services& _Services);
		void TESTtriangle(test_services& _Services);
		void TESTwindow_in_window(test_services& _Services);

	} // namespace tests
} // namespace ouro

#endif
