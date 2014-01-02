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
#include <cerrno>

#define oCHECK_IS_TEXTURE(_pResource) do \
{	D3D11_RESOURCE_DIMENSION type; \
	_pResource->GetType(&type); \
	if (type != D3D11_RESOURCE_DIMENSION_TEXTURE1D && type != D3D11_RESOURCE_DIMENSION_TEXTURE2D && type != D3D11_RESOURCE_DIMENSION_TEXTURE3D) \
	{	mstring buf; \
		oTHROW_INVARG("Only textures types are currently supported. (resource %s)", debug_name(buf, _pResource)); \
	} \
} while (false)

typedef ouro::guid oGUID;
#define threadsafe volatile
const oGUID& oGetGUID(threadsafe const ID3D11Device* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11Device); }
const oGUID& oGetGUID(threadsafe const ID3D11DeviceContext* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11DeviceContext); }
const oGUID& oGetGUID(threadsafe const ID3D11RenderTargetView* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11RenderTargetView); }
const oGUID& oGetGUID(threadsafe const ID3D11Texture2D* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11Texture2D); }

namespace ouro {

const char* as_string(const D3D11_BIND_FLAG& _Flag)
{
	switch (_Flag)
	{
		case D3D11_BIND_VERTEX_BUFFER: return "D3D11_BIND_VERTEX_BUFFER";
		case D3D11_BIND_INDEX_BUFFER: return "D3D11_BIND_INDEX_BUFFER";
		case D3D11_BIND_CONSTANT_BUFFER: return "D3D11_BIND_CONSTANT_BUFFER";
		case D3D11_BIND_SHADER_RESOURCE: return "D3D11_BIND_SHADER_RESOURCE";
		case D3D11_BIND_STREAM_OUTPUT: return "D3D11_BIND_STREAM_OUTPUT";
		case D3D11_BIND_RENDER_TARGET: return "D3D11_BIND_RENDER_TARGET";
		case D3D11_BIND_DEPTH_STENCIL: return "D3D11_BIND_DEPTH_STENCIL";
		case D3D11_BIND_UNORDERED_ACCESS: return "D3D11_BIND_UNORDERED_ACCESS";
		//case D3D11_BIND_DECODER: return "D3D11_BIND_DECODER";
		//case D3D11_BIND_VIDEO_ENCODER: return "D3D11_BIND_VIDEO_ENCODER";
		default: break;
	}
	return "?";
}

const char* as_string(const D3D11_CPU_ACCESS_FLAG& _Flag)
{
	switch (_Flag)
	{
		case D3D11_CPU_ACCESS_WRITE: return "D3D11_CPU_ACCESS_WRITE";
		case D3D11_CPU_ACCESS_READ: return "D3D11_CPU_ACCESS_READ";
		default: break;
	}
	return "?";
}

const char* as_string(const D3D11_RESOURCE_MISC_FLAG& _Flag)
{
	switch (_Flag)
	{
		case D3D11_RESOURCE_MISC_GENERATE_MIPS: return "D3D11_RESOURCE_MISC_GENERATE_MIPS";
		case D3D11_RESOURCE_MISC_SHARED: return "D3D11_RESOURCE_MISC_SHARED";
		case D3D11_RESOURCE_MISC_TEXTURECUBE: return "D3D11_RESOURCE_MISC_TEXTURECUBE";
		case D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS: return "D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS";
		case D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS: return "D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS";
		case D3D11_RESOURCE_MISC_BUFFER_STRUCTURED: return "D3D11_RESOURCE_MISC_BUFFER_STRUCTURED";
		case D3D11_RESOURCE_MISC_RESOURCE_CLAMP: return "D3D11_RESOURCE_MISC_RESOURCE_CLAMP";
		case D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX: return "D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX";
		case D3D11_RESOURCE_MISC_GDI_COMPATIBLE: return "D3D11_RESOURCE_MISC_GDI_COMPATIBLE";
		//case D3D11_RESOURCE_MISC_SHARED_NTHANDLE: return "D3D11_RESOURCE_MISC_SHARED_NTHANDLE";
		//case D3D11_RESOURCE_MISC_RESTRICTED_CONTENT: return "D3D11_RESOURCE_MISC_RESTRICTED_CONTENT";
		//case D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE: return "D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE";
		//case D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER: return "D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER";
		default: break;
	}
	return "?";
}

const char* as_string(const D3D11_RESOURCE_DIMENSION& _Type)
{
	switch (_Type)
	{
		case D3D11_RESOURCE_DIMENSION_UNKNOWN: return "D3D11_RESOURCE_DIMENSION_UNKNOWN	=";
		case D3D11_RESOURCE_DIMENSION_BUFFER: return "D3D11_RESOURCE_DIMENSION_BUFFER	=";
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D: return "D3D11_RESOURCE_DIMENSION_TEXTURE1D	=";
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D: return "D3D11_RESOURCE_DIMENSION_TEXTURE2D	=";
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D: return "D3D11_RESOURCE_DIMENSION_TEXTURE3D	=";
		default: break;
	}
	return "?";
}

const char* as_string(const D3D11_UAV_DIMENSION& _Type)
{
	switch (_Type)
	{
		case D3D11_UAV_DIMENSION_UNKNOWN: return "D3D11_UAV_DIMENSION_UNKNOWN";
		case D3D11_UAV_DIMENSION_BUFFER: return "D3D11_UAV_DIMENSION_BUFFER";
		case D3D11_UAV_DIMENSION_TEXTURE1D: return "D3D11_UAV_DIMENSION_TEXTURE1D";
		case D3D11_UAV_DIMENSION_TEXTURE1DARRAY: return "D3D11_UAV_DIMENSION_TEXTURE1DARRAY";
		case D3D11_UAV_DIMENSION_TEXTURE2D: return "D3D11_UAV_DIMENSION_TEXTURE2D";
		case D3D11_UAV_DIMENSION_TEXTURE2DARRAY: return "D3D11_UAV_DIMENSION_TEXTURE2DARRAY";
		case D3D11_UAV_DIMENSION_TEXTURE3D: return "D3D11_UAV_DIMENSION_TEXTURE3D";
		default: break;
	}
	return "?";
}

const char* as_string(const D3D11_USAGE& _Usage)
{
	switch (_Usage)
	{
		case D3D11_USAGE_DEFAULT: return "D3D11_USAGE_DEFAULT";
		case D3D11_USAGE_IMMUTABLE: return "D3D11_USAGE_IMMUTABLE";
		case D3D11_USAGE_DYNAMIC: return "D3D11_USAGE_DYNAMIC";
		case D3D11_USAGE_STAGING: return "D3D11_USAGE_STAGING";
		default: break;
	}
	return "?";
}

