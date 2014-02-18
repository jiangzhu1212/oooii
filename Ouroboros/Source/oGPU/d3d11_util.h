/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
// Simplify commonly used D3D11 API.
#pragma once
#ifndef oGPU_d3d11_util_h
#define oGPU_d3d11_util_h

#include <oSurface/buffer.h>
#include <oBasis/oGPUConcepts.h>
#include <vector>

#include <oCore/windows/win_error.h>
#include <d3d11.h>

namespace ouro {
	namespace d3d11 {

// _____________________________________________________________________________
// Debugging API

// Set/Get a name that appears in D3D11's debug layer
void debug_name(ID3D11Device* _pDevice, const char* _Name);
void debug_name(ID3D11DeviceChild* _pDeviceChild, const char* _Name);

char* debug_name(char* _StrDestination, size_t _SizeofStrDestination, const ID3D11Device* _pDevice);
template<size_t size> char* debug_name(char (&_StrDestination)[size], const ID3D11Device* _pDevice) { return debug_name(_StrDestination, size, _pDevice); }
template<size_t capacity> char* debug_name(fixed_string<char, capacity>& _StrDestination, const ID3D11Device* _pDevice) { return debug_name(_StrDestination, _StrDestination.capacity(), _pDevice); }

// Fills the specified buffer with the string set with oD3D11SetDebugName().
char* debug_name(char* _StrDestination, size_t _SizeofStrDestination, const ID3D11DeviceChild* _pDeviceChild);
template<size_t size> char* debug_name(char (&_StrDestination)[size], const ID3D11DeviceChild* _pDeviceChild) { return debug_name(_StrDestination, size, _pDeviceChild); }
template<size_t capacity> char* debug_name(fixed_string<char, capacity>& _StrDestination, const ID3D11DeviceChild* _pDeviceChild) { return debug_name(_StrDestination, _StrDestination.capacity(), _pDeviceChild); }

// Use ID3D11InfoQueue::AddApplicationMessage to trace user errors.
int vtrace(ID3D11InfoQueue* _pInfoQueue, D3D11_MESSAGE_SEVERITY _Severity, const char* _Format, va_list _Args);
inline int vtrace(ID3D11Device* _pDevice, D3D11_MESSAGE_SEVERITY _Severity, const char* _Format, va_list _Args) { intrusive_ptr<ID3D11InfoQueue> InfoQueue; oV(_pDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&InfoQueue)); return vtrace(InfoQueue, _Severity, _Format, _Args); }
inline int vtrace(ID3D11DeviceContext* _pDeviceContext, D3D11_MESSAGE_SEVERITY _Severity, const char* _Format, va_list _Args) { intrusive_ptr<ID3D11Device> Device; _pDeviceContext->GetDevice(&Device); return vtrace(Device, _Severity, _Format, _Args); }
inline int trace(ID3D11InfoQueue* _pInfoQueue, D3D11_MESSAGE_SEVERITY _Severity, const char* _Format, ...) { va_list args; va_start(args, _Format); int len = vtrace(_pInfoQueue, _Severity, _Format, args); va_end(args); return len; }
inline int trace(ID3D11Device* _pDevice, D3D11_MESSAGE_SEVERITY _Severity, const char* _Format, ...) { va_list args; va_start(args, _Format); int len = vtrace(_pDevice, _Severity, _Format, args); va_end(args); return len; }
inline int trace(ID3D11DeviceContext* _pDeviceContext, D3D11_MESSAGE_SEVERITY _Severity, const char* _Format, ...) { va_list args; va_start(args, _Format); int len = vtrace(_pDeviceContext, _Severity, _Format, args); va_end(args); return len; }

// _____________________________________________________________________________
// Utility API

// Creates a device with the specified description.
intrusive_ptr<ID3D11Device> make_device(const gpu::device_init& _Init);

// Convenience wrappers that create and name a particular type of shader.
intrusive_ptr<ID3D11VertexShader> make_vertex_shader(ID3D11Device* _pDevice, const void* _pByteCode, const char* _DebugName);
intrusive_ptr<ID3D11HullShader> make_hull_shader(ID3D11Device* _pDevice, const void* _pByteCode, const char* _DebugName);
intrusive_ptr<ID3D11DomainShader> make_domain_shader(ID3D11Device* _pDevice, const void* _pByteCode, const char* _DebugName);
intrusive_ptr<ID3D11GeometryShader> make_geometry_shader(ID3D11Device* _pDevice, const void* _pByteCode, const char* _DebugName);
intrusive_ptr<ID3D11PixelShader> make_pixel_shader(ID3D11Device* _pDevice, const void* _pByteCode, const char* _DebugName);
intrusive_ptr<ID3D11ComputeShader> make_compute_kernel(ID3D11Device* _pDevice, const void* _pByteCode, const char* _DebugName);

// Returns information about the specified device. There's no way to determine
// if the device is software, so pass that through.
gpu::device_info get_info(ID3D11Device* _pDevice, bool _IsSoftwareEmulation);

// Returns the D3D11 equivalent.
D3D11_PRIMITIVE_TOPOLOGY from_primitive_type(const mesh::primitive_type::value& _Type);
mesh::primitive_type::value to_primitive_type(D3D11_PRIMITIVE_TOPOLOGY _Type);

// Returns the number of elements as described the specified topology given
// the number of primitives. An element can refer to indices or vertices, but
// basically if there are 3 lines, then there are 6 elements. If there are 3
// lines in a line strip, then there are 4 elements.
unsigned int num_elements(D3D_PRIMITIVE_TOPOLOGY _PrimitiveTopology, unsigned int _NumPrimitives);

// Given a shader model (i.e. 4.0) return a feature level (i.e. D3D_FEATURE_LEVEL_10_1)
D3D_FEATURE_LEVEL feature_level(const version& _ShaderModel);

// Returns the shader profile for the specified stage of the specified feature
// level. If the specified feature level does not support the specified stage,
// this will return nullptr.
const char* shader_profile(D3D_FEATURE_LEVEL _Level, gpu::pipeline_stage::value _Stage);

// Given a buffer of compiled byte code, return its size.
size_t byte_code_size(const void* _pByteCode);

// _____________________________________________________________________________
// Buffer API

// Allow ID3D11Buffers to be a bit more self-describing - mainly for index 
// buffers.
void set_info(ID3D11Resource* _pBuffer, const gpu::buffer_info& _Info);
gpu::buffer_info get_info(const ID3D11Resource* _pBuffer);

// Create common interfaces. NOTE: It is often benchmarked as faster due to 
// driver PCIE usage to use D3D11_USAGE_DEFAULT and UpdateSubresource rather 
// than D3D11_USAGE_DYNAMIC and Map/Unmap. Benchmark to be sure, but it is 
// generally a cleaner design and easier to code if all resources are DEFAULT. 
// Pointers to initial data can be null, but number/size values are used to 
// allocate the buffer and thus must always be specified.

intrusive_ptr<ID3D11Buffer> make_buffer(ID3D11Device* _pDevice
	, const char* _DebugName
	, const gpu::buffer_info& _Info
	, const void* _pInitBuffer
	, ID3D11UnorderedAccessView** _ppUAV = nullptr
	, ID3D11ShaderResourceView** _ppSRV = nullptr);

// Copies the contents of the specified texture to _pBuffer, which is assumed to
// be properly allocated to receive the contents. If _FlipVertical is true, then
// the bitmap data will be copied such that the destination will be upside-down 
// compared to the source.
void copy(ID3D11Resource* _pTexture
	, unsigned int _Subresource
	, surface::mapped_subresource* _pDstSubresource
	, bool _FlipVertically = false);

// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476486(v=vs.85).aspx
// This is not cheap enough to reevaluate for each call to update_subresource, so
// call this once and cache the result per device and pass it to update_subresource
// as appropriate.
bool supports_deferred_contexts(ID3D11Device* _pDevice);

// If _pDstResource is D3D11_USAGE_DEFAULT, then this calls UpdateSubresource. 
// If resource is D3D11_USAGE_DYNAMIC or D3D11_USAGE_STAGING then map, memcpy, 
// unmap is used. This exists because some forms of D3D11 (D3D11 API with D3D10 
// feature level) don't always work as advertised. For more info about the last
// parameter follow this link:

void update_subresource(ID3D11DeviceContext* _pDeviceContext
	, ID3D11Resource* _pDstResource
	, unsigned int _DstSubresource
	, const D3D11_BOX* _pDstBox
	, const surface::const_mapped_subresource& _Source
	, bool _DeviceSupportsDeferredContexts);

// If _pResource is D3D11_USAGE_DEFAULT, then this allocates a buffer using new
// and returns it in _pMappedResource. If _pResource is D3D11_USAGE_DYNAMIC or 
// D3D11_USAGE_STAGING, then Map with WRITE_DISCARD is called. Use oD3D11Unmap 
// to either unmap or free the memory. This wrapper is provided because 
// sometimes it's faster to do the malloc, memcpy and UpdateSubresource on a 
// default resource than it is to map/unmap a dynamic resource, so using this 
// API allows better abstraction based on resource specification rather than 
// coding a particular choice throughout user code.
void map_write_discard(ID3D11DeviceContext* _pDeviceContext
	, ID3D11Resource* _pResource
	, unsigned int _Subresource
	, surface::mapped_subresource* _pMappedResource);

// Undo what oD3D11MapWriteDiscard did. Because it may have to free memory, the
// mapped subresource should be passed to this.
void unmap(ID3D11DeviceContext* _pDeviceContext
	, ID3D11Resource* _pResource
	, unsigned int _Subresource
	, surface::mapped_subresource& _MappedResource);

// Uses oD3D11MapWriteDiscard/oD3D11Unmap to copy the specified source buffer
// into the specified index buffer, swizzling the data as appropriate. No error
// checking is done when swizzling from unsigned int to ushort - upper indexing will be
// lost. The source buffer is assumed to contain the full number of indices to
// fill the specified index buffer.
void update_index_buffer(ID3D11DeviceContext* _pDeviceContext
	, ID3D11Buffer* _pIndexBuffer
	, const unsigned int* _pSourceIndices);

void update_index_buffer(ID3D11DeviceContext* _pDeviceContext
	, ID3D11Buffer* _pIndexBuffer
	, const unsigned short* _pSourceIndices);

// _____________________________________________________________________________
// Texture API

// Uses oTRACE to display the fields of the specified desc.
void trace_texture2d_desc(const D3D11_TEXTURE2D_DESC& _Desc, const char* _Prefix = "\t");

// Fills the specified texture_info with the description from the specified 
// resource. The resource can be: ID3D11Texture1D, ID3D11Texture2D
// ID3D11Texture3D or ID3D11Buffer. If an ID3D11Buffer the size of a single
// struct is in dimensions.x, and the number of structures is in dimensions.y.
// The y value will be replicated in array_size as well.
gpu::texture_info get_texture_info(ID3D11Resource* _pResource, bool _AsArray = false, D3D11_USAGE* _pUsage = nullptr);

// This converts back from a texture_info to typical fields in texture-related
// structs (including D3DX11_IMAGE_LOAD_INFO, specify the info's format as 
// unknown to use DXGI_FORMAT_FROM_FILE)
void init_values(const gpu::texture_info& _Info
	, DXGI_FORMAT* _pFormat
	, D3D11_USAGE* _pUsage
	, unsigned int* _pCPUAccessFlags
	, unsigned int* _pBindFlags
	, unsigned int* _pMipLevels
	, unsigned int* _pMiscFlags = nullptr);

// From the specified texture, create the correct shader resource view
intrusive_ptr<ID3D11ShaderResourceView> make_srv(const char* _DebugName, ID3D11Resource* _pTexture, bool _AsArray = false);

intrusive_ptr<ID3D11View> make_rtv(const char* _DebugName, ID3D11Resource* _pTexture);
template<typename ViewT> void make_rtv(const char* _DebugName, ID3D11Resource* _pTexture, intrusive_ptr<ViewT>& _View) 
	{ _View = static_cast<ViewT*>(make_rtv(_DebugName, _pTexture).c_ptr()); }

// Creates a UAV that matches the meta-data of the specified texture along with
// the specified mip and array topology for the specified texture.
intrusive_ptr<ID3D11UnorderedAccessView> make_uav(const char* _DebugName
	, ID3D11Resource* _pTexture, unsigned int _MipSlice, unsigned int _ArraySlice);

// Creates a copy of the specified UAV that clears any raw/append/counter flags
intrusive_ptr<ID3D11UnorderedAccessView> make_unflagged_copy(ID3D11UnorderedAccessView* _pSourceUAV);

// Creates a texture according to the specified desc. If the DESC describes the 
// texture as a render target, it will be created properly and the view will be 
// filled in. If a depth format is specified then the view will be an 
// ID3D11DepthStencilView instead of an ID3D11RenderTargetView.

struct new_texture
{
	new_texture()
		: pResource(nullptr)
		, pSRV(nullptr)
		, pView(nullptr)
	{}

