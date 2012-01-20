/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/

// Soft-link D3D11 and some utility functions to address common challenges in 
// D3D11.
#pragma once
#ifndef oD3D11_h
#define oD3D11_h

#include <oBasis/oRef.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oWindows.h>
#include <vector>

#include <oBC6HBC7EncoderDecoder.h>

// _____________________________________________________________________________
// Soft-link

struct oD3D11 : oModuleSingleton<oD3D11>
{
	oD3D11();
	~oD3D11();

	HRESULT (__stdcall *D3D11CreateDevice)(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

protected:
	oHMODULE hD3D11;
};

struct oD3D11_DEVICE_DESC
{
	enum INDEX_TYPE
	{
		INDEX_HARDWARE, // indexes with the same index oGPUEnum uses (all on system as system reports them)
		INDEX_CAPABLE, // indexes starting at the first device capable of meeting the DESC requirements
	};

	oD3D11_DEVICE_DESC(const char* _DebugName)
		: DebugName(_DebugName)
		, GPUIndexType(INDEX_CAPABLE)
		, GPUIndex(0)
		, MinimumAPIFeatureLevel(11,0)
		, VirtualDesktopPosition(oDEFAULT, oDEFAULT)
		, Accelerated(true)
		, Debug(false)
	{}

	const char* DebugName;

	// if GPUIndex is oInvalid, the value 0 (meaning first-found) will be used, so
	// if the desired behavior is use whatever the first capable HW is, use 
	// INDEX_CAPABLE for GPUIndexType and oInvalid for GPUIndex.
	INDEX_TYPE GPUIndexType; 
	int GPUIndex; // Use this to specify a particular GPU without searching for a best-match (oInvalid for "don't care")
	oVersion MinimumAPIFeatureLevel; // Look for a GPU with at least this feature set. This should be the D3D/OGL version number
	int2 VirtualDesktopPosition; // Use int2(oDEFAULT, oDEFAULT) for "don't care"
	bool Accelerated; // if false, use a reference or emulation version
	bool Debug;
};

// There are quite a few options when creating a device, especially when debug
// tools and multi-GPU systems are considered, so present enumerated options and
// a central place for ensuring all features and developement tools are 
// supported.
bool oD3D11CreateDevice(const oD3D11_DEVICE_DESC& _Desc, ID3D11Device** _ppDevice);

// _____________________________________________________________________________
// Texture API

struct oD3D11_TEXTURE_DESC
{
	// multiple texture desc types is annoying and results in often overly complex
	// user code, so here is a one, true DESC object to cover the complexities of
	// textures.
	// Texture1D: Width = N, Height = 1, Depth = 1 (or ArraySize is accurate)
	// Texture2D: Width = N, Height = M, Depth = 1 (or ArraySize is accurate)
	// Texture3D: Width = N, Height = M, Depth = P
	// This CAN be called on an ID3D11Buffer, which treats the buffer more or less
	// like a Texture1D of unknown type.

	UINT Width;
	UINT Height;
	UINT Depth;
	UINT ArraySize;

