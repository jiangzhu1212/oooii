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
// This header is designed to cross-compile in both C++ and HLSL. This defines
// for C++ the HLSL language feature of swizzling tuple elements. This has to be 
// implemented as macros for C++, so this header can be very dangerous, so the 
// pops to match pushes and redefinitions in oHLSLSwizzlesOn.h has been defined 
// in this header. Shared code implemented in headers should guard the header 
// with these on/off pairs to prevent client code pollution with these macros.

// Force reevaluation each time
//#ifndef oHLSL
//	#pragma once
//#endif
//#ifndef oHLSLSwizzlesOff_h
//#define oHLSLSwizzlesOff_h

#ifndef oHLSL

#pragma pop_macro("xy")
#pragma pop_macro("yx")
#pragma pop_macro("xx")
#pragma pop_macro("yy")
#pragma pop_macro("xyz")
#pragma pop_macro("xzy")
#pragma pop_macro("yxz")
#pragma pop_macro("yzx")
#pragma pop_macro("zxy")
#pragma pop_macro("zyx")
#pragma pop_macro("xxx")
#pragma pop_macro("yyy")
#pragma pop_macro("zzz")
#pragma pop_macro("xz")
#pragma pop_macro("yx")
#pragma pop_macro("yz")
#pragma pop_macro("zx")
#pragma pop_macro("zy")
#pragma pop_macro("xxxx")
#pragma pop_macro("yyyy")
#pragma pop_macro("xyzw")
#pragma pop_macro("wxyz")
#pragma pop_macro("zwxy")
#pragma pop_macro("yzwx")
#pragma pop_macro("xyz")

#endif
//#endif