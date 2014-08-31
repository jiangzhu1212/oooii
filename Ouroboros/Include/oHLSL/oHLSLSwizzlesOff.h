// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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