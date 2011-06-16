// $(header)
#pragma once
#ifndef SYS4Render_h
#define SYS4Render_h

#include <SYS4/SYS4Render.h>
#include <SYS4/SYS4RenderState.h>

#include <oooii/oInterface.h>

// Main SW abstraction for a GPU
interface oGPUDevice;

interface oGPUDeviceChild : oInterface
{
	// All GPU objects are related to a device

	// fill the specified pointer with this child's associated
	// device. The device's refcount is incremented.
	virtual void GetDevice(threadsafe oGPUDevice** _ppDevice) const threadsafe = 0;
};

interface oGPUResource : oGPUDeviceChild
{
	enum TYPE
	{
		CONTEXT,
		MATERIAL,
		MESH,
		RENDERTARGET,
		TEXTURE,
	};

	// Returns the type of this resource.
	virtual TYPE GetType() const threadsafe = 0;

	// Returns the name with which this object was created
	virtual const char* GetName() const threadsafe = 0;

	// Returns the resolved name of a GPU-friendly cached
	// version of the original resource specified by GetName()
	virtual const char* GetCacheName() const threadsafe = 0;
};

interface oGPUMaterial : oGPUResource
{
	struct DESC : oCBMaterial
	{
		Textures[oTEXTURE_CHANNEL_COUNT]
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPUMesh : oGPUResource
{
	struct DESC
	{
		uint NumIndices;
		uint NumVertices;
		uint NumRanges;
		oAABoxf LocalSpaceBounds;
		bool FrequentIndexUpdate;
		bool FrequentVertexUpdate;

		DESC()
			: NumIndices(0)
			, NumVertices(0)
			, NumRanges(0)
			, FrequentIndexUpdate(false)
			, FrequentVertexUpdate(false)
			, IsAnimated(false)
		{}
	};

	enum SUBRESOURCE
	{
		RANGES,
		INDICES, // always 32-bit uints when mapped
		VERTICES,
		SKINNING, // weights and indices
	};

	struct RANGE
	{
		uint StartTriangle; // index buffer offset in # of triangles
		uint NumTriangles;
		uint MinVertex; // min/max indices of vertices that will be  
		uint MaxVertex; // accessed by this range
	};

	struct VERTEX
	{
		float3 Position; 
		float2 Texcoord;
		float3 Normal;
		float4 Tangent;
	};

	struct SKINNING
	{
		float4 Weights;
		unsigned char Indices[4];
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPURenderTarget
{
	static const size_t MAX_MRT_COUNT = 8;

	struct DESC
	{
		uint Width;
		uint Height;
		uint MRTCount;
		uint ArraySize;
		oSurface::FORMAT Format[MAX_MRT_COUNT];
		oSurface::FORMAT DepthStencilFormat; // Use UNKNOWN for no depth
		oColor ClearColor[MAX_MRT_COUNT];
		float DepthClearValue;
		UINT8 StencilClearValue;
		bool GenerateMips;
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPUTexture : oGPUResource
{
	enum TYPE
	{
		TEXTURE2D,
		TEXTURE3D,
		CUBEMAP,
	};

	struct DESC
	{
		uint Width;
		uint Height;
		uint Depth;
		uint NumMips; // 0 means auto-gen mips
		uint NumSlices;
		oSurface::FORMAT ColorFormat;
		oSurface::FORMAT DepthStencilFormat; // oSurface::UNKNOWN means none
		TYPE Type;

		DESC()
			: Width(1)
			, Height(1)
			, Depth(1)
			, NumMips(0)
			, NumSlices(1)
			, ColorFormat(oSurface::RGBA)
			, DepthStencilFormat(oSurface::UNKNOWN)
			, Type(TEXTURE2D)
		{}
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPUContext : oGPUResource
{
	enum PIPELINE_STATE
	{
		OPAQUE_GEOMETRY,
		ALPHATESTED_GEOMETRY,
		ALPHABLENDED_GEOMETRY,
		LINES,
		NONSHADOWING_LIGHTS,
		SHADOWING_LIGHTS,
		DEBUG,
	};

	enum DEBUG_VISUALIZATION
	{
		LIT,
		GE_VIEWSPACE_LINEAR_DEPTH,
		GE_VIEWSPACE_NORMALS,
		GE_VIEWSPACE_NORMALS_X,
		GE_VIEWSPACE_NORMALS_Y,
		GE_VIEWSPACE_NORMALS_Z,
		GE_CONTINUITY_IDS,
		GE_INSTANCE_IDS,
		GE_SPECULAR_EXPONENT,
		LI_DIFFUSE,
		LI_SPECULAR,
		NUM_DEBUG_VISUALIZATIONS,
	};

	struct DESC
	{
		uint DrawOrder;
	};

	struct MAPPING
	{
		void* pData;
		uint RowPitch;
		uint SlicePitch;
	};
	
	struct RENDER_STATE
	{
		float4x4 View;
		float4x4 Projection;
		PIPELINE_STATE State;
		DEBUG_VISUALIZATION DebugVisualization;
	};

	struct LINE
	{
		float3 Start;
		oColor StartColor;
		float3 End;
		oColor EndColor;
	};

	struct LIGHT
	{
		float3 Position;
		float Intensity;
		oColor Color;
	};

	// Begins recording of GPU command submissions. All rendering for this 
	// context should occur between Begin() and End().
	virtual void Begin(const RENDER_STATE& _RenderState, const oGPURenderTarget* _pRenderTarget) = 0;

	// Ends recording of GPU submissions and caches a command list
	virtual void End() = 0;

	virtual void SetRasterizerState(oRASTERIZER_STATE _RasterizerState) = 0;
	virtual void SetBlendState(oBLEND_STATE _BlendState) = 0;
	virtual void SetMaterial(const oGPUMaterial* _pMaterial) = 0;
	virtual void SetSamplerStates(size_t _StartSlot, size_t _NumSamplerStates, const oSAMPLER_STATE* _pSamplerStates, const oMIP_BIAS* _pMipBiases) = 0;
	virtual void SetTextures(size_t _StartSlot, size_t _NumTextures, const oGPUTexture* const* _ppTextures) = 0;

	virtual void Map(oGPUResource* _pResource, size_t _SubresourceIndex) = 0;
	virtual void Unmap(oGPUResource* _pResource, size_t _SubresourceIndex) = 0;
	
	// Drawing a null mesh draws a full screen quad
	virtual void Draw(float4x4& _Transform, uint _MeshID, const oGPUMesh* _pMesh, size_t _SectionIndex) = 0;

	//virtual LINE* LNBegin(size_t _LineCapacity) = 0;
	//virtual void LNEnd(size_t _NumLines) = 0;

	//virtual LIGHT* LIBegin(size_t _LightCapacity) = 0;
	//virtual void LIEnd(size_t _NumLights) = 0;
};

interface oGPUDevice : oInterface
{
	struct DESC
	{
		float Version;
		bool UseSoftwareEmulation;
		bool EnableDebugReporting;
	};

	virtual bool CreateContext(const char* _Name, const oGPUContext::DESC& _Desc, oGPUContext** _ppContext) threadsafe = 0;
	virtual bool CreateMaterial(const char* _Name, const oGPUMaterial::DESC& _Desc, threadsafe oGPUMaterial** _ppMaterial) threadsafe = 0;
	virtual bool CreateMesh(const char* _Name, const oGPUMesh::DESC& _Desc, threadsafe oGPUMesh** _ppMesh) threadsafe = 0;
	virtual bool CreateRenderTarget(const char* _Name, const oGPURenderTarget::DESC& _Desc, threadsafe oGPURenderTarget** _ppRenderTarget) threadsafe = 0;
	virtual bool CreateTexture(const char* _Name, const oGPUTexture::DESC& _Desc, threadsafe oGPUTexture** _ppTexture) threadsafe = 0;

	virtual void Begin() = 0;
	virtual void End() = 0;
};

oAPI bool oCreateGPUDevice(const oGPUDevice::DESC& _Desc, threadsafe oGPUDevice** _ppDevice);

#endif
