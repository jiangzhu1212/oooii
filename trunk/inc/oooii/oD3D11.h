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
// Soft-link d3d11 and some utility functions to address common challenges in 
// D3D11.
#pragma once
#ifndef oD3D11_h
#define oD3D11_h

#include <oooii/oSingleton.h>
#include <oooii/oStdio.h>
#include <oooii/oWindows.h>

// _____________________________________________________________________________
// Soft-link

struct oD3D11 : oModuleSingleton<oD3D11>
{
	oD3D11();
	~oD3D11();

	HRESULT (__stdcall *D3D11CreateDevice)(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

protected:
	oHDLL hD3D11;
};

// _____________________________________________________________________________
// Buffer Creation API

// Convenience wrappers for creating various buffers in DX11. The const void*
// pointers for init data can be set to NULL.
bool oD3D11CreateConstantBuffer(ID3D11Device* _pDevice, bool _CPUWrite, const void* _pBufferStruct, size_t _SizeofBufferStruct, size_t _StructCount, ID3D11Buffer** _ppConstantBuffer);
bool oD3D11CreateIndexBuffer(ID3D11Device* _pDevice, bool _CPUWrite, const void* _pIndices, size_t _NumIndices, bool _Use16BitIndices, ID3D11Buffer** _ppIndexBuffer);
bool oD3D11CreateVertexBuffer(ID3D11Device* _pDevice, bool _CPUWrite, const void* _pVertices, size_t _NumVertices, size_t _VertexStride, ID3D11Buffer** _ppVertexBuffer);

// Creates a 2D texture fit for rendering to and also create its associated 
// writing view. If the format is a depth format, then the view created will 
// be an ID3D11DepthStencilView, else it will be an ID3D11RenderTargetView.
// If a view is not desired, pass NULL.
bool oD3D11CreateRenderTarget(ID3D11Device* _pDevice, unsigned int _Width, unsigned int _Height, DXGI_FORMAT _Format, ID3D11Texture2D** _ppRenderTarget, ID3D11View** _ppRenderTargetView, ID3D11ShaderResourceView** _ppShaderResourceView);
inline bool oD3D11CreateRenderTarget(ID3D11Device* _pDevice, unsigned int _Width, unsigned int _Height, DXGI_FORMAT _Format, ID3D11Texture2D** _ppRenderTarget, ID3D11RenderTargetView** _ppRenderTargetView, ID3D11ShaderResourceView** _ppShaderResourceView) { return oD3D11CreateRenderTarget(_pDevice, _Width, _Height, _Format, _ppRenderTarget, (ID3D11View**)_ppRenderTargetView, _ppShaderResourceView); }
inline bool oD3D11CreateRenderTarget(ID3D11Device* _pDevice, unsigned int _Width, unsigned int _Height, DXGI_FORMAT _Format, ID3D11Texture2D** _ppRenderTarget, ID3D11DepthStencilView** _ppRenderTargetView, ID3D11ShaderResourceView** _ppShaderResourceView) { return oD3D11CreateRenderTarget(_pDevice, _Width, _Height, _Format, _ppRenderTarget, (ID3D11View**)_ppRenderTargetView, _ppShaderResourceView); }

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

void oD3D11SetConstantBuffers(ID3D11DeviceContext* _pDeviceContext, UINT _StartSlot, UINT _NumBuffers, ID3D11Buffer* const* _ppConstantBuffers);
void oD3D11SetSamplers(ID3D11DeviceContext* _pDeviceContext, UINT _StartSlot, UINT _NumSamplers, ID3D11SamplerState* const* _ppSamplers);

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

