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
#include "d3d11_util.h"
#include <oBase/assert.h>
#include <oBase/byte.h>
#include <oCore/windows/win_util.h>
#include "dxgi_util.h"
#include "d3d_compile.h"
#include "d3d_debug.h"
#include "d3d_resource.h"
#include <cerrno>

using namespace ouro::gpu::d3d;

typedef ouro::guid oGUID;
#define threadsafe volatile
const oGUID& oGetGUID(threadsafe const ID3D11Device* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11Device); }
const oGUID& oGetGUID(threadsafe const ID3D11DeviceContext* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11DeviceContext); }
const oGUID& oGetGUID(threadsafe const ID3D11RenderTargetView* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11RenderTargetView); }
const oGUID& oGetGUID(threadsafe const ID3D11Texture2D* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11Texture2D); }

namespace ouro {
	namespace gpu {
		namespace d3d11 {

int vtrace(ID3D11InfoQueue* _pInfoQueue, D3D11_MESSAGE_SEVERITY _Severity, const char* _Format, va_list _Args)
{
	xlstring buf;
	int len = vsnprintf(buf, _Format, _Args);
	if (len == -1)
		oTHROW0(no_buffer_space);
	_pInfoQueue->AddApplicationMessage(_Severity, buf);
	return len;
}

intrusive_ptr<ID3D11Device> make_device(const gpu::device_init& _Init)
{
	if (_Init.version < version(9,0))
		throw std::invalid_argument("must be D3D 9.0 or above");

	intrusive_ptr<IDXGIAdapter> Adapter;

	if (!_Init.use_software_emulation)
	{
		adapter::info adapter_info = adapter::find(_Init.virtual_desktop_position
			, _Init.min_driver_version, _Init.use_exact_driver_version);
		Adapter = dxgi::get_adapter(adapter_info.id);
	}

	unsigned int Flags = 0;
	bool UsingDebug = false;
	if (_Init.driver_debug_level != gpu::debug_level::none)
	{
		Flags |= D3D11_CREATE_DEVICE_DEBUG;
		UsingDebug = true;
	}

	if (!_Init.multithreaded)
		Flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;

	intrusive_ptr<ID3D11Device> Device;
	D3D_FEATURE_LEVEL FeatureLevel;
	HRESULT hr = D3D11CreateDevice(
		Adapter
		, _Init.use_software_emulation ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_UNKNOWN
		, 0
		, Flags
		, nullptr
		, 0
		, D3D11_SDK_VERSION
		, &Device
		, &FeatureLevel
		, nullptr);

	// http://stackoverflow.com/questions/10586956/what-can-cause-d3d11createdevice-to-fail-with-e-fail
	// It's possible that the debug lib isn't installed, so try again without debug.

	if (hr == 0x887a002d)
	{
		#if NTDDI_VERSION < NTDDI_WIN8
			oTRACE("The DirectX SDK must be installed for driver-level debugging.");
		#else
			oTRACE("Thie Windows SDK must be installed for driver-level debugging.");
		#endif

		Flags &=~ D3D11_CREATE_DEVICE_DEBUG;
		UsingDebug = false;
		hr = E_FAIL;
	}

	if (hr == E_FAIL)
	{
		oTRACE("The first-chance _com_error exception above is because there is no debug layer present during the creation of a D3D device, trying again without debug");

		Flags &=~ D3D11_CREATE_DEVICE_DEBUG;
		UsingDebug = false;

		oV(D3D11CreateDevice(
			Adapter
			, _Init.use_software_emulation ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_UNKNOWN
			, 0
			, Flags
			, nullptr
			, 0
			, D3D11_SDK_VERSION
			, &Device
			, &FeatureLevel
			, nullptr));

		oTRACE("Debug D3D11 not found: device created in non-debug mode so driver error reporting will not be available.");
	}
	else
		oV(hr);

	version D3DVersion = version((FeatureLevel>>12) & 0xffff, (FeatureLevel>>8) & 0xffff);
	if (D3DVersion < _Init.version)
	{
		sstring StrVer;
		oTHROW(not_supported, "Failed to create an ID3D11Device with a minimum feature set of D3D %s!", to_string(StrVer, _Init.version));
	}

	debug_name(Device, oSAFESTRN(_Init.debug_name));

	// Setup filter of superfluous warnings

	// ??SetShaderResources and ??SetConstantBuffers will trigger a warning if 
	// that buffer has a view bound as a target, such as SetUnorderedAccessViews.
	// The driver also then sets the targets to null. It is unsafe to assume too
	// much about the state of the device context, especially when considering 
	// submission of several device contexts that were constructed in parallel, so
	// allow for API to leave state dirty and assume the last-barrage of state
	// setting is the authoritative one, thus allow the driver to auto-clear this
	// specific stale state.
	static const D3D11_MESSAGE_CATEGORY sDisabledD3D11Categories[] = 
	{
		D3D11_MESSAGE_CATEGORY_STATE_CREATION,
	};
	
	static const D3D11_MESSAGE_ID sDisabledD3D11Messages[] = 
	{
		D3D11_MESSAGE_ID_DEVICE_VSSETSHADERRESOURCES_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_VSSETCONSTANTBUFFERS_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_HSSETSHADERRESOURCES_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_HSSETCONSTANTBUFFERS_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_DSSETSHADERRESOURCES_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_DSSETCONSTANTBUFFERS_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_GSSETSHADERRESOURCES_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_GSSETCONSTANTBUFFERS_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_PSSETSHADERRESOURCES_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_PSSETCONSTANTBUFFERS_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_CSSETSHADERRESOURCES_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_CSSETCONSTANTBUFFERS_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_CSSETUNORDEREDACCESSVIEWS_HAZARD,
		D3D11_MESSAGE_ID_DEVICE_OMSETRENDERTARGETSANDUNORDEREDACCESSVIEWS_HAZARD,

		// Create/Destroy flow control objects
		D3D11_MESSAGE_ID_CREATE_CONTEXT,
		D3D11_MESSAGE_ID_DESTROY_CONTEXT,
		D3D11_MESSAGE_ID_CREATE_QUERY,
		D3D11_MESSAGE_ID_DESTROY_QUERY,
		D3D11_MESSAGE_ID_CREATE_PREDICATE,
		D3D11_MESSAGE_ID_DESTROY_PREDICATE,
		D3D11_MESSAGE_ID_CREATE_COUNTER,
		D3D11_MESSAGE_ID_DESTROY_COUNTER,
		D3D11_MESSAGE_ID_CREATE_COMMANDLIST,
		D3D11_MESSAGE_ID_DESTROY_COMMANDLIST,
		D3D11_MESSAGE_ID_CREATE_CLASSINSTANCE,
		D3D11_MESSAGE_ID_DESTROY_CLASSINSTANCE,
		D3D11_MESSAGE_ID_CREATE_CLASSLINKAGE,
		D3D11_MESSAGE_ID_DESTROY_CLASSLINKAGE,

		// Create/Destroy buffers
		D3D11_MESSAGE_ID_CREATE_BUFFER,
		D3D11_MESSAGE_ID_DESTROY_BUFFER,
		D3D11_MESSAGE_ID_CREATE_TEXTURE1D,
		D3D11_MESSAGE_ID_DESTROY_TEXTURE1D,
		D3D11_MESSAGE_ID_CREATE_TEXTURE2D,
		D3D11_MESSAGE_ID_DESTROY_TEXTURE2D,
		D3D11_MESSAGE_ID_CREATE_TEXTURE3D,
		D3D11_MESSAGE_ID_DESTROY_TEXTURE3D,

		// Create/Destroy views
		D3D11_MESSAGE_ID_CREATE_SHADERRESOURCEVIEW,
		D3D11_MESSAGE_ID_DESTROY_SHADERRESOURCEVIEW,
		D3D11_MESSAGE_ID_CREATE_RENDERTARGETVIEW,
		D3D11_MESSAGE_ID_DESTROY_RENDERTARGETVIEW,
		D3D11_MESSAGE_ID_CREATE_DEPTHSTENCILVIEW,
		D3D11_MESSAGE_ID_DESTROY_DEPTHSTENCILVIEW,
		D3D11_MESSAGE_ID_CREATE_UNORDEREDACCESSVIEW,
		D3D11_MESSAGE_ID_DESTROY_UNORDEREDACCESSVIEW,

		// Create/Destroy shaders
		D3D11_MESSAGE_ID_CREATE_INPUTLAYOUT,
		D3D11_MESSAGE_ID_DESTROY_INPUTLAYOUT,
		D3D11_MESSAGE_ID_CREATE_VERTEXSHADER,
		D3D11_MESSAGE_ID_DESTROY_VERTEXSHADER,
		D3D11_MESSAGE_ID_CREATE_HULLSHADER,
		D3D11_MESSAGE_ID_DESTROY_HULLSHADER,
		D3D11_MESSAGE_ID_CREATE_DOMAINSHADER,
		D3D11_MESSAGE_ID_DESTROY_DOMAINSHADER,
		D3D11_MESSAGE_ID_CREATE_GEOMETRYSHADER,
		D3D11_MESSAGE_ID_DESTROY_GEOMETRYSHADER,
		D3D11_MESSAGE_ID_CREATE_PIXELSHADER,
		D3D11_MESSAGE_ID_DESTROY_PIXELSHADER,
		D3D11_MESSAGE_ID_CREATE_COMPUTESHADER,
		D3D11_MESSAGE_ID_DESTROY_COMPUTESHADER,

		// Create/Destroy states
		D3D11_MESSAGE_ID_CREATE_SAMPLER,
		D3D11_MESSAGE_ID_DESTROY_SAMPLER,
		D3D11_MESSAGE_ID_CREATE_BLENDSTATE,
		D3D11_MESSAGE_ID_DESTROY_BLENDSTATE,
		D3D11_MESSAGE_ID_CREATE_DEPTHSTENCILSTATE,
		D3D11_MESSAGE_ID_DESTROY_DEPTHSTENCILSTATE,
		D3D11_MESSAGE_ID_CREATE_RASTERIZERSTATE,
		D3D11_MESSAGE_ID_DESTROY_RASTERIZERSTATE,
	};

	if (UsingDebug && _Init.driver_debug_level == gpu::debug_level::normal)
	{
		intrusive_ptr<ID3D11InfoQueue> IQ;
		oV(Device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&IQ));
		D3D11_INFO_QUEUE_FILTER filter = {0};
		filter.DenyList.NumCategories = oCOUNTOF(sDisabledD3D11Categories);
		filter.DenyList.pCategoryList = const_cast<D3D11_MESSAGE_CATEGORY*>(sDisabledD3D11Categories);
		filter.DenyList.NumIDs = oCOUNTOF(sDisabledD3D11Messages);
		filter.DenyList.pIDList = const_cast<D3D11_MESSAGE_ID*>(sDisabledD3D11Messages);
		IQ->PushStorageFilter(&filter);
	}