	DXGI_SAMPLE_DESC SampleDesc;
	UINT MipLevels;
	DXGI_FORMAT Format;
	D3D11_USAGE Usage;
	UINT BindFlags;
	UINT CPUAccessFlags;
	UINT MiscFlags;
};

// Fills the specified oD3D11_TEXTURE_DESC with the description from the 
// specified resource. The resource can be: ID3D11Texture1D, ID3D11Texture2D
// ID3D11Texture3D, or ID3D11Buffer.
void oD3D11GetTextureDesc(ID3D11Resource* _pResource, oD3D11_TEXTURE_DESC* _pDesc);
bool oD3D11CreateShaderResourceView(const char* _DebugName, ID3D11Resource* _pTexture, ID3D11ShaderResourceView** _ppShaderResourceView);
bool oD3D11CreateRenderTargetView(const char* _DebugName, ID3D11Resource* _pTexture, ID3D11View** _ppView);

// Copies the contents of the specified texture to _pBuffer, which is assumed to
// be properly allocated to receive the contents. If _FlipVertical is true, then
// the bitmap data will be copied such that the destination will be upside-down 
// compared to the source.
bool oD3D11CopyToBuffer(ID3D11Texture2D* _pTexture, void* _pBuffer, size_t _BufferRowPitch, bool _FlipVertical = false);

// If a depth format is specified, this binds D3D11_BIND_DEPTH_STENCIL instead of
// D3D11_BIND_RENDER_TARGET.

enum oD3D11_TEXTURE_CREATION_TYPE
{
	oD3D11_DYNAMIC_TEXTURE,
	oD3D11_MIPPED_TEXTURE,
	oD3D11_STAGING_TEXTURE,
	oD3D11_RENDER_TARGET,
	oD3D11_MIPPED_RENDER_TARGET,
};

bool oD3D11CreateTexture2D(ID3D11Device* _pDevice, const char* _DebugName, UINT _Width, UINT _Height, UINT _ArraySize, DXGI_FORMAT _Format, oD3D11_TEXTURE_CREATION_TYPE _CreationType, D3D11_SUBRESOURCE_DATA* _pInitData, ID3D11Texture2D** _ppTexture, ID3D11ShaderResourceView** _ppShaderResourceView);
bool oD3D11CreateTexture2D(ID3D11Device* _pDevice, const char* _DebugName, oD3D11_TEXTURE_CREATION_TYPE _CreationType, const oImage* _pImage, ID3D11Texture2D** _ppTexture, ID3D11ShaderResourceView** _ppShaderResourceView);

bool oD3D11CreateSnapshot(ID3D11Texture2D* _pRenderTarget, oImage** _ppImage);
bool oD3D11CreateSnapshot(ID3D11Texture2D* _pRenderTarget, const char* _Path);

// Returns an IFF based on the extension specified in the file path
D3DX11_IMAGE_FILE_FORMAT oD3D11GetFormatFromPath(const char* _Path);

// Saves image to the specified memory buffer, which must be allocated large
// enough to receive the specified image as its file form.
// NOTE: BC6HS, BX6HU and BC7 are brand new formats that seemingly no one on 
// earth supprots. The only way to really know if it works is to use a BC6/7
// format as a texture, or convert a BC6/7 DDS back to something else and view
// that result in a tool.

bool oD3D11Save(ID3D11Texture2D* _pTexture, D3DX11_IMAGE_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer);
bool oD3D11Save(const oImage* _pImage, D3DX11_IMAGE_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer);

// Saves image directly to the specified path. If _Format is UNKNOWN, then a 
// format will be derived from the extension of the specified path.
bool oD3D11Save(ID3D11Texture2D* _pTexture, D3DX11_IMAGE_FILE_FORMAT _Format, const char* _Path);
bool oD3D11Save(const oImage* _pImage, D3DX11_IMAGE_FILE_FORMAT _Format, const char* _Path);

// Creates a new texture by parsing _pBuffer as a D3DX11-supported file format
// Specify DXGI_FORMAT_UNKNOWN or DXGI_FORMAT_FROM_FILE for the "don't care"
// option for _ForceFormat.
bool oD3D11Load(ID3D11Device* _pDevice, DXGI_FORMAT _ForceFormat, oD3D11_TEXTURE_CREATION_TYPE _CreationType, const char* _DebugName, const void* _pBuffer, size_t _SizeofBuffer, ID3D11Resource** _ppTexture);

// Uses GPU acceleration for BC6H and BC7 conversions if the source is in the 
// correct format. All other conversions go through D3DX11LoadTextureFromTexture
bool oD3D11Convert(ID3D11Texture2D* _pSourceTexture, DXGI_FORMAT _NewFormat, ID3D11Texture2D** _ppDestinationTexture);

// _____________________________________________________________________________
// Buffer Creation API

// Convenience wrappers for creating various buffers in DX11. The const void*
// pointers for init data can be set to NULL.
bool oD3D11CreateConstantBuffer(ID3D11Device* _pDevice, const char* _DebugName, bool _CPUWrite, const void* _pBufferStruct, size_t _SizeofBufferStruct, size_t _StructCount, ID3D11Buffer** _ppConstantBuffer);
bool oD3D11CreateIndexBuffer(ID3D11Device* _pDevice, const char* _DebugName, bool _CPUWrite, const void* _pIndices, size_t _NumIndices, bool _Use16BitIndices, ID3D11Buffer** _ppIndexBuffer);
bool oD3D11CreateVertexBuffer(ID3D11Device* _pDevice, const char* _DebugName, bool _CPUWrite, const void* _pVertices, size_t _NumVertices, size_t _VertexStride, ID3D11Buffer** _ppVertexBuffer);

// Creates a 2D texture fit for rendering to and also create its associated 
// writing view. If the format is a depth format, then the view created will 
// be an ID3D11DepthStencilView, else it will be an ID3D11RenderTargetView.
// If a view is not desired, pass NULL.
bool oD3D11CreateRenderTarget(ID3D11Device* _pDevice, const char* _DebugName, UINT _Width, UINT _Height, UINT _ArraySize, DXGI_FORMAT _Format, ID3D11Texture2D** _ppRenderTarget, ID3D11View** _ppRenderTargetView, ID3D11ShaderResourceView** _ppShaderResourceView);
inline bool oD3D11CreateRenderTarget(ID3D11Device* _pDevice, const char* _DebugName, UINT _Width, UINT _Height, UINT _ArraySize, DXGI_FORMAT _Format, ID3D11Texture2D** _ppRenderTarget, ID3D11RenderTargetView** _ppRenderTargetView, ID3D11ShaderResourceView** _ppShaderResourceView) { return oD3D11CreateRenderTarget(_pDevice, _DebugName, _Width, _Height, _ArraySize, _Format, _ppRenderTarget, (ID3D11View**)_ppRenderTargetView, _ppShaderResourceView); }
inline bool oD3D11CreateRenderTarget(ID3D11Device* _pDevice, const char* _DebugName, UINT _Width, UINT _Height, UINT _ArraySize, DXGI_FORMAT _Format, ID3D11Texture2D** _ppRenderTarget, ID3D11DepthStencilView** _ppRenderTargetView, ID3D11ShaderResourceView** _ppShaderResourceView) { return oD3D11CreateRenderTarget(_pDevice, _DebugName, _Width, _Height, _ArraySize, _Format, _ppRenderTarget, (ID3D11View**)_ppRenderTargetView, _ppShaderResourceView); }

// Set a name that appears in D3D11's debug layer
bool oD3D11SetDebugName(ID3D11Device* _pDevice, const char* _Name);
bool oD3D11SetDebugName(ID3D11DeviceChild* _pDeviceChild, const char* _Name);

// Fills the specified buffer with the string set with oD3D11SetDebugName().
char* oD3D11GetDebugName(char* _StrDestination, size_t _SizeofStrDestination, const ID3D11DeviceChild* _pDeviceChild);
template<size_t size> char* oD3D11GetDebugName(char (&_StrDestination)[size], const ID3D11DeviceChild* _pDeviceChild) { return oD3D11GetDebugName(_StrDestination, size, _pDeviceChild); }

struct oD3D_BUFFER_TOPOLOGY
{
	UINT ElementStride;
	UINT ElementCount;
};

// Allow ID3D11Buffers to be a bit more self-describing.
bool oD3D11SetBufferDescription(ID3D11Buffer* _pBuffer, const oD3D_BUFFER_TOPOLOGY& _Topology);
bool oD3D11GetBufferDescription(const ID3D11Buffer* _pBuffer, oD3D_BUFFER_TOPOLOGY* _pTopology);

// _____________________________________________________________________________
// Shader compilation API

enum oD3D11_PIPELINE_STAGE
{
	oD3D11_PIPELINE_STAGE_VERTEX,
	oD3D11_PIPELINE_STAGE_HULL,
	oD3D11_PIPELINE_STAGE_DOMAIN,
	oD3D11_PIPELINE_STAGE_GEOMETRY,
	oD3D11_PIPELINE_STAGE_PIXEL,
	oD3D11_PIPELINE_STAGE_COMPUTE,
};

const char* oD3D11GetShaderProfile(ID3D11Device* _pDevice, oD3D11_PIPELINE_STAGE _Stage);

// The error message returned from D3DX11CompileFromMemory is not fit for
// passing to printf directly, so pass it to this to create a cleaner string.
// _pErrorMessages can be NULL, but if there is a message and there is an 
// error filling the specified string buffer.
bool oD3D11ConvertCompileErrorBuffer(char* _OutErrorMessageString, size_t _SizeofOutErrorMessageString, ID3DBlob* _pErrorMessages);

// Thin wrapper for D3DX11CompileFromMemory() that handles an annoying feature
// where the error messages could be formated such that using them with
// printf statements (probably you're very next step if this function fails to
// report the compile error) causes bad behavior.

// @oooii-tony: This function has been inlined because I get a link error in
// programs that link to oooii.lib, but don't use DX. Since this is a light
// and somewhat "why would you do this" wrapper, inlining for the purposes of 
// documentation is how I'll justify my linker error work-around for now.

bool oD3D11ConvertCompileErrorBuffer(char* _OutErrorMessageString, size_t _SizeofOutErrorMessageString, ID3DBlob* _pErrorMessages);

// Returns the size of the byte code as it is encoded inside the byte code
// itself. (Makes you wonder why you need to specify it's size elsewhere)
size_t oD3D11GetEncodedByteCodeSize(const BYTE* _pByteCode);

// _____________________________________________________________________________
// Shader uniformity API

// A lot of times it's desirable to have all stages act the same way, so wrap
// per-stage API here in a way that does the same thing for every stage.

void oD3D11SetConstantBuffers(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot, size_t _NumBuffers, const ID3D11Buffer* const* _ppConstantBuffers);
void oD3D11SetSamplers(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot, size_t _NumSamplers, const ID3D11SamplerState* const* _ppSamplers);
void oD3D11SetShaderResourceViews(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot, size_t _NumShaderResourceViews, const ID3D11ShaderResourceView* const* _ppViews);

// _____________________________________________________________________________
// Misc API (not yet categorized)

#define oD3D11_CHECK_STRUCT_SIZE(_Struct) static_assert((sizeof(_Struct) % 16) == 0, "Alignment of the specified struct is incompatible")

template<typename T> inline bool oD3D11IsValidConstantBufferSize(const T& _ConstantBufferStruct) { return (sizeof(_ConstantBufferStruct) % 16) == 0; }
template<> inline bool oD3D11IsValidConstantBufferSize(const size_t& _ConstantBufferStructSize) { return (_ConstantBufferStructSize % 16) == 0; }

// Returns the number of elements as described the specified topology given
// the number of primitives. An element can refer to indices or vertices, but
// basically if there are 3 lines, then there are 6 elements. If there are 3
// lines in a line string, then there are 4 elements.
size_t oD3D11GetNumElements(D3D11_PRIMITIVE_TOPOLOGY _PrimitiveTopology, size_t _NumPrimitives);

// Encapsulate any possible method of drawing in DX11 into one function that
// lists out everything one would need to specify. If NULL or 0 is specified 
// for various parameters, the proper flavor of draw will be used for drawing.
void oD3D11Draw(ID3D11DeviceContext* _pDeviceContext
							 , size_t _NumElements // vertices or indices use oD3D11GetNumElements() to calculate this from number of primitives
							 , size_t _NumVertexBuffers
							 , const ID3D11Buffer* const* _ppVertexBuffers
							 , const UINT* _VertexStrides
							 , size_t _IndexOfFirstVertexToDraw
							 , size_t _OffsetToAddToEachVertexIndex
							 , const ID3D11Buffer* _IndexBuffer
							 , bool _32BitIndexBuffer
							 , size_t _IndexOfFirstIndexToDraw
							 , size_t _NumInstances = 0 // 0 means don't use instanced drawing
							 , size_t _IndexOfFirstInstanceIndexToDraw = 0);

// This returns the number of primitives drawn (more than the specified number
// of primitives if instancing is used). This does not set input layout.
inline void oD3D11Draw(ID3D11DeviceContext* _pDeviceContext
							 , D3D11_PRIMITIVE_TOPOLOGY _PrimitiveTopology
							 , size_t _NumPrimitives
							 , size_t _NumVertexBuffers
							 , const ID3D11Buffer* const* _ppVertexBuffers
							 , const UINT* _VertexStrides
							 , size_t _IndexOfFirstVertexToDraw
							 , size_t _OffsetToAddToEachVertexIndex
							 , const ID3D11Buffer* _IndexBuffer
							 , bool _32BitIndexBuffer
							 , size_t _IndexOfFirstIndexToDraw
							 , size_t _NumInstances = 0 // 0 means don't use instanced drawing
							 , size_t _IndexOfFirstInstanceIndexToDraw = 0)
{
	_pDeviceContext->IASetPrimitiveTopology(_PrimitiveTopology);
	const size_t nElements = oD3D11GetNumElements(_PrimitiveTopology, _NumPrimitives);
	oD3D11Draw(_pDeviceContext, nElements, _NumVertexBuffers, _ppVertexBuffers, _VertexStrides, _IndexOfFirstVertexToDraw, _OffsetToAddToEachVertexIndex, _IndexBuffer, _32BitIndexBuffer, _IndexOfFirstIndexToDraw, _NumInstances, _IndexOfFirstInstanceIndexToDraw);
}

// A neat little trick when drawing quads, fullscreen or otherwise. Submits a 
// triangle strip with no vertices and optionally a null instance buffer. Use 
// oExtractQuadInfoFromVertexID() in oHLSL.h to reconstruct position and 
// texcoords using SV_VertexID and optionally SV_InstanceID.
// NOTE: This leaves primitive topology set to tristrips.
void oD3D11DrawSVQuad(ID3D11DeviceContext* _pDeviceContext, size_t _NumInstances = 1);

// Sets the viewport on the specified device context to use the entire region
// of the specified render target
void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11Texture2D* _pRenderTargetResource, float _MinDepth = 0.0f, float _MaxDepth = 1.0f);

