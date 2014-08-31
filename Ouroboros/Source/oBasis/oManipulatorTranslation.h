// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oManipulatorTransition_h
#define oManipulatorTransition_h

#include "oManipulatorbase.h"

// {0E3FDA0A-2397-4964-A3C6-3E013E63C476}
oDEFINE_GUID_S(oManipulatorTranslation, 0xe3fda0a, 0x2397, 0x4964, 0xa3, 0xc6, 0x3e, 0x1, 0x3e, 0x63, 0xc4, 0x76);
struct oManipulatorTranslation : oManipulatorBase
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oManipulatorTranslation);

	oManipulatorTranslation(const DESC& _Desc, bool *_pSuccess);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,);

	virtual void BeginManipulation(AXIS _Axis, const float2& _ScreenPosition);
	virtual void GetTransform(float4x4 *_pTransform) const;
	virtual void GetMaxSizes(size_t &_maxNumLines, size_t &_maxNumPickGeometry) const {_maxNumLines = 4; _maxNumPickGeometry = 4;}
private:
	void UpdateImpl() override;
	float3 CalcTranslation(float3 axis); 
	bool CalcGeometry(AXIS _Axis); //returns whether the line/geom should be clipped
	void CalcCap(AXIS _Axis, float4 vert, const float4x4 &_capRotation); 
};

#endif // oManipulatorTransition_h