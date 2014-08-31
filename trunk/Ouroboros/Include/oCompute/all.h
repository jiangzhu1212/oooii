// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Convenience "all headers" header for precompiled header files. Do NOT use 
// this to be lazy when including headers in .cpp files. Be explicit.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oCompute_h
#define oCompute_h
#include <oCompute/arcball.h>
#include <oCompute/eye.h>
#include <oCompute/linear_algebra.h>
#include <oCompute/oComputeConstants.h>
#include <oCompute/oComputeFilter.h>
#include <oCompute/oComputeGBuffer.h>
#include <oCompute/oComputePhong.h>
#include <oCompute/oComputeMorton.h>
#include <oCompute/oComputeProcedural.h>
#include <oCompute/oQuaternion.h>
#include <oCompute/oComputeRaycast.h>
#include <oCompute/oComputeSimplex.h>
#include <oCompute/oComputeUtil.h>
#include <oCompute/oFrustum.h>
#endif
