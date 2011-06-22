// $(header)
#pragma once
#ifndef oGfxState_h
#define oGfxState_h

enum oOMSTATE // Output Merge (Blend) State
{
	oOM_NONE, // src rgba (opaque)
	oOM_TEST, // alpha test
	oOM_ACCUMULATE, // src rgba + dst rgba
	oOM_ADDITIVE, // src rgb * src a  +  dst rgb
	oOM_TRANSLUCENT, // src rgb * src a  +  dst rgb * (1 - src a)
	oOM_COUNT,
};

enum oRSSTATE // Rasterizer State
{
	oRS_FRONT_FACE,
	oRS_BACK_FACE,
	oRS_TWO_SIDED,
	oRS_FRONT_WIREFRAME,
	oRS_BACK_WIREFRAME,
	oRS_TWO_SIDED_WIREFRAME,
	oRS_FRONT_POINTS,
	oRS_BACK_POINTS,
	oRS_TWO_SIDED_POINTS,
	oRS_COUNT,
};

enum oDSSTATE // Depth-Stencil State
{
	oDS_NONE,
	oDS_TEST_AND_WRITE,
	oDS_TEST,
	oDS_STATE_COUNT,
};

enum oSASTATE // Sampler State
{
	oSA_POINT_CLAMP,
	oSA_POINT_WRAP,
	oSA_LINEAR_CLAMP,
	oSA_LINEAR_WRAP,
	oSA_ANISOTROPIC_CLAMP,
	oSA_ANISOTROPIC_WRAP,
	oSA_COUNT,
};

enum oMBSTATE // Mip-Bias State
{
	oMB_NONE,
	oMB_UP1,
	oMB_UP2,
	oMB_DOWN1,
	oMB_DOWN2,
	oMB_COUNT,
};

#endif