// Extracts the resource from the view and calls the above function
void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11RenderTargetView* _pRenderTargetView, float _MinDepth = 0.0f, float _MaxDepth = 1.0f);

// This assumes the specified buffer is properly sized and then copies the
// specified array of structs into the buffer.
void oD3D11CopyStructs(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pBuffer, const void* _pStructs);

// _____________________________________________________________________________
// Pipeline stage state API

// GPUs are programmable, but graphics applications tend to decide their own 
// flavor of features, then fix the pipeline to make that specific combination 
// as efficient as possible. Towards this end, here's a useful pattern of 
// enumerating DX11 state in very typical usage patterns that seem to be 
// somewhat portable.

struct oD3D11RasterizerState
{
	// This also set primitive topology

	enum STATE
	{
		FRONT_FACE,
		BACK_FACE,
		TWO_SIDED,
		FRONT_WIREFRAME,
		BACK_WIREFRAME,
		TWO_SIDED_WIREFRAME,
		FRONT_POINTS,
		BACK_POINTS,
		TWO_SIDED_POINTS,
		NUM_STATES,
	};

	oD3D11RasterizerState(const char* _DebugNamePrefix, ID3D11Device* _pDevice);
	~oD3D11RasterizerState();
	void SetState(ID3D11DeviceContext* _pDeviceContext, STATE _State);
	inline void SetDefaultState(ID3D11DeviceContext* _pDeviceContext) { _pDeviceContext->RSSetState(0); }

protected:
	ID3D11RasterizerState* States[NUM_STATES];
};

