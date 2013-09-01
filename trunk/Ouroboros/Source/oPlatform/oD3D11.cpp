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
#include <oPlatform/Windows/oD3D11.h>
#include <oStd/assert.h>
#include <oStd/byte.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oStream.h>
#include <oPlatform/oStreamUtil.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/Windows/oDXGI.h>
#include "SoftLink/oD3DX11.h"
#include <cerrno>

// {13BA565C-4766-49C4-8C1C-C1F459F00A65}
static const GUID oWKPDID_oGPU_BUFFER_DESC = { 0x13ba565c, 0x4766, 0x49c4, { 0x8c, 0x1c, 0xc1, 0xf4, 0x59, 0xf0, 0xa, 0x65 } };

// {6489B24E-C12E-40C2-A9EF-249353888612}
static const GUID oWKPDID_oBackPointer = { 0x6489b24e, 0xc12e, 0x40c2, { 0xa9, 0xef, 0x24, 0x93, 0x53, 0x88, 0x86, 0x12 } };

// Value obtained from D3Dcommon.h and reproduced here because of soft-linking
static const GUID oWKPDID_D3DDebugObjectName = { 0x429b8c22, 0x9188, 0x4b0c, { 0x87, 0x42, 0xac, 0xb0, 0xbf, 0x85, 0xc2, 0x00} };

const oGUID& oGetGUID(threadsafe const ID3D11Device* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11Device); }
const oGUID& oGetGUID(threadsafe const ID3D11DeviceContext* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11DeviceContext); }
const oGUID& oGetGUID(threadsafe const ID3D11RenderTargetView* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11RenderTargetView); }
const oGUID& oGetGUID(threadsafe const ID3D11Texture2D* threadsafe const*) { return (const oGUID&)__uuidof(ID3D11Texture2D); }

static const char* d3d11_dll_functions[] = 
{
	"D3D11CreateDevice",
};

oD3D11::oD3D11()
{
	hModule = oModuleLinkSafe("d3d11.dll", d3d11_dll_functions, (void**)&D3D11CreateDevice, oCOUNTOF(d3d11_dll_functions));
	oASSERT(hModule, "");
}

oD3D11::~oD3D11()
{
	oModuleUnlink(hModule);
}

// {2BCF2584-3DE9-4192-89D3-0787E4DE32F9}
const oGUID oD3D11::GUID = { 0x2bcf2584, 0x3de9, 0x4192, { 0x89, 0xd3, 0x7, 0x87, 0xe4, 0xde, 0x32, 0xf9 } };
oSINGLETON_REGISTER(oD3D11);

static bool oD3D11FindAdapter(int _Index, const int2& _VirtualDesktopPosition, const oVersion& _MinVersion, bool _ExactVersion, IDXGIAdapter** _ppAdapter)
{
	bool ForceOneGPU = false;
	char buf[32];
	if (oSystemGetEnvironmentVariable(buf, "OOOii.D3D11.ForceOneGPU") && oStd::from_string(&ForceOneGPU, buf) && ForceOneGPU)
	{
		oTRACE("Forcing D3D11Device index %d to 0 because OOOii.D3D11.ForceOneGPU is set.", _Index);
		_Index = 0;
	}

	oRef<IDXGIFactory> Factory;
	if (!oDXGICreateFactory(&Factory))
		return oErrorSetLast(std::errc::no_such_device, "Failed to create DXGI factory");

	if (any_equal(_VirtualDesktopPosition, int2(oDEFAULT, oDEFAULT)))
	{
		if (_Index < 0)
			return oErrorSetLast(std::errc::invalid_argument, "An invalid index and an invalid virtual desktop point where specified. One must be valid.");

		if (DXGI_ERROR_NOT_FOUND == Factory->EnumAdapters(_Index, _ppAdapter))
			return oErrorSetLast(std::errc::no_such_device, "An IDXGIAdapter could not be found at index %d", _Index);
	}

	else
	{
		oRef<IDXGIOutput> Output;
			if (!oDXGIFindOutput(Factory, _VirtualDesktopPosition, &Output))
				return oErrorSetLast(std::errc::no_such_device, "No output found displaying virtual desktop position %d, %d", _VirtualDesktopPosition.x, _VirtualDesktopPosition.y);
		oVERIFY(oDXGIGetAdapter(Output, _ppAdapter));
	}

	int AdapterIndex = oDXGIGetAdapterIndex(*_ppAdapter);
	if (AdapterIndex == oInvalid)
		return oErrorSetLast(std::errc::no_such_device, "No GPUIndex could be found from the DXGI adapter found");

	oDISPLAY_ADAPTER_DRIVER_DESC add;
	oDXGIGetAdapterDriverDesc(*_ppAdapter, &add);

	oVersion RequiredVersion = _MinVersion;

	if (!RequiredVersion.IsValid())
	{
		switch (add.Vendor)
		{
			case oGPU_VENDOR_NVIDIA:
				RequiredVersion = oVersion(oNVVER_MAJOR, oNVVER_MINOR);
				break;
			case oGPU_VENDOR_AMD:
				RequiredVersion = oVersion(oAMDVER_MAJOR, oAMDVER_MINOR);
				break;
			default:
				break;
		}
	}

	if (_ExactVersion)
	{
		if (add.Version != RequiredVersion)
			return oErrorSetLast(std::errc::invalid_argument, "Exact video driver version %d.%d required, but current driver is %d.%d", RequiredVersion.Major, RequiredVersion.Minor, add.Version.Major, add.Version.Minor);
	}

	else if (add.Version < RequiredVersion)
		return oErrorSetLast(std::errc::invalid_argument, "Video driver version %d.%d or newer required, but current driver is %d.%d", RequiredVersion.Major, RequiredVersion.Minor, add.Version.Major, add.Version.Minor);

	return true;
}

bool oD3D11CreateDevice(const oGPU_DEVICE_INIT& _Init, bool _SingleThreaded, ID3D11Device** _ppDevice)
{
	if (!_ppDevice || _Init.Version < oVersion(9,0))
		return oErrorSetLast(std::errc::invalid_argument);

	oRef<IDXGIAdapter> Adapter;

	if (!_Init.UseSoftwareEmulation)
	{
		if (!oD3D11FindAdapter(_Init.AdapterIndex, _Init.VirtualDesktopPosition, _Init.MinDriverVersion, _Init.DriverVersionMustBeExact, &Adapter))
			return false; // pass through error
	}

	UINT Flags = 0;
	bool UsingDebug = false;
	if (_Init.DriverDebugLevel != oGPU_DEBUG_NONE)
	{
		Flags |= D3D11_CREATE_DEVICE_DEBUG;
		UsingDebug = true;
	}

	if (_SingleThreaded)
		Flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;

	D3D_FEATURE_LEVEL FeatureLevel;
	HRESULT hr = oD3D11::Singleton()->D3D11CreateDevice(
		Adapter
		, _Init.UseSoftwareEmulation ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_UNKNOWN
		, 0
		, Flags
		, nullptr
		, 0
		, D3D11_SDK_VERSION
		, _ppDevice
		, &FeatureLevel
		, nullptr);

	// http://stackoverflow.com/questions/10586956/what-can-cause-d3d11createdevice-to-fail-with-e-fail
	// It's possible that the debug lib isn't installed, so try again without 
	// debug.
	if (hr == E_FAIL)
	{
		Flags &=~ D3D11_CREATE_DEVICE_DEBUG;
		UsingDebug = false;

		oVB_RETURN2(oD3D11::Singleton()->D3D11CreateDevice(
			Adapter
			, _Init.UseSoftwareEmulation ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_UNKNOWN
			, 0
			, Flags
			, nullptr
			, 0
			, D3D11_SDK_VERSION
			, _ppDevice
			, &FeatureLevel
			, nullptr));

		oTRACE("Debug D3D11 not found: device created in non-debug mode so driver error reporting will not be available.");
	}

	oVersion D3DVersion = oD3D11GetFeatureVersion(FeatureLevel);
	if (D3DVersion < _Init.Version)
	{
		if (*_ppDevice) 
		{
			(*_ppDevice)->Release();
			*_ppDevice = nullptr;
		}
		return oErrorSetLast(std::errc::not_supported, "Failed to create an ID3D11Device with a minimum feature set of DX %d.%d!", _Init.Version.Major, _Init.Version.Minor);
	}

	oVERIFY(oD3D11SetDebugName(*_ppDevice, oSAFESTRN(_Init.DebugName)));

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

	if (UsingDebug && _Init.DriverDebugLevel == oGPU_DEBUG_NORMAL)
	{
		oRef<ID3D11InfoQueue> IQ;
		oV((*_ppDevice)->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&IQ));
		D3D11_INFO_QUEUE_FILTER filter = {0};
		filter.DenyList.NumCategories = oCOUNTOF(sDisabledD3D11Categories);
		filter.DenyList.pCategoryList = const_cast<D3D11_MESSAGE_CATEGORY*>(sDisabledD3D11Categories);
		filter.DenyList.NumIDs = oCOUNTOF(sDisabledD3D11Messages);
		filter.DenyList.pIDList = const_cast<D3D11_MESSAGE_ID*>(sDisabledD3D11Messages);
		IQ->PushStorageFilter(&filter);
	}

	return true;
}

bool oD3D11DeviceGetDesc(ID3D11Device* _pDevice, bool _IsSoftwareEmulation, oGPU_DEVICE_DESC* _pDesc)
{
	oD3D11GetDebugName(_pDesc->DebugName, _pDevice);

	oRef<IDXGIAdapter> Adapter;
	if (!oD3D11GetAdapter(_pDevice, &Adapter))
		return false;

	DXGI_ADAPTER_DESC ad;
	Adapter->GetDesc(&ad);
	oDISPLAY_ADAPTER_DRIVER_DESC add;
	oDXGIGetAdapterDriverDesc(Adapter, &add);

	_pDesc->DeviceDescription = ad.Description;
	_pDesc->DriverDescription = add.Description;
	_pDesc->NativeMemory = ad.DedicatedVideoMemory;
	_pDesc->DedicatedSystemMemory = ad.DedicatedSystemMemory;
	_pDesc->SharedSystemMemory = ad.SharedSystemMemory;
	_pDesc->DriverVersion = add.Version;
	_pDesc->FeatureVersion = oDXGIGetFeatureLevel(Adapter);
	_pDesc->InterfaceVersion = oDXGIGetInterfaceVersion(Adapter);
	_pDesc->AdapterIndex = oDXGIGetAdapterIndex(Adapter);
	_pDesc->API = oGPU_API_D3D;
	_pDesc->Vendor = add.Vendor;
	_pDesc->IsSoftwareEmulation = _IsSoftwareEmulation;
	_pDesc->DebugReportingEnabled = !!(_pDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_DEBUG);
	return true;
}