	return Device;
}

gpu::device_info get_info(ID3D11Device* _pDevice, bool _IsSoftwareEmulation)
{
	gpu::device_info d;
	debug_name(d.debug_name, _pDevice);

	intrusive_ptr<IDXGIAdapter> Adapter;
	{
		intrusive_ptr<IDXGIDevice> DXGIDevice;
		oV(_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&DXGIDevice));
		DXGIDevice->GetAdapter(&Adapter);
	}

	DXGI_ADAPTER_DESC ad;
	Adapter->GetDesc(&ad);
	adapter::info adapter_info = dxgi::get_info(Adapter);

	d.device_description = ad.Description;
	d.driver_description = adapter_info.description;
	d.native_memory = ad.DedicatedVideoMemory;
	d.dedicated_system_memory = ad.DedicatedSystemMemory;
	d.shared_system_memory = ad.SharedSystemMemory;
	d.driver_version = adapter_info.version;
	d.feature_version = adapter_info.feature_level;
	d.adapter_index = *(int*)&adapter_info.id;
	d.api = gpu_api::d3d11;
	d.vendor = adapter_info.vendor;
	d.is_software_emulation = _IsSoftwareEmulation;
	d.debug_reporting_enabled = !!(_pDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_DEBUG);
	return d;
}

D3D11_PRIMITIVE_TOPOLOGY from_primitive_type(const mesh::primitive_type::value& _Type)
{
	return D3D11_PRIMITIVE_TOPOLOGY(_Type);
}

mesh::primitive_type::value to_primitive_type(D3D11_PRIMITIVE_TOPOLOGY _Type)
{
	return mesh::primitive_type::value(_Type);
}

unsigned int num_elements(D3D_PRIMITIVE_TOPOLOGY _PrimitiveTopology, unsigned int _NumPrimitives)
{
	switch (_PrimitiveTopology)
	{
		case D3D_PRIMITIVE_TOPOLOGY_POINTLIST: return _NumPrimitives;
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST: return _NumPrimitives * 2;
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP: return _NumPrimitives + 1;
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST: return _NumPrimitives * 3;
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return _NumPrimitives + 2;
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ: return _NumPrimitives * 2 * 2;
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ: return (_NumPrimitives + 1) * 2;
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ: return _NumPrimitives * 3 * 2;
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ: return (_NumPrimitives + 2) * 2;
		case D3D_PRIMITIVE_TOPOLOGY_UNDEFINED: return 0;
		default: break;
	}
	return _NumPrimitives * (_PrimitiveTopology-D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST+1);
}

D3D_FEATURE_LEVEL feature_level(const version& _ShaderModel)
{
	D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_9_1;
	if (_ShaderModel == version(3,0)) level = D3D_FEATURE_LEVEL_9_3;
	else if (_ShaderModel == version(4,0)) level = D3D_FEATURE_LEVEL_10_0;
	else if (_ShaderModel == version(4,1)) level = D3D_FEATURE_LEVEL_10_1;
	else if (_ShaderModel == version(5,0)) level = D3D_FEATURE_LEVEL_11_0;
	return level;
}

bool supports_deferred_contexts(ID3D11Device* _pDevice)
{
	D3D11_FEATURE_DATA_THREADING threadingCaps = { FALSE, FALSE };
	oV(_pDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingCaps, sizeof(threadingCaps)));
	return !!threadingCaps.DriverCommandLists;
}

