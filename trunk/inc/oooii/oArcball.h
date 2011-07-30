// $(header)

// Define very basic orbiting camera controls for focus on a particular point.
// This is useful for very early 3D renderer bringup to be able to move around
// a model.
#pragma once
#ifndef oArcball_h
#define oArcball_h

#include <oooii/oInterface.h>
#include <oooii/oMath.h>

interface oArcball : oInterface
{
	enum EVENT
	{
		BEGIN_ROTATING,
		ROTATING,
		BEGIN_TRANSLATING,
		TRANSLATING,
	};

	struct DESC
	{
		DESC()
			: TranslationScale(1.0f, 1.0f, 0.001f)
			, Radius(1.0f)
		{}
		float2 WindowDimensions;

		// XY is screen space, Z is mouse wheel zoom scale. A typical 
		// value would be float3(1.0f, 1.0f, 0.001f)... Z is very 
		// sensitive.
		float3 TranslationScale;
		float Radius;
	};

	static bool Create(const DESC& _Desc, oArcball** _ppArcball);

	virtual void SetDesc(const DESC* _pDesc) = 0;
	virtual void GetDesc(DESC* _pDesc) const = 0;

	// Resets to a camera that uses the specific config
	virtual void Reset(float3 _Eye, float3 _LookAt, float3 _Up) = 0;

	// Resets to an identity rotation and no translation
	virtual void Reset() = 0;

	// Converts a view matrix to a valid Arcball state
	virtual void ToView(float4x4* _pView) const = 0;

	// screenPosition is generally XY, and Z is the wheel value of 
	// the mouse. This method should be hooked up to the application's 
	// input system to drive the state of the arcball.
	virtual void Trigger(EVENT _Event, const float3& _ScreenPosition) = 0;
};

#endif