struct oD3D11BlendState
{
	enum STATE
	{
		NO_BLEND, // (opaque, Windows defines OPAQUE so I can't use that)
		ACCUMULATE, // destbuffer rgba + srcbuffer rgba
		ADDITIVE, // srcbuffer rgb * srcbuffer a  + destbuffer rgba additive blend mode
		TRANSLUCENT, // over-blend mode
		NUM_STATES,
	};

	oD3D11BlendState(const char* _DebugNamePrefix, ID3D11Device* _pDevice);
	~oD3D11BlendState();
	void SetState(ID3D11DeviceContext* _pDeviceContext, STATE _State);
	void SetDefaultState(ID3D11DeviceContext* _pDeviceContext);

protected:
	ID3D11BlendState* States[NUM_STATES];
};

struct oD3D11DepthStencilState
{
	enum STATE
	{
		DEPTH_STENCIL_OFF,
		DEPTH_TEST_AND_WRITE,
		DEPTH_TEST,
		NUM_STATES,
	};

	oD3D11DepthStencilState(const char* _DebugNamePrefix, ID3D11Device* _pDevice);
	~oD3D11DepthStencilState();
	inline void SetState(ID3D11DeviceContext* _pDeviceContext, STATE _State) { _pDeviceContext->OMSetDepthStencilState(States[_State], 0); }
	inline void SetDefaultState(ID3D11DeviceContext* _pDeviceContext) { _pDeviceContext->OMSetDepthStencilState(0, 0); }

protected:
	ID3D11DepthStencilState* States[NUM_STATES];
};