void set_constant_buffers(ID3D11DeviceContext* _pDeviceContext
	, unsigned int _StartSlot
	, unsigned int _NumBuffers
	, const ID3D11Buffer* const* _ppConstantBuffers)
{
	ID3D11Buffer* const* ppConstantBuffers = const_cast<ID3D11Buffer* const*>(_ppConstantBuffers);
	_pDeviceContext->VSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
	_pDeviceContext->HSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
	_pDeviceContext->DSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
	_pDeviceContext->GSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
	_pDeviceContext->PSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
	_pDeviceContext->CSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
}

void set_samplers(ID3D11DeviceContext* _pDeviceContext
	, unsigned int _StartSlot
	, unsigned int _NumSamplers
	, const ID3D11SamplerState* const* _ppSamplers)
{
	ID3D11SamplerState* const* ppSamplers = const_cast<ID3D11SamplerState* const*>(_ppSamplers);
	_pDeviceContext->VSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
	_pDeviceContext->HSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
	_pDeviceContext->DSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
	_pDeviceContext->GSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
	_pDeviceContext->PSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
	_pDeviceContext->CSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
}

#if 0
void set_srvs(ID3D11DeviceContext* _pDeviceContext
	, unsigned int _StartSlot
	, unsigned int _NumShaderResourceViews
	, const ID3D11ShaderResourceView* const* _ppViews)
{
	ID3D11ShaderResourceView* const* ppViews = const_cast<ID3D11ShaderResourceView* const*>(_ppViews);
	bool OneUnorderedBufferFound = false;
	for (unsigned int i = 0; i < _NumShaderResourceViews; i++)
	{
		if (_ppViews[i])
		{
			intrusive_ptr<ID3D11Resource> Resource;
			const_cast<ID3D11ShaderResourceView*>(_ppViews[i])->GetResource(&Resource);
			gpu::texture1_info i = get_texture_info1(Resource);
			if (gpu::is_unordered(i.type))
			{
				OneUnorderedBufferFound = true;
				break;
			}
		}
	}

	if (!OneUnorderedBufferFound)
	{
		_pDeviceContext->VSSetShaderResources(_StartSlot, _NumShaderResourceViews, ppViews);
		_pDeviceContext->HSSetShaderResources(_StartSlot, _NumShaderResourceViews, ppViews);
		_pDeviceContext->DSSetShaderResources(_StartSlot, _NumShaderResourceViews, ppViews);
		_pDeviceContext->GSSetShaderResources(_StartSlot, _NumShaderResourceViews, ppViews);
	}

	_pDeviceContext->PSSetShaderResources(_StartSlot, _NumShaderResourceViews, ppViews);
	_pDeviceContext->CSSetShaderResources(_StartSlot, _NumShaderResourceViews, ppViews);
}
#endif

