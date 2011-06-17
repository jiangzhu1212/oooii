// $(header)
#pragma once
#ifndef oManipulator_h
#define oManipulator_h

#include <oooii/oInterface.h>
#include <oooii/oMath.h>
#include <vector>

interface oManipulator : oInterface
{
	static const unsigned int ROTATION_CIRCLE_VCOUNT = 360;
	static const unsigned int ROTATION_PICK_TORUS_DIVIDE = 120;
	static const unsigned int ROTATION_PICK_TORUS_FACET = 8;
	static const unsigned int ROTATION_PICK_ARCBALL_VCOUNT = 64;

	enum AXIS
	{
		X,
		Y,
		Z,
		SCREEN,
		FREE, //only for rotation, arcball
		NUM_AXES,
	};
	
	struct DESC
	{
		enum MODE
		{
			OBJECT,
			WORLD,
			NUM_MODES,
		};

		enum TYPE
		{
			TRANSLATION,
			SCALE,
			ROTATION,
			NUM_TYPES,
		};

		DESC()
			: Type(TRANSLATION), Mode(WORLD), PickWidth(0.04f)
		{}
		TYPE Type;
		MODE Mode;
		float PickWidth;
	};

	static bool Create(const DESC& _Desc, oManipulator** _ppManipulator);

	virtual void SetDesc(const DESC* _pDesc) = 0;
	virtual void GetDesc(DESC* _pDesc) const = 0;

	virtual void BeginManipulation(AXIS _Axis, const float2& _ScreenPosition) = 0;
	virtual void EndManipulation() = 0;
	virtual void Manipulate(const float2& _ScreenPosition) = 0;
	virtual void Update(const float4x4 &_World, const float4x4 &_View, const float4x4 &_Proj) = 0;
	virtual void GetTransform(float4x4 *_pTransform) const = 0;
	virtual void GetMaxSizes(size_t &_maxNumLines, size_t &_maxNumPickGeometry) const = 0;
	virtual void GetLines(AXIS _Axis, float3 *_lines, size_t &_numWritten) const = 0;
	virtual void GetPickGeometry(AXIS _Axis, float3 *_vertices, size_t &_numWritten) const = 0;
	virtual void GetCapTransform(AXIS _Axis, float4x4 &_transforms) const = 0;
	virtual bool IsClipped(AXIS _Axis) const = 0;
};
	
inline void operator++(oManipulator::AXIS &_arg)
{
	_arg = static_cast<oManipulator::AXIS>(_arg + 1);
}

#endif // oManipulator_h
