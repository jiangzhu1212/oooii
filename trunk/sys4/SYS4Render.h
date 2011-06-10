// $(header)
#pragma once
#ifndef SYS4Render_h
#define SYS4Render_h

#include <SYS4/SYS4Render.h>

enum oBLEND_STATE
{
	oBLEND_STATE_NONE, // src rgba (opaque)
	oBLEND_STATE_TEST, // alpha test
	oBLEND_STATE_ACCUMULATE, // src rgba + dst rgba
	oBLEND_STATE_ADDITIVE, // src rgb * src a  +  dst rgb
	oBLEND_STATE_TRANSLUCENT, // src rgb * src a  +  dst rgb * (1 - src a)
	oBLEND_STATE_PARTIAL_TRANSLUCENT, // treats values above alpha-test threshold as opaque, and then the rest as TRANSLUCENT
	oBLEND_STATE_COUNT,
};

enum oRASTERIZER_STATE
{
	oRASTERIZER_STATE_FRONT_FACE,
	oRASTERIZER_STATE_BACK_FACE,
	oRASTERIZER_STATE_TWO_SIDED,
	oRASTERIZER_STATE_FRONT_WIREFRAME,
	oRASTERIZER_STATE_BACK_WIREFRAME,
	oRASTERIZER_STATE_TWO_SIDED_WIREFRAME,
	oRASTERIZER_STATE_FRONT_POINTS,
	oRASTERIZER_STATE_BACK_POINTS,
	oRASTERIZER_STATE_TWO_SIDED_POINTS,
	oRASTERIZER_STATE_COUNT,
};

enum oSAMPLER_STATE
{
	oSAMPLER_STATE_POINT_CLAMP,
	oSAMPLER_STATE_POINT_WRAP,
	oSAMPLER_STATE_LINEAR_CLAMP,
	oSAMPLER_STATE_LINEAR_WRAP,
	oSAMPLER_STATE_ANISOTROPIC_CLAMP,
	oSAMPLER_STATE_ANISOTROPIC_WRAP,
};

enum oMIP_BIAS
{
	oMIP_BIAS_NONE,
	oMIP_BIAS_UP1,
	oMIP_BIAS_UP2,
	oMIP_BIAS_DOWN1,
	oMIP_BIAS_DOWN2,
	oMIP_BIAS_COUNT,
};


// Need HLSL-C++ header with constant buffers:

interface oGPUDevice;
interface oGPUDeviceChild : oInterface
{
	virtual void GetDevice(threadsafe oGPUDevice** _ppDevice) const threadsafe = 0;
};

interface oGPUResource : oGPUDeviceChild
{
	enum TYPE
	{
		MESH,
		TEXTURE,
		MATERIAL,
		CONTEXT,
	};