bool oD3D11GetAdapter(ID3D11Device* _pDevice, IDXGIAdapter** _ppAdapter)
{
	if (!_pDevice)
		return oErrorSetLast(std::errc::invalid_argument);

	oRef<IDXGIDevice> DXGIDevice;
	oVB_RETURN2(_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&DXGIDevice));

	DXGIDevice->GetAdapter(_ppAdapter);
	return true;
}

namespace oStd {

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
		oNODEFAULT;
	}
}

const char* as_string(const D3D11_CPU_ACCESS_FLAG& _Flag)
{
	switch (_Flag)
	{
		case D3D11_CPU_ACCESS_WRITE: return "D3D11_CPU_ACCESS_WRITE";
		case D3D11_CPU_ACCESS_READ: return "D3D11_CPU_ACCESS_READ";
		oNODEFAULT;
	}
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
		oNODEFAULT;
	}
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
		oNODEFAULT;
	}
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
		oNODEFAULT;
	}
}

const char* as_string(const D3D11_USAGE& _Usage)
{
	switch (_Usage)
	{
		case D3D11_USAGE_DEFAULT: return "D3D11_USAGE_DEFAULT";
		case D3D11_USAGE_IMMUTABLE: return "D3D11_USAGE_IMMUTABLE";
		case D3D11_USAGE_DYNAMIC: return "D3D11_USAGE_DYNAMIC";
		case D3D11_USAGE_STAGING: return "D3D11_USAGE_STAGING";
		oNODEFAULT;
	}
}

} // namespace oStd

oVersion oD3D11GetFeatureVersion(D3D_FEATURE_LEVEL _Level)
{
	switch (_Level)
	{
		case D3D_FEATURE_LEVEL_11_0: return oVersion(11,0);
		case D3D_FEATURE_LEVEL_10_1: return oVersion(10,1);
		case D3D_FEATURE_LEVEL_10_0: return oVersion(10,0);
		case D3D_FEATURE_LEVEL_9_3: return oVersion(9,3);
		case D3D_FEATURE_LEVEL_9_2: return oVersion(9,2);
		case D3D_FEATURE_LEVEL_9_1: return oVersion(9,1);
		oNODEFAULT;
	}
}

bool oD3D11SetDebugName(ID3D11Device* _pDevice, const char* _Name)
{
	if (!_pDevice || !_Name)
		return oErrorSetLast(std::errc::invalid_argument);

	UINT CreationFlags = _pDevice->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
	{
		HRESULT hr = _pDevice->SetPrivateData(oWKPDID_D3DDebugObjectName, static_cast<UINT>(oStrlen(_Name) + 1), _Name);
		if (FAILED(hr))
			return oWinSetLastError(hr);
	}

	return true;
}

char* oD3D11GetDebugName(char* _StrDestination, size_t _SizeofStrDestination, const ID3D11Device* _pDevice)
{
	if (!_pDevice || !_StrDestination || _SizeofStrDestination == 0)
	{
		oErrorSetLast(std::errc::invalid_argument);
		return nullptr;
	}

	UINT size = oUInt(_SizeofStrDestination);
	UINT CreationFlags = const_cast<ID3D11Device*>(_pDevice)->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		return S_OK == const_cast<ID3D11Device*>(_pDevice)->GetPrivateData(oWKPDID_D3DDebugObjectName, &size, _StrDestination) ? _StrDestination : "(null)";
	else
		return oStrcpy(_StrDestination, _SizeofStrDestination, "non-debug device");
}

bool oD3D11SetDebugName(ID3D11DeviceChild* _pDeviceChild, const char* _Name)
{
	if (!_pDeviceChild || !_Name)
		return oErrorSetLast(std::errc::invalid_argument);

	oRef<ID3D11Device> D3DDevice;
	_pDeviceChild->GetDevice(&D3DDevice);
	UINT CreationFlags = D3DDevice->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
	{
		oStd::uri_string tmp(_Name);
		HRESULT hr = _pDeviceChild->SetPrivateData(oWKPDID_D3DDebugObjectName, oUInt(tmp.capacity()), tmp.c_str());
		if (FAILED(hr))
			return oWinSetLastError(hr);
	}

	return true;
}

char* oD3D11GetDebugName(char* _StrDestination, size_t _SizeofStrDestination, const ID3D11DeviceChild* _pDeviceChild)
{
	UINT size = oUInt(_SizeofStrDestination);
	oRef<ID3D11Device> D3DDevice;
	const_cast<ID3D11DeviceChild*>(_pDeviceChild)->GetDevice(&D3DDevice);
	UINT CreationFlags = D3DDevice->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		return S_OK == const_cast<ID3D11DeviceChild*>(_pDeviceChild)->GetPrivateData(oWKPDID_D3DDebugObjectName, &size, _StrDestination) ? _StrDestination : "(null)";
	else
		oStrcpy(_StrDestination, _SizeofStrDestination, "non-debug device child");
	return _StrDestination;
}

D3DX11_IMAGE_FILE_FORMAT oD3D11GetFormatFromPath(const char* _Path)
{
	const char* ext = oGetFileExtension(_Path);

	struct EXT_MAPPING
	{
		const char* Extension;
		D3DX11_IMAGE_FILE_FORMAT Format;
	};

	static EXT_MAPPING sExtensions[] =
	{
		{ ".bmp", D3DX11_IFF_BMP }, 
		{ ".jpg", D3DX11_IFF_JPG }, 
		{ ".png", D3DX11_IFF_PNG }, 
		{ ".dds", D3DX11_IFF_DDS }, 
		{ ".tif", D3DX11_IFF_TIFF }, 
		{ ".gif", D3DX11_IFF_GIF }, 
		{ ".wmp", D3DX11_IFF_WMP },
	};

	oFORI(i, sExtensions)
		if (!oStricmp(sExtensions[i].Extension, ext))
			return sExtensions[i].Format;

	return D3DX11_IFF_DDS;
}

bool oD3D11BufferSetDesc(ID3D11Resource* _pBuffer, const oGPU_BUFFER_DESC& _Desc)
{
	return S_OK == _pBuffer->SetPrivateData(oWKPDID_oGPU_BUFFER_DESC, sizeof(_Desc), &_Desc);
}

bool oD3D11BufferGetDesc(const ID3D11Resource* _pBuffer, oGPU_BUFFER_DESC* _pDesc)
{
	UINT size = sizeof(oGPU_BUFFER_DESC);
	return S_OK == const_cast<ID3D11Resource*>(_pBuffer)->GetPrivateData(oWKPDID_oGPU_BUFFER_DESC, &size, _pDesc);
}

bool oD3D11SetContainerBackPointer(ID3D11DeviceChild* _pChild, oInterface* _pContainer)
{
	return S_OK == _pChild->SetPrivateData(oWKPDID_oBackPointer, sizeof(oInterface*), &_pContainer);
}

bool oD3D11GetContainerBackPointer(const ID3D11DeviceChild* _pChild, oInterface** _ppContainer)
{
	UINT size = sizeof(oInterface*);
	HRESULT hr = const_cast<ID3D11DeviceChild*>(_pChild)->GetPrivateData(oWKPDID_oBackPointer, &size, _ppContainer);
	if (FAILED(hr))
		return oWinSetLastError(hr);
	(*_ppContainer)->Reference();
	return true;
}

D3D11_PRIMITIVE_TOPOLOGY oD3D11ToPrimitiveTopology(oGPU_PRIMITIVE_TYPE _Type)
{
	return D3D11_PRIMITIVE_TOPOLOGY(_Type);
}

uint oD3D11GetNumElements(D3D_PRIMITIVE_TOPOLOGY _PrimitiveTopology, uint _NumPrimitives)
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
		default: return _NumPrimitives * (_PrimitiveTopology-D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST+1);
	}
}

bool oD3D11GetFeatureLevel(const oVersion& _ShaderModel, D3D_FEATURE_LEVEL* _pLevel)
{
	*_pLevel = D3D_FEATURE_LEVEL_9_1;

	if (_ShaderModel == oVersion(3,0))
		*_pLevel = D3D_FEATURE_LEVEL_9_3;
	else if (_ShaderModel == oVersion(4,0))
		*_pLevel = D3D_FEATURE_LEVEL_10_0;
	else if (_ShaderModel == oVersion(4,1))
		*_pLevel = D3D_FEATURE_LEVEL_10_1;
	else if (_ShaderModel == oVersion(5,0))
		*_pLevel = D3D_FEATURE_LEVEL_11_0;

	return *_pLevel != D3D_FEATURE_LEVEL_9_1;
}

const char* oD3D11GetShaderProfile(D3D_FEATURE_LEVEL _Level, oGPU_PIPELINE_STAGE _Stage)
{
	static const char* sDX9Profiles[] = 
	{
		"vs_3_0",
		0,
		0,
		0,
		"ps_3_0",
		0,
	};

	static const char* sDX10Profiles[] = 
	{
		"vs_4_0",
		0,
		0,
		"gs_4_0",
		"ps_4_0",
		0,
	};

	static const char* sDX10_1Profiles[] = 
	{
		"vs_4_1",
		0,
		0,
		"gs_4_1",
		"ps_4_1",
		0,
	};

	static const char* sDX11Profiles[] = 
	{
		"vs_5_0",
		"hs_5_0",
		"ds_5_0",
		"gs_5_0",
		"ps_5_0",
		"cs_5_0",
	};

	const char** profiles = 0;
	switch (_Level)
	{
		case D3D_FEATURE_LEVEL_9_1:
		case D3D_FEATURE_LEVEL_9_2:
		case D3D_FEATURE_LEVEL_9_3:
			profiles = sDX9Profiles;
			break;
		case D3D_FEATURE_LEVEL_10_0:
			profiles = sDX10Profiles;
			break;
		case D3D_FEATURE_LEVEL_10_1:
			profiles = sDX10_1Profiles;
			break;
		case D3D_FEATURE_LEVEL_11_0:
			profiles = sDX11Profiles;
			break;
		oNODEFAULT;
	}

	const char* profile = profiles[_Stage];
	if (!profile)
	{
		oVersion ver = oD3D11GetFeatureVersion(_Level);
		oErrorSetLast(std::errc::not_supported, "Shader profile does not exist for D3D%d.%d's stage %s", ver.Major, ver.Minor, oStd::as_string(_Stage));
	}

	return profile;
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
		oStd::replace_all(tmp, "%", "%%");

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

		oStrcpy(_OutErrorMessageString, _SizeofOutErrorMessageString, tmp.c_str());
	}

	else
		*_OutErrorMessageString = 0;

	return true;
}

int oD3D11VTrace(ID3D11InfoQueue* _pInfoQueue, D3D11_MESSAGE_SEVERITY _Severity, const char* _Format, va_list _Args)
{
	oStd::xlstring buf;
	int len = oVPrintf(buf, _Format, _Args);
	if (len == oInvalid)
	{
		oErrorSetLast(std::errc::no_buffer_space, "message too long");
		return len;
	}

	_pInfoQueue->AddApplicationMessage(_Severity, buf);
	return len;
}

