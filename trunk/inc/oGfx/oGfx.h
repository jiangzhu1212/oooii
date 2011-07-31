// $(header)
// Cross-platform API for the major vocabulary of 3D 
// rendering while trying to remain policy-agnostic
#pragma once
#ifndef oGfx_h
#define oGfx_h

#include <oGfx/oGfxState.h>
#include <oooii/oColor.h>
#include <oooii/oInterface.h>
#include <oooii/oSurface.h>

// Main SW abstraction for a graphics processor
interface oGfxDevice;
interface oGfxDeviceChild : oInterface
{
	// Anything allocated from oGfxDevice is an oGfxDeviceChild

	// fill the specified pointer with this resources's associated
	// device. The device's ref count is incremented.
	virtual void GetDevice(threadsafe oGfxDevice** _ppDevice) const threadsafe = 0;

	// Returns the name specified at create time
	virtual const char* GetName() const threadsafe = 0;
};

interface oGfxResource : oGfxDeviceChild
{
	// Anything that contains data intended primarily for readonly
	// access by the GPU processor is a resource. This does not 
	// exclude write access, but generally differentiates these
	// objects from process and target interfaces

	enum TYPE
	{
		LINELIST,
		MATERIAL,
		MESH,
		TEXTURE,
	};

	// Returns the type of this resource.
	virtual TYPE GetType() const threadsafe = 0;

	// Returns the name of the original source for this resource.
	// GetName() returns the name of the actual, reformatted,
	// runtime-optimal source. This is the artist-friendly 
	// development name.
	virtual const char* GetSourceName() const threadsafe = 0;

	// Returns an ID for this resource fit for use as a hash
	virtual uint GetID() const threadsafe = 0;
};

interface oGfxLineList : oGfxResource
{
	struct DESC
	{
		DESC()
			: MaxNumlines(0)
			, NumLines(0)
		{}

		uint MaxNumLines;
		uint NumLines;
	};

	struct LINE
	{
		LINE()
			: Start(float3(0.0f, 0.0f, 0.0f))
			, StartColor(std::Black)
			, End(float3(0.0f, 0.0f, 0.0f))
			, EndColor(std::Black)
		{}

		float3 Start;
		oColor StartColor;
		float3 End;
		oColor EndColor;
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
	virtual void Resize(uint _MaxNumLines) = 0;
	virtual void SetNumLines(uint _NumLines) = 0;
};

interface oGfxMaterial : oGfxResource
{
	// A material is the package of constants only. Textures
	// are stored separately. It is up to the user to ensure
	// that material constants, textures, and pipeline states
	// match up for the intended effect.

	struct DESC
	{
		uint ByteSize;
		uint ArraySize;
		bool FrequentUpdate;