	virtual TYPE GetType() const threadsafe = 0;
	virtual const char* GetName() const threadsafe = 0;
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
		VERTICES,
		INDICES, // always 32-bit uints when mapped
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
		quatf ObjectSpaceNormalRotation; // Quaternion to rotate the object space normal map vector or (0.0f, 1.0f, 0.0f) when no normalmap is bound to the object space normal 
		uint ContinuityID; // Triangles with vertices with the same ID are considered continuous (no edges/creases). This number must be 8-bit [0,255]
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPUTexture : oGPUResource
{
	struct DESC
	{
		uint Width;
		uint Height;
		uint Depth;
		uint NumMips; // 0 means auto-gen mips
		uint NumSlices;
		oSurface::FORMAT ColorFormat;
		oSurface::FORMAT DepthStencilFormat; // oSurface::UNKNOWN means none
		bool IsRenderTarget;
		bool IsCubeMap;

		DESC()
			: Width(1)
			, Height(1)
			, Depth(1)
			, NumMips(0)
			, NumSlices(1)
			, ColorFormat(oSurface::RGBA)
			, DepthStencilFormat(oSurface::UNKNOWN)
			, IsRenderTarget(false)
			, IsCubeMap(false)
		{}
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPUMaterial : oGPUResource
{
	struct DESC
	{
		oHLSLColor DiffuseColor;
		float Opacity; // 0: clear | 1: opaque

		oHLSLColor SpecularColor;
		float SpecularAmount; // 0: no specular | 1: max specular

		oHLSLColor EmissiveColor;
		float AlphaTest;

		float2 TexcoordScale;
		float2 TexcoordBias;

		float2 TexcoordScrollRate;
		float LightingContribution; // 0: emissive only | 1: phong lighting
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPUDeviceContext : oGPUResource
{
	enum PIPELINE_STATE
	{
		GE_OPAQUE,
		GE_ALPHATESTED,
		GE_ALPHABLENDED,
		SH_NONSHADOWING,
		SH_SHADOWING,
		MA_OPAQUE,
		MA_ALPHATESTED,
		MA_ALPHABLENDED,
		LI_OPAQUE,
		PI_OPAQUE,
		PI_ALPHATESTED,
		PI_ALPHABLENDED,
		PI_LINES,
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
		oColor ClearColor;
		float ClearDepthValue;
		unsigned char ClearStencilValue;
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
	virtual void Begin(const RENDER_STATE& _RenderState) = 0;

	// Ends recording of GPU submissions and caches a command list
	virtual void End() = 0;

	virtual void SetRasterizerState(RASTERIZER_STATE _RasterizerState) = 0;
	virtual void SetBlendState(BLEND_STATE _BlendState) = 0;
	virtual void SetMaterial(const MATERIAL& _Material) = 0;
	virtual void SetSamplerStates(size_t _StartSlot, size_t _NumSamplerStates, const SAMPLER_STATE* _pSamplerStates, const MIP_BIAS* _pMipBiases) = 0;
	virtual void SetTextures(size_t _StartSlot, size_t _NumTextures, const oGPUTexture* const* _ppTextures) = 0;

	virtual void Map(oGPUResource* _pResource, size_t _SubresourceIndex) = 0;
	virtual void Unmap(oGPUResource* _pResource, size_t _SubresourceIndex) = 0;
	
	// Drawing a null mesh draws a full screen quad
	virtual void Draw(float4x4& _Transform, uint _MeshID, const oGPUMesh* _pMesh, size_t _SectionIndex) = 0;

	virtual LINE* LNBegin(size_t _LineCapacity) = 0;
	virtual void LNEnd(size_t _NumLines) = 0;

	virtual LIGHT* LIBegin(size_t _LightCapacity) = 0;
	virtual void LIEnd(size_t _NumLights) = 0;
};

interface oGPUDevice : oInterface
{
	struct DESC
	{
		bool UseSoftwareEmulation;
		bool EnableDebugReporting;
	};

	virtual bool CreateMesh(const char* _Name, const oGPUMesh::DESC& _Desc, threadsafe oGPUMesh** _ppMesh) threadsafe = 0;
	virtual bool CreateTexture(const char* _Name, const oGPUTexture::DESC& _Desc, threadsafe oGPUTexture** _ppTexture) threadsafe = 0;
	virtual bool CreateMaterial(const char* _Name, const oGPUMaterial::DESC& _Desc, threadsafe oGPUMaterial** _ppMaterial) threadsafe = 0;
	virtual bool CreateContext(const char* _Name, oGPUDeviceContext** _ppContext) threadsafe = 0;

	virtual void Begin() = 0;
	virtual void End() = 0;
};

struct oHLSLColor
{
	oHLSLColor() : Color(0.0f, 0.0f, 0.0f) {}
	oHLSLColor(const float3& _Color) : Color(_Color) {}
	oHLSLColor(const oColor& _Color) { float a; oDecomposeColor(_Color, &Color.x, &Color.y, &Color.z, &a); }

	inline operator float3&() { return Color; }
	inline operator const float3&() const { return Color; }
	inline operator oColor() const { return oComposeColor(Color.x, Color.y, Color.z, 1.0f); }
	inline const oHLSLColor& operator=(const oHLSLColor& _Color) { Color = _Color.Color; return *this; }
	inline const oHLSLColor& operator=(const float3& _Color) { Color = _Color; return *this; }
	inline const oHLSLColor& operator=(const oColor& _Color) { float a; oDecomposeColor(_Color, &Color.x, &Color.y, &Color.z, &a); return *this; }
protected:
	float3 Color;
};

oAPI bool oCreateGPUDevice(const DESC& _Desc, threadsafe oGPUDevice** _ppDevice);

#endif