	namespace d3d11 {

static bool muting_infos_or_state_creation(ID3D11Device* _pDevice)
{
	intrusive_ptr<ID3D11InfoQueue> InfoQueue;
	oV(_pDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&InfoQueue));
	SIZE_T size = 0;
	oV(InfoQueue->GetStorageFilter(nullptr, &size));
	D3D11_INFO_QUEUE_FILTER* f = (D3D11_INFO_QUEUE_FILTER*)alloca(size);
	oV(InfoQueue->GetStorageFilter(f, &size));
	for (unsigned int i = 0 ; i < f->DenyList.NumSeverities; i++)
		if (f->DenyList.pSeverityList[i] == D3D11_MESSAGE_SEVERITY_INFO)
			return true;
	for (unsigned int i = 0 ; i < f->DenyList.NumCategories; i++)
		if (f->DenyList.pCategoryList[i] == D3D11_MESSAGE_CATEGORY_STATE_CREATION)
			return true;
	return false;
}

static void trace_debug_name(ID3D11Device* _pDevice, const char* _Name)
{
	if ((_pDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_DEBUG) && !muting_infos_or_state_creation(_pDevice))
		oTRACEA("d3d11: INFO: Name Buffer: Name=\"%s\", Addr=0x%p, ExtRef=1, IntRef=0 [ STATE_CREATION INFO #OURO: NAME_BUFFER ]", _Name, _pDevice);
}

void debug_name(ID3D11Device* _pDevice, const char* _Name)
{
	unsigned int CreationFlags = _pDevice->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		oV(_pDevice->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<unsigned int>(strlen(_Name) + 1), _Name));
	trace_debug_name(_pDevice, _Name);
}

char* debug_name(char* _StrDestination, size_t _SizeofStrDestination, const ID3D11Device* _pDevice)
{
	unsigned int size = as_uint(_SizeofStrDestination);
	unsigned int CreationFlags = const_cast<ID3D11Device*>(_pDevice)->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		oV(const_cast<ID3D11Device*>(_pDevice)->GetPrivateData(WKPDID_D3DDebugObjectName, &size, _StrDestination));
	else if (strlcpy(_StrDestination, "non-debug device", _SizeofStrDestination) >= _SizeofStrDestination)
		oTHROW0(no_buffer_space);
	return _StrDestination;
}

void debug_name(ID3D11DeviceChild* _pDeviceChild, const char* _Name)
{
	intrusive_ptr<ID3D11Device> Device;
	_pDeviceChild->GetDevice(&Device);
	unsigned int CreationFlags = Device->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		oV(_pDeviceChild->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<unsigned int>(strlen(_Name) + 1), _Name));
	trace_debug_name(Device, _Name);
}

char* debug_name(char* _StrDestination, size_t _SizeofStrDestination, const ID3D11DeviceChild* _pDeviceChild)
{
	unsigned int size = as_uint(_SizeofStrDestination);
	intrusive_ptr<ID3D11Device> Device;
	const_cast<ID3D11DeviceChild*>(_pDeviceChild)->GetDevice(&Device);
	unsigned int CreationFlags = Device->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		oV(const_cast<ID3D11DeviceChild*>(_pDeviceChild)->GetPrivateData(WKPDID_D3DDebugObjectName, &size, _StrDestination));
	else if (strlcpy(_StrDestination, "non-debug device child", _SizeofStrDestination) >= _SizeofStrDestination)
		oTHROW0(no_buffer_space);
	return _StrDestination;
}

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
		#if NTDDI_VERSION >= NTDDI_WIN8
		oTRACE("Win8 SDK is very particular - or I may have to D/L the Win 8.1 dev sdk");

		#else
			Flags |= D3D11_CREATE_DEVICE_DEBUG;
			UsingDebug = true;
		#endif
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
	// It's possible that the debug lib isn't installed, so try again without 
	// debug.
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
	d.api = gpu::api::d3d11;
	d.vendor = adapter_info.vendor;
	d.is_software_emulation = _IsSoftwareEmulation;
	d.debug_reporting_enabled = !!(_pDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_DEBUG);
	return d;
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

const char* shader_profile(D3D_FEATURE_LEVEL _Level, gpu::pipeline_stage::value _Stage)
{
	static const char* sDX9Profiles[] = { "vs_3_0", nullptr, nullptr, nullptr, "ps_3_0", nullptr, };
	static const char* sDX10Profiles[] = { "vs_4_0", nullptr, nullptr, "gs_4_0", "ps_4_0", nullptr, };
	static const char* sDX10_1Profiles[] = { "vs_4_1", nullptr, nullptr, "gs_4_1", "ps_4_1", nullptr, };
	static const char* sDX11Profiles[] = { "vs_5_0", "hs_5_0", "ds_5_0", "gs_5_0", "ps_5_0", "cs_5_0", };

	const char** profiles = 0;
	switch (_Level)
	{
		case D3D_FEATURE_LEVEL_9_1: case D3D_FEATURE_LEVEL_9_2: case D3D_FEATURE_LEVEL_9_3: profiles = sDX9Profiles; break;
		case D3D_FEATURE_LEVEL_10_0: profiles = sDX10Profiles; break;
		case D3D_FEATURE_LEVEL_10_1: profiles = sDX10_1Profiles; break;
		case D3D_FEATURE_LEVEL_11_0: profiles = sDX11Profiles; break;
		oNODEFAULT;
	}

	const char* profile = profiles[_Stage];
	if (!profile)
	{
		version ver = version((_Level>>12) & 0xffff, (_Level>>8) & 0xffff);
		sstring StrVer;
		oTHROW(not_supported, "Shader profile does not exist for D3D%s's stage %s", to_string2(StrVer, ver), as_string(_Stage));
	}

	return profile;
}

size_t byte_code_size(const void* _pByteCode)
{
	// Discovered empirically
	return _pByteCode ? ((const unsigned int*)_pByteCode)[6] : 0;
}

// {13BA565C-4766-49C4-8C1C-C1F459F00A65}
static const GUID oWKPDID_oGPU_BUFFER_INFO = { 0x13ba565c, 0x4766, 0x49c4, { 0x8c, 0x1c, 0xc1, 0xf4, 0x59, 0xf0, 0xa, 0x65 } };

void set_info(ID3D11Resource* _pBuffer, const gpu::buffer_info& _Desc)
{
	oV(_pBuffer->SetPrivateData(oWKPDID_oGPU_BUFFER_INFO, sizeof(_Desc), &_Desc));
}

gpu::buffer_info get_info(const ID3D11Resource* _pBuffer)
{
	unsigned int size = sizeof(gpu::buffer_info);
	gpu::buffer_info i;
	oV(const_cast<ID3D11Resource*>(_pBuffer)->GetPrivateData(oWKPDID_oGPU_BUFFER_INFO, &size, &i));
	return i;
}