void oD3D11CheckBoundRTAndUAV(ID3D11DeviceContext* _pDeviceContext, int _NumBuffers, ID3D11Buffer** _ppBuffers)
{
	oRef<ID3D11UnorderedAccessView> UAVs[D3D11_PS_CS_UAV_REGISTER_COUNT];
	_pDeviceContext->OMGetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, D3D11_PS_CS_UAV_REGISTER_COUNT, (ID3D11UnorderedAccessView**)UAVs);
	for (int r = 0; r < D3D11_PS_CS_UAV_REGISTER_COUNT; r++)
	{
		if (UAVs[r])
		{
			for (int b = 0; b < _NumBuffers; b++)
			{
				oRef<ID3D11Resource> Resource;
				UAVs[r]->GetResource(&Resource);
				if (Resource && Resource == _ppBuffers[b])
					oD3D11Trace(_pDeviceContext, D3D11_MESSAGE_SEVERITY_ERROR, "The specified buffer in slot %d is bound using OMSetRenderTargetsAndUnorderedAccessViews slot %d. Behavior will be unexpected since the buffer may not be flushed for reading.", b, r);
			}
		}
	}
}

void oD3D11CheckBoundCSSetUAV(ID3D11DeviceContext* _pDeviceContext, int _NumBuffers, ID3D11Buffer** _ppBuffers)
{
	oRef<ID3D11UnorderedAccessView> UAVs[D3D11_PS_CS_UAV_REGISTER_COUNT];
	_pDeviceContext->CSGetUnorderedAccessViews(0, D3D11_PS_CS_UAV_REGISTER_COUNT, (ID3D11UnorderedAccessView**)UAVs);
	for (int r = 0; r < D3D11_PS_CS_UAV_REGISTER_COUNT; r++)
	{
		if (UAVs[r])
		{
			for (int b = 0; b < _NumBuffers; b++)
			{
				oRef<ID3D11Resource> Resource;
				UAVs[r]->GetResource(&Resource);
				if (Resource && Resource == _ppBuffers[b])
					oD3D11Trace(_pDeviceContext, D3D11_MESSAGE_SEVERITY_ERROR, "The specified buffer in slot %d is bound to CSSetUnorderedAccessViews slot %d. Behavior will be unexpected because the buffer may be bound for reading and writing at the same time.", b, r);
			}
		}
	}
}

template<typename DescT> static void FillNonDimensions(const DescT& _Desc, oGPU_TEXTURE_TYPE _BasicType, oGPU_TEXTURE_DESC* _pDesc)
{
	if (_Desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
		_BasicType = oGPU_TEXTURE_CUBE_MAP;

	_pDesc->Format = oDXGIToSurfaceFormat(_Desc.Format);

	_pDesc->Type = _BasicType;
	if (_Desc.MipLevels > 1)
		_pDesc->Type = oGPUTextureTypeGetMipMapType(_pDesc->Type);

	if (_Desc.Usage == D3D11_USAGE_STAGING)
		_pDesc->Type = oGPUTextureTypeGetReadbackType(_pDesc->Type);

	if (_Desc.BindFlags & (D3D11_BIND_RENDER_TARGET|D3D11_BIND_DEPTH_STENCIL))
		_pDesc->Type = oGPUTextureTypeGetRenderTargetType(_pDesc->Type);

	if (_Desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		oASSERT(_pDesc->Type == oGPU_TEXTURE_2D_MAP, "Invalid/unhandled type");
		_pDesc->Type = oGPU_TEXTURE_2D_MAP_UNORDERED;
	}
}

static UINT oD3D11GetCPUWriteFlags(D3D11_USAGE _Usage)
{
	switch (_Usage)
	{
		case D3D11_USAGE_DYNAMIC: return D3D11_CPU_ACCESS_WRITE;
		case D3D11_USAGE_STAGING: return D3D11_CPU_ACCESS_WRITE|D3D11_CPU_ACCESS_READ;
		default: return 0;
	}
}

static void oD3D11InitBufferDesc(UINT _BindFlags, D3D11_USAGE _Usage, uint _Size, uint _Count, bool _IsStructured, bool _IsDispatchable, D3D11_BUFFER_DESC* _pDesc)
{
	_pDesc->ByteWidth = _Size * _Count;
	_pDesc->Usage = _Usage;
	_pDesc->BindFlags = _BindFlags;
	_pDesc->StructureByteStride = _Size;
	_pDesc->CPUAccessFlags = oD3D11GetCPUWriteFlags(_pDesc->Usage);
	_pDesc->MiscFlags = 0;
	if (_IsStructured)
		_pDesc->MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	if (_IsDispatchable)
		_pDesc->MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS|D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
}

static void oD3D11InitValues(const oGPU_TEXTURE_DESC& _Desc, DXGI_FORMAT* _pFormat, D3D11_USAGE* _pUsage, UINT* _pCPUAccessFlags, UINT* _pBindFlags, UINT* _pMipLevels, UINT* _pMiscFlags = nullptr)
{
	DXGI_FORMAT DSVF, SRVF;
	oDXGIGetCompatibleFormats(oDXGIFromSurfaceFormat(_Desc.Format), _pFormat, &DSVF, &SRVF);
	if (*_pFormat == DXGI_FORMAT_UNKNOWN)
		*_pFormat = DXGI_FORMAT_FROM_FILE;

	*_pUsage = D3D11_USAGE_DEFAULT;
	if (oGPUTextureTypeIsReadback(_Desc.Type))
		*_pUsage = D3D11_USAGE_STAGING;

	if (_pMiscFlags)
		*_pMiscFlags = 0;

	*_pCPUAccessFlags = oD3D11GetCPUWriteFlags(*_pUsage);
	*_pMipLevels = oGPUTextureTypeHasMips(_Desc.Type) ? 0 : 1;

	if (_pMiscFlags && oGPUTextureTypeIsCubeMap(_Desc.Type))
			*_pMiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

	*_pBindFlags = 0;
	if (*_pUsage != D3D11_USAGE_STAGING)
	{
		*_pBindFlags = D3D11_BIND_SHADER_RESOURCE;
		if (oGPUTextureTypeIsRenderTarget(_Desc.Type))
		{
			*_pBindFlags |= D3D11_BIND_RENDER_TARGET;

			// D3D11_RESOURCE_MISC_GENERATE_MIPS is only valid for render targets.
			// It is up to client code to handle default textures and depth textures
			// rebound for sampling.
			if (_pMiscFlags && oGPUTextureTypeHasMips(_Desc.Type))
				*_pMiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
	}

	if (oDXGIIsDepthFormat(*_pFormat))
	{
		*_pBindFlags &=~ D3D11_BIND_RENDER_TARGET;
		*_pBindFlags |= D3D11_BIND_DEPTH_STENCIL;
	}

	if (oGPUTextureTypeIsUnordered(_Desc.Type))
		*_pBindFlags |= D3D11_BIND_UNORDERED_ACCESS;
}

static bool oD3D11DeviceIsMutingInfosOrStateCreation(ID3D11Device* _pDevice)
{
	oRef<ID3D11InfoQueue> InfoQueue;
	oV(_pDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&InfoQueue));
	SIZE_T size = 0;
	oV(InfoQueue->GetStorageFilter(nullptr, &size));
	D3D11_INFO_QUEUE_FILTER* f = (D3D11_INFO_QUEUE_FILTER*)alloca(size);
	oV(InfoQueue->GetStorageFilter(f, &size));
	for (uint i = 0 ; i < f->DenyList.NumSeverities; i++)
		if (f->DenyList.pSeverityList[i] == D3D11_MESSAGE_SEVERITY_INFO)
			return true;
	for (uint i = 0 ; i < f->DenyList.NumCategories; i++)
		if (f->DenyList.pCategoryList[i] == D3D11_MESSAGE_CATEGORY_STATE_CREATION)
			return true;
	return false;
}

#define oDEBUG_CHECK_BUFFER(fnName, ppOut) \
	if (FAILED(hr)) \
	{	return oWinSetLastError(oDEFAULT, #fnName " failed: "); \
	} \
	else if (_DebugName && *_DebugName) \
	{	oD3D11SetDebugName(*ppOut, _DebugName); \
		oRef<ID3D11Device> Dev; \
		(*ppOut)->GetDevice(&Dev); \
		if ((Dev->GetCreationFlags() & D3D11_CREATE_DEVICE_DEBUG) && !oD3D11DeviceIsMutingInfosOrStateCreation(Dev)) \
			oTRACEA("oD3D11: INFO: Name Buffer: Name=\"%s\", Addr=0x%p, ExtRef=1, IntRef=0 [ STATE_CREATION INFO #OOOii: NAME_BUFFER ]", _DebugName, *ppOut); \
	}

#define oCHECK_IS_TEXTURE(_pResource) do \
{	D3D11_RESOURCE_DIMENSION type; \
	_pResource->GetType(&type); \
	if (type != D3D11_RESOURCE_DIMENSION_TEXTURE1D && type != D3D11_RESOURCE_DIMENSION_TEXTURE2D && type != D3D11_RESOURCE_DIMENSION_TEXTURE3D) \
	{	oStd::mstring buf; \
		return oErrorSetLast(std::errc::invalid_argument, "Only textures types are currently supported. (resource %s)", oD3D11GetDebugName(buf, _pResource)); \
	} \
} while (false)

#ifdef _DEBUG
	#define oDEBUG_CHECK_SAME_DEVICE(_pContext, _pSrc, _pDst) do \
	{	oRef<ID3D11Device> Dev1, Dev2, Dev3; \
		_pContext->GetDevice(&Dev1); _pSrc->GetDevice(&Dev2); _pDst->GetDevice(&Dev2); \
		oASSERT(Dev1 == Dev2 && Dev2 == Dev3, "Context and resources are not from the same device"); \
	} while (false)
#else
	 #define oDEBUG_CHECK_SAME_DEVICE(_pContext, _pSrc, _pDst)
#endif

bool oD3D11BufferCreate(ID3D11Device* _pDevice, const char* _DebugName, const oGPU_BUFFER_DESC& _Desc, const void* _pInitBuffer, ID3D11Buffer** _ppBuffer, ID3D11UnorderedAccessView** _ppUAV, ID3D11ShaderResourceView** _ppSRV)
{
	D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
	UINT BindFlags = 0;
	oSURFACE_FORMAT Format = _Desc.Format;

	switch (_Desc.Type)
	{
		case oGPU_BUFFER_DEFAULT:
			BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			if (!oStd::byte_aligned(_Desc.StructByteSize, 16) || _Desc.StructByteSize > 65535)
				return oErrorSetLast(std::errc::invalid_argument, "A constant buffer must specify a StructByteSize that is 16-byte-aligned and <= 65536 bytes. (size %u bytes specified)", _Desc.StructByteSize);
			break;
		case oGPU_BUFFER_READBACK:
			Usage = D3D11_USAGE_STAGING;
			break;
		case oGPU_BUFFER_INDEX:
			if (_Desc.Format != oSURFACE_R16_UINT && _Desc.Format != oSURFACE_R32_UINT)
				return oErrorSetLast(std::errc::invalid_argument, "An index buffer must specify a format of oSURFACE_R16_UINT or oSURFACE_R32_UINT only (%s specified).", oStd::as_string(_Desc.Format));

			if (_Desc.StructByteSize != oInvalid && _Desc.StructByteSize != oUInt(oSurfaceFormatGetSize(_Desc.Format)))
				return oErrorSetLast(std::errc::invalid_argument, "An index buffer must specify StructByteSize properly, or set it to oInvalid.");

			BindFlags = D3D11_BIND_INDEX_BUFFER;
			break;
		case oGPU_BUFFER_INDEX_READBACK:
			Usage = D3D11_USAGE_STAGING;
			break;
		case oGPU_BUFFER_VERTEX:
			BindFlags = D3D11_BIND_VERTEX_BUFFER;
			break;
		case oGPU_BUFFER_VERTEX_READBACK:
			Usage = D3D11_USAGE_STAGING;
			break;
		case oGPU_BUFFER_UNORDERED_RAW:
			BindFlags = D3D11_BIND_UNORDERED_ACCESS;
			Format = oSURFACE_R32_TYPELESS;
			if (_Desc.StructByteSize != sizeof(uint))
				return oErrorSetLast(std::errc::invalid_argument, "A raw buffer must specify a StructByteSize of 4.");
			if (_Desc.ArraySize < 3)
				return oErrorSetLast(std::errc::invalid_argument, "A raw buffer must have at least 3 elements.");
			break;
		case oGPU_BUFFER_UNORDERED_UNSTRUCTURED:
			BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			if (Format == oSURFACE_UNKNOWN)
				return oErrorSetLast(std::errc::invalid_argument, "An unordered, unstructured buffer requires a valid surface format to be specified.");
			break;
		case oGPU_BUFFER_UNORDERED_STRUCTURED:
		case oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND:
		case oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER:
			BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			break;
		oNODEFAULT;
	}

	// @oooii-tony: why oh why do I need to do things this way? IBs/VBs seem fine,
	// but constant buffers seem affected by this.
	if (BindFlags != D3D11_BIND_INDEX_BUFFER && BindFlags != D3D11_BIND_VERTEX_BUFFER)
	{
		if (Usage == D3D11_USAGE_DEFAULT && D3D_FEATURE_LEVEL_11_0 > _pDevice->GetFeatureLevel())
			Usage = D3D11_USAGE_DYNAMIC;
	}

	uint ElementStride = _Desc.StructByteSize;
	if (ElementStride == oInvalid && Format != oSURFACE_UNKNOWN)
		ElementStride = oSurfaceFormatGetSize(Format);

	if (ElementStride == 0 || ElementStride == oInvalid)
		return oErrorSetLast(std::errc::invalid_argument, "A structured buffer requires a valid non-zero buffer size to be specified.");

	D3D11_BUFFER_DESC desc;
	oD3D11InitBufferDesc(BindFlags, Usage, ElementStride, _Desc.ArraySize, _Desc.Type >= oGPU_BUFFER_UNORDERED_STRUCTURED, _Desc.Type == oGPU_BUFFER_UNORDERED_RAW, &desc);

	D3D11_SUBRESOURCE_DATA SRD;
	SRD.pSysMem = _pInitBuffer;
	HRESULT hr = _pDevice->CreateBuffer(&desc, _pInitBuffer ? &SRD : nullptr, _ppBuffer);
	oDEBUG_CHECK_BUFFER(oD3D11CreateBuffer, _ppBuffer);
	
	// Add this mainly for index buffers so they can describe their own 
	// StructureByteStride.
	// @oooii-tony: Is this becoming defunct? This is meant so that D3D11 objects 
	// can be self-describing, but with a clean and not-D3D11 oGPU_BUFFER_DESC, 
	// does that hold all the info needed and we just ensure it always gets 
	// populated as expected (unlike D3D11's StructByteSize)? Probably, but this
	// needs to stick around a bit longer until it can truly be orphaned.
	oGPU_BUFFER_DESC d(_Desc);
	d.StructByteSize = ElementStride;
	oVERIFY(oD3D11BufferSetDesc(*_ppBuffer, d));

	if (_Desc.Type >= oGPU_BUFFER_UNORDERED_RAW)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC UAVD;
		UAVD.Format = oDXGIFromSurfaceFormat(Format);
		UAVD.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		UAVD.Buffer.FirstElement = 0;
		UAVD.Buffer.NumElements = _Desc.ArraySize;
		UAVD.Buffer.Flags = 0;

		switch (_Desc.Type)
		{
			case oGPU_BUFFER_UNORDERED_RAW:
				UAVD.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
				break;
			case oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND:
				UAVD.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
				break;
			case oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER:
				UAVD.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
				break;
			default:
				break;
		}

		hr = _pDevice->CreateUnorderedAccessView(*_ppBuffer, &UAVD, _ppUAV);
		oDEBUG_CHECK_BUFFER(oD3D11CreateConstantBuffer, _ppUAV);

		if( _Desc.Type >= oGPU_BUFFER_UNORDERED_STRUCTURED)
		{
			hr = _pDevice->CreateShaderResourceView(*_ppBuffer, nullptr, _ppSRV);
			oDEBUG_CHECK_BUFFER(oD3D11CreateConstantBuffer, _ppUAV);
		}
	}
	
	else if (_ppUAV)
		*_ppUAV = nullptr;

	return true;
}

bool oD3D11CreateUnflaggedUAV(ID3D11UnorderedAccessView* _pSourceUAV, ID3D11UnorderedAccessView** _ppUnflaggedUAV)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVD;
	_pSourceUAV->GetDesc(&UAVD);
	if (UAVD.ViewDimension == D3D11_UAV_DIMENSION_BUFFER)
	{
		oRef<ID3D11Resource> Resource;
		_pSourceUAV->GetResource(&Resource);
		oRef<ID3D11Device> Device;
		Resource->GetDevice(&Device);

		UAVD.Buffer.Flags = 0;
		oVB_RETURN2(Device->CreateUnorderedAccessView(Resource, &UAVD, _ppUnflaggedUAV));

		return true;
	}
	
	return oErrorSetLast(std::errc::invalid_argument, "Only D3D11_UAV_DIMENSION_BUFFER views supported");
}

bool oD3D11CopyTo(ID3D11Resource* _pTexture, uint _Subresource, void* _pDestination, uint _DestinationRowPitch, bool _FlipVertically)
{
	if (!_pTexture)
		return oErrorSetLast(std::errc::invalid_argument);

	oCHECK_IS_TEXTURE(_pTexture);

	oRef<ID3D11Device> D3DDevice;
	_pTexture->GetDevice(&D3DDevice);
	oRef<ID3D11DeviceContext> D3DDeviceContext;
	D3DDevice->GetImmediateContext(&D3DDeviceContext);

	oGPU_TEXTURE_DESC desc;
	oD3D11GetTextureDesc(_pTexture, &desc);

	if (!oGPUTextureTypeIsReadback(desc.Type))
	{
		oStd::mstring buf;
		return oErrorSetLast(std::errc::invalid_argument, "The specified texture %s does not have CPU read access", oD3D11GetDebugName(buf, _pTexture));
	}

	D3D11_MAPPED_SUBRESOURCE source;
	HRESULT hr = D3DDeviceContext->Map(_pTexture, _Subresource, D3D11_MAP_READ, 0, &source);
	if (FAILED(hr))
		return oWinSetLastError(hr);

	int2 ByteDimensions = oSurfaceMipCalcByteDimensions(desc.Format, desc.Dimensions);
	oStd::memcpy2d(_pDestination, _DestinationRowPitch, source.pData, source.RowPitch, ByteDimensions.x, ByteDimensions.y, _FlipVertically);
	D3DDeviceContext->Unmap(_pTexture, _Subresource);
	return true;
}

void oD3D11UpdateSubresource(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pDstResource, UINT _DstSubresource, const D3D11_BOX* _pDstBox, const void* _pSrcData, uint _SrcRowPitch, uint _SrcDepthPitch)
{
	D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
	oGPU_TEXTURE_DESC d;
	oD3D11GetTextureDesc(_pDstResource, &d, &Usage);

	switch (Usage)
	{
		case D3D11_USAGE_DEFAULT:
			_pDeviceContext->UpdateSubresource(_pDstResource, _DstSubresource, _pDstBox, _pSrcData, _SrcRowPitch, _SrcDepthPitch);
			break;
		case D3D11_USAGE_STAGING:
		case D3D11_USAGE_DYNAMIC:
		{
			D3D11_RESOURCE_DIMENSION type;
			_pDstResource->GetType(&type);

			int2 ByteDimensions;
			if (type == D3D11_RESOURCE_DIMENSION_BUFFER)
				ByteDimensions = d.Dimensions.xy();
			else
				ByteDimensions = oSurfaceMipCalcByteDimensions(d.Format, d.Dimensions);

			D3D11_MAPPED_SUBRESOURCE msr;
			_pDeviceContext->Map(_pDstResource, _DstSubresource, D3D11_MAP_WRITE_DISCARD, 0, &msr);
			oStd::memcpy2d(msr.pData, msr.RowPitch, _pSrcData, _SrcRowPitch, ByteDimensions.x, ByteDimensions.y);
			_pDeviceContext->Unmap(_pDstResource, _DstSubresource);
			break;
		}
		oNODEFAULT;
	}
}

void oD3D11MapWriteDiscard(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pResource, uint _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedResource)
{
	D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
	oGPU_TEXTURE_DESC d;
	oD3D11GetTextureDesc(_pResource, &d, &Usage);

	switch (Usage)
	{
		case D3D11_USAGE_DEFAULT:
		{
			int2 ByteDimensions = oSurfaceMipCalcByteDimensions(d.Format, d.Dimensions);
			_pMappedResource->DepthPitch = ByteDimensions.x * ByteDimensions.y;
			_pMappedResource->RowPitch = ByteDimensions.x;
			_pMappedResource->pData = new char[_pMappedResource->DepthPitch];
			break;
		}
		
		case D3D11_USAGE_STAGING:
		case D3D11_USAGE_DYNAMIC:
			_pDeviceContext->Map(_pResource, _Subresource, D3D11_MAP_WRITE_DISCARD, 0, (D3D11_MAPPED_SUBRESOURCE*)_pMappedResource);
			break;

		oNODEFAULT;
	}
}

void oD3D11Unmap(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pResource, uint _Subresource, oSURFACE_MAPPED_SUBRESOURCE& _MappedResource)
{
	D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
	oGPU_TEXTURE_DESC d;
	oD3D11GetTextureDesc(_pResource, &d, &Usage);
	switch (Usage)
	{
		case D3D11_USAGE_DEFAULT:
		{
			delete [] _MappedResource.pData;
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

	oSURFACE_MAPPED_SUBRESOURCE msr;
	oD3D11MapWriteDiscard(_pDeviceContext, _pIndexBuffer, 0, &msr);

	if (d.StructByteSize == sizeof(T))
		memcpy(msr.pData, _pSourceIndices, d.ArraySize * d.StructByteSize);

	if (d.StructByteSize == 2)
	{
		oASSERT(sizeof(T) == 4, "");
		oStd::memcpyuitous((ushort*)msr.pData, (const uint*)_pSourceIndices, d.ArraySize);
	}

	else
	{
		oASSERT(sizeof(T) == 2 && d.StructByteSize == 4, "");
		oStd::memcpyustoui((uint*)msr.pData, (const ushort*)_pSourceIndices, d.ArraySize);
	}

	oD3D11Unmap(_pDeviceContext, _pIndexBuffer, 0, msr);
}

void oD3D11UpdateIndexBuffer(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pIndexBuffer, const uint* _pSourceIndices)
{
	oD3D11UpdateIndexBuffer<uint>(_pDeviceContext, _pIndexBuffer, _pSourceIndices);
}

void oD3D11UpdateIndexBuffer(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pIndexBuffer, const ushort* _pSourceIndices)
{
	oD3D11UpdateIndexBuffer<ushort>(_pDeviceContext, _pIndexBuffer, _pSourceIndices);
}

void oD3D11CopySubresource2D(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pDstResource, uint _DstSubresource, ID3D11Resource* _pSrcResource, uint _SrcSubresource)
{
	oDEBUG_CHECK_SAME_DEVICE(_pDeviceContext, _pDstResource, _pSrcResource);
	_pDeviceContext->CopySubresourceRegion(_pDstResource, _DstSubresource, 0, 0, 0, _pSrcResource, _SrcSubresource, nullptr);
}

void oD3D11CopySubresourceRegion2D(ID3D11DeviceContext* _pDeviceContext, ID3D11Resource* _pDstResource, uint _DstSubresource, const int2& _DstTopLeft, ID3D11Resource* _pSrcResource, uint _SrcSubresource, const int2& _SrcTopLeft, const int2& _NumPixels)
{
	oDEBUG_CHECK_SAME_DEVICE(_pDeviceContext, _pDstResource, _pSrcResource);
	D3D11_BOX SrcBox;
	SrcBox.left = _SrcTopLeft.x;
	SrcBox.top = _SrcTopLeft.y;
	SrcBox.front = 0;
	SrcBox.right = _SrcTopLeft.x + _NumPixels.x;
	SrcBox.bottom = _SrcTopLeft.y + _NumPixels.y;
	SrcBox.back = 1;
	_pDeviceContext->CopySubresourceRegion(_pDstResource, _DstSubresource, _DstTopLeft.x, _DstTopLeft.y, 0, _pSrcResource, _SrcSubresource, &SrcBox);
}

void oD3D11GetTextureDesc(ID3D11Resource* _pResource, oGPU_TEXTURE_DESC* _pDesc, D3D11_USAGE* _pUsage)
{
	D3D11_RESOURCE_DIMENSION type = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	_pResource->GetType(&type);
	switch (type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			D3D11_TEXTURE1D_DESC desc;
			static_cast<ID3D11Texture1D*>(_pResource)->GetDesc(&desc);
			_pDesc->Dimensions = int3(oInt(desc.Width), 1, 1);
			_pDesc->ArraySize = oInt(desc.ArraySize);
			FillNonDimensions(desc, oGPU_TEXTURE_2D_MAP, _pDesc);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			D3D11_TEXTURE2D_DESC desc;
			static_cast<ID3D11Texture2D*>(_pResource)->GetDesc(&desc);
			_pDesc->Dimensions = int3(oInt(desc.Width), oInt(desc.Height), 1);
			_pDesc->ArraySize = oInt(desc.ArraySize);
			FillNonDimensions(desc, oGPU_TEXTURE_2D_MAP, _pDesc);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			static_cast<ID3D11Texture3D*>(_pResource)->GetDesc(&desc);
			_pDesc->Dimensions = int3(oInt(desc.Width), oInt(desc.Height), oInt(desc.Depth));
			_pDesc->ArraySize = 1;
			FillNonDimensions(desc, oGPU_TEXTURE_3D_MAP, _pDesc);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			oGPU_BUFFER_DESC d;
			oVERIFY(oD3D11BufferGetDesc(_pResource, &d));

			D3D11_BUFFER_DESC desc;
			static_cast<ID3D11Buffer*>(_pResource)->GetDesc(&desc);
			_pDesc->Dimensions = int3(oInt(d.StructByteSize), oInt(d.ArraySize), 1);
			_pDesc->ArraySize = oInt(d.ArraySize);
			_pDesc->Format = d.Format;
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		};

		oNODEFAULT;
	}
}

static void oD3D11InitSRVDesc(const oGPU_TEXTURE_DESC& _Desc, D3D11_RESOURCE_DIMENSION _Type, D3D11_SHADER_RESOURCE_VIEW_DESC* _pSRVDesc)
{
	DXGI_FORMAT TF, DSVF;
	oDXGIGetCompatibleFormats(oDXGIFromSurfaceFormat(_Desc.Format), &TF, &DSVF, &_pSRVDesc->Format);

	// All texture share basically the same memory footprint, so just write once
	_pSRVDesc->Texture2DArray.MostDetailedMip = 0;
	_pSRVDesc->Texture2DArray.MipLevels = oSurfaceCalcNumMips(oGPUTextureTypeHasMips(_Desc.Type), _Desc.Dimensions);
	_pSRVDesc->Texture2DArray.FirstArraySlice = 0;
	_pSRVDesc->Texture2DArray.ArraySize = _Desc.ArraySize;

	switch (_Type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			_pSRVDesc->ViewDimension = oGPUTextureTypeIsArray(_Desc.Type) ? D3D11_SRV_DIMENSION_TEXTURE1DARRAY : D3D11_SRV_DIMENSION_TEXTURE1D;
			break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			if (oGPUTextureTypeIsCubeMap(_Desc.Type))
				_pSRVDesc->ViewDimension = oGPUTextureTypeIsArray(_Desc.Type) ? D3D11_SRV_DIMENSION_TEXTURECUBEARRAY : D3D11_SRV_DIMENSION_TEXTURECUBE;
			else
				_pSRVDesc->ViewDimension = oGPUTextureTypeIsArray(_Desc.Type) ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D;
			break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			_pSRVDesc->ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			break;
		oNODEFAULT;
	}
}

bool oD3D11CreateShaderResourceView(const char* _DebugName, ID3D11Resource* _pTexture, ID3D11ShaderResourceView** _ppShaderResourceView)
{
	// If a depth-stencil resource is specified we have to convert the specified 
	// format to a compatible color one
	D3D11_SHADER_RESOURCE_VIEW_DESC srv;
	D3D11_SHADER_RESOURCE_VIEW_DESC* pSRV = nullptr;
	oGPU_TEXTURE_DESC desc;
	oD3D11GetTextureDesc(_pTexture, &desc);
	if (oSurfaceFormatIsDepth(desc.Format))
	{
		D3D11_RESOURCE_DIMENSION type;
		_pTexture->GetType(&type);
		oD3D11InitSRVDesc(desc, type, &srv);
		pSRV = &srv;
	}

	oRef<ID3D11Device> D3DDevice;
	_pTexture->GetDevice(&D3DDevice);
	HRESULT hr = D3DDevice->CreateShaderResourceView(_pTexture, pSRV, _ppShaderResourceView);
	oDEBUG_CHECK_BUFFER(oD3D11CreateShaderResourceView, _ppShaderResourceView);
	return true;
}

static void oD3D11InitDSVDesc(const oGPU_TEXTURE_DESC& _Desc, D3D11_RESOURCE_DIMENSION _Type, D3D11_DEPTH_STENCIL_VIEW_DESC* _pDSVDesc)
{
	oASSERT(_Type == D3D11_RESOURCE_DIMENSION_TEXTURE2D, "Unsupported resource dimension (%s)", oStd::as_string(_Type));
	DXGI_FORMAT TF, SRVF;
	oDXGIGetCompatibleFormats(oDXGIFromSurfaceFormat(_Desc.Format), &TF, &_pDSVDesc->Format, &SRVF);
	_pDSVDesc->ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	_pDSVDesc->Texture2D.MipSlice = 0;
	_pDSVDesc->Flags = 0;
}

bool oD3D11CreateRenderTargetView(const char* _DebugName, ID3D11Resource* _pTexture, ID3D11View** _ppView)
{
	oRef<ID3D11Device> D3DDevice;
	_pTexture->GetDevice(&D3DDevice);
	HRESULT hr = S_OK;
	oGPU_TEXTURE_DESC desc;
	oD3D11GetTextureDesc(_pTexture, &desc);
	if (oSurfaceFormatIsDepth(desc.Format))
	{
		D3D11_RESOURCE_DIMENSION type;
		_pTexture->GetType(&type);
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv;
		oD3D11InitDSVDesc(desc, type, &dsv);
		hr = D3DDevice->CreateDepthStencilView(_pTexture, &dsv, (ID3D11DepthStencilView**)_ppView);
	}
	else
		hr = D3DDevice->CreateRenderTargetView(_pTexture, nullptr, (ID3D11RenderTargetView**)_ppView);

	oDEBUG_CHECK_BUFFER(oD3D11CreateRenderTargetView, _ppView);
	return true;
}

bool oD3D11CreateUnorderedAccessView(const char* _DebugName, ID3D11Resource* _pTexture, uint _MipSlice, uint _ArraySlice, ID3D11UnorderedAccessView** _ppUnorderedAccessView)
{
	oRef<ID3D11Device> D3DDevice;
	_pTexture->GetDevice(&D3DDevice);
	oGPU_TEXTURE_DESC desc;
	oD3D11GetTextureDesc(_pTexture, &desc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVD;
	UAVD.Format = oDXGIFromSurfaceFormat(desc.Format);

	switch (desc.Type)
	{
		// @oooii-tony: When adding more cases to this, try to use oGPU_TEXTURE_DESC's
		// ArraySize.

		case oGPU_TEXTURE_2D_MAP_UNORDERED:
			UAVD.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			UAVD.Texture2D.MipSlice = 0;
			break;
		default:
			return oErrorSetLast(std::errc::invalid_argument, "Invalid texture type %s specified for UAV creation", oStd::as_string(desc.Type));
	}

	HRESULT hr = D3DDevice->CreateUnorderedAccessView(_pTexture, &UAVD, _ppUnorderedAccessView);
	oDEBUG_CHECK_BUFFER(oD3D11CreateUnorderedAccessView, _ppUnorderedAccessView);
	return true;
}

bool oD3D11CreateTexture(ID3D11Device* _pDevice, const char* _DebugName, const oGPU_TEXTURE_DESC& _Desc, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pInitData, ID3D11Resource** _ppTexture, ID3D11ShaderResourceView** _ppShaderResourceView, ID3D11View** _ppRenderTargetView)
{
	bool IsShaderResource = false;
	HRESULT hr = S_OK;
	switch (oGPUTextureTypeGetBasicType(_Desc.Type))
	{
		case oGPU_TEXTURE_1D_MAP:
		{
			D3D11_TEXTURE1D_DESC desc;
			desc.Width = _Desc.Dimensions.x;
			desc.ArraySize = __max(1, _Desc.ArraySize);
			oD3D11InitValues(_Desc, &desc.Format, &desc.Usage, &desc.CPUAccessFlags, &desc.BindFlags, &desc.MipLevels, &desc.MiscFlags);
			IsShaderResource = !!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);
			hr = _pDevice->CreateTexture1D(&desc, (D3D11_SUBRESOURCE_DATA*)_pInitData, (ID3D11Texture1D**)_ppTexture);
			break;
		}

		case oGPU_TEXTURE_2D_MAP:
		case oGPU_TEXTURE_CUBE_MAP:
		{
			oASSERT(!oGPUTextureTypeIsCubeMap(_Desc.Type) || _Desc.ArraySize == 6, "Cube maps must have ArraySize == 6, ArraySize=%d specified", _Desc.ArraySize);

			D3D11_TEXTURE2D_DESC desc;
			desc.Width = _Desc.Dimensions.x;
			desc.Height = _Desc.Dimensions.y;
			desc.ArraySize = __max(1, _Desc.ArraySize);
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			oD3D11InitValues(_Desc, &desc.Format, &desc.Usage, &desc.CPUAccessFlags, &desc.BindFlags, &desc.MipLevels, &desc.MiscFlags);
			IsShaderResource = !!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);
			hr = _pDevice->CreateTexture2D(&desc, (D3D11_SUBRESOURCE_DATA*)_pInitData, (ID3D11Texture2D**)_ppTexture);
			break;
		}

		case oGPU_TEXTURE_3D_MAP:
		{
			oASSERT(_Desc.ArraySize == 1, "3d textures don't support slices, ArraySize=%d specified", _Desc.ArraySize);

			D3D11_TEXTURE3D_DESC desc;
			desc.Width = _Desc.Dimensions.x;
			desc.Height = _Desc.Dimensions.y;
			desc.Depth = _Desc.Dimensions.z;
			oD3D11InitValues(_Desc, &desc.Format, &desc.Usage, &desc.CPUAccessFlags, &desc.BindFlags, &desc.MipLevels, &desc.MiscFlags);
			IsShaderResource = !!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);
			hr = _pDevice->CreateTexture3D(&desc, (D3D11_SUBRESOURCE_DATA*)_pInitData, (ID3D11Texture3D**)_ppTexture);
			break;
		}
		oNODEFAULT;
	}

	oDEBUG_CHECK_BUFFER(oD3D11CreateTexture, _ppTexture);
	if (_ppShaderResourceView)
	{
		if (!IsShaderResource)
			*_ppShaderResourceView = nullptr;
		else if (!oD3D11CreateShaderResourceView(_DebugName, *_ppTexture, _ppShaderResourceView))
			return false;
	}

	if (_ppRenderTargetView && !oD3D11CreateRenderTargetView(_DebugName, *_ppTexture, _ppRenderTargetView))
	{
		(*_ppTexture)->Release();
		*_ppTexture = nullptr;

		if (_ppShaderResourceView && *_ppShaderResourceView)
		{
			(*_ppShaderResourceView)->Release();
			*_ppShaderResourceView = nullptr;
		}

		return oErrorSetLast(std::errc::invalid_argument, "oD3D11CreateRenderTargetView failed, check DX debug output");
	}

	return true;
}

bool oD3D11CreateCPUCopy(ID3D11Resource* _pResource, ID3D11Resource** _ppCPUCopy)
{
	if (!_pResource || !_ppCPUCopy)
		return oErrorSetLast(std::errc::invalid_argument);

	oRef<ID3D11Device> D3D11Device;
	_pResource->GetDevice(&D3D11Device);
	oRef<ID3D11DeviceContext> D3D11DeviceContext;
	D3D11Device->GetImmediateContext(&D3D11DeviceContext);

	oStd::lstring RTName;
	oD3D11GetDebugName(RTName, _pResource);

	oStd::lstring copyName;
	oPrintf(copyName, "%s.CPUCopy", RTName.c_str());

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
			oV(D3D11Device->CreateBuffer(&d, nullptr, (ID3D11Buffer**)_ppCPUCopy));
			oD3D11SetDebugName(*_ppCPUCopy, copyName);
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			oGPU_TEXTURE_DESC d;
			oD3D11GetTextureDesc(_pResource, &d);
			d.Type = oGPUTextureTypeGetReadbackType(d.Type);
			if (!oD3D11CreateTexture(D3D11Device, copyName, d, nullptr, _ppCPUCopy))
				return false; // pass through error

			break;
		}

		default:
			return oErrorSetLast(std::errc::invalid_argument, "Unknown resource type");
	}

	oGPU_BUFFER_DESC d;
	if (oD3D11BufferGetDesc(_pResource, &d))
		oV(oD3D11BufferSetDesc(*_ppCPUCopy, d));
	
	D3D11DeviceContext->CopyResource(*_ppCPUCopy, _pResource);
	D3D11DeviceContext->Flush();
	return true;
}

bool oD3D11CreateSnapshot(ID3D11Texture2D* _pRenderTarget, interface oImage** _ppImage)
{
	if (!_pRenderTarget || !_ppImage)
		return oErrorSetLast(std::errc::invalid_argument);

	oRef<ID3D11Texture2D> CPUTexture;
	if (!oD3D11CreateCPUCopy(_pRenderTarget, &CPUTexture))
		return false; // pass through error

	D3D11_TEXTURE2D_DESC d;
	CPUTexture->GetDesc(&d);

	oImage::FORMAT ImageFormat = oImageFormatFromSurfaceFormat(oDXGIToSurfaceFormat(d.Format));
	if (ImageFormat == oImage::UNKNOWN)
		return oErrorSetLast(std::errc::invalid_argument, "The specified texture's format %s is not supported by oImage", oStd::as_string(d.Format));

	oImage::DESC idesc;
	idesc.RowPitch = oImageCalcRowPitch(ImageFormat, d.Width);
	idesc.Dimensions.x = d.Width;
	idesc.Dimensions.y = d.Height;
	idesc.Format = oImage::BGRA32;
	oVERIFY(oImageCreate("Temp Image", idesc, _ppImage));
	return oD3D11CopyTo(CPUTexture, 0, (*_ppImage)->GetData(), idesc.RowPitch); // pass error through
}

bool oD3D11CreateSnapshot(ID3D11Texture2D* _pRenderTarget, D3DX11_IMAGE_FILE_FORMAT _Format, const char* _Path)
{
	if (!_pRenderTarget || !oSTRVALID(_Path))
		return oErrorSetLast(std::errc::invalid_argument);

	oRef<ID3D11Texture2D> D3DTexture;
	if (!oD3D11CreateCPUCopy(_pRenderTarget, &D3DTexture))
		return false; // pass through error

	oRef<ID3D11Device> D3DDevice;
	D3DTexture->GetDevice(&D3DDevice);
	oRef<ID3D11DeviceContext> D3DImmediateContext;
	D3DDevice->GetImmediateContext(&D3DImmediateContext);

	oV(oD3DX11::Singleton()->D3DX11SaveTextureToFileA(D3DImmediateContext, D3DTexture, _Format, _Path));
	return true;
}

static bool oD3D11Save_PrepareCPUCopy(ID3D11Resource* _pTexture, D3DX11_IMAGE_FILE_FORMAT _Format, ID3D11DeviceContext** _ppDeviceContext, ID3D11Resource** _ppCPUResource)
{
	oRef<ID3D11Device> D3DDevice;
	_pTexture->GetDevice(&D3DDevice);
	D3DDevice->GetImmediateContext(_ppDeviceContext);

	oGPU_TEXTURE_DESC desc;
	oD3D11GetTextureDesc(_pTexture, &desc);
	if (oSurfaceFormatIsBlockCompressed(desc.Format) && _Format != D3DX11_IFF_DDS)
		return oErrorSetLast(std::errc::invalid_argument, "D3DX11 can save block compressed formats only to .dds files.");

	if (oGPUTextureTypeIsReadback(desc.Type))
	{
		_pTexture->AddRef();
		*_ppCPUResource = _pTexture;
	}
	
	else if (!oD3D11CreateCPUCopy(_pTexture, _ppCPUResource))
	{
		oStd::mstring buf;
		return oErrorSetLast(std::errc::invalid_argument, "The specified texture \"%s\" is not CPU-accessible and a copy could not be made", oD3D11GetDebugName(buf, _pTexture));
	}

	return true;
}

static bool oD3D11Save_PrepareCPUCopy(const oImage* _pImage, D3DX11_IMAGE_FILE_FORMAT _Format, ID3D11Resource** _ppCPUResource)
{
	oRef<ID3D11Device> D3DDevice;
	if (!oD3D11CreateDevice(oGPU_DEVICE_INIT("oD3D11Save.Temp"), true, &D3DDevice))
		return false; // pass through error

	oImage::DESC idesc;
	_pImage->GetDesc(&idesc);

	oGPU_TEXTURE_DESC desc;
	desc.Dimensions = int3(idesc.Dimensions, 1);
	desc.Format = oImageFormatToSurfaceFormat(idesc.Format);
	desc.ArraySize = 1;
	desc.Type = oGPU_TEXTURE_2D_READBACK;

	if (desc.Format == oSURFACE_UNKNOWN)
		return oErrorSetLast(std::errc::invalid_argument, "Image format %s cannot be saved", oStd::as_string(idesc.Format));

	oSURFACE_CONST_MAPPED_SUBRESOURCE msr;
	msr.pData = _pImage->GetData();
	msr.RowPitch = idesc.RowPitch;
	msr.DepthPitch = oImageCalcSize(idesc.Format, idesc.Dimensions);

	if (!oD3D11CreateTexture(D3DDevice, "oD3D11Save.Temp", desc, &msr, _ppCPUResource, nullptr))
		return false; // pass through error

	return true;
}

static void oD3D11GetImageLoadInfo(const oGPU_TEXTURE_DESC& _Desc, D3DX11_IMAGE_LOAD_INFO* _pImageLoadInfo)
{
	_pImageLoadInfo->Width = _Desc.Dimensions.x <= 0 ? D3DX11_DEFAULT : _Desc.Dimensions.x;
	_pImageLoadInfo->Height = _Desc.Dimensions.y <= 0 ? D3DX11_DEFAULT : _Desc.Dimensions.y;
	_pImageLoadInfo->Depth = _Desc.ArraySize <= 0 ? D3DX11_DEFAULT : _Desc.ArraySize;
	_pImageLoadInfo->FirstMipLevel = D3DX11_DEFAULT;
	_pImageLoadInfo->Filter = D3DX11_DEFAULT;
	_pImageLoadInfo->MipFilter = D3DX11_DEFAULT;
	_pImageLoadInfo->pSrcInfo = nullptr;
	oD3D11InitValues(_Desc, &_pImageLoadInfo->Format, &_pImageLoadInfo->Usage, &_pImageLoadInfo->CpuAccessFlags, &_pImageLoadInfo->BindFlags, &_pImageLoadInfo->MipLevels);
}

bool oD3D11Save(ID3D11Resource* _pTexture, D3DX11_IMAGE_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer)
{
	if (!_pTexture || !_pBuffer || !_SizeofBuffer)
		return oErrorSetLast(std::errc::invalid_argument);

	oRef<ID3D11Resource> CPUCopy;
	oRef<ID3D11DeviceContext> D3DImmediateContext;
	if (!oD3D11Save_PrepareCPUCopy(_pTexture, _Format, &D3DImmediateContext, &CPUCopy))
		return false; // pass through error

	oRef<ID3D10Blob> Blob;
	oV(oD3DX11::Singleton()->D3DX11SaveTextureToMemory(D3DImmediateContext, CPUCopy, _Format, &Blob, 0));

	if (Blob->GetBufferSize() > _SizeofBuffer)
		return oErrorSetLast(std::errc::no_buffer_space, "Buffer is too small for image");

	memcpy(_pBuffer, Blob->GetBufferPointer(), Blob->GetBufferSize());
	return true;
}

bool oD3D11Save(const oImage* _pImage, D3DX11_IMAGE_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer)
{
	oRef<ID3D11Device> D3DDevice;
	if (!oD3D11CreateDevice(oGPU_DEVICE_INIT("oD3D11Save Temp Device"), true, &D3DDevice))
		return false; // pass through error

	oRef<ID3D11Resource> CPUCopy;
	if (!oD3D11Save_PrepareCPUCopy(_pImage, _Format, &CPUCopy))
		return false; // pass through error

	return oD3D11Save(CPUCopy, _Format, _pBuffer, _SizeofBuffer); // pass through error
}

bool oD3D11Save(ID3D11Resource* _pTexture, D3DX11_IMAGE_FILE_FORMAT _Format, const char* _Path)
{
	if (!_pTexture || !oSTRVALID(_Path))
		return oErrorSetLast(std::errc::invalid_argument);

	oRef<ID3D11Resource> CPUCopy;
	oRef<ID3D11DeviceContext> D3DImmediateContext;
	if (!oD3D11Save_PrepareCPUCopy(_pTexture, _Format, &D3DImmediateContext, &CPUCopy))
		return false; // pass through error

	if (!oFileEnsureParentFolderExists(_Path))
		return false; // pass through error

	oV(D3DX11SaveTextureToFileA(D3DImmediateContext, CPUCopy, _Format, _Path));
	return true;
}

bool oD3D11Save(const oImage* _pImage, D3DX11_IMAGE_FILE_FORMAT _Format, const char* _Path)
{
	oRef<ID3D11Resource> CPUCopy;
	if (!oD3D11Save_PrepareCPUCopy(_pImage, _Format, &CPUCopy))
		return false; // pass through error

	return oD3D11Save(CPUCopy, _Format, _Path); // pass through error
}

bool oD3D11Load(ID3D11Device* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _Path, const char* _DebugName, ID3D11Resource** _ppTexture)
{
	D3DX11_IMAGE_LOAD_INFO li;
	oD3D11GetImageLoadInfo(_Desc, &li);
	HRESULT hr = oD3DX11::Singleton()->D3DX11CreateTextureFromFile(_pDevice
		, _Path
		, &li
		, nullptr
		, _ppTexture
		, nullptr);
	if (FAILED(hr))
		return oWinSetLastError(hr);
	oVB(oD3D11SetDebugName(*_ppTexture, _DebugName));
	return true;
}

bool oD3D11Load(ID3D11Device* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _DebugName, const void* _pBuffer, size_t _SizeofBuffer, ID3D11Resource** _ppTexture)
{
	D3DX11_IMAGE_LOAD_INFO li;
	oD3D11GetImageLoadInfo(_Desc, &li);
	HRESULT hr = oD3DX11::Singleton()->D3DX11CreateTextureFromMemory(_pDevice
		, _pBuffer
		, _SizeofBuffer
		, &li
		, nullptr
		, _ppTexture
		, nullptr);
	if (FAILED(hr))
		return oWinSetLastError(hr);
	oVB(oD3D11SetDebugName(*_ppTexture, _DebugName));
	return true;
}

bool oD3D11Convert(ID3D11Texture2D* _pSourceTexture, oSURFACE_FORMAT _NewFormat, ID3D11Texture2D** _ppDestinationTexture)
{
	if (!_pSourceTexture || !_ppDestinationTexture)
		return oErrorSetLast(std::errc::invalid_argument);

	D3D11_TEXTURE2D_DESC desc;
	_pSourceTexture->GetDesc(&desc);

	if (_NewFormat == DXGI_FORMAT_BC7_UNORM)
	{
		if (oD3D11EncodeBC7(_pSourceTexture, true, _ppDestinationTexture))
			return true;
		oTRACE("GPU BC7 encode failed, falling back to CPU encode (may take a while)...");
	}

	else if (_NewFormat == DXGI_FORMAT_BC6H_SF16)
	{
		if (oD3D11EncodeBC6HS(_pSourceTexture, true, _ppDestinationTexture))
			return true;
		oTRACE("GPU BC6HS encode failed, falling back to CPU encode (may take a while)...");
	}

	else if (_NewFormat == DXGI_FORMAT_BC6H_UF16)
	{
		if (oD3D11EncodeBC6HU(_pSourceTexture, true, _ppDestinationTexture))
			return true;
		oTRACE("GPU BC6HU encode failed, falling back to CPU encode (may take a while)...");
	}

	else if (desc.Format == DXGI_FORMAT_BC6H_SF16 || desc.Format == DXGI_FORMAT_BC6H_UF16 || desc.Format == DXGI_FORMAT_BC7_UNORM)
	{
		// Decode requires a CPU-accessible source because CS4x can't sample from
		// BC7 or BC6, so make a copy if needed

		oRef<ID3D11Texture2D> CPUAccessible;
		if (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)
			CPUAccessible = _pSourceTexture;
		else if (!oD3D11CreateCPUCopy(_pSourceTexture, &CPUAccessible))
		{
			oStd::mstring buf;
			return oErrorSetLast(std::errc::invalid_argument, "The specified texture \"%s\" is not CPU-accessible and a copy could not be made", oD3D11GetDebugName(buf, _pSourceTexture));
		}

		oRef<ID3D11Texture2D> NewTexture;
		if (!oD3D11DecodeBC6orBC7(CPUAccessible, true, &NewTexture))
			return false; // pass through error

		NewTexture->GetDesc(&desc);
		if (_NewFormat == desc.Format)
		{
			*_ppDestinationTexture = NewTexture;
			(*_ppDestinationTexture)->AddRef();
			return true;
		}

		// recurse now that we've got a more vanilla format
		return oD3D11Convert(NewTexture, _NewFormat, _ppDestinationTexture);
	}

	oRef<ID3D11Device> D3DDevice;
	_pSourceTexture->GetDevice(&D3DDevice);

	oGPU_TEXTURE_DESC NewDesc;
	NewDesc.Dimensions = int3(oInt(desc.Width), oInt(desc.Height), 1);
	NewDesc.ArraySize = oInt(desc.ArraySize);
	NewDesc.Format = _NewFormat;
	NewDesc.Type = oGPUTextureTypeGetReadbackType(NewDesc.Type); // @oooii-tony: this should probably come from somewhere better.

	oRef<ID3D11Texture2D> NewTexture;
	if (!oD3D11CreateTexture(D3DDevice, "oD3D11Convert.Temp", NewDesc, nullptr, &NewTexture))
		return false; // pass through error

	oRef<ID3D11DeviceContext> D3DContext;
	D3DDevice->GetImmediateContext(&D3DContext);
	HRESULT hr = oD3DX11::Singleton()->D3DX11LoadTextureFromTexture(D3DContext, _pSourceTexture, nullptr, NewTexture);
	if (FAILED(hr))
		return oWinSetLastError(hr);

	*_ppDestinationTexture = NewTexture;
	(*_ppDestinationTexture)->AddRef();

	return true;
}

bool oD3D11Convert(ID3D11Device* _pDevice, oSURFACE_MAPPED_SUBRESOURCE& _Destination, oSURFACE_FORMAT _DestinationFormat, oSURFACE_CONST_MAPPED_SUBRESOURCE& _Source, oSURFACE_FORMAT _SourceFormat, const int2& _MipDimensions)
{
	oGPU_TEXTURE_DESC d;
	d.Dimensions = int3(_MipDimensions, 1);
	d.ArraySize = 1;
	d.Format = _SourceFormat;
	d.Type = oGPU_TEXTURE_2D_MAP;

	oRef<ID3D11Texture2D> SourceTexture;
	if (!oD3D11CreateTexture(_pDevice, "oD3D11Convert.TempSource", d, &_Source, &SourceTexture))
		return false; // pass through error

	oRef<ID3D11Texture2D> DestinationTexture;
	oTRACE("oD3D11Convert begin 0x%p (can take a while)...", SourceTexture);
	bool convertSuccessful = oD3D11Convert(SourceTexture, _DestinationFormat, &DestinationTexture);
	oTRACE("oD3D11Convert end 0x%p", DestinationTexture);
	if (!convertSuccessful)
		return false; // pass through error

	return oD3D11CopyTo(DestinationTexture, 0, _Destination.pData, _Destination.RowPitch); // pass through error
}

void oD3D11SetConstantBuffers(ID3D11DeviceContext* _pDeviceContext, uint _StartSlot, uint _NumBuffers, const ID3D11Buffer* const* _ppConstantBuffers)
{
	// COM-base APIs like DirectX have legitimate const-correctness issues: how 
	// can you call AddRef() or Release() on a const interface? But that's 
	// somewhat pedantic for the most case. Especially in DX11 where the device/
	// context no longer refs the set objects. So in this wrapper encapsulate the 
	// weirdness and expose a more expected API.
	ID3D11Buffer* const* ppConstantBuffers = const_cast<ID3D11Buffer* const*>(_ppConstantBuffers);
	_pDeviceContext->VSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
	_pDeviceContext->HSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
	_pDeviceContext->DSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
	_pDeviceContext->GSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
	_pDeviceContext->PSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
	_pDeviceContext->CSSetConstantBuffers(_StartSlot, _NumBuffers, ppConstantBuffers);
}

void oD3D11SetSamplers(ID3D11DeviceContext* _pDeviceContext, uint _StartSlot, uint _NumSamplers, const ID3D11SamplerState* const* _ppSamplers)
{
	// See oD3D11SetConstantBuffers for an explanation of this cast
	ID3D11SamplerState* const* ppSamplers = const_cast<ID3D11SamplerState* const*>(_ppSamplers);
	_pDeviceContext->VSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
	_pDeviceContext->HSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
	_pDeviceContext->DSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
	_pDeviceContext->GSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
	_pDeviceContext->PSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
	_pDeviceContext->CSSetSamplers(_StartSlot, _NumSamplers, ppSamplers);
}

void oD3D11SetShaderResourceViews(ID3D11DeviceContext* _pDeviceContext, uint _StartSlot, uint _NumShaderResourceViews, const ID3D11ShaderResourceView* const* _ppViews)
{
	// See oD3D11SetConstantBuffers for an explanation of this cast
	ID3D11ShaderResourceView* const* ppViews = const_cast<ID3D11ShaderResourceView* const*>(_ppViews);

	bool OneUnorderedBufferFound = false;
	for (uint i = 0; i < _NumShaderResourceViews; i++)
	{
		if (_ppViews[i])
		{
			oRef<ID3D11Resource> Resource;
			const_cast<ID3D11ShaderResourceView*>(_ppViews[i])->GetResource(&Resource);
			oGPU_TEXTURE_DESC td;
			oD3D11GetTextureDesc(Resource, &td);
			if (oGPUTextureTypeIsUnordered(td.Type))
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

void oD3D11FromViewport(const D3D11_VIEWPORT& _Viewport, oAABoxf* _pBox)
{
	_pBox->Min = float3(_Viewport.TopLeftX, _Viewport.TopLeftY, _Viewport.MinDepth);
	_pBox->Max = float3(_Viewport.Width, _Viewport.Height, _Viewport.MaxDepth);
}

void oD3D11ToViewport(const oAABoxf& _Source, D3D11_VIEWPORT* _pViewport)
{
	_pViewport->TopLeftX = _Source.Min.x;
	_pViewport->TopLeftY = _Source.Min.y;
	_pViewport->MinDepth = _Source.Min.z;
	_pViewport->Width = _Source.size().x;
	_pViewport->Height = _Source.size().y;
	_pViewport->MaxDepth = _Source.Max.z;
}

void oD3D11ToViewport(const int2& _RenderTargetDimensions, D3D11_VIEWPORT* _pViewport)
{
	_pViewport->TopLeftX = 0.0f;
	_pViewport->TopLeftY = 0.0f;
	_pViewport->Width = static_cast<float>(_RenderTargetDimensions.x);
	_pViewport->Height = static_cast<float>(_RenderTargetDimensions.y);
	_pViewport->MinDepth = 0.0f;
	_pViewport->MaxDepth = 1.0f;
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
	oRef<ID3D11Texture2D> t;
	_pRenderTargetView->GetResource((ID3D11Resource**)&t);
	#ifdef _DEBUG
		D3D11_RESOURCE_DIMENSION type;
		t->GetType(&type);
		oASSERT(type == D3D11_RESOURCE_DIMENSION_TEXTURE2D, "Unexpected resource type");
	#endif
	oD3D11SetFullTargetViewport(_pDeviceContext, t, _MinDepth, _MaxDepth);
}

void oD3D11Draw(ID3D11DeviceContext* _pDeviceContext
	, uint _NumElements
	, uint _NumVertexBuffers
	, const ID3D11Buffer* const* _ppVertexBuffers
	, const UINT* _VertexStrides
	, uint _IndexOfFirstVertexToDraw
	, uint _OffsetToAddToEachVertexIndex
	, const ID3D11Buffer* _pIndexBuffer
	, uint _IndexOfFirstIndexToDraw
	, uint _NumInstances
	, uint _IndexOfFirstInstanceIndexToDraw)
{
	uint sOffsets[16] = {0};

	// DirectX as an API has a funny definition for const, probably because of 
	// ref-counting, so consider it a platform-quirk and keep const correctness
	// above this API, but cast it away as DirectX requires here...

	_pDeviceContext->IASetVertexBuffers(0, _NumVertexBuffers, const_cast<ID3D11Buffer* const*>(_ppVertexBuffers), _VertexStrides, sOffsets);

	if (_pIndexBuffer)
	{
		oGPU_BUFFER_DESC d;
		if (!oD3D11BufferGetDesc(_pIndexBuffer, &d))
			oASSERT(false, "oD3D11Draw: The index buffer passed must have had oD3D11SetBufferDescription on it with appropriate values.");

		_pDeviceContext->IASetIndexBuffer(const_cast<ID3D11Buffer*>(_pIndexBuffer), oDXGIFromSurfaceFormat(d.Format), _IndexOfFirstIndexToDraw * d.StructByteSize);

		if (_NumInstances)
			_pDeviceContext->DrawIndexedInstanced(_NumElements, _NumInstances, _IndexOfFirstIndexToDraw, _OffsetToAddToEachVertexIndex, _IndexOfFirstInstanceIndexToDraw);
		else
			_pDeviceContext->DrawIndexed(_NumElements, 0, _OffsetToAddToEachVertexIndex);
	}

	else
	{
		if (_NumInstances)
			_pDeviceContext->DrawInstanced(_NumElements, _NumInstances, _IndexOfFirstVertexToDraw, _IndexOfFirstInstanceIndexToDraw);
		else
			_pDeviceContext->Draw(_NumElements, _IndexOfFirstVertexToDraw);
	}
}

void oD3D11DrawSVQuad(ID3D11DeviceContext* _pDeviceContext, uint _NumInstances)
{
	ID3D11Buffer* pVertexBuffers[] = { nullptr, nullptr };
	uint pStrides[] = { 0, 0 };
	uint pOffsets[] = { 0, 0 };
	_pDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffers, pStrides, pOffsets);
	_pDeviceContext->DrawInstanced(4, _NumInstances, 0, 0);
}

void oD3D11DebugTraceTexture2DDesc(D3D11_TEXTURE2D_DESC _Desc, const char* _Prefix)
{
	#define oD3D11_TRACE_UINT(x) oTRACE("%s" #x "=%u", oSAFESTR(_Prefix), _Desc.x)
	#define oD3D11_TRACE_ENUM(x) oTRACE("%s" #x "=%s", oSAFESTR(_Prefix), oStd::as_string(_Desc.x))
	#define oD3D11_TRACE_FLAGS(_FlagEnumType, _FlagsVar, _AllZeroString) do { char buf[512]; oStd::strbitmask(buf, _Desc._FlagsVar, _AllZeroString, oStd::as_string<_FlagEnumType>); oTRACE("%s" #_FlagsVar "=%s", oSAFESTR(_Prefix), buf); } while(false)

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
}

oD3D11ScopedMessageDisabler::oD3D11ScopedMessageDisabler(ID3D11DeviceContext* _pDeviceContext, const D3D11_MESSAGE_ID* _pMessageIDs, size_t _NumMessageIDs)
{
	#ifdef _DEBUG
		oRef<ID3D11Device> Device;
		_pDeviceContext->GetDevice(&Device);
		oV(Device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&pInfoQueue));
		D3D11_INFO_QUEUE_FILTER filter = {0};
		filter.DenyList.NumIDs = (UINT)_NumMessageIDs;
		filter.DenyList.pIDList = const_cast<D3D11_MESSAGE_ID*>(_pMessageIDs);
		pInfoQueue->PushStorageFilter(&filter);
	#endif
}

oD3D11ScopedMessageDisabler::oD3D11ScopedMessageDisabler(ID3D11Device* _pDevice, const D3D11_MESSAGE_ID* _pMessageIDs, size_t _NumMessageIDs)
{
	#ifdef _DEBUG
		oV(_pDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&pInfoQueue));
		D3D11_INFO_QUEUE_FILTER filter = {0};
		filter.DenyList.NumIDs = (UINT)_NumMessageIDs;
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
	IFACEMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
	IFACEMETHOD(Close)(THIS_ LPCVOID pData);

protected:
	const char* ShaderSourcePath;
	const std::vector<const char*>& HeaderSearchPaths;

	struct BUFFER
	{
		BUFFER() : pData(nullptr), Size(0) {}
		BUFFER(LPCVOID _pData, UINT _Size) : pData(_pData), Size(_Size) {}
		LPCVOID pData;
		UINT Size;
	};

	oStd::unordered_map<oURI, BUFFER> Cache;
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

HRESULT oD3DInclude::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
{
	auto it = Cache.find(pFileName);
	if (it != Cache.end())
	{
		*ppData = it->second.pData;
		*pBytes = it->second.Size;
		return S_OK;
	}

	bool exists = oStreamExists(pFileName);
	oStd::path_string Path;

	if (exists)
		Path = pFileName;
	else
	{
		oFOR(const char* p, HeaderSearchPaths)
		{
			oPrintf(Path, "%s/%s", p, pFileName);
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

	*pBytes = oUInt(size);
	
	Cache[oURI(pFileName)] = BUFFER(*ppData, *pBytes);
	return S_OK;
}

size_t oD3D11GetHLSLByteCodeSize(const void* _pByteCode)
{
	// Discovered empirically
	return _pByteCode ? ((const unsigned int*)_pByteCode)[6] : 0;
}

bool oFXC(const char* _CommandLineOptions, const char* _ShaderSourceFilePath, const char* _ShaderSource, oBuffer** _ppBuffer)
{
	int argc = 0;
	const char** argv = oWinCommandLineToArgvA(false, _CommandLineOptions, &argc);
	oStd::finally OSCFreeArgv([&] { oWinCommandLineToArgvAFree(argv); });

	std::string UnsupportedOptions("Unsupported options: ");
	size_t UnsupportedOptionsEmptyLen = UnsupportedOptions.size();

	const char* TargetProfile = "";
	const char* EntryPoint = "main";
	std::vector<const char*> IncludePaths;
	std::vector<std::pair<std::string, std::string>> Defines;
	UINT Flags1 = 0, Flags2 = 0;

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
						sep = k + oStrlen(k);

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
		oStrcpy((char*)(*_ppBuffer)->GetData(), (*_ppBuffer)->GetSize(), UnsupportedOptions.c_str());
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

	oStd::uri_string SourceName;
	oPrintf(SourceName, "%s", _ShaderSourceFilePath);

	oD3DInclude D3DInclude(_ShaderSourceFilePath, IncludePaths);
	oRef<ID3DBlob> Code, Errors;
	HRESULT hr = D3DCompile(_ShaderSource
		, oStrlen(_ShaderSource)
		, SourceName
		, oStd::data(Macros)
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
		oVERIFY(oD3D11ConvertCompileErrorBuffer((char*)(*_ppBuffer)->GetData(), (*_ppBuffer)->GetSize(), Errors, oStd::data(IncludePaths), IncludePaths.size()));
		return oErrorSetLast(std::errc::io_error, "shader compilation error: %s", oSAFESTRN(_ShaderSourceFilePath));
	}

	oVERIFY(oBufferCreate("Compile Errors", oBuffer::New(Code->GetBufferSize()), Code->GetBufferSize(), oBuffer::Delete, _ppBuffer));
	memcpy((*_ppBuffer)->GetData(), Code->GetBufferPointer(), (*_ppBuffer)->GetSize());
	return true;
}
