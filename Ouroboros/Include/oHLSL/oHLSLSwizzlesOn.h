// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// This header is designed to cross-compile in both C++ and HLSL. This defines
// for C++ the HLSL language feature of swizzling tuple elements. This has to be 
// implemented as macros for C++, so this header can be very dangerous, so the 
// pushes and redefinitions has been defined in this header, and the pops and 
// restorations has been pushed to oHLSLSwizzlesOff.h. Shared code implemented
// in headers should guard the header with these on/off pairs to prevent client 
// code pollution with these macros.
// Force reevaluation each time
//#ifndef oHLSL
//	#pragma once
//#endif
//#ifndef oHLSLSwizzlesOn_h
//#define oHLSLSwizzlesOn_h

#ifndef oHLSL

#pragma push_macro("xy")
#pragma push_macro("yx")
#pragma push_macro("xx")
#pragma push_macro("yy")
#pragma push_macro("xyz")
#pragma push_macro("xzy")
#pragma push_macro("yxz")
#pragma push_macro("yzx")
#pragma push_macro("zxy")
#pragma push_macro("zyx")
#pragma push_macro("xxx")
#pragma push_macro("yyy")
#pragma push_macro("zzz")
#pragma push_macro("xz")
#pragma push_macro("yx")
#pragma push_macro("yz")
#pragma push_macro("zx")
#pragma push_macro("zy")
#pragma push_macro("xxxx")
#pragma push_macro("yyyy")
#pragma push_macro("xyzw")
#pragma push_macro("wxyz")
#pragma push_macro("zwxy")
#pragma push_macro("yzwx")
#pragma push_macro("xyz")

#undef xy
#undef yx
#undef xx
#undef yy

#undef xyz
#undef xzy
#undef yxz
#undef yzx
#undef zxy
#undef zyx
#undef xxx
#undef yyy
#undef zzz
#undef xz
#undef yx
#undef yz
#undef zx
#undef zy

#undef xxxx
#undef yyyy
#undef xyzw
#undef wxyz
#undef zwxy
#undef yzwx
#undef xyz

#define xy xy()
#define yx yx()
#define xx xx()
#define yy yy()

#define xyz xyz()
#define xzy xzy()
#define yxz yxz()
#define yzx yzx()
#define zxy zxy()
#define zyx zyx()
#define xxx xxx()
#define yyy yyy()
#define zzz zzz()
#define xz xz()
#define yx yx()
#define yz yz()
#define zx zx()
#define zy zy()

#define xxxx xxxx()
#define yyyy yyyy()
#define xyzw xyzw()
#define wxyz wxyz()
#define zwxy zwxy()
#define yzwx yzwx()
#define xyz xyz()

#endif
//#endif