struct oD3D11SamplerState
{
	enum SAMPLER_STATE
	{
		POINT_CLAMP,
		POINT_WRAP,
		LINEAR_CLAMP,
		LINEAR_WRAP,
		ANISOTROPIC_CLAMP,
		ANISOTROPIC_WRAP,
		NUM_SAMPLER_STATES,
	};

	enum MIP_BIAS_LEVEL
	{
		NONE,
		UP1,
		UP2,
		DOWN1,
		DOWN2,
		NUM_MIP_BIAS_LEVELS
	};

	enum PIPELINE_STAGE
	{
		VERTEX_SHADER,
		HULL_SHADER,
		DOMAIN_SHADER,
		GEOMETRY_SHADER,
		PIXEL_SHADER,
	};

	oD3D11SamplerState(const char* _DebugNamePrefix, ID3D11Device* _pDevice);
	~oD3D11SamplerState();

	// Set samplers only for a specific pipeline state
	void SetState(ID3D11DeviceContext* _pDeviceContext, PIPELINE_STAGE _Stage, size_t _StartSlot, const SAMPLER_STATE* _States, const MIP_BIAS_LEVEL* _Levels, size_t _NumSamplers);
	inline void SetState(ID3D11DeviceContext* _pDeviceContext, PIPELINE_STAGE _Stage, size_t _StartSlot, const SAMPLER_STATE _States, const MIP_BIAS_LEVEL _Levels) { SetState(_pDeviceContext, _Stage, _StartSlot, &_States, &_Levels, 1); }