aaboxf from_viewport(const D3D11_VIEWPORT& _Viewport)
{
	return aaboxf(aaboxf::min_max, float3(_Viewport.TopLeftX, _Viewport.TopLeftY, _Viewport.MinDepth), float3(_Viewport.Width, _Viewport.Height, _Viewport.MaxDepth));
}

D3D11_VIEWPORT to_viewport(const aaboxf& _Source)
{
	D3D11_VIEWPORT v;
	float3 Min = _Source.Min;
	float3 Max = _Source.Max;
	float3 Size = _Source.size();
	v.TopLeftX = Min.x;
	v.TopLeftY = Min.y;
	v.MinDepth = Min.z;
	v.Width = Size.x;
	v.Height = Size.y;
	v.MaxDepth = Max.z;
	return v;
}

D3D11_VIEWPORT to_viewport(const int2& _RenderTargetDimensions)
{
	D3D11_VIEWPORT v;
	v.TopLeftX = 0.0f;
	v.TopLeftY = 0.0f;
	v.Width = static_cast<float>(_RenderTargetDimensions.x);
	v.Height = static_cast<float>(_RenderTargetDimensions.y);
	v.MinDepth = 0.0f;
	v.MaxDepth = 1.0f;
	return v;
}

void check_bound_rts_and_uavs(ID3D11DeviceContext* _pDeviceContext, int _NumBuffers, ID3D11Buffer** _ppBuffers)
{
	intrusive_ptr<ID3D11UnorderedAccessView> UAVs[D3D11_PS_CS_UAV_REGISTER_COUNT];
	_pDeviceContext->OMGetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, D3D11_PS_CS_UAV_REGISTER_COUNT, (ID3D11UnorderedAccessView**)UAVs);
	for (int r = 0; r < D3D11_PS_CS_UAV_REGISTER_COUNT; r++)
	{
		if (UAVs[r])
		{
			for (int b = 0; b < _NumBuffers; b++)
			{
				intrusive_ptr<ID3D11Resource> Resource;
				UAVs[r]->GetResource(&Resource);
				if (Resource && Resource == _ppBuffers[b])
					trace(_pDeviceContext, D3D11_MESSAGE_SEVERITY_ERROR, "The specified buffer in slot %d is bound using OMSetRenderTargetsAndUnorderedAccessViews slot %d. Behavior will be unexpected since the buffer may not be flushed for reading.", b, r);
			}
		}
	}
}

