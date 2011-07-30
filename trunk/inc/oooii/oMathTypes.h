// $(header)

// Declare the public types for oMath. This header can be included separately 
// from oMath to keep public header contents simple. The basic types are 
// intended to be the same as in HLSL Shader Model 5.0.

#pragma once
#ifndef oMathTypes_h
#define oMathTypes_h

#include <oooii/oMathInternalTypes.h>

// _____________________________________________________________________________
// HLSL types

typedef unsigned short ushort;
typedef TVEC2<short> short2; typedef TVEC2<ushort> ushort2;
typedef TVEC3<short> short3; typedef TVEC3<ushort> ushort3;
typedef TVEC4<short> short4; typedef TVEC4<ushort> ushort4;

typedef unsigned int uint;
typedef TVEC2<int> int2; typedef TVEC2<uint> uint2;
typedef TVEC3<int> int3; typedef TVEC3<uint> uint3;
typedef TVEC4<int> int4; typedef TVEC4<uint> uint4;

typedef unsigned long long ulonglong;
typedef TVEC2<long long> longlong2; typedef TVEC2<ulonglong> ulonglong2;
typedef TVEC3<long long> longlong3; typedef TVEC3<ulonglong> ulonglong3;
typedef TVEC4<long long> longlong4; typedef TVEC4<ulonglong> ulonglong4;

typedef TVEC2<float> float2; typedef TVEC2<double> double2;
typedef TVEC3<float> float3; typedef TVEC3<double> double3;
typedef TVEC4<float> float4; typedef TVEC4<double> double4;

typedef TMAT3<float> float3x3; typedef TMAT3<double> double3x3;
typedef TMAT4<float> float4x4; typedef TMAT4<double> double4x4;

// _____________________________________________________________________________
// Additional types

typedef TQUAT<float> quatf; typedef TQUAT<double> quatd;
typedef TFRUSTUM<float> oFrustumf; //typedef TFRUSTUM<double> oFrustumd; // @oooii-tony: Need an oIntersects for double
typedef TSPHERE<float> oSpheref; typedef TSPHERE<double> oSphered;
typedef TPLANE<float> oPlanef; typedef TPLANE<double> oPlaned;
typedef TAABOX<float, TVEC3<float>> oAABoxf; typedef TAABOX<double, TVEC3<float>> oAABoxd;
typedef TAABOX<int, TVEC2<int>> oRECT;
typedef TAABOX<float, TVEC2<float>> oRECTF;

#endif