	// Set samplers for all pipeline stages
	void SetState(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot, const SAMPLER_STATE* _States, const MIP_BIAS_LEVEL* _Levels, size_t _NumSamplers);
	inline void SetState(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot, const SAMPLER_STATE _States, const MIP_BIAS_LEVEL _Levels) { SetState(_pDeviceContext, _StartSlot, &_States, &_Levels, 1); }

protected:
	ID3D11SamplerState* States[NUM_SAMPLER_STATES][NUM_MIP_BIAS_LEVELS];
};

struct oD3D11ShaderState
{
	// Unlike other states, it's not easy to fix/enumerate shaders, so this  
	// interface provides API to hook all that up in user space

	// Pointers to the source elements that can be compiled into STATEs
	// The recommended approach is to use the custom build rules for FXC found in
	// {OOOiiSDK}/MSSupport/oCustomBuildRules.rules to compile shader files into
	// C++ bytecode header files and link that buffer by defining it as part of an
	// instance of this struct.
	struct STATE_BYTE_CODE
	{
		const char* DebugName;
		const D3D11_INPUT_ELEMENT_DESC* InputLayoutDesc;
		size_t NumInputElements;
		const BYTE* pVertexShader;
		const BYTE* pHullShader;
		const BYTE* pDomainShader;
		const BYTE* pGeometryShader;
		const BYTE* pPixelShader;
	};

	struct STATE
	{
		oRef<ID3D11InputLayout> InputLayout;
		oRef<ID3D11VertexShader> VertexShader;
		oRef<ID3D11HullShader> HullShader;
		oRef<ID3D11DomainShader> DomainShader;
		oRef<ID3D11GeometryShader> GeometryShader;
		oRef<ID3D11PixelShader> PixelShader;

		inline void Clear()
		{
			InputLayout = 0;
			VertexShader = 0;
			HullShader = 0;
			DomainShader = 0;
			GeometryShader = 0;
			PixelShader = 0;
		}
	};

	void SetState(ID3D11DeviceContext* _pDeviceContext, size_t _Index);

	// Set all shaders and input layout to null.
	void ClearState(ID3D11DeviceContext* _pDeviceContext);

	// Registers a state so it can be set using SetState(). Optionally you can 
	// choose to reference the objects passed in, as one would expect in a ref-
	// counted system. However to maintain coding simplicity, it's most likely
	// user code will call Create*Shader() directly on the elements of a STATE
	// object - so they'll already have a ref - and then turn over ownership to 
	// an instance of this object. So rather than adding extra user code to 
	// release after calling this function, just skip the internal reference.
	void RegisterState(const char* _DebugNamePrefix, size_t _Index, STATE& _State);

	// Given the source material for a STATE, create the elements and register it
	// with this object.
	void RegisterStates(const char* _DebugNamePrefix, ID3D11Device* _pDevice, const STATE_BYTE_CODE* _pStates, size_t _NumStates);
	template<size_t size> inline void RegisterStates(const char* _DebugNamePrefix, ID3D11Device* _pDevice, const STATE_BYTE_CODE (&_pStates)[size]) { return RegisterStates(_DebugNamePrefix, _pDevice, _pStates, size); }

	// From the specified byte code, fill the specified STATE struct with 
	// shaders. This does no changing of ref counts to prior pointers. NULL is
	// allowed for unused stages.
	static void CreateState(const char* _DebugNamePrefix
		, ID3D11Device* _pDevice
		, STATE* _pState
		, const D3D11_INPUT_ELEMENT_DESC* _pElements, size_t _NumElements
		, const BYTE* _pByteCodeVS, size_t _SizeofByteCodeVS
		, const BYTE* _pByteCodeHS, size_t _SizeofByteCodeHS
		, const BYTE* _pByteCodeDS, size_t _SizeofByteCodeDS
		, const BYTE* _pByteCodeGS, size_t _SizeofByteCodeGS
		, const BYTE* _pByteCodePS, size_t _SizeofByteCodePS);