static unsigned int cpu_write_flags(D3D11_USAGE _Usage)
{
	switch (_Usage)
	{
		case D3D11_USAGE_DYNAMIC: return D3D11_CPU_ACCESS_WRITE;
		case D3D11_USAGE_STAGING: return D3D11_CPU_ACCESS_WRITE|D3D11_CPU_ACCESS_READ;
		default: return 0;
	}
}

intrusive_ptr<ID3D11Buffer> make_buffer(ID3D11Device* _pDevice
	, const char* _DebugName
	, const gpu::buffer_info& _Info
	, const void* _pInitBuffer
	, ID3D11UnorderedAccessView** _ppUAV
	, ID3D11ShaderResourceView** _ppSRV)
{
	D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
	unsigned int BindFlags = 0;
	surface::format Format = _Info.format;

	switch (_Info.type)
	{
		case gpu::buffer_type::constant:
			BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			if (!byte_aligned(_Info.struct_byte_size, 16) || _Info.struct_byte_size > 65535)
				throw std::invalid_argument(formatf("A constant buffer must specify a struct_byte_size that is 16-byte-aligned and <= 65536 bytes. (size %u bytes specified)", _Info.struct_byte_size));
			break;
		case gpu::buffer_type::readback:
			Usage = D3D11_USAGE_STAGING;
			break;
		case gpu::buffer_type::index:
			if (_Info.format != surface::r16_uint && _Info.format != surface::r32_uint)
				throw std::invalid_argument(formatf("An index buffer must specify a format of r16_uint or r32_uint only (%s specified).", as_string(_Info.format)));
			if (_Info.struct_byte_size != invalid && _Info.struct_byte_size != static_cast<unsigned int>(surface::element_size(_Info.format)))
				throw std::invalid_argument("An index buffer must specify struct_byte_size properly, or set it to 0.");
			BindFlags = D3D11_BIND_INDEX_BUFFER;
			break;
		case gpu::buffer_type::index_readback:
			Usage = D3D11_USAGE_STAGING;
			break;
		case gpu::buffer_type::vertex:
			BindFlags = D3D11_BIND_VERTEX_BUFFER;
			break;
		case gpu::buffer_type::vertex_readback:
			Usage = D3D11_USAGE_STAGING;
			break;
		case gpu::buffer_type::unordered_raw:
			BindFlags = D3D11_BIND_UNORDERED_ACCESS;
			Format = surface::r32_typeless;
			if (_Info.struct_byte_size != sizeof(unsigned int))
				throw std::invalid_argument("A raw buffer must specify a struct_byte_size of 4.");
			if (_Info.array_size < 3)
				throw std::invalid_argument("A raw buffer must have at least 3 elements.");
			break;
		case gpu::buffer_type::unordered_unstructured:
			BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			if (Format == surface::unknown)
				throw std::invalid_argument("An unordered, unstructured buffer requires a valid surface format to be specified.");
			break;
		case gpu::buffer_type::unordered_structured:
		case gpu::buffer_type::unordered_structured_append:
		case gpu::buffer_type::unordered_structured_counter:
			BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			break;
		oNODEFAULT;
	}

	// @tony: Why do I need to do things this way? IBs/VBs seem fine, but constant 
	// buffers seem affected by this.
	if (BindFlags != D3D11_BIND_INDEX_BUFFER && BindFlags != D3D11_BIND_VERTEX_BUFFER)
	{
		if (Usage == D3D11_USAGE_DEFAULT && D3D_FEATURE_LEVEL_11_0 > _pDevice->GetFeatureLevel())
			Usage = D3D11_USAGE_DYNAMIC;
	}

	unsigned int ElementStride = _Info.struct_byte_size;
	if (ElementStride == invalid && Format != surface::unknown)
		ElementStride = surface::element_size(Format);

	if (ElementStride == 0 || ElementStride == invalid)
		throw std::invalid_argument("A structured buffer requires a valid non-zero buffer size to be specified.");

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = ElementStride * _Info.array_size;
	desc.Usage = Usage;
	desc.BindFlags = BindFlags;
	desc.StructureByteStride = ElementStride;
	desc.CPUAccessFlags = cpu_write_flags(desc.Usage);
	desc.MiscFlags = 0;
	if (_Info.type >= gpu::buffer_type::unordered_structured)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	if (_Info.type == gpu::buffer_type::unordered_raw)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS|D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	D3D11_SUBRESOURCE_DATA SRD;
	SRD.pSysMem = _pInitBuffer;
	intrusive_ptr<ID3D11Buffer> Buffer;

	oV(_pDevice->CreateBuffer(&desc, _pInitBuffer ? &SRD : nullptr, &Buffer));
	debug_name(Buffer, _DebugName);
	
	// Add this mainly for index buffers so they can describe their own 
	// StructureByteStride.
	// @tony: Is this becoming defunct? This is meant so that D3D11 objects can be 
	// self-describing, but with a clean and not-D3D11 oGPU_BUFFER_DESC, does that 
	// hold all the info needed and we just ensure it always gets populated as 
	// expected (unlike D3D11's StructByteSize)? Probably, but this needs to stick 
	// around a bit longer until it can truly be orphaned.
	gpu::buffer_info i(_Info);
	i.struct_byte_size = as_ushort(ElementStride);
	set_info(Buffer, i);

	if (_Info.type >= gpu::buffer_type::unordered_raw)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC UAVD;
		UAVD.Format = dxgi::from_surface_format(Format);
		UAVD.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		UAVD.Buffer.FirstElement = 0;
		UAVD.Buffer.NumElements = _Info.array_size;
		UAVD.Buffer.Flags = 0;

		switch (_Info.type)
		{
			case gpu::buffer_type::unordered_raw: UAVD.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW; break;
			case gpu::buffer_type::unordered_structured_append: UAVD.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND; break;
			case gpu::buffer_type::unordered_structured_counter: UAVD.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER; break;
			default: break;
		}

		oV(_pDevice->CreateUnorderedAccessView(Buffer, &UAVD, _ppUAV));
		debug_name(*_ppUAV, _DebugName);

		if (_Info.type >= gpu::buffer_type::unordered_structured)
		{
			oV(_pDevice->CreateShaderResourceView(Buffer, nullptr, _ppSRV));
			debug_name(*_ppSRV, _DebugName);
		}
	}
	
	else if (_ppUAV)
		*_ppUAV = nullptr;

	return Buffer;
}