void check_bound_cs_uavs(ID3D11DeviceContext* _pDeviceContext, int _NumBuffers
	, ID3D11Buffer** _ppBuffers)
{
	intrusive_ptr<ID3D11UnorderedAccessView> UAVs[D3D11_PS_CS_UAV_REGISTER_COUNT];
	_pDeviceContext->CSGetUnorderedAccessViews(0, D3D11_PS_CS_UAV_REGISTER_COUNT, (ID3D11UnorderedAccessView**)UAVs);
	for (int r = 0; r < D3D11_PS_CS_UAV_REGISTER_COUNT; r++)
	{
		if (UAVs[r])
		{
			for (int b = 0; b < _NumBuffers; b++)
			{
				intrusive_ptr<ID3D11Resource> Resource;
				UAVs[r]->GetResource(&Resource);
				if (Resource && Resource == _ppBuffers[b])
					trace(_pDeviceContext, D3D11_MESSAGE_SEVERITY_ERROR, "The specified buffer in slot %d is bound to CSSetUnorderedAccessViews slot %d. Behavior will be unexpected because the buffer may be bound for reading and writing at the same time.", b, r);
			}
		}
	}
}

#if 0

// {6489B24E-C12E-40C2-A9EF-249353888612}
static const GUID oWKPDID_oBackPointer = { 0x6489b24e, 0xc12e, 0x40c2, { 0xa9, 0xef, 0x24, 0x93, 0x53, 0x88, 0x86, 0x12 } };

