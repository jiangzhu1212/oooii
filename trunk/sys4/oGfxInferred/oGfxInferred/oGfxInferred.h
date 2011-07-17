// $(header)
#pragma once
#ifndef oGfxInferred_h
#define oGfxInferred_h

#include <oooii/oInterface.h>

interface oGfxScene;
interface oGfxSceneChild : oInterface
{
	virtual void GetScene(threadsafe oGfxScene** _ppScene) const threadsafe = 0;

	virtual const char* GetName() const threadsafe = 0;
};

interface oGfxSceneResource : oGfxSceneChild
{
	enum TYPE
	{
		ACTOR,
		LIGHT,
		VIEWER,
	};

	virtual TYPE GetType() const threadsafe = 0;
};

interface oGfxActor : oGfxSceneResource
{
	struct DESC
	{
		float4x4 Transform;
		oAABoxf LocalSpaceBounds;
		oStringID Mesh;
		oStringID MaterialSetOverride; // overrides the Mesh's material set
		oStringID MaterialSetOverrideOverride; // overrides the material set override or Mesh's material set
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
	virtual void SetDesc(const DESC& _Desc) threadsafe = 0;
};

interface oGfxScene : oInterface
{
	enum DEBUG_VISUALIZATION
	{
		// Replace normal rendering with a view into an isolated component of the
		// rendering pipeline. NONE implies normal/full rendering.
		NONE,
		GE_VIEWSPACE_LINEAR_DEPTH,
		GE_VIEWSPACE_NORMALS,
		GE_VIEWSPACE_NORMALS_X,
		GE_VIEWSPACE_NORMALS_Y,
		GE_VIEWSPACE_NORMALS_Z,
		GE_CONTINUITY_IDS,
		GE_INSTANCE_IDS,
		GE_SHININESS,
		LI_DIFFUSE,
		LI_SPECULAR,
		NUM_DEBUG_VISUALIZATIONS,
	};

	enum PIPELINE_STATE
	{
		GE_OPAQUE,
		GE_TEST,
		GE_BLEND,
		LI_NONSHADOWING,
		LI_SHADOWING,
		MA_OPAQUE,
		MA_TEST,
		MA_BLEND,
		LN_OPAQUE,
		PI_OPAQUE,
		PI_TEST,
		PI_BLEND,
		PI_LINES,
		NUM_PIPELINE_STATES,
	};

	virtual bool CreateActor(const char* _Name, const oGfxActor::DESC& _Desc, threadsafe oGfxActor** _ppActor) threadsafe = 0;
	virtual bool CreateLight(threadsafe oGfxLight** _ppLight) threadsafe = 0;
	virtual bool CreateViewer(threadsafe oGfxViewer** _ppViewer) threadsafe = 0;

	virtual void SetDebugVisualization(DEBUG_VISUALIZATION _DebugVisualization) threadsafe = 0;
	virtual DEBUG_VISUALIZATION GetDebugVisualization() const threadsafe = 0;
};

oAPI bool oGfxInferredCreateDevice(threadsafe oGfxDevice* _pGfxDevice, threadsafe oGfxInferredDevice** _ppDevice);

#endif