void copy(ID3D11Resource* _pTexture
	, unsigned int _Subresource
	, surface::mapped_subresource* _pDstSubresource
	, bool _FlipVertically)
{
	if (!_pTexture)
		oTHROW_INVARG0();

	oCHECK_IS_TEXTURE(_pTexture);

	intrusive_ptr<ID3D11Device> Device;
	_pTexture->GetDevice(&Device);
	intrusive_ptr<ID3D11DeviceContext> D3DDeviceContext;
	Device->GetImmediateContext(&D3DDeviceContext);

	gpu::texture_info info = get_texture_info(_pTexture);

	if (!gpu::is_readback(info.type))
	{
		mstring buf;
		oTHROW_INVARG("The specified texture %s does not have CPU read access", debug_name(buf, _pTexture));
	}

	D3D11_MAPPED_SUBRESOURCE source;
	oV(D3DDeviceContext->Map(_pTexture, _Subresource, D3D11_MAP_READ, 0, &source));

	int2 ByteDimensions = surface::byte_dimensions(info.format, info.dimensions);
	memcpy2d(_pDstSubresource->data, _pDstSubresource->row_pitch, source.pData, source.RowPitch, ByteDimensions.x, ByteDimensions.y, _FlipVertically);
	D3DDeviceContext->Unmap(_pTexture, _Subresource);
}

void update_subresource(ID3D11DeviceContext* _pDeviceContext
	, ID3D11Resource* _pDstResource
	, unsigned int _DstSubresource
	, const D3D11_BOX* _pDstBox
	, const surface::const_mapped_subresource& _Source)
{
	D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
	gpu::texture_info info = get_texture_info(_pDstResource, &Usage);

	switch (Usage)
	{
		case D3D11_USAGE_DEFAULT:
			_pDeviceContext->UpdateSubresource(_pDstResource, _DstSubresource, _pDstBox, _Source.data, _Source.row_pitch, _Source.depth_pitch);
			break;
		case D3D11_USAGE_STAGING:
		case D3D11_USAGE_DYNAMIC:
		{
			D3D11_RESOURCE_DIMENSION type;
			_pDstResource->GetType(&type);

			int2 ByteDimensions;
			if (type == D3D11_RESOURCE_DIMENSION_BUFFER)
				ByteDimensions = info.dimensions.xy();
			else
				ByteDimensions = surface::byte_dimensions(info.format, info.dimensions);

			D3D11_MAPPED_SUBRESOURCE msr;
			_pDeviceContext->Map(_pDstResource, _DstSubresource, D3D11_MAP_WRITE_DISCARD, 0, &msr);
			memcpy2d(msr.pData, msr.RowPitch, _Source.data, _Source.row_pitch, ByteDimensions.x, ByteDimensions.y);
			_pDeviceContext->Unmap(_pDstResource, _DstSubresource);
			break;
		}
		oNODEFAULT;
	}
}

#define oD3D11_TRACE_UINT_S(struct_, x) oTRACE("%s" #x "=%u", oSAFESTR(_Prefix), struct_.x)
#define oD3D11_TRACE_ENUM_S(struct_, x) oTRACE("%s" #x "=%s", oSAFESTR(_Prefix), as_string(struct_.x))
#define oD3D11_TRACE_FLAGS_S(_FlagEnumType, struct_, _FlagsVar, _AllZeroString) do { char buf[512]; strbitmask(buf, struct_._FlagsVar, _AllZeroString, as_string<_FlagEnumType>); oTRACE("%s" #_FlagsVar "=%s", oSAFESTR(_Prefix), buf); } while(false)

void trace_texture2d_desc(const D3D11_TEXTURE2D_DESC& _Desc, const char* _Prefix)
{
	#define oD3D11_TRACE_UINT(x) oD3D11_TRACE_UINT_S(_Desc, x)
	#define oD3D11_TRACE_ENUM(x) oD3D11_TRACE_ENUM_S(_Desc, x)
	#define oD3D11_TRACE_FLAGS(_FlagEnumType, _FlagsVar, _AllZeroString) oD3D11_TRACE_FLAGS_S(_FlagEnumType, _Desc, _FlagsVar, _AllZeroString)

	oD3D11_TRACE_UINT(Width);
	oD3D11_TRACE_UINT(Height);
	oD3D11_TRACE_UINT(MipLevels);
	oD3D11_TRACE_UINT(ArraySize);
	oD3D11_TRACE_ENUM(Format);
	oD3D11_TRACE_UINT(SampleDesc.Count);
	oD3D11_TRACE_UINT(SampleDesc.Quality);
	oD3D11_TRACE_ENUM(Usage);
	oD3D11_TRACE_FLAGS(D3D11_BIND_FLAG, BindFlags, "(none)");
	oD3D11_TRACE_FLAGS(D3D11_CPU_ACCESS_FLAG, CPUAccessFlags, "(none)");
	oD3D11_TRACE_FLAGS(D3D11_RESOURCE_MISC_FLAG, MiscFlags, "(none)");

	#undef oD3D11_TRACE_UINT
	#undef oD3D11_TRACE_ENUM
	#undef oD3D11_TRACE_FLAGS
}

template<typename DescT> static void fill_non_dimensions(const DescT& _Desc, gpu::texture_type::value _BasicType, gpu::texture_info* _pInfo)
{
	if (_Desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
		_BasicType = gpu::texture_type::default_cube;

	_pInfo->format = dxgi::to_surface_format(_Desc.Format);

	_pInfo->type = _BasicType;
	if (_Desc.MipLevels > 1)
		_pInfo->type = gpu::add_mipped(_pInfo->type);

	if (_Desc.Usage == D3D11_USAGE_STAGING)
		_pInfo->type = gpu::add_readback(_pInfo->type);

	if (_Desc.BindFlags & (D3D11_BIND_RENDER_TARGET|D3D11_BIND_DEPTH_STENCIL))
		_pInfo->type = gpu::add_render_target(_pInfo->type);

	if (_Desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		oASSERT(_pInfo->type == gpu::texture_type::default_2d, "Invalid/unhandled type");
		_pInfo->type = gpu::texture_type::unordered_2d;
	}
}

gpu::texture_info get_texture_info(ID3D11Resource* _pResource, D3D11_USAGE* _pUsage)
{
	gpu::texture_info info;

	D3D11_RESOURCE_DIMENSION type = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	_pResource->GetType(&type);
	switch (type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			D3D11_TEXTURE1D_DESC desc;
			static_cast<ID3D11Texture1D*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(static_cast<unsigned short>(desc.Width), 1, 1);
			info.array_size = static_cast<unsigned short>(desc.ArraySize);
			fill_non_dimensions(desc, gpu::texture_type::default_1d, &info);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			D3D11_TEXTURE2D_DESC desc;
			static_cast<ID3D11Texture2D*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(static_cast<unsigned short>(desc.Width)
				, static_cast<unsigned short>(desc.Height), 1);
			info.array_size = static_cast<unsigned short>(desc.ArraySize);
			fill_non_dimensions(desc, gpu::texture_type::default_2d, &info);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			static_cast<ID3D11Texture3D*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(static_cast<unsigned short>(desc.Width)
				, static_cast<unsigned short>(desc.Height)
				, static_cast<unsigned short>(desc.Depth));
			info.array_size = 1;
			fill_non_dimensions(desc, gpu::texture_type::default_3d, &info);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			gpu::buffer_info i = get_info(_pResource);
			D3D11_BUFFER_DESC desc;
			static_cast<ID3D11Buffer*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(i.struct_byte_size, static_cast<ushort>(i.array_size), 1);
			info.array_size = i.array_size;
			info.format = i.format;
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		};

		oNODEFAULT;
	}

	return info;
}