// to oAlgorithm... should we permutate for various sources?
#ifdef _DEBUG
	#define oDEBUG_CHECK_SAME_DEVICE(_pContext, _pSrc, _pDst) do \
	{	intrusive_ptr<ID3D11Device> Dev1, Dev2, Dev3; \
		_pContext->GetDevice(&Dev1); _pSrc->GetDevice(&Dev2); _pDst->GetDevice(&Dev2); \
		oASSERT(Dev1 == Dev2 && Dev2 == Dev3, "Context and resources are not from the same device"); \
	} while (false)
#else
	 #define oDEBUG_CHECK_SAME_DEVICE(_pContext, _pSrc, _pDst)
#endif

void oD3D11MapWriteDiscard(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pResource, unsigned int _Subresource, surface::mapped_subresource* _pMappedResource)
{
	D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
	texture1_info d;
	oD3D11GetTextureDesc(_pResource, &d, &Usage);

	switch (Usage)
	{
		case D3D11_USAGE_DEFAULT:
		{
			int2 ByteDimensions = surface::byte_dimensions(d.Format, d.Dimensions);
			_pMappedResource->depth_pitch = ByteDimensions.x * ByteDimensions.y;
			_pMappedResource->row_pitch = ByteDimensions.x;
			_pMappedResource->data = new char[_pMappedResource->depth_pitch];
			break;
		}
		
		case D3D11_USAGE_STAGING:
		case D3D11_USAGE_DYNAMIC:
			_pDeviceContext->Map(_pResource, _Subresource, D3D11_MAP_WRITE_DISCARD, 0, (D3D11_MAPPED_SUBRESOURCE*)_pMappedResource);
			break;

		oNODEFAULT;
	}
}

void oD3D11Unmap(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pResource, unsigned int _Subresource, surface::mapped_subresource& _MappedResource)
{
	D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
	texture1_info d;
	oD3D11GetTextureDesc(_pResource, &d, &Usage);
	switch (Usage)
	{
		case D3D11_USAGE_DEFAULT:
		{
			delete [] _MappedResource.data;
			break;
		}

		case D3D11_USAGE_STAGING:
		case D3D11_USAGE_DYNAMIC:
			_pDeviceContext->Unmap(_pResource, _Subresource);
			break;

		oNODEFAULT;
	}
}

template<typename T> void oD3D11UpdateIndexBuffer(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pIndexBuffer, const T* _pSourceIndices)
{
	#ifdef _DEBUG
	{
		D3D11_BUFFER_DESC d;
		_pIndexBuffer->GetDesc(&d);
		oASSERT((d.BindFlags & D3D11_BIND_INDEX_BUFFER) == D3D11_BIND_INDEX_BUFFER, "This only works for index buffers");
	}
	#endif

	oGPU_BUFFER_DESC d;
	oVERIFY(oD3D11BufferGetDesc(_pIndexBuffer, &d));

	surface::mapped_subresource msr;
	oD3D11MapWriteDiscard(_pDeviceContext, _pIndexBuffer, 0, &msr);

	if (d.struct_byte_size == sizeof(T))
		memcpy(msr.data, _pSourceIndices, d.ArraySize * d.struct_byte_size);

	if (d.struct_byte_size == 2)
	{
		oASSERT(sizeof(T) == 4, "");
		memcpyuitous((ushort*)msr.data, (const unsigned int*)_pSourceIndices, d.ArraySize);
	}

	else
	{
		oASSERT(sizeof(T) == 2 && d.StructByteSize == 4, "");
		memcpyustoui((unsigned int*)msr.data, (const ushort*)_pSourceIndices, d.ArraySize);
	}

	oD3D11Unmap(_pDeviceContext, _pIndexBuffer, 0, msr);
}