	new_texture(const new_texture& _That) { operator=(_That); }
	const new_texture& operator=(const new_texture& _That)
	{
		clear();
		pResource = _That.pResource; if (pResource) pResource->AddRef();
		pSRV = _That.pSRV; if (pSRV) pSRV->AddRef();
		pView = _That.pView; if (pView) pView->AddRef();
		return *this;
	}

	new_texture(new_texture&& _That) { eviscerate(std::move(_That)); }
	new_texture& operator=(new_texture&& _That)
	{
		if (this != &_That)
		{
			clear();
			eviscerate(std::move(_That));
		}
		return *this;
	}

	~new_texture() { clear(); }

	void eviscerate(new_texture&& _That)
	{
		pResource = _That.pResource; _That.pResource = nullptr;
		pSRV = _That.pSRV; _That.pSRV = nullptr;
		pView = _That.pView; _That.pView = nullptr;
	}

	void clear()
	{
		if (pResource) pResource->Release();
		if (pSRV) pSRV->Release();
		if (pView) pView->Release();
	}

	union
	{
		ID3D11Resource* pResource;
		ID3D11Texture1D* pTexture1D;
		ID3D11Texture2D* pTexture2D;
		ID3D11Texture3D* pTexture3D;
	};
	ID3D11ShaderResourceView* pSRV;
	union
	{
		ID3D11View* pView;
		ID3D11RenderTargetView* pRTV;
		ID3D11DepthStencilView* pDSV;
	};
};

new_texture make_texture(ID3D11Device* _pDevice
	, const char* _DebugName
	, const gpu::texture_info& _Info
	, surface::const_mapped_subresource* _pInitData = nullptr);

// Creates a CPU-readable copy of the specified texture/render target. Only 
// textures are currently supported.
intrusive_ptr<ID3D11Resource> make_cpu_copy(ID3D11Resource* _pResource);

// Copies the specified render target to the specified image/path
std::shared_ptr<surface::buffer> make_snapshot(ID3D11Texture2D* _pRenderTarget);

// _____________________________________________________________________________
// State/Flush API

// Sets the specified buffers on all pipeline stages. This may not be the most
// efficient thing to do, but is convenient during initial renderer bringup.
void set_constant_buffers(ID3D11DeviceContext* _pDeviceContext
	, unsigned int _StartSlot
	, unsigned int _NumBuffers
	, const ID3D11Buffer* const* _ppConstantBuffers);

inline void set_constant_buffers(ID3D11DeviceContext* _pDeviceContext
	, unsigned int _StartSlot
	, unsigned int _NumBuffers
	, intrusive_ptr<ID3D11Buffer>* _ppConstantBuffers) 
		{ set_constant_buffers(_pDeviceContext, _StartSlot, _NumBuffers, (const ID3D11Buffer* const*)_ppConstantBuffers); }

// Sets the specified sampler sates on all pipeline stages. This may not be the 
// most efficient thing to do, but is convenient during initial renderer 
// bringup.
void set_samplers(ID3D11DeviceContext* _pDeviceContext
	, unsigned int _StartSlot
	, unsigned int _NumSamplers
	, const ID3D11SamplerState* const* _ppSamplers);

inline void set_samplers(ID3D11DeviceContext* _pDeviceContext
	, unsigned int _StartSlot
	, unsigned int _NumSamplers
	, intrusive_ptr<ID3D11SamplerState>* _ppSamplers) 
		{ set_samplers(_pDeviceContext, _StartSlot, _NumSamplers, (const ID3D11SamplerState* const*)_ppSamplers); }

// Sets the specified SRVs on all pipeline stages. This may not be the most 
// efficient thing to do, but is convenient during initial renderer bringup.
void set_srvs(ID3D11DeviceContext* _pDeviceContext
	, unsigned int _StartSlot
	, unsigned int _NumShaderResourceViews
	, const ID3D11ShaderResourceView* const* _ppViews);

inline void set_srvs(ID3D11DeviceContext* _pDeviceContext
	, unsigned int _StartSlot
	, unsigned int _NumShaderResourceViews
	, intrusive_ptr<ID3D11ShaderResourceView>* _ppViews) 
		{ set_srvs(_pDeviceContext, _StartSlot, _NumShaderResourceViews, (const ID3D11ShaderResourceView* const*)_ppViews); }

// Converts a viewport to an oAABoxf.
oAABoxf from_viewport(const D3D11_VIEWPORT& _Viewport);

// Convert an oAABoxf (very similar in structure) to a D3D11_VIEWPORT
D3D11_VIEWPORT to_viewport(const oAABoxf& _Source);

// Creats a viewport that uses the full render target (depth, [0,1])
D3D11_VIEWPORT to_viewport(const int2& _RenderTargetDimensions);

// Implementations of scanning the specified buffers to see if they're bound to
// certain quiet/noop-y behaviors when using compute. IMHO certain DX warnings
// should be errors and certain silences should make noise. These API are 
// exposed here as much a documentation/callout to the user as they are part of
// implementing a robust checking system in the cross-platform API. Basically
// D3D11 will promote a read-only binding to read-write, but will not all a 
// read-write binding to be replaced/concurrent with a read-only binding.
void check_bound_rts_and_uavs(ID3D11DeviceContext* _pDeviceContext
	, int _NumBuffers, ID3D11Buffer** _ppBuffers);

// check that none of the specified buffers are bound to CS UAVs.
void check_bound_cs_uavs(ID3D11DeviceContext* _pDeviceContext, int _NumBuffers
	, ID3D11Buffer** _ppBuffers);

#if 0

// The error message returned from D3DX11CompileFromMemory is not fit for
// passing to printf directly, so pass it to this to create a cleaner string.
// _pErrorMessages can be NULL, but if there is a message and there is an 
// error filling the specified string buffer.
bool oD3D11ConvertCompileErrorBuffer(char* _OutErrorMessageString, size_t _SizeofOutErrorMessageString, ID3DBlob* _pErrorMessages, const char** _pIncludePaths, size_t _NumIncludePaths);
template<size_t size> bool oD3D11ConvertCompileErrorBuffer(char (&_OutErrorMessageString)[size], ID3DBlob* _pErrorMessages, const char** _pIncludePaths, size_t _NumIncludePaths) { return oD3D11ConvertCompileErrorBuffer(_OutErrorMessageString, size, _pErrorMessages, _pIncludePaths, _NumIncludePaths); }
template<size_t capacity> bool oD3D11ConvertCompileErrorBuffer(fixed_string<char, capacity>& _OutErrorMessageString, ID3DBlob* _pErrorMessages, const char** _pIncludePaths, size_t _NumIncludePaths) { return oD3D11ConvertCompileErrorBuffer(_OutErrorMessageString, _OutErrorMessageString.capacity(), _pErrorMessages, _pIncludePaths, _NumIncludePaths); }


// Create and set a single viewport that uses the entire render target.
void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11Texture2D* _pRenderTarget, float _MinDepth = 0.0f, float _MaxDepth = 1.0f);