static D3D11_SHADER_RESOURCE_VIEW_DESC get_srv_desc(const gpu::texture_info& _Info, D3D11_RESOURCE_DIMENSION _Type)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC d;

	DXGI_FORMAT TF, DSVF;
	dxgi::get_compatible_formats(dxgi::from_surface_format(_Info.format), &TF, &DSVF, &d.Format);

	// All texture share basically the same memory footprint, so just write once
	d.Texture2DArray.MostDetailedMip = 0;
	d.Texture2DArray.MipLevels = surface::num_mips(gpu::is_mipped(_Info.type), _Info.dimensions);
	d.Texture2DArray.FirstArraySlice = 0;
	d.Texture2DArray.ArraySize = _Info.array_size;

	switch (_Type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			d.ViewDimension = gpu::is_array(_Info.type) ? D3D11_SRV_DIMENSION_TEXTURE1DARRAY : D3D11_SRV_DIMENSION_TEXTURE1D;
			break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			if (gpu::is_cube(_Info.type))
				d.ViewDimension = gpu::is_array(_Info.type) ? D3D11_SRV_DIMENSION_TEXTURECUBEARRAY : D3D11_SRV_DIMENSION_TEXTURECUBE;
			else
				d.ViewDimension = gpu::is_array(_Info.type) ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D;
			break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			break;
		oNODEFAULT;
	}

	return d;
}

intrusive_ptr<ID3D11ShaderResourceView> make_srv(const char* _DebugName, ID3D11Resource* _pTexture)
{
	// If a depth-stencil resource is specified we have to convert the specified 
	// format to a compatible color one
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC* pSRVDesc = nullptr;
	gpu::texture_info info = get_texture_info(_pTexture);
	if (surface::is_depth(info.format))
	{
		D3D11_RESOURCE_DIMENSION type;
		_pTexture->GetType(&type);
		SRVDesc = get_srv_desc(info, type);
		pSRVDesc = &SRVDesc;
	}

	intrusive_ptr<ID3D11Device> Device;
	_pTexture->GetDevice(&Device);
	intrusive_ptr<ID3D11ShaderResourceView> SRV;
	oV(Device->CreateShaderResourceView(_pTexture, pSRVDesc, &SRV));
	debug_name(SRV, _DebugName);
	return SRV;
}

static D3D11_DEPTH_STENCIL_VIEW_DESC get_dsv_desc(const gpu::texture_info& _Info, D3D11_RESOURCE_DIMENSION _Type)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	if (_Type != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
		throw std::invalid_argument(formatf("Unsupported resource dimension (%s)", as_string(_Type)));
	DXGI_FORMAT TF, SRVF;
	dxgi::get_compatible_formats(dxgi::from_surface_format(_Info.format), &TF, &DSVDesc.Format, &SRVF);
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Texture2D.MipSlice = 0;
	DSVDesc.Flags = 0;
	return DSVDesc;
}

intrusive_ptr<ID3D11View> make_rtv(const char* _DebugName, ID3D11Resource* _pTexture)
{
	intrusive_ptr<ID3D11Device> Device;
	_pTexture->GetDevice(&Device);
	HRESULT hr = S_OK;
	gpu::texture_info info = get_texture_info(_pTexture);

	intrusive_ptr<ID3D11View> View;
	if (surface::is_depth(info.format))
	{
		D3D11_RESOURCE_DIMENSION type;
		_pTexture->GetType(&type);
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv = get_dsv_desc(info, type);
		oV(Device->CreateDepthStencilView(_pTexture, &dsv, (ID3D11DepthStencilView**)&View));
	}
	else
		oV(Device->CreateRenderTargetView(_pTexture, nullptr, (ID3D11RenderTargetView**)&View));

	debug_name(View, _DebugName);
	return View;
}

intrusive_ptr<ID3D11UnorderedAccessView> make_uav(const char* _DebugName
	, ID3D11Resource* _pTexture, unsigned int _MipSlice, unsigned int _ArraySlice)
{
	intrusive_ptr<ID3D11Device> Device;
	_pTexture->GetDevice(&Device);
	gpu::texture_info info = get_texture_info(_pTexture);
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	UAVDesc.Format = dxgi::from_surface_format(info.format);
	switch (info.type)
	{
		// @tony: When adding more cases to this, try to use texture_info::array_size

		case gpu::texture_type::unordered_2d:
			UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			UAVDesc.Texture2D.MipSlice = 0;
			break;
		default:
			throw std::invalid_argument(formatf("Invalid texture type %s specified for UAV creation", as_string(info.type)));
	}

	intrusive_ptr<ID3D11UnorderedAccessView> UAV;
	oV(Device->CreateUnorderedAccessView(_pTexture, &UAVDesc, &UAV));
	debug_name(UAV, _DebugName);
	return UAV;
}

intrusive_ptr<ID3D11UnorderedAccessView> make_unflagged_copy(ID3D11UnorderedAccessView* _pSourceUAV)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVD;
	_pSourceUAV->GetDesc(&UAVD);
	if (UAVD.ViewDimension != D3D11_UAV_DIMENSION_BUFFER)
		throw std::invalid_argument("Only D3D11_UAV_DIMENSION_BUFFER views supported");
	intrusive_ptr<ID3D11Resource> Resource;
	_pSourceUAV->GetResource(&Resource);
	intrusive_ptr<ID3D11Device> Device;
	Resource->GetDevice(&Device);
	UAVD.Buffer.Flags = 0;
	intrusive_ptr<ID3D11UnorderedAccessView> UAV;
	oV(Device->CreateUnorderedAccessView(Resource, &UAVD, &UAV));
	return UAV;
}