void oD3D11UpdateIndexBuffer(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pIndexBuffer, const unsigned int* _pSourceIndices)
{
	oD3D11UpdateIndexBuffer<unsigned int>(_pDeviceContext, _pIndexBuffer, _pSourceIndices);
}

void oD3D11UpdateIndexBuffer(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pIndexBuffer, const ushort* _pSourceIndices)
{
	oD3D11UpdateIndexBuffer<ushort>(_pDeviceContext, _pIndexBuffer, _pSourceIndices);
}


void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11Texture2D* _pRenderTargetResource, float _MinDepth, float _MaxDepth)
{
	D3D11_TEXTURE2D_DESC desc;
	_pRenderTargetResource->GetDesc(&desc);

	D3D11_VIEWPORT v;
	oD3D11ToViewport(int2(desc.Width, desc.Height), &v);
	_pDeviceContext->RSSetViewports(1, &v);
}

void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11RenderTargetView* _pRenderTargetView, float _MinDepth, float _MaxDepth)
{
	intrusive_ptr<ID3D11Texture2D> t;
	_pRenderTargetView->GetResource((ID3D11Resource**)&t);
	#ifdef _DEBUG
		D3D11_RESOURCE_DIMENSION type;
		t->GetType(&type);
		oASSERT(type == D3D11_RESOURCE_DIMENSION_TEXTURE2D, "Unexpected resource type");
	#endif
	oD3D11SetFullTargetViewport(_pDeviceContext, t, _MinDepth, _MaxDepth);
}

void oD3D11DrawSVQuad(ID3D11DeviceContext* _pDeviceContext, unsigned int _NumInstances)
{
	ID3D11Buffer* pVertexBuffers[] = { nullptr, nullptr };
	unsigned int pStrides[] = { 0, 0 };
	unsigned int pOffsets[] = { 0, 0 };
	_pDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffers, pStrides, pOffsets);
	_pDeviceContext->DrawInstanced(4, _NumInstances, 0, 0);
}

oD3D11ScopedMessageDisabler::oD3D11ScopedMessageDisabler(ID3D11DeviceContext* _pDeviceContext, const D3D11_MESSAGE_ID* _pMessageIDs, size_t _NumMessageIDs)
{
	#ifdef _DEBUG
		intrusive_ptr<ID3D11Device> Device;
		_pDeviceContext->GetDevice(&Device);
		oV(Device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&pInfoQueue));
		D3D11_INFO_QUEUE_FILTER filter = {0};
		filter.DenyList.NumIDs = (unsigned int)_NumMessageIDs;
		filter.DenyList.pIDList = const_cast<D3D11_MESSAGE_ID*>(_pMessageIDs);
		pInfoQueue->PushStorageFilter(&filter);
	#endif
}

oD3D11ScopedMessageDisabler::oD3D11ScopedMessageDisabler(ID3D11Device* _pDevice, const D3D11_MESSAGE_ID* _pMessageIDs, size_t _NumMessageIDs)
{
	#ifdef _DEBUG
		oV(_pDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&pInfoQueue));
		D3D11_INFO_QUEUE_FILTER filter = {0};
		filter.DenyList.NumIDs = (unsigned int)_NumMessageIDs;
		filter.DenyList.pIDList = const_cast<D3D11_MESSAGE_ID*>(_pMessageIDs);
		pInfoQueue->PushStorageFilter(&filter);
	#endif
}

oD3D11ScopedMessageDisabler::~oD3D11ScopedMessageDisabler()
{
	#ifdef _DEBUG
		pInfoQueue->PopStorageFilter();
		pInfoQueue->Release();
	#endif
}

#endif

		} // namespace d3d11
	} // namespace gpu
} // namespace ouro
