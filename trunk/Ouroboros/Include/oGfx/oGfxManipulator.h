// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#ifndef oGfxManipulator_h
#define oGfxManipulator_h

#include <oBasis/oManipulator.h>
#include <oGPU/oGPUUtilMesh.h>

interface oGfxManipulator : oInterface
{
public:
	struct DESC : public oManipulator::DESC
	{
	};

	virtual void GetDesc(DESC* _pDesc) const = 0;

	// Returns true if the manipulator 
	virtual bool Pick(uint _ObjectID) = 0;
	virtual void StopPick() = 0;

	// Mouse position must be in screen space. i.e. -1 to 1 for both x and y
	virtual void SetMousePosition(float2 _position) = 0;
	virtual void SetViewProjection(float4x4 _ViewMatrixLH, float4x4 _ProjectionMatrixLH) = 0;
	virtual void GetTransform(float4x4 *_pTransform) const = 0;
	virtual void SetTransform(const float4x4& _Transform) = 0;
#if 0
	// A GPU Manipulator is made up of Lines and Meshes for visulazation.  
	// These lines and meshes should be drawn with some sort of ObjectID
	// picking shader to determine if they are picked.  When drawing lines
	// for this purpase a special mesh that is thicker than the actual line
	// is drawn hence GetManipulatorPickLineMeshes
	virtual void GetManipulatorPickLineMeshes(ouro::gpu::command_list* _pCommandList, std::function<void(ouro::gpu::util_mesh* _pMesh, uint _ObjectID)> _Callback) = 0;
	virtual void GetManipulatorVisualLines(ouro::gpu::command_list* _pCommandList, std::function<void(ouro::gpu::buffer* _pLineList, uint _NumLines)> _Callback) = 0;
	virtual void GetManipulatorMeshes(ouro::gpu::command_list* _pCommandList, std::function<void(ouro::gpu::util_mesh* _pMesh, float4x4 _Transform, ouro::color _MeshColor, uint _ObjectID)> _Callback) = 0;
#endif
};

bool oGfxManipulatorCreate(const char* _Name, const oGfxManipulator::DESC& _Desc, ouro::gpu::device* _pDevice, oGfxManipulator** _ppManipulator);

#endif