		DESC()
			: ByteSize(0)
			, ArraySize(1)
			, FrequentUpdate(false)
		{}
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGfxMesh : oGfxResource
{
	// Represents irregular geometry data sampling and 
	// the topology connecting those sample points to 
	// approximate geometry with a surface composed of 
	// triangles. Ranges of triangles are grouped, but
	// kept separate from one another so that a 
	// continuous shape can be constructed by multiple
	// draw calls, each with a different render state.

	struct DESC
	{
		uint VertexByteSize[3];
		uint NumIndices;
		uint NumVertices;
		uint NumRanges;
		oAABoxf LocalSpaceBounds;
		bool FrequentIndexUpdate;
		bool FrequentVertexUpdate[3];

		DESC()
			: NumIndices(0)
			, NumVertices(0)
			, NumRanges(0)
			, FrequentIndexUpdate(false)
		{
			for (size_t i = 0; i < oCOUNTOF(VertexByteSize); i++)
			{
				VertexByteSize[i] = 0;
				FrequentVertexUpdate[i] = false;
			}
		}
	};

	enum SUBRESOURCE
	{
		RANGES,
		INDICES, // always 32-bit uints when mapped
		VERTICES0,
		VERTICES1,
		VERTICES2,
	};

	struct RANGE
	{
		uint StartTriangle; // index buffer offset in # of triangles
		uint NumTriangles;
		uint MinVertex; // min/max indices of vertices that will be  
		uint MaxVertex; // accessed by this range
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGfxMeshInstances : oGfxResource
{
	// Contains instance data for drawing instances
	// of a mesh. The contents of each instance's data
	// is user-defined using IAELEMENTs. This
	// is more or less a different semantic of vertex
	// attributes

	struct DESC
	{
		const oIAELEMENT* pElements;
		uint NumElements;
		uint MaxNumInstances;
		uint NumInstances;

		DESC()
			: pElements(nullptr)
			, NumElements(0)
			, MaxNumInstances(0)
			, NumInstances(0)
		{}
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
	virtual void Resize(uint _MaxNumInstances) = 0;
	virtual void SetNumInstances(uint _NumInstances) = 0;
};

interface oGfxTexture : oGfxResource
{
	// A large buffer filled with surface data. Most often
	// this is a 2D plane wrapped onto the surface of a screen
	// or mesh. Indeed a CUBEMAP is an extension of this concept
	// mapping a view in each of the 6 principal axes onto a plane.
	// A TEXTURE3D is an approximation of a 3D space by sampling
	// at discreet planes called slices throughout the volume.

	enum TYPE
	{
		TEXTURE2D,
		TEXTURE3D,
		CUBEMAP,
	};

	struct DESC
	{
		DESC()
			: Width(1)
			, Height(1)
			, Depth(1)
			, NumMips(0)
			, NumSlices(1)
			, ColorFormat(oSurface::R8G8B8A8_UNORM)
			, DepthStencilFormat(oSurface::UNKNOWN)
			, Type(TEXTURE2D)
		{}

		uint Width;
		uint Height;
		uint Depth;
		uint NumMips; // 0 means auto-gen mips
		uint NumSlices;
		oSurface::FORMAT ColorFormat;
		oSurface::FORMAT DepthStencilFormat; // oSurface::UNKNOWN means none
		TYPE Type;
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGfxPipeline : oGfxDeviceChild
{
	// A pipeline is the result of setting all stages
	// of the programmable pipeline (all shaders) and
	// the vertex input format to that pipeline.

	struct DESC
	{
		DESC()
			: pElements(nullptr)
			, NumElements(0)
			, pVSByteCode(nullptr)
			, pHSByteCode(nullptr)
			, pDSByteCode(nullptr)
			, pGSByteCode(nullptr)
			, pPSByteCode(nullptr)
		{}

		const oIAELEMENT* pElements;
		uint NumElements;
		const uchar* pVSByteCode;
		const uchar* pHSByteCode;
		const uchar* pDSByteCode;
		const uchar* pGSByteCode;
		const uchar* pPSByteCode;
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGfxRenderTarget : oGfxDeviceChild
{
	// A 2D plane onto which rendering occurs

	// MRT: Multiple Render Target
	static const size_t MAX_MRT_COUNT = 8;
	static const size_t DS_INDEX = ~0u;

	struct CLEAR_DESC
	{
		oColor ClearColor[MAX_MRT_COUNT];
		float DepthClearValue;
		unsigned char StencilClearValue;

		CLEAR_DESC()
			: DepthClearValue(1.0f)
			, StencilClearValue(0)
		{
			for (size_t i = 0; i < MAX_MRT_COUNT; i++)
				ClearColor[i] = 0;
		}
	};

	struct DESC
	{
		uint Width;
		uint Height;
		uint MRTCount;
		uint ArraySize;
		oSurface::FORMAT Format[MAX_MRT_COUNT];
		oSurface::FORMAT DepthStencilFormat; // Use UNKNOWN for no depth
		CLEAR_DESC ClearDesc;
		bool GenerateMips;

		DESC()
			: Width(256)
			, Height(256)
			, MRTCount(1)
			, ArraySize(1)
			, DepthStencilFormat(oSurface::UNKNOWN)
			, GenerateMips(false)
		{
			for (size_t i = 0; i < MAX_MRT_COUNT; i++)
				Format[i] = oSurface::UNKNOWN;
		}
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Modifies the values for clearing without modifying 
	// other topology descriptions
	virtual void SetClearDesc(const CLEAR_DESC& _ClearDesc) threadsafe = 0;

	// Resizes all buffers without changing formats and 
	// other topology descriptions
	virtual void Resize(uint _Width, uint _Height) = 0;
};

interface oGfxCommandList : oGfxDeviceChild
{
	// A container for a list of commands issued by the user
	// to the graphics device. All operations herein are 
	// single-threaded. For parallelism separate command 
	// lists can be built in different threads.

	enum CLEAR_TYPE
	{
		DEPTH,
		STENCIL,
		DEPTH_STENCIL,
		COLOR,
		COLOR_DEPTH,
		COLOR_STENCIL,
		COLOR_DEPTH_STENCIL,
	};

	struct VIEWPORT
	{
		float TopLeftX;
		float TopLeftY;
		float Width;
		float Height;
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
	
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Begins recording of GPU command submissions. All rendering for this 
	// context should occur between Begin() and End(). NOTE: If _NumViewports
	// is 0 and/or _pViewports is nullptr, a default full-rendertarget 
	// viewport will be calculated and used.
	virtual void Begin(
		const float4x4& _View
		, const float4x4& _Projection
		, const oGfxPipeline* _pPipeline
		, const oGfxRenderTarget* _pRenderTarget
		, size_t _RenderTargetIndex
		, size_t _NumViewports
		, const VIEWPORT* _pViewports) = 0;

	// Ends recording of GPU submissions and caches a command list
	virtual void End() = 0;

	// Set the rasterization state in this context
	virtual void RSSetState(oRSSTATE _State) = 0;

	// Set the output merger (blend) state in this context
	virtual void OMSetState(oOMSTATE _State) = 0;

	// Set the depth-stencil state in this context
	virtual void DSSetState(oDSSTATE _State) = 0;

	// Set the texture sampler states in this context
	virtual void SASetStates(size_t _StartSlot, size_t _NumStates, const oSASTATE* _pSAStates, const oMBSTATE* _pMBStates) = 0;

	// Set the textures in this context
	virtual void SetTextures(size_t _StartSlot, size_t _NumTextures, const oGfxTexture* const* _ppTextures) = 0;

	// Set the material constants in this context
	virtual void SetMaterials(size_t _StartSlot, size_t _NumMaterials, const oGfxMaterial* const* _ppMaterials) = 0;

	// Maps the specified resource and returns a pointer to a writable 
	// buffer. It is up to the user to ensure sizes and protect against
	// memory overwrites. The mapped buffer is not readable, i.e. does
	// not have the buffer's current/prior value in it. This is a 
	// necessary behavior so the graphics device does not have to keep
	// a copy or stall while it sync's data to the buffer. 
	// The value of _SubresourceIndex indicates:
	// Material: The nth material as specified by ArraySize
	// Mesh: A oMesh::SUBRESOURCE value
	// Texture: A value returned by oSurface::CalculateSubresource()
	virtual void Map(oGfxResource* _pResource, size_t _SubresourceIndex, MAPPING* _pMapping) = 0;
	virtual void Unmap(oGfxResource* _pResource, size_t _SubresourceIndex) = 0;

	// Uses a render target's CLEAR_DESC to clear all associated buffers
	// according to the type of clear specified here.
	virtual void Clear(CLEAR_TYPE _ClearType) = 0;

	// Submits an oGfxMesh for drawing using the current state of the 
	// command list.
	virtual void DrawMesh(float4x4& _Transform, uint _MeshID, const oGfxMesh* _pMesh, size_t _RangeIndex) = 0;

	// Draws a set of worldspace lines. Use Map/Unmap to set up line lists
	// writing an array of type oGfxLineList::LINEs.
	virtual void DrawLines(uint _LineListID, const oGfxLineList* _pLineList) = 0;

	// Draws a quad in clip space (-1,-1 top-left to 1,1 bottom-right)
	// with texture coords in screen space (0,0 top-left to 1,1 bottom right)
	// If View, Projection, and _Transform are all identity, this would
	// be a fullscreen quad.
	virtual void DrawQuad(float4x4& _Transform, uint _QuadID) = 0;
};

interface oGfxDevice : oInterface
{
	// Main SW abstraction for a graphics processor

	struct DESC
	{
		float Version;
		bool UseSoftwareEmulation;
		bool EnableDebugReporting;
	};

	virtual bool CreateCommandList(const char* _Name, const oGfxCommandList::DESC& _Desc, oGfxCommandList** _ppCommandList) threadsafe = 0;
	virtual bool CreateLineList(const char* _Name, const oGfxLineList::DESC& _Desc, oGfxLineList** _ppLineList) threadsafe = 0;
	virtual bool CreatePipeline(const char* _Name, const oGfxPipeline::DESC& _Desc, oGfxPipeline** _ppPipeline) threadsafe = 0;
	virtual bool CreateRenderTarget(const char* _Name, const oGfxRenderTarget::DESC& _Desc, oGfxRenderTarget** _ppRenderTarget) threadsafe = 0;
	virtual bool CreateRenderTarget(const char* _Name, const oWindow* _pWindow, oGfxRenderTarget2** _ppRenderTarget) threadsafe = 0;
	virtual bool CreateMaterial(const char* _Name, const oGfxMaterial::DESC& _Desc, oGfxMaterial** _ppMaterial) threadsafe = 0;
	virtual bool CreateMesh(const char* _Name, const oGfxMesh::DESC& _Desc, oGfxMesh** _ppMesh) threadsafe = 0;
	virtual bool CreateMeshInstances(const char* _Name, const oGfxMeshInstances::DESC& _Desc, oGfxMeshInstances** _ppMeshInstances) threadsafe = 0;
	virtual bool CreateTexture(const char* _Name, const oGfxTexture::DESC& _Desc, oGfxTexture** _ppTexture) threadsafe = 0;

	// Submits all command lists in their draw order
	virtual void Submit() = 0;
};

oAPI bool oGfxCreateDevice(const oGfxDevice::DESC& _Desc, threadsafe oGfxDevice** _ppDevice);

oAPI const char* oAsString(const oGfxResource::TYPE& _Type);
oAPI const char* oAsString(const oGfxMesh::SUBRESOURCE& _Subresource);
oAPI const char* oAsString(const oGfxTexture::TYPE& _Type);
oAPI const char* oAsString(const oGfxCommandList::CLEAR_TYPE& _Type);

#endif