	union
	{
		UINT Depth;
		UINT ArraySize;
	};

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

// _____________________________________________________________________________
// Misc API (not yet categorized)

#define oD3D11_CHECK_STRUCT_SIZE(_Struct) oSTATICASSERT((sizeof(_Struct) % 16) == 0)

template<typename T> inline bool oD3D11IsValidConstantBufferSize(const T& _ConstantBufferStruct) { return (sizeof(_ConstantBufferStruct) % 16) == 0; }
template<> inline bool oD3D11IsValidConstantBufferSize(const size_t& _ConstantBufferStructSize) { return (_ConstantBufferStructSize % 16) == 0; }

// Returns the number of elements as described the specified topology given
// the number of primitives. An element can refer to indices or vertices, but
// basically if there are 3 lines, then there are 6 elements. If there are 3
// lines in a line string, then there are 4 elements.
UINT oD3D11GetNumElements(D3D11_PRIMITIVE_TOPOLOGY _PrimitiveTopology, UINT _NumPrimitives);

// Encapsulate any possible method of drawing in DX11 into one function that
// lists out everything one would need to specify. If NULL or 0 is specified 
// for various parameters, the proper flavor of draw will be used for drawing.
// This returns the number of primitives drawn (more than the specified number
// of primitives if instancing is used). This does not set input layout.
UINT oD3D11Draw(ID3D11DeviceContext* _pDeviceContext
							 , D3D11_PRIMITIVE_TOPOLOGY _PrimitiveTopology
							 , UINT _NumPrimitives
							 , UINT _NumVertexBuffers
							 , const ID3D11Buffer* const* _ppVertexBuffers
							 , const UINT* _VertexStrides
							 , UINT _IndexOfFirstVertexToDraw
							 , UINT _OffsetToAddToEachVertexIndex
							 , const ID3D11Buffer* _IndexBuffer
							 , bool _32BitIndexBuffer
							 , UINT _IndexOfFirstIndexToDraw
							 , UINT _NumInstances = 0 // 0 means don't use instanced drawing
							 , UINT _IndexOfFirstInstanceIndexToDraw = 0);

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
	enum STATE
	{
		FRONT_FACE,
		BACK_FACE,
		TWO_SIDED,
		WIREFRAME,
		NUM_STATES,
	};

	oD3D11RasterizerState(ID3D11Device* _pDevice);
	~oD3D11RasterizerState();
	inline void SetState(ID3D11DeviceContext* _pDeviceContext, STATE _State) { _pDeviceContext->RSSetState(States[_State]); }
	inline void SetDefaultState(ID3D11DeviceContext* _pDeviceContext) { _pDeviceContext->RSSetState(0); }

protected:
	ID3D11RasterizerState* States[NUM_STATES];
};

struct oD3D11BlendState
{
	enum STATE
	{
		NO_BLEND, // (opaque, Windows defines OPAQUE so I can't use that)
		ADDITIVE, // additive blend mode
		TRANSLUCENT, // over-blend mode
		NUM_STATES,
	};

	oD3D11BlendState(ID3D11Device* _pDevice);
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

	oD3D11DepthStencilState(ID3D11Device* _pDevice);
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

	oD3D11SamplerState(ID3D11Device* _pDevice);
	~oD3D11SamplerState();

	// Set samplers only for a specific pipeline state
	void SetState(ID3D11DeviceContext* _pDeviceContext, PIPELINE_STAGE _Stage, const SAMPLER_STATE* _States, const MIP_BIAS_LEVEL* _Levels, size_t _NumSamplers);
	inline void SetState(ID3D11DeviceContext* _pDeviceContext, PIPELINE_STAGE _Stage, const SAMPLER_STATE _States, const MIP_BIAS_LEVEL _Levels) { SetState(_pDeviceContext, _Stage, &_States, &_Levels, 1); }

	// Set samplers for all pipeline stages
	void SetState(ID3D11DeviceContext* _pDeviceContext, const SAMPLER_STATE* _States, const MIP_BIAS_LEVEL* _Levels, size_t _NumSamplers);
	inline void SetState(ID3D11DeviceContext* _pDeviceContext, const SAMPLER_STATE _States, const MIP_BIAS_LEVEL _Levels) { SetState(_pDeviceContext, &_States, &_Levels, 1); }

protected:
	ID3D11SamplerState* States[NUM_SAMPLER_STATES][NUM_MIP_BIAS_LEVELS];
};

