// $(header)
#pragma once
#ifndef oGfx_h
#define oGfx_h

#include <oGfx/oGfxState.h>

#include <oooii/oInterface.h>
#include <oooii/oSurface.h>
#include <oooii/oStringID.h>

// Main SW abstraction for a graphics processor
interface oGfxDevice;

interface oGfxDeviceChild : oInterface
{
	// Anything allocated from oGfxDevice is an oGfxDeviceChild

	// fill the specified pointer with this resources's associated
	// device. The device's ref count is incremented.
	virtual void GetDevice(threadsafe interface oGfxDevice** _ppDevice) const threadsafe = 0;

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
		MATERIAL,
		MESH,
		TEXTURE,
	};

	// Returns the type of this resource.
	virtual TYPE GetType() const threadsafe = 0;
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
			, ColorFormat(oSurface::R8G8B8A8_UNORM)
			, DepthStencilFormat(oSurface::UNKNOWN)
			, Type(TEXTURE2D)
		{}
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGfxPipeline : oGfxDeviceChild
{
	struct DESC
	{
		const void* pInputLayout;
		unsigned int InputLayoutByteWidth;
		const unsigned char* pVertexShader;
		const unsigned char* pHullShader;
		const unsigned char* pDomainShader;
		const unsigned char* pGeometryShader;
		const unsigned char* pPixelShader;
	};
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
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Modifies the values for clearing without modifying 
	// other topology descriptions
	virtual void SetClearDesc(const CLEAR_DESC& _ClearDesc) threadsafe = 0;

	// Resizes all buffers without changing formats and 
	// other topology descriptions
	virtual void Resize(uint _Width, uint _Height) threadsafe = 0;
};

interface oGfxCommandList : oGfxDeviceChild
{
	// A container for a list of commands issued by the user
	// to the graphics device. All operations herein are 
	// single-threaded. Separate command lists can be built
	// in different threads.

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
	// context should occur between Begin() and End(). NOTE: If _NumViewports
	// is 0 and/or _pViewports is NULL, a default full-rendertarget viewport
	// will be calculated and used.
	virtual void Begin(
		const float4x4& View
		, const float4x4& Projection
		, const oGfxPipeline* _pPipeline
		, const oGfxRenderTarget* _pRenderTarget
		, size_t _NumViewports
		, const VIEWPORT* _pViewports) = 0;

	// Ends recording of GPU submissions and caches a command list
	virtual void End() = 0;

	// Set the reasterization state in this context
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

	// Submits an oGfxMesh for drawing using the current state
	// of the command list.
	virtual void Draw(float4x4& _Transform, uint _MeshID, const oGfxMesh* _pMesh, size_t _SectionIndex) = 0;

	// Draws a quad in clip space (-1,-1 top-left to 1,1 bottom-right)
	// with texture coords in screen space (0,0 top-left to 1,1 bottom right)
	// If View, Projection, and _Transform are all identity, this would
	// be a fullscreen quad.
	virtual void DrawQuad(float4x4& _Transform, uint _MeshID) = 0;

	// Draws a worldspace line
	virtual void DrawLine(uint _LineID, const LINE& _Line) = 0;
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
	virtual void CreatePipeline(const char* _Name, const oGfxPipeline::DESC& _Desc, oGfxPipeline** _ppPipeline) threadsafe = 0;
	virtual bool CreateRenderTarget(const char* _Name, const oGfxRenderTarget::DESC& _Desc, oGfxRenderTarget** _ppRenderTarget) threadsafe = 0;
	virtual bool CreateMaterial(const char* _Name, const oGfxMaterial::DESC& _Desc, oGfxMaterial** _ppMaterial) threadsafe = 0;
	virtual bool CreateMesh(const char* _Name, const oGfxMesh::DESC& _Desc, oGfxMesh** _ppMesh) threadsafe = 0;
	virtual bool CreateTexture(const char* _Name, const oGfxTexture::DESC& _Desc, oGfxTexture** _ppTexture) threadsafe = 0;

	// Submits all command lists in their draw order
	virtual void Submit() = 0;
};

oAPI bool oGfxCreateDevice(const oGfxDevice::DESC& _Desc, threadsafe oGfxDevice** _ppDevice);
oAPI const char* oGfxGetResourceTypename(const threadsafe oGfxResource::TYPE _Type);

#endif