// Extracts the resource from the view and calls the above 
// oD3D11SetFullTargetViewport.
void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11RenderTargetView* _pRenderTargetView, float _MinDepth = 0.0f, float _MaxDepth = 1.0f);

// A neat little trick when drawing quads, fullscreen or otherwise. Submits a 
// triangle strip with no vertices and optionally a null instance buffer. Use 
// oExtractQuadInfoFromVertexID() in oHLSL.h to reconstruct position and 
// texcoords using SV_VertexID and optionally SV_InstanceID.
// NOTE: This leaves primitive topology set to tristrips.
void oD3D11DrawSVQuad(ID3D11DeviceContext* _pDeviceContext, unsigned int _NumInstances = 1);

// _____________________________________________________________________________
// Pipeline stage state API

// GPUs are programmable, but graphics applications tend to decide their own 
// flavor of features, then fix the pipeline to make that specific combination 
// as efficient as possible. Towards this end, here's a useful pattern of 
// enumerating DX11 state in very typical usage patterns that seem to be 
// somewhat portable.

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

// Create a set of command line options that you would pass to FXC and the 
// loaded source to compile to create a buffer filled with bytecode. If false,
// oBuffer will be allocated and filled with a nul-terminated string that 
// describes the error as reported from the underlying compiler. oErrorGetLast
// will also include useful - but not as specified - information.
bool oFXC(const char* _CommandLineOptions, const char* _ShaderSourceFilePath, const char* _ShaderSource, oBuffer** _ppBuffer);

#endif

	} // namespace d3d11
} // namespace ouro

// These functions repeated from <oBC6HBC7EncoderDecoder.h> in External so as
// not to require extra path info in build settings to get at that header. 
// @tony: Probably it'd be better to rename these in that header and wrap an 
// impl and leave these as the "public" api.
bool oD3D11EncodeBC7(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppCompressedTexture);
bool oD3D11EncodeBC6HS(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppCompressedTexture);
bool oD3D11EncodeBC6HU(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppCompressedTexture);
bool oD3D11DecodeBC6orBC7(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppUncompressedTexture);

static const DXGI_FORMAT oD3D11BC7RequiredSourceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
static const DXGI_FORMAT oD3D11BC6HRequiredSourceFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

#endif
