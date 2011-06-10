// $(header)
#pragma once
#ifndef SYS4RenderState_h
#define SYS4RenderState_h

enum oBLEND_STATE
{
	oBLEND_STATE_NONE, // src rgba (opaque)
	oBLEND_STATE_TEST, // alpha test
	oBLEND_STATE_ACCUMULATE, // src rgba + dst rgba
	oBLEND_STATE_ADDITIVE, // src rgb * src a  +  dst rgb
	oBLEND_STATE_TRANSLUCENT, // src rgb * src a  +  dst rgb * (1 - src a)
	oBLEND_STATE_PARTIAL_TRANSLUCENT, // treats values above alpha-test threshold as opaque, and then the rest as TRANSLUCENT
	oBLEND_STATE_COUNT,
};

enum oRASTERIZER_STATE
{
	oRASTERIZER_STATE_FRONT_FACE,
	oRASTERIZER_STATE_BACK_FACE,
	oRASTERIZER_STATE_TWO_SIDED,
	oRASTERIZER_STATE_FRONT_WIREFRAME,
	oRASTERIZER_STATE_BACK_WIREFRAME,
	oRASTERIZER_STATE_TWO_SIDED_WIREFRAME,
	oRASTERIZER_STATE_FRONT_POINTS,
	oRASTERIZER_STATE_BACK_POINTS,
	oRASTERIZER_STATE_TWO_SIDED_POINTS,
	oRASTERIZER_STATE_COUNT,
};

enum oSAMPLER_STATE
{
	oSAMPLER_STATE_POINT_CLAMP,
	oSAMPLER_STATE_POINT_WRAP,
	oSAMPLER_STATE_LINEAR_CLAMP,
	oSAMPLER_STATE_LINEAR_WRAP,
	oSAMPLER_STATE_ANISOTROPIC_CLAMP,
	oSAMPLER_STATE_ANISOTROPIC_WRAP,
};

enum oMIP_BIAS
{
	oMIP_BIAS_NONE,
	oMIP_BIAS_UP1,
	oMIP_BIAS_UP2,
	oMIP_BIAS_DOWN1,
	oMIP_BIAS_DOWN2,
	oMIP_BIAS_COUNT,
};

#endif