	// compiled into code/a fixed array, here's a templated version to simplify
	// the code.
	template<size_t I, size_t V, size_t H, size_t D, size_t G, size_t P>
	static void CreateState(const char* _DebugNamePrefix
		, ID3D11Device* _pDevice
		, STATE* _pState
		, const D3D11_INPUT_ELEMENT_DESC (&_pElements)[I]
		, const BYTE (&_pByteCodeVS)[V]
		, const BYTE (&_pByteCodeHS)[H]
		, const BYTE (&_pByteCodeDS)[D]
		, const BYTE (&_pByteCodeGS)[G]
		, const BYTE (&_pByteCodePS)[P])
	{
		CreateState(_DebugNamePrefix, _pDevice, _pState, _pElements, I, _pByteCodeVS, V, _pByteCodeHS, H, _pByteCodeDS, D, _pByteCodeGS, G, _pByteCodePS, P);
	}

	// Flavor that uses the size that's already inside the byte code itself
	static inline void CreateState(const char* _DebugNamePrefix
		, ID3D11Device* _pDevice
		, STATE* _pState
		, const D3D11_INPUT_ELEMENT_DESC* _pElements, size_t _NumElements
		, const BYTE* _pByteCodeVS
		, const BYTE* _pByteCodeHS
		, const BYTE* _pByteCodeDS
		, const BYTE* _pByteCodeGS
		, const BYTE* _pByteCodePS)
	{
		CreateState(_DebugNamePrefix, _pDevice, _pState, _pElements, _NumElements
			, _pByteCodeVS, oD3D11GetEncodedByteCodeSize(_pByteCodeVS)
			, _pByteCodeHS, oD3D11GetEncodedByteCodeSize(_pByteCodeHS)
			, _pByteCodeDS, oD3D11GetEncodedByteCodeSize(_pByteCodeDS)
			, _pByteCodeGS, oD3D11GetEncodedByteCodeSize(_pByteCodeGS)
			, _pByteCodePS, oD3D11GetEncodedByteCodeSize(_pByteCodePS));
	}

	template<size_t I> static void CreateState(const char* _DebugNamePrefix
		, ID3D11Device* _pDevice
		, STATE* _pState
		, const D3D11_INPUT_ELEMENT_DESC (&_pElements)[I]
		, const BYTE* _pByteCodeVS
		, const BYTE* _pByteCodeHS
		, const BYTE* _pByteCodeDS
		, const BYTE* _pByteCodeGS
		, const BYTE* _pByteCodePS)
	{
		CreateState(_DebugNamePrefix, _pDevice, _pState, _pElements, I
			, _pByteCodeVS, oD3D11GetEncodedByteCodeSize(_pByteCodeVS)
			, _pByteCodeHS, oD3D11GetEncodedByteCodeSize(_pByteCodeHS)
			, _pByteCodeDS, oD3D11GetEncodedByteCodeSize(_pByteCodeDS)
			, _pByteCodeGS, oD3D11GetEncodedByteCodeSize(_pByteCodeGS)
			, _pByteCodePS, oD3D11GetEncodedByteCodeSize(_pByteCodePS));
	}

protected:
	std::vector<STATE> States;
};

// _____________________________________________________________________________
// Common rendering building blocks

struct oD3D11RenderTarget
{
	// Wrapper for MRT render targets that simplify operations that occur in 
	// similar manners across all targets such as lifetime, resize, clear, and
	// binding. NOTE: If a depth format is specified, it is set as the depth
	// target in OMSetRenderTargets(). For SRVs, it is appended to the end of
	// the list of color targets, so indexing inside shaders should remain 
	// consistent with indexing here.

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

	static const unsigned int DEPTH_STENCIL_INDEX = UINT_MAX;