#define oDXGI_FORMAT_FROM_FILE ((DXGI_FORMAT)-3)
void init_values(const gpu::texture_info& _Info
	, DXGI_FORMAT* _pFormat
	, D3D11_USAGE* _pUsage
	, unsigned int* _pCPUAccessFlags
	, unsigned int* _pBindFlags
	, unsigned int* _pMipLevels
	, unsigned int* _pMiscFlags)
{
	DXGI_FORMAT DSVF, SRVF;
	dxgi::get_compatible_formats(dxgi::from_surface_format(_Info.format), _pFormat, &DSVF, &SRVF);
	if (*_pFormat == DXGI_FORMAT_UNKNOWN)
		*_pFormat = oDXGI_FORMAT_FROM_FILE;

	*_pUsage = D3D11_USAGE_DEFAULT;
	if (gpu::is_readback(_Info.type))
		*_pUsage = D3D11_USAGE_STAGING;

	if (_pMiscFlags)
		*_pMiscFlags = 0;

	*_pCPUAccessFlags = cpu_write_flags(*_pUsage);
	*_pMipLevels = gpu::is_mipped(_Info.type) ? 0 : 1;

	if (_pMiscFlags && gpu::is_cube(_Info.type))
			*_pMiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

	*_pBindFlags = 0;
	if (*_pUsage != D3D11_USAGE_STAGING)
	{
		*_pBindFlags = D3D11_BIND_SHADER_RESOURCE;
		if (gpu::is_render_target(_Info.type))
		{
			*_pBindFlags |= D3D11_BIND_RENDER_TARGET;

			// D3D11_RESOURCE_MISC_GENERATE_MIPS is only valid for render targets.
			// It is up to client code to handle default textures and depth textures
			// rebound for sampling.
			if (_pMiscFlags && gpu::is_mipped(_Info.type))
				*_pMiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
	}

	if (surface::is_depth(dxgi::to_surface_format(*_pFormat)) && *_pFormat != oDXGI_FORMAT_FROM_FILE)
	{
		*_pBindFlags &=~ D3D11_BIND_RENDER_TARGET;
		*_pBindFlags |= D3D11_BIND_DEPTH_STENCIL;
	}

	if (gpu::is_unordered(_Info.type))
		*_pBindFlags |= D3D11_BIND_UNORDERED_ACCESS;
}

new_texture make_texture(ID3D11Device* _pDevice
	, const char* _DebugName
	, const gpu::texture_info& _Info
	, surface::const_mapped_subresource* _pInitData)
{
	new_texture NewTexture;
	bool IsShaderResource = false;
	intrusive_ptr<ID3D11Resource> Texture;
	switch (gpu::get_basic(_Info.type))
	{
		case gpu::texture_type::default_1d:
		{
			D3D11_TEXTURE1D_DESC desc;
			desc.Width = _Info.dimensions.x;
			desc.ArraySize = __max(1, _Info.array_size);
			init_values(_Info, &desc.Format, &desc.Usage, &desc.CPUAccessFlags, &desc.BindFlags, &desc.MipLevels, &desc.MiscFlags);
			IsShaderResource = !!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);
			oV(_pDevice->CreateTexture1D(&desc, (D3D11_SUBRESOURCE_DATA*)_pInitData, &NewTexture.pTexture1D));
			break;
		}

		case gpu::texture_type::default_2d:
		case gpu::texture_type::default_cube:
		{
			if (gpu::is_cube(_Info.type) && _Info.array_size != 6)
				throw std::invalid_argument(formatf("Cube maps must have ArraySize == 6, ArraySize=%d specified", _Info.array_size));

			D3D11_TEXTURE2D_DESC desc;
			desc.Width = _Info.dimensions.x;
			desc.Height = _Info.dimensions.y;
			desc.ArraySize = __max(1, _Info.array_size);
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			init_values(_Info, &desc.Format, &desc.Usage, &desc.CPUAccessFlags, &desc.BindFlags, &desc.MipLevels, &desc.MiscFlags);
			IsShaderResource = !!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);
			oV(_pDevice->CreateTexture2D(&desc, (D3D11_SUBRESOURCE_DATA*)_pInitData, &NewTexture.pTexture2D));
			break;
		}

		case gpu::texture_type::default_3d:
		{
			if (_Info.array_size > 1)
				throw std::invalid_argument(formatf("3d textures don't support slices, ArraySize=%d specified", _Info.array_size));
			D3D11_TEXTURE3D_DESC desc;
			desc.Width = _Info.dimensions.x;
			desc.Height = _Info.dimensions.y;
			desc.Depth = _Info.dimensions.z;
			init_values(_Info, &desc.Format, &desc.Usage, &desc.CPUAccessFlags, &desc.BindFlags, &desc.MipLevels, &desc.MiscFlags);
			IsShaderResource = !!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);
			oV(_pDevice->CreateTexture3D(&desc, (D3D11_SUBRESOURCE_DATA*)_pInitData, &NewTexture.pTexture3D));
			break;
		}
		oNODEFAULT;
	}

	debug_name(NewTexture.pResource, _DebugName);
	if (IsShaderResource)
	{
		auto srv = make_srv(_DebugName, NewTexture.pResource);
		NewTexture.pSRV = srv;
		NewTexture.pSRV->AddRef();
	}

	if (gpu::is_render_target(_Info.type))
	{
		auto view = make_rtv(_DebugName, NewTexture.pResource);
		NewTexture.pView = view;
		NewTexture.pView->AddRef();
	}

	return NewTexture;
}

intrusive_ptr<ID3D11Resource> make_cpu_copy(ID3D11Resource* _pResource)
{
	intrusive_ptr<ID3D11Device> D3D11Device;
	_pResource->GetDevice(&D3D11Device);
	intrusive_ptr<ID3D11DeviceContext> D3D11DeviceContext;
	D3D11Device->GetImmediateContext(&D3D11DeviceContext);

	lstring RTName;
	debug_name(RTName, _pResource);

	lstring copyName;
	snprintf(copyName, "%s.CPUCopy", RTName.c_str());

	intrusive_ptr<ID3D11Resource> CPUCopy;

	D3D11_RESOURCE_DIMENSION type;
	_pResource->GetType(&type);
	switch (type)
	{
		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			D3D11_BUFFER_DESC d;
			static_cast<ID3D11Buffer*>(_pResource)->GetDesc(&d);
			d.Usage = D3D11_USAGE_STAGING;
			d.CPUAccessFlags = /*D3D11_CPU_ACCESS_WRITE|*/D3D11_CPU_ACCESS_READ;
			d.BindFlags &=~ (D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_STREAM_OUTPUT|D3D11_BIND_RENDER_TARGET|D3D11_BIND_DEPTH_STENCIL);
			oV(D3D11Device->CreateBuffer(&d, nullptr, (ID3D11Buffer**)&CPUCopy));
			debug_name(CPUCopy, copyName);
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			gpu::texture_info i = get_texture_info(_pResource);
			i.type = gpu::add_readback(i.type);
			new_texture NewTexture = make_texture(D3D11Device, copyName, i, nullptr);
			CPUCopy = NewTexture.pResource;
			break;
		}

		default:
			throw std::invalid_argument("unknown resource type");
	}

	try
	{
		gpu::buffer_info i = get_info(_pResource);
		set_info(CPUCopy, i);
	}
	catch(std::exception&) {}
	
	D3D11DeviceContext->CopyResource(CPUCopy, _pResource);
	D3D11DeviceContext->Flush();
	return CPUCopy;
}

