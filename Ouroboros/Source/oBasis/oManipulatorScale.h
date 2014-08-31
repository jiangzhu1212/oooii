// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oManipulatorScale_h
#define oManipulatorScale_h

#include "oManipulatorBase.h"

// {143C663B-7BF4-499a-B4BF-F6E852455A15}
oDEFINE_GUID_S(oManipulatorScale, 0x143c663b, 0x7bf4, 0x499a, 0xb4, 0xbf, 0xf6, 0xe8, 0x52, 0x45, 0x5a, 0x15);
struct oManipulatorScale : oManipulatorBase
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oManipulatorScale);

	oManipulatorScale(const DESC& _Desc, bool *_pSuccess);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,);

	virtual void BeginManipulation(AXIS _Axis, const float2& _ScreenPosition);
	virtual void GetTransform(float4x4 *_pTransform) const;
	virtual void GetMaxSizes(size_t &_maxNumLines, size_t &_maxNumPickGeometry) const {_maxNumLines = 4; _maxNumPickGeometry = 4;}
private:
	void UpdateImpl() override;
	float3 CalcScale(float3 axis); 
	bool CalcGeometry(AXIS _Axis); //returns whether the line/geom should be clipped
	void CalcCap(AXIS _Axis, float4 vert, const float4x4 &_capRotation);
};

#endif // oManipulatorScale_h