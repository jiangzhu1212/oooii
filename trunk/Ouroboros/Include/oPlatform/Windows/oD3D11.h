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
#include <oBasis/oGeometry.h> // @oooii-tony: For oD3D11MosaicDraw... this should get better encapsulated.
#include <oBasis/oGPUEnums.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oHLSL.h> // oHLSLGetByteCodeSize() ... @oooii-tony: perhaps those functions should be separate so that HLSL iteration doesn't require this to recompile?
#include <oPlatform/oImage.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/Windows/oWindows.h>
#include <vector>

// @oooii-tony: TIME TO REFACTOR! oGPU_ should depend on oPlatform and NOT vice
// versa. For now... start replacing symbols defined here with the more cross-
// platform oGPU_ stuff... then once other refactors are more baked, move oD3D11
// to oGPU_.
#include <oGPU/oGPUUtil.h>

// _____________________________________________________________________________
// Soft-link

struct oD3D11 : oProcessSingleton<oD3D11>
{
	static const oGUID GUID;
	oD3D11();
	~oD3D11();

	HRESULT (__stdcall *D3D11CreateDevice)(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

protected:
	oHMODULE hModule;
};

// _____________________________________________________________________________
// Utility API

// D3D11 doesn't keep track of D3D11_BUFFER_DESC::StructureByteStride for at 
// least index buffers, so to keep it self-contained, tack on this struct so 
// that an ID3D11Buffer can be queried for whether it's a 16-bit or 32-bit index
// buffer.
struct oD3D11_BUFFER_TOPOLOGY
{
	uint ElementStride;
	uint ElementCount;
};

// Creates a device with the specified description.
bool oD3D11CreateDevice(const oGPU_DEVICE_INIT& _Init, bool _SingleThreaded, ID3D11Device** _ppDevice);

// Returns information about the specified device. There's no way to determine
// if the device is software, so pass that through.
bool oD3D11DeviceGetDesc(ID3D11Device* _pDevice, bool _IsSoftwareEmulation, oGPU_DEVICE_DESC* _pDesc);

// Get the adapter with which the specified device was created
bool oD3D11GetAdapter(ID3D11Device* _pDevice, IDXGIAdapter** _ppAdapter);

// Convert oGPU_VERTEX_ELEMENT's Semantic fourcc code to an HLSL semantic 
// name.
const char* oD3D11AsSemantic(const oFourCC& _FourCC);

// Utilities for checking that structs are aligned to 16 bytes, which is a 
// required alignment for most GPU languages.
#define oD3D11_CHECK_STRUCT_SIZE(_Struct) static_assert((sizeof(_Struct) % 16) == 0, "Alignment of the specified struct is incompatible")
template<typename T> inline bool oD3D11IsValidConstantBufferSize(const T& _ConstantBufferStruct) { return (sizeof(_ConstantBufferStruct) % 16) == 0; }

const char* oAsString(D3D11_BIND_FLAG _Flag);
const char* oAsString(D3D11_CPU_ACCESS_FLAG _Flag);
const char* oAsString(D3D11_RESOURCE_DIMENSION _Type);
const char* oAsString(D3D11_RESOURCE_MISC_FLAG _Flag);
const char* oAsString(D3D11_USAGE _Usage);

// Converts the specified D3D_FEATURE_LEVEL into an oVersion.
oVersion oD3D11GetFeatureVersion(D3D_FEATURE_LEVEL _Level);

// Set a name that appears in D3D11's debug layer
bool oD3D11SetDebugName(ID3D11Device* _pDevice, const char* _Name);
bool oD3D11SetDebugName(ID3D11DeviceChild* _pDeviceChild, const char* _Name);

char* oD3D11GetDebugName(char* _StrDestination, size_t _SizeofStrDestination, const ID3D11Device* _pDevice);
template<size_t size> char* oD3D11GetDebugName(char (&_StrDestination)[size], const ID3D11Device* _pDevice) { return oD3D11GetDebugName(_StrDestination, size, _pDevice); }
template<size_t capacity> char* oD3D11GetDebugName(oFixedString<char, capacity>& _StrDestination, const ID3D11Device* _pDevice) { return oD3D11GetDebugName(_StrDestination, _StrDestination.capacity(), _pDevice); }

// Fills the specified buffer with the string set with oD3D11SetDebugName().
char* oD3D11GetDebugName(char* _StrDestination, size_t _SizeofStrDestination, const ID3D11DeviceChild* _pDeviceChild);
template<size_t size> char* oD3D11GetDebugName(char (&_StrDestination)[size], const ID3D11DeviceChild* _pDeviceChild) { return oD3D11GetDebugName(_StrDestination, size, _pDeviceChild); }
template<size_t capacity> char* oD3D11GetDebugName(oFixedString<char, capacity>& _StrDestination, const ID3D11DeviceChild* _pDeviceChild) { return oD3D11GetDebugName(_StrDestination, _StrDestination.capacity(), _pDeviceChild); }

// Returns an IFF based on the extension specified in the file path
D3DX11_IMAGE_FILE_FORMAT oD3D11GetFormatFromPath(const char* _Path);

// Allow ID3D11Buffers to be a bit more self-describing.
bool oD3D11SetBufferTopology(ID3D11Resource* _pBuffer, const oD3D11_BUFFER_TOPOLOGY& _Topology);
bool oD3D11GetBufferTopology(const ID3D11Resource* _pBuffer, oD3D11_BUFFER_TOPOLOGY* _pTopology);

// Returns the number of elements as described the specified topology given
// the number of primitives. An element can refer to indices or vertices, but
// basically if there are 3 lines, then there are 6 elements. If there are 3
// lines in a line strip, then there are 4 elements.
uint oD3D11GetNumElements(D3D11_PRIMITIVE_TOPOLOGY _PrimitiveTopology, uint _NumPrimitives);

// Given a shader model (i.e. 4.0) return a feature level (i.e. D3D_FEATURE_LEVEL_10_1)
bool oD3D11GetFeatureLevel(const oVersion& _ShaderModel, D3D_FEATURE_LEVEL* _pLevel);

// Returns the shader profile for the specified stage of the specified feature
// level. If the specified feature level does not support the specified stage,
// this will return nullptr.
const char* oD3D11GetShaderProfile(D3D_FEATURE_LEVEL _Level, oGPU_PIPELINE_STAGE _Stage);

// The error message returned from D3DX11CompileFromMemory is not fit for
// passing to printf directly, so pass it to this to create a cleaner string.
// _pErrorMessages can be NULL, but if there is a message and there is an 
// error filling the specified string buffer.
bool oD3D11ConvertCompileErrorBuffer(char* _OutErrorMessageString, size_t _SizeofOutErrorMessageString, ID3DBlob* _pErrorMessages, const char** _pIncludePaths, size_t _NumIncludePaths);
template<size_t size> bool oD3D11ConvertCompileErrorBuffer(char (&_OutErrorMessageString)[size], ID3DBlob* _pErrorMessages, const char** _pIncludePaths, size_t _NumIncludePaths) { return oD3D11ConvertCompileErrorBuffer(_OutErrorMessageString, size, _pErrorMessages, _pIncludePaths, _NumIncludePaths); }
template<size_t capacity> bool oD3D11ConvertCompileErrorBuffer(oFixedString<char, capacity>& _OutErrorMessageString, ID3DBlob* _pErrorMessages, const char** _pIncludePaths, size_t _NumIncludePaths) { return oD3D11ConvertCompileErrorBuffer(_OutErrorMessageString, _OutErrorMessageString.capacity(), _pErrorMessages, _pIncludePaths, _NumIncludePaths); }

// _____________________________________________________________________________
// Buffer API

// Create common interfaces. NOTE: It is often benchmarked as faster due to 
// driver PCIE usage to use D3D11_USAGE_DEFAULT and UpdateSubresource rather 
// than D3D11_USAGE_DYNAMIC and Map/Unmap. Benchmark to be sure, but it is 
// generally a cleaner design and easier to code if all resources are DEFAULT. 
// Pointers to initial data can be null, but number/size values are used to 
// allocate the buffer and thus must always be specified.

bool oD3D11CreateBuffer(ID3D11Device* _pDevice, const char* _DebugName, const oGPU_BUFFER_DESC& _Desc, const void* _pInitBuffer, ID3D11Buffer** _ppConstantBuffer, ID3D11UnorderedAccessView** _ppUAV = nullptr);
bool oD3D11CreateIndexBuffer(ID3D11Device* _pDevice, const char* _DebugName, D3D11_USAGE _Usage, const void* _pIndices, uint _NumIndices, bool _Use16BitIndices, ID3D11Buffer** _ppIndexBuffer);
bool oD3D11CreateVertexBuffer(ID3D11Device* _pDevice, const char* _DebugName, D3D11_USAGE _Usage, const void* _pVertices, uint _NumVertices, uint _VertexStride, ID3D11Buffer** _ppVertexBuffer);

// Copies the contents of the specified texture to _pBuffer, which is assumed to
// be properly allocated to receive the contents. If _FlipVertical is true, then
// the bitmap data will be copied such that the destination will be upside-down 
// compared to the source.
bool oD3D11CopyTo(ID3D11Resource* _pTexture, uint _Subresource, void* _pDestination, uint _DestinationRowPitch, bool _FlipVertically = false);

// If _pDstResource is D3D11_USAGE_DEFAULT, then this calls UpdateSubresource. 
// If resource is D3D11_USAGE_DYNAMIC or D3D11_USAGE_STAGING then map, memcpy, 
// unmap is used. This exists because some forms of D3D11 (D3D11 API with D3D10 
// feature level) don't always work as advertised.
void oD3D11UpdateSubresource(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pDstResource, uint _DstSubresource, const D3D11_BOX* _pDstBox, const void* _pSrcData, uint _SrcRowPitch, uint _SrcDepthPitch);

// If _pResource is D3D11_USAGE_DEFAULT, then this allocates a buffer using new
// and returns it in _pMappedResource. If _pResource is D3D11_USAGE_DYNAMIC or 
// D3D11_USAGE_STAGING, then Map with WRITE_DISCARD is called. Use oD3D11Unmap 
// to either unmap or free the memory. This wrapper is provided because 
// sometimes it's faster to do the malloc, memcpy and UpdateSubresource on a 
// default resource than it is to map/unmap a dynamic resource, so using this 
// API allows better abstraction based on resource specification rather than 
// coding a particular choice throughout user code.
void oD3D11MapWriteDiscard(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pResource, uint _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedResource);

// Undo what oD3D11MapWriteDiscard did. Because it may have to free memory, the
// mapped subresource should be passed to this.
void oD3D11Unmap(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pResource, uint _Subresource, oSURFACE_MAPPED_SUBRESOURCE& _MappedResource);

// Uses oD3D11MapWriteDiscard/oD3D11Unmap to copy the specified source buffer
// into the specified index buffer, swizzling the data as appropriate. No error
// checking is done when swizzling from uint to ushort - upper indexing will be
// lost. The source buffer is assumed to contain the full number of indices to
// fill the specified index buffer.
void oD3D11UpdateIndexBuffer(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pIndexBuffer, const uint* _pSourceIndices);
void oD3D11UpdateIndexBuffer(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pIndexBuffer, const ushort* _pSourceIndices);

// Copies the entire subresource of one on-GPU resource to another on-GPU 
// resource. All requirements of D3D11 must be met for the resources (see D3D11 
// documentation on CopySubresourceRegion). This does some extra error checking
// and simplifies the API.
void oD3D11CopySubresource2D(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pDstResource, uint _DstSubresource, ID3D11Resource* _pSrcResource, uint _SrcSubresource);

// Copies a subregion of a subresource from an on-GPU resource to another on-GPU
// resource. All requirements of D3D11 must be met for the resources (see D3D11 
// documentation on CopySubresourceRegion). This does some extra error checking 
// and simplifies the API.
void oD3D11CopySubresourceRegion2D(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pDstResource, uint _DstSubresource, const int2& _DstTopLeft, ID3D11Resource* _pSrcResource, uint _SrcSubresource, const int2& _SrcTopLeft, const int2& _NumPixels);

// _____________________________________________________________________________
// Texture API

// @oooii-tony: This should probably be generalized to oGPU_TEXTURE_DESC, but
// this goes to the original source in case there's a bug in translation.
void oD3D11DebugTraceTexture2DDesc(D3D11_TEXTURE2D_DESC _Desc, const char* _Prefix = "\t");

// Fills the specified oGPU_TEXTURE_DESC with the description from the 
// specified resource. The resource can be: ID3D11Texture1D, ID3D11Texture2D
// ID3D11Texture3D, or ID3D11Buffer. If an ID3D11Buffer, the entire byte width 
// of the buffer is in Dimensions.x, Dimensions.y is 1, and NumSlices is the 
// number of elements stored in the buffer, so the stride of a single item is 
// uint Stride = Dimensions.x / NumSlices;
void oD3D11GetTextureDesc(ID3D11Resource* _pResource, oGPU_TEXTURE_DESC* _pDesc, D3D11_USAGE* _pUsage = nullptr);

// From the specified texture, create the correct shader resource view
bool oD3D11CreateShaderResourceView(const char* _DebugName, ID3D11Resource* _pTexture, ID3D11ShaderResourceView** _ppShaderResourceView);
bool oD3D11CreateRenderTargetView(const char* _DebugName, ID3D11Resource* _pTexture, ID3D11View** _ppView);
template<typename T> bool oD3D11CreateRenderTargetView(const char* _DebugName, ID3D11Resource* _pTexture, T** _ppView) { return oD3D11CreateRenderTargetView(_DebugName, _pTexture, (ID3D11View**)_ppView); }

// Creates a UAV that matches the meta-data of the specified texture along with
// the specified mip and array topology for the specified texture.
bool oD3D11CreateUnorderedAccessView(const char* _DebugName, ID3D11Resource* _pTexture, uint _MipSlice, uint _ArraySlice, ID3D11UnorderedAccessView** _ppUnorderedAccessView);

// Creates a texture according to the specified desc. If a shader resource view 
// is specified, one is created using oD3D11CreateShaderResourceView. If the 
// DESC describes the texture as a render target, it will be created properly,
// and optionally _ppRenderTargetView will be filled in. If a depth format is 
// specified, then the view will be an ID3D11DepthStencilView instead of an
// ID3D11RenderTargetView.
bool oD3D11CreateTexture(ID3D11Device* _pDevice, const char* _DebugName, const oGPU_TEXTURE_DESC& _Desc, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pInitData, ID3D11Resource** _ppTexture, ID3D11ShaderResourceView** _ppShaderResourceView = nullptr, ID3D11View** _ppRenderTargetView = nullptr);
template<typename TextureT> bool oD3D11CreateTexture(ID3D11Device* _pDevice, const char* _DebugName, const oGPU_TEXTURE_DESC& _Desc, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pInitData, TextureT** _ppTexture, ID3D11ShaderResourceView** _ppShaderResourceView = nullptr, ID3D11View** _ppRenderTargetView = nullptr) { return oD3D11CreateTexture(_pDevice, _DebugName, _Desc, _pInitData, (ID3D11Resource**)_ppTexture, _ppShaderResourceView, _ppRenderTargetView); }
template<typename TextureT, typename ViewT> bool oD3D11CreateTexture(ID3D11Device* _pDevice, const char* _DebugName, const oGPU_TEXTURE_DESC& _Desc, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pInitData, TextureT** _ppTexture, ID3D11ShaderResourceView** _ppShaderResourceView = nullptr, ViewT** _ppRenderTargetView = nullptr) { return oD3D11CreateTexture(_pDevice, _DebugName, _Desc, _pInitData, (ID3D11Resource**)_ppTexture, _ppShaderResourceView, (ID3D11View**)_ppRenderTargetView); }

// Creates a CPU-readable copy of the specified texture/render target. Only 
// textures are currently supported.
bool oD3D11CreateCPUCopy(ID3D11Resource* _pTexture, ID3D11Resource** _ppCPUCopy);
template<typename TextureT> bool oD3D11CreateCPUCopy(ID3D11Resource* _pTexture, TextureT** _ppCPUCopy) { return oD3D11CreateCPUCopy(_pTexture, (ID3D11Resource**)_ppCPUCopy); }

// _____________________________________________________________________________
// Texture Serialization API

// Copies the specified render target to the specified image/path
bool oD3D11CreateSnapshot(ID3D11Texture2D* _pRenderTarget, oImage** _ppImage);
bool oD3D11CreateSnapshot(ID3D11Texture2D* _pRenderTarget, D3DX11_IMAGE_FILE_FORMAT _Format, const char* _Path);

// Saves image to the specified memory buffer, which must be allocated large
// enough to receive the specified image as its file form.
// NOTE: BC6HS, BX6HU and BC7 are brand new formats that seemingly no one on 
// earth supports. The only way to really know if it works is to use a BC6/7
// format as a texture, or convert a BC6/7 DDS back to something else and view
// that result in a tool. Remember, only DDS really supports all surface formats.
bool oD3D11Save(ID3D11Resource* _pTexture, D3DX11_IMAGE_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer);
bool oD3D11Save(const oImage* _pImage, D3DX11_IMAGE_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer);
bool oD3D11Save(ID3D11Resource* _pTexture, D3DX11_IMAGE_FILE_FORMAT _Format, const char* _Path);
bool oD3D11Save(const oImage* _pImage, D3DX11_IMAGE_FILE_FORMAT _Format, const char* _Path);

// Creates a new texture by parsing _pBuffer as a D3DX11-supported file format
// Specify oSURFACE_UNKNOWN and oDEFAULT for x, y or NumSlices in the _Desc to 
// use values from the specified file. If mips is specified as HAS_MIPs, then 
// mips will be allocated, but not filled in. If AUTO_MIPS is specified, then 
// mips will be generated.
bool oD3D11Load(ID3D11Device* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _Path, const char* _DebugName, ID3D11Resource** _ppTexture);
bool oD3D11Load(ID3D11Device* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _DebugName, const void* _pBuffer, size_t _SizeofBuffer, ID3D11Resource** _ppTexture);

// Uses GPU acceleration for BC6H and BC7 conversions if the source is in the 
// correct format. All other conversions go through D3DX11LoadTextureFromTexture
bool oD3D11Convert(ID3D11Texture2D* _pSourceTexture, oSURFACE_FORMAT _NewFormat, ID3D11Texture2D** _ppDestinationTexture);

// Use the above oD3D11Convert(), but the parameters could be CPU buffers. This
// means there's a copy-in and a copy-out to set up the source and destination
// textures for the internal call.
bool oD3D11Convert(ID3D11Device* _pDevice, oSURFACE_MAPPED_SUBRESOURCE& _Destination, oSURFACE_FORMAT _DestinationFormat, oSURFACE_CONST_MAPPED_SUBRESOURCE& _Source, oSURFACE_FORMAT _SourceFormat, const int2& _MipDimensions);

// These functions repeated from <oBC6HBC7EncoderDecoder.h> in External so as
// not to require extra path info in build settings to get at that header. 
// @oooii-tony: Probably it'd be better to rename these in that header and wrap
// an impl and leave these as the "public" api.
bool oD3D11EncodeBC7(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppCompressedTexture);
bool oD3D11EncodeBC6HS(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppCompressedTexture);
bool oD3D11EncodeBC6HU(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppCompressedTexture);
bool oD3D11DecodeBC6orBC7(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppUncompressedTexture);

static const DXGI_FORMAT oD3D11BC7RequiredSourceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
static const DXGI_FORMAT oD3D11BC6HRequiredSourceFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

// _____________________________________________________________________________
// State/Flush API

// Sets the specified buffers on all pipeline stages. This may not be the most
// efficient thing to do, but is convenient during initial renderer bringup.
void oD3D11SetConstantBuffers(ID3D11DeviceContext* _pDeviceContext, uint _StartSlot, uint _NumBuffers, const ID3D11Buffer* const* _ppConstantBuffers);
inline void oD3D11SetConstantBuffers(ID3D11DeviceContext* _pDeviceContext, uint _StartSlot, uint _NumBuffers, oRef<ID3D11Buffer>* _ppConstantBuffers) { oD3D11SetConstantBuffers(_pDeviceContext, _StartSlot, _NumBuffers, (const ID3D11Buffer* const*)_ppConstantBuffers); }

// Sets the specified sampler sates on all pipeline stages. This may not be the 
// most efficient thing to do, but is convenient during initial renderer 
// bringup.
void oD3D11SetSamplers(ID3D11DeviceContext* _pDeviceContext, uint _StartSlot, uint _NumSamplers, const ID3D11SamplerState* const* _ppSamplers);
inline void oD3D11SetSamplers(ID3D11DeviceContext* _pDeviceContext, uint _StartSlot, uint _NumSamplers, oRef<ID3D11SamplerState>* _ppSamplers) { oD3D11SetSamplers(_pDeviceContext, _StartSlot, _NumSamplers, (const ID3D11SamplerState* const*)_ppSamplers); }

// Sets the specified SRVs on all pipeline stages. This may not be the most 
// efficient thing to do, but is convenient during initial renderer bringup.
void oD3D11SetShaderResourceViews(ID3D11DeviceContext* _pDeviceContext, uint _StartSlot, uint _NumShaderResourceViews, const ID3D11ShaderResourceView* const* _ppViews);
inline void oD3D11SetShaderResourceViews(ID3D11DeviceContext* _pDeviceContext, uint _StartSlot, uint _NumShaderResourceViews, oRef<ID3D11ShaderResourceView>* _ppViews) { oD3D11SetShaderResourceViews(_pDeviceContext, _StartSlot, _NumShaderResourceViews, (const ID3D11ShaderResourceView* const*)_ppViews); }

// Convert an oAABoxf (very similar in structure) to a D3D11_VIEWPORT
void oD3D11InitViewport(const oAABoxf& _Source, D3D11_VIEWPORT* _pViewport);

// Creats a viewport that uses the full render target (depth, [0,1])
void oD3D11InitViewport(const int2& _RenderTargetDimensions, D3D11_VIEWPORT* _pViewport);

// Create and set a single viewport that uses the entire render target.
void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11Texture2D* _pRenderTarget, float _MinDepth = 0.0f, float _MaxDepth = 1.0f);

// Extracts the resource from the view and calls the above 
// oD3D11SetFullTargetViewport.
void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11RenderTargetView* _pRenderTargetView, float _MinDepth = 0.0f, float _MaxDepth = 1.0f);

// Encapsulate any possible method of drawing in DX11 into one function that
// lists out everything one would need to specify. If NULL or 0 is specified 
// for various parameters, the proper flavor of draw will be used for drawing.
void oD3D11Draw(ID3D11DeviceContext* _pDeviceContext
	, uint _NumElements
	, uint _NumVertexBuffers
	, const ID3D11Buffer* const* _ppVertexBuffers
	, const UINT* _VertexStrides
	, uint _IndexOfFirstVertexToDraw
	, uint _OffsetToAddToEachVertexIndex
	, const ID3D11Buffer* _pIndexBuffer
	, uint _IndexOfFirstIndexToDraw = 0
	, uint _NumInstances = 0 // 0 means don't use instanced drawing
	, uint _IndexOfFirstInstanceIndexToDraw = 0);

// This returns the number of primitives drawn (more than the specified number
// of primitives if instancing is used). This does not set input layout.
inline void oD3D11Draw(ID3D11DeviceContext* _pDeviceContext
	, D3D11_PRIMITIVE_TOPOLOGY _PrimitiveTopology
	, uint _NumPrimitives
	, uint _NumVertexBuffers
	, const ID3D11Buffer* const* _ppVertexBuffers
	, const UINT* _VertexStrides
	, uint _IndexOfFirstVertexToDraw
	, uint _OffsetToAddToEachVertexIndex
	, const ID3D11Buffer* _pIndexBuffer
	, uint _IndexOfFirstIndexToDraw = 0
	, uint _NumInstances = 0 // 0 means don't use instanced drawing
	, uint _IndexOfFirstInstanceIndexToDraw = 0)
{
	_pDeviceContext->IASetPrimitiveTopology(_PrimitiveTopology);
	const uint nElements = oD3D11GetNumElements(_PrimitiveTopology, _NumPrimitives);
	oD3D11Draw(_pDeviceContext, nElements, _NumVertexBuffers, _ppVertexBuffers, _VertexStrides, _IndexOfFirstVertexToDraw, _OffsetToAddToEachVertexIndex, _pIndexBuffer, _IndexOfFirstIndexToDraw, _NumInstances, _IndexOfFirstInstanceIndexToDraw);
}

// A neat little trick when drawing quads, fullscreen or otherwise. Submits a 
// triangle strip with no vertices and optionally a null instance buffer. Use 
// oExtractQuadInfoFromVertexID() in oHLSL.h to reconstruct position and 
// texcoords using SV_VertexID and optionally SV_InstanceID.
// NOTE: This leaves primitive topology set to tristrips.
void oD3D11DrawSVQuad(ID3D11DeviceContext* _pDeviceContext, uint _NumInstances = 1);

// _____________________________________________________________________________
// Misc API (not yet categorized)

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