std::shared_ptr<surface::buffer> make_snapshot(ID3D11Texture2D* _pRenderTarget)
{
	if (!_pRenderTarget)
		throw std::invalid_argument("invalid render target");

	intrusive_ptr<ID3D11Resource> CPUTexture = make_cpu_copy(_pRenderTarget);
	ID3D11Texture2D* CPUTexture2D = (ID3D11Texture2D*)CPUTexture.c_ptr();

	D3D11_TEXTURE2D_DESC d;
	CPUTexture2D->GetDesc(&d);

	if (d.Format == DXGI_FORMAT_UNKNOWN)
		throw std::invalid_argument(formatf("The specified texture's format %s is not supported by oImage", as_string(d.Format)));

	surface::info si;
	si.format = surface::b8g8r8a8_unorm;
	si.dimensions = int3(d.Width, d.Height, 1);
	std::shared_ptr<surface::buffer> s = surface::buffer::make(si);

	surface::lock_guard lock(s);
	copy(CPUTexture2D, 0, &lock.mapped);

	return s;
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
			gpu::texture_info i = get_texture_info(Resource);
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

oAABoxf from_viewport(const D3D11_VIEWPORT& _Viewport)
{
	oAABoxf b;
	b.Min = float3(_Viewport.TopLeftX, _Viewport.TopLeftY, _Viewport.MinDepth);
	b.Max = float3(_Viewport.Width, _Viewport.Height, _Viewport.MaxDepth);
	return b;
}

D3D11_VIEWPORT to_viewport(const oAABoxf& _Source)
{
	D3D11_VIEWPORT v;
	v.TopLeftX = _Source.Min.x;
	v.TopLeftY = _Source.Min.y;
	v.MinDepth = _Source.Min.z;
	v.Width = _Source.size().x;
	v.Height = _Source.size().y;
	v.MaxDepth = _Source.Max.z;
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

D3D11_PRIMITIVE_TOPOLOGY oD3D11ToPrimitiveTopology(oGPU_PRIMITIVE_TYPE _Type)
{
	return D3D11_PRIMITIVE_TOPOLOGY(_Type);
}

// to oAlgorithm... should we permutate for various sources?
bool oD3D11ConvertCompileErrorBuffer(char* _OutErrorMessageString, size_t _SizeofOutErrorMessageString, ID3DBlob* _pErrorMessages, const char** _pIncludePaths, size_t _NumIncludePaths)
{
	const char* msg = (const char*)_pErrorMessages->GetBufferPointer();

	if (!_OutErrorMessageString)
		return oErrorSetLast(std::errc::invalid_argument);

	if (_pErrorMessages)
	{
		std::string tmp;
		tmp.reserve(oKB(10));
		tmp.assign(msg);
		replace_all(tmp, "%", "%%");

		// Now make sure header errors include their full paths
		if (_pIncludePaths && _NumIncludePaths)
		{
			size_t posShortHeaderEnd = tmp.find(".h(");
			while (posShortHeaderEnd != std::string::npos)
			{
				size_t posShortHeader = tmp.find_last_of("\n", posShortHeaderEnd);
				if (posShortHeader == std::string::npos)
					posShortHeader = 0;
				else
					posShortHeader++;

				posShortHeaderEnd += 2; // absorb ".h" from search

				size_t shortHeaderLen = posShortHeaderEnd - posShortHeader;

				std::string shortPath;
				shortPath.assign(tmp, posShortHeader, shortHeaderLen);
				std::string path;
				path.reserve(oKB(1));
				
				for (size_t i = 0; i < _NumIncludePaths; i++)
				{
					path.assign(_pIncludePaths[i]);
					path.append("/");
					path.append(shortPath);

					if (oStreamExists(path.c_str()))
					{
						tmp.replace(posShortHeader, shortHeaderLen, path.c_str());
						posShortHeaderEnd = tmp.find("\n", posShortHeaderEnd); // move to end of line
						break;
					}
				}

				posShortHeaderEnd = tmp.find(".h(", posShortHeaderEnd);
			}
		}

		strlcpy(_OutErrorMessageString, tmp.c_str(), _SizeofOutErrorMessageString);
	}

	else
		*_OutErrorMessageString = 0;

	return true;
}

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
	ouro::gpu::texture_info d;
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
	ouro::gpu::texture_info d;
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

class oD3DInclude : public ID3DInclude
{
public:
	oD3DInclude(const char* _ShaderSourcePath, const std::vector<const char*>& _HeaderSearchPaths)
		: ShaderSourcePath(_ShaderSourcePath)
		, HeaderSearchPaths(_HeaderSearchPaths)
	{}

	~oD3DInclude();

	ULONG AddRef() { return 0; }
	ULONG Release() { return 0; }
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObject) { return ppvObject ? E_NOINTERFACE : E_POINTER; }
	IFACEMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, unsigned int *pBytes);
	IFACEMETHOD(Close)(THIS_ LPCVOID pData);

protected:
	const char* ShaderSourcePath;
	const std::vector<const char*>& HeaderSearchPaths;

	struct BUFFER
	{
		BUFFER() : pData(nullptr), Size(0) {}
		BUFFER(LPCVOID _pData, unsigned int _Size) : pData(_pData), Size(_Size) {}
		LPCVOID pData;
		unsigned int Size;
	};

	unordered_map<oURI, BUFFER> Cache;
};

oD3DInclude::~oD3DInclude()
{
	oFOR(auto& pair, Cache)
		free((void*)pair.second.pData);
}

HRESULT oD3DInclude::Close(LPCVOID pData)
{
	// don't destroy the cached files...
	//free((void*)pData);
	return S_OK;
}

HRESULT oD3DInclude::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, unsigned int* pBytes)
{
	auto it = Cache.find(pFileName);
	if (it != Cache.end())
	{
		*ppData = it->second.pData;
		*pBytes = it->second.Size;
		return S_OK;
	}

	bool exists = oStreamExists(pFileName);
	path_string Path;

	if (exists)
		Path = pFileName;
	else
	{
		oFOR(const char* p, HeaderSearchPaths)
		{
			snprintf(Path, "%s/%s", p, pFileName);
			exists = oStreamExists(Path);
			if (exists)
				goto hack_break; // @oooii-tony: oFOR uses a double-for loop, so one break isn't enough. We should fix this!
		}
	}

hack_break:
	if (!exists)
	{
		oErrorSetLast(std::errc::no_such_file_or_directory, "Header %s not found in search path", pFileName);
		oTRACE("%s", oErrorGetLastString());
		return E_FAIL;
	}

	size_t size = 0;
	if (!oStreamLoad(ppData, &size, malloc, free, Path, true))
	{
		oTRACE("%s", oErrorGetLastString());
		return E_FAIL;
	}

	*pBytes = ounsigned int(size);
	
	Cache[oURI(pFileName)] = BUFFER(*ppData, *pBytes);
	return S_OK;
}