struct oD3D11ShaderState
{
	// Unlike other states, it's not easy to fix/enumerate shaders, so this  
	// interface provides API to hook all that up in user space

	struct STATE
	{
		STATE()
			: pInputLayout(0)
			, pVertexShader(0)
			, pHullShader(0)
			, pDomainShader(0)
			, pGeometryShader(0)
			, pPixelShader(0)
		{}

		ID3D11InputLayout* pInputLayout;
		ID3D11VertexShader* pVertexShader;
		ID3D11HullShader* pHullShader;
		ID3D11DomainShader* pDomainShader;
		ID3D11GeometryShader* pGeometryShader;
		ID3D11PixelShader* pPixelShader;
	};

	oD3D11ShaderState();
	~oD3D11ShaderState();

	void SetState(ID3D11DeviceContext* _pDeviceContext, unsigned int _Index);

	// Set all shaders and input layout to null.
	void ClearState(ID3D11DeviceContext* _pDeviceContext);

	// Registers a state so it can be set using SetState(). Optionally you can 
	// choose to reference the objects passed in, as one would expect in a ref-
	// counted system. However to maintain coding simplicity, it's most likely
	// user code will call Create*Shader() directly on the elements of a STATE
	// object - so they'll already have a ref - and then turn over ownership to 
	// an instance of this object. So rather than adding extra user code to 
	// release after calling this function, just skip the internal reference.
	void RegisterState(unsigned int _Index, STATE& _State, bool _ReferenceObjects = false);

	// From the specified byte code, fill the specified STATE struct with 
	// shaders. This does no changing of ref counts to prior pointers. NULL is
	// allowed for unused stages.
	static void CreateShaders(ID3D11Device* _pDevice
		, oD3D11ShaderState::STATE* _pState
		, const BYTE* _pByteCodeVS, size_t _SizeofByteCodeVS
		, const BYTE* _pByteCodeHS, size_t _SizeofByteCodeHS
		, const BYTE* _pByteCodeDS, size_t _SizeofByteCodeDS
		, const BYTE* _pByteCodeGS, size_t _SizeofByteCodeGS
		, const BYTE* _pByteCodePS, size_t _SizeofByteCodePS);

	// compiled into code/a fixed array, here's a templated version to simplify
	// the code.
	template<size_t V, size_t H, size_t D, size_t G, size_t P>
	static inline void CreateShaders(ID3D11Device* _pDevice
		, oD3D11ShaderState::STATE* _pState
		, const BYTE (&_pByteCodeVS)[V]
	, const BYTE (&_pByteCodeHS)[H]
	, const BYTE (&_pByteCodeDS)[D]
	, const BYTE (&_pByteCodeGS)[G]
	, const BYTE (&_pByteCodePS)[P])
	{
		CreateShaders(_pDevice, _pState, _pByteCodeVS, V, _pByteCodeHS, H, _pByteCodeDS, D, _pByteCodeGS, G, _pByteCodePS, P);
	}

	// Flavor that uses the size that's already inside the byte code itself
	static inline void CreateShaders(ID3D11Device* _pDevice
		, oD3D11ShaderState::STATE* _pState
		, const BYTE* _pByteCodeVS
		, const BYTE* _pByteCodeHS
		, const BYTE* _pByteCodeDS
		, const BYTE* _pByteCodeGS
		, const BYTE* _pByteCodePS)
	{
		CreateShaders(_pDevice, _pState
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

struct oD3D11FullScreenQuad
{
	// Creates a clip-space quad that covers the entire screen with texcoords 
	// from 0,0 in lower left to 1,1, in upper right. This is forward facing with
	// default DX11 raster state.
	//
	// To get this to be full-screen, WVP should be identity

	oD3D11FullScreenQuad(ID3D11Device* _pDevice, bool _CCWWinding = false);
	~oD3D11FullScreenQuad();

	void Draw(ID3D11DeviceContext* _pDeviceContext);

protected:
	ID3D11Buffer* pVertices;
};

#endif