	struct DESC
	{
		DESC()
			: Width(256)
			, Height(256)
			, NumTargets(1)
			, ArraySize(1)
			, DepthStencilFormat(DXGI_FORMAT_UNKNOWN)
			, DepthClearValue(1.0f)
			, StencilClearValue(0)
		{
			for (size_t i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
			{
				Format[i] = DXGI_FORMAT_UNKNOWN;
				ClearColor[i][0] = ClearColor[i][1] = ClearColor[i][2] = ClearColor[i][3] = 0.0f;
			}
		}

		UINT Width;
		UINT Height;
		UINT NumTargets;
		UINT ArraySize;
		DXGI_FORMAT Format[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		DXGI_FORMAT DepthStencilFormat; // Use DXGI_UNKNOWN for no depth
		float ClearColor[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT][4];
		float DepthClearValue;
		UINT8 StencilClearValue;
	};

	oD3D11RenderTarget(const char* _DebugName, ID3D11Device* _pDevice);

	inline const char* GetDebugName() const { return DebugName; }

	// Returns the description of the current configuration of the
	// render target.
	void GetDesc(DESC* _pDesc) const;

	// Reformats the underlying resources if size or format changes. If
	// Clear colors or values change, then resources are not recreated.
	void SetDesc(const DESC& _Desc);

	// Because of depth-stencil being used as an SRV, there's 
	// actually NumTargets+1 SRVs bound if depth-stencil is defined
	size_t GetNumSRVs() const;

	// Resizes all associated render targets to the new specification.
	// Resizing to 0 releases all resources except the device specified
	// in the ctor.
	virtual void Resize(UINT _Width, UINT _Height);

	// Clears all render targets using the clear colors specified in SetDesc()
	void Clear(ID3D11DeviceContext* _pDeviceContext, CLEAR_TYPE _ClearType);

	// Sets all render targets. If depth is specified, it is bound as depth.
	void SetRenderTargets(ID3D11DeviceContext* _pDeviceContext);

	// Sets a viewport sized to the full size of the render targets
	void SetDefaultViewport(ID3D11DeviceContext* _pDeviceContext);

	inline void SetFullViewportRenderTargets(ID3D11DeviceContext* _pDeviceContext)
	{
		SetRenderTargets(_pDeviceContext);
		SetDefaultViewport(_pDeviceContext);
	}

	// Sets all resources in order at the specified start slot. If depth is
	// specified, it is appended to the end of the SRVs set. For depth, use
	// SetSingleShaderResource() and use DEPTH_STENCIL_INDEX.
	void SetShaderResources(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot = 0);
	void SetCSShaderResources(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot = 0);

	// Set a specific SRV - useful for debug visualization.
	void SetSingleShaderResource(ID3D11DeviceContext* _pDeviceContext, size_t _RenderTargetIndex, size_t _StartSlot = 0);

protected:
	oRef<ID3D11Texture2D> Texture[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	oRef<ID3D11RenderTargetView> RTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	oRef<ID3D11ShaderResourceView> SRVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	oRef<ID3D11Texture2D> Depth;
	oRef<ID3D11DepthStencilView> DSV;
	oRef<ID3D11ShaderResourceView> SRVDepth;

	oRef<ID3D11Device> Device;
	DESC Desc;
	char DebugName[64];
};

struct oD3D11ScopedMessageDisabler
{
	// Sometimes you intend to do something in Direct3D that generates a warning,
	// and often that warning is "it's ok to do this". I know! so disable that
	// message with this interface.

	oD3D11ScopedMessageDisabler(ID3D11DeviceContext* _pDeviceContext, const D3D11_MESSAGE_ID* _pMessageIDs, size_t _NumMessageIDs);
	oD3D11ScopedMessageDisabler(ID3D11Device* _pDevice, const D3D11_MESSAGE_ID* _pMessageIDs, size_t _NumMessageIDs);
	~oD3D11ScopedMessageDisabler();

protected:
	ID3D11InfoQueue* pInfoQueue;
};

//oooii-Eric TODO: This code is not implemented. copied from oD3D10.h. used by oVideoDecoderD3D. D3D11 version not currently used.
//	D3D10 version used code like this, so either need to implement this, or modify D3D10 version to do something else.
struct oD3D11FullscreenQuad
{
	void Create(ID3D11Device* _pDevice, const BYTE* _pPixelShaderByteCode, size_t _szBiteCode) {};
	void Draw(ID3D11Device* _pDevice) {};

protected:
	void CreateStates(ID3D11Device* _pDevice) {};
	void SetStates(ID3D11Device* _pDevice) {};

	oD3D11ShaderState::STATE ShaderState;
	oRef<ID3D11RasterizerState> RasterState;
	oRef<ID3D11DepthStencilState> DepthState;
	oRef<ID3D11BlendState> BlendState;
};

struct oD3D11ScreenQuad : private oD3D11FullscreenQuad
{
	void Create(ID3D11Device* _pDevice, const BYTE* _pPixelShaderByteCode, size_t _szBiteCode) {};
	void Draw(ID3D11Device* _pDevice, oRECTF _Destination, oRECTF _Source) {};

private:
	struct VSIN
	{
		float2 Position;
		float2 Texcoord;
	};

	oRef<ID3D11InputLayout> VertexLayout;
	oRef<ID3D11Buffer> VertexBuffer;
};

#endif