bool oFXC(const char* _CommandLineOptions, const char* _ShaderSourceFilePath, const char* _ShaderSource, oBuffer** _ppBuffer)
{
	int argc = 0;
	const char** argv = oWinCommandLineToArgvA(false, _CommandLineOptions, &argc);
	finally OSCFreeArgv([&] { oWinCommandLineToArgvAFree(argv); });

	std::string UnsupportedOptions("Unsupported options: ");
	size_t UnsupportedOptionsEmptyLen = UnsupportedOptions.size();

	const char* TargetProfile = "";
	const char* EntryPoint = "main";
	std::vector<const char*> IncludePaths;
	std::vector<std::pair<std::string, std::string>> Defines;
	unsigned int Flags1 = 0, Flags2 = 0;

	for (int i = 0; i < argc; i++)
	{
		const char* sw = argv[i];
		const int o = toupper(*(sw+1));
		const int o2 = toupper(*(sw+2));
		const int o3 = toupper(*(sw+3));

		std::string StrSw(sw);

		#define TRIML(str) ((str) + strspn(str, oWHITESPACE))

		if (*sw == '/')
		{
			switch (o)
			{
				case 'T':
					TargetProfile = TRIML(sw+2);
					break;
				case 'E':
					EntryPoint = TRIML(sw+2);
					break;
				case 'I':
					IncludePaths.push_back(TRIML(sw+2));
					break;
				case 'O':
				{
					switch (o2)
					{
						case 'D':	Flags1 |= D3D10_SHADER_SKIP_OPTIMIZATION; break;
						case 'P':	Flags1 |= D3D10_SHADER_NO_PRESHADER; break;
						case '0': Flags1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0; break;
						case '1': Flags1 |= D3D10_SHADER_OPTIMIZATION_LEVEL1; break;
						case '2': Flags1 |= D3D10_SHADER_OPTIMIZATION_LEVEL2; break;
						case '3': Flags1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3; break;
						default: UnsupportedOptions.append(" " + StrSw); break;
					}

					break;
				}
				case 'W':
				{
					if (o2 == 'X') Flags1 |= D3D10_SHADER_WARNINGS_ARE_ERRORS;
					else UnsupportedOptions.append(" " + StrSw);
					break;
				}
				case 'V':
				{
					if (o2 == 'D') Flags1 |= D3D10_SHADER_SKIP_VALIDATION;
					else UnsupportedOptions.append(" " + StrSw);
					break;
				}
				case 'Z':
				{
					switch (o2)
					{
						case 'I':
							Flags1 |= D3D10_SHADER_DEBUG;
							break;
						case 'P':
						{
							switch (o3)
							{
								case 'R': Flags1 |= D3D10_SHADER_PACK_MATRIX_ROW_MAJOR; break;
								case 'C': Flags1 |= D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR; break;
								default: UnsupportedOptions.append(" " + StrSw); break;
							}

							break;
						}
						default: UnsupportedOptions.append(" " + StrSw); break;
					}
						
					break;
				}
				case 'G':
				{
					if (o2 == 'P' && o3 == 'P') Flags1 |= D3D10_SHADER_PARTIAL_PRECISION;
					else if (o2 == 'F' && o3 == 'A') Flags1 |= D3D10_SHADER_AVOID_FLOW_CONTROL;
					else if (o2 == 'F' && o3 == 'P') Flags1 |= D3D10_SHADER_PREFER_FLOW_CONTROL;
					else if (o2 == 'D' && o3 == 'P') Flags2 |= D3D10_EFFECT_COMPILE_ALLOW_SLOW_OPS;
					else if (o2 == 'E' && o3 == 'S') Flags1 |= D3D10_SHADER_ENABLE_STRICTNESS;
					else if (o2 == 'E' && o3 == 'C') Flags1 |= D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY;
					else if (o2 == 'I' && o3 == 'S') Flags1 |= D3D10_SHADER_IEEE_STRICTNESS;
					else if (o2 == 'C' && o3 == 'H') Flags2 |= D3D10_EFFECT_COMPILE_CHILD_EFFECT;
					else UnsupportedOptions.append(" " + StrSw);
					break;
				}
				case 'D':
				{
					const char* k = TRIML(sw+2);
					const char* sep = strchr(k, '=');
					const char* v = "1";
					if (sep)
						v = TRIML(v);
					else
						sep = k + strlen(k);

					Defines.resize(Defines.size() + 1);
					Defines.back().first.assign(k, sep-k);
					Defines.back().second.assign(v);
					break;
				}
				
				default: UnsupportedOptions.append(" " + StrSw); break;
			}
		}
	}

	if (UnsupportedOptionsEmptyLen != UnsupportedOptions.size())
	{
		size_t size = UnsupportedOptions.length() + 1;
		oVERIFY(oBufferCreate("Parameter Errors", oBuffer::New(size), size, oBuffer::Delete, _ppBuffer));
		strlcpy((char*)(*_ppBuffer)->GetData(), UnsupportedOptions.c_str(), (*_ppBuffer)->GetSize());
		return oErrorSetLast(std::errc::invalid_argument);
	}

	std::vector<D3D_SHADER_MACRO> Macros;
	Macros.resize(Defines.size() + 1);
	for (size_t i = 0; i < Defines.size(); i++)
	{
		Macros[i].Name = Defines[i].first.c_str();
		Macros[i].Definition = Defines[i].second.c_str();
	}

	Macros.back().Name = nullptr;
	Macros.back().Definition = nullptr;

	uri_string SourceName;
	snprintf(SourceName, "%s", _ShaderSourceFilePath);

	oD3DInclude D3DInclude(_ShaderSourceFilePath, IncludePaths);
	intrusive_ptr<ID3DBlob> Code, Errors;
	HRESULT hr = D3DCompile(_ShaderSource
		, strlen(_ShaderSource)
		, SourceName
		, data(Macros)
		, &D3DInclude
		, EntryPoint
		, TargetProfile
		, Flags1
		, Flags2
		, &Code
		, &Errors);

	if (FAILED(hr))
	{
		size_t size = Errors->GetBufferSize() + 1 + oKB(10); // conversion can expand buffer, but not by very much, so pad a lot and hope expansion stays small
		oVERIFY(oBufferCreate("Compile Errors", oBuffer::New(size), size, oBuffer::Delete, _ppBuffer));
		oVERIFY(oD3D11ConvertCompileErrorBuffer((char*)(*_ppBuffer)->GetData(), (*_ppBuffer)->GetSize(), Errors, data(IncludePaths), IncludePaths.size()));
		return oErrorSetLast(std::errc::io_error, "shader compilation error: %s", oSAFESTRN(_ShaderSourceFilePath));
	}

	oVERIFY(oBufferCreate("Compile Errors", oBuffer::New(Code->GetBufferSize()), Code->GetBufferSize(), oBuffer::Delete, _ppBuffer));
	memcpy((*_ppBuffer)->GetData(), Code->GetBufferPointer(), (*_ppBuffer)->GetSize());
	return true;
}
#endif

	} // namespace d3d11
} // namespace ouro
