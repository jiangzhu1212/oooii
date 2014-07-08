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
#include "d3d_device.h"
#include <oBase/version.h>
#include <oCore/windows/win_util.h>
#include "dxgi_util.h"
#include "d3d_debug.h"

namespace ouro { namespace gpu { namespace d3d {

intrusive_ptr<Device> make_device(const gpu::device_init& init)
{
	oCHECK_ARG(init.version >= version(9,0), "must be D3D 9.0 or above");

	intrusive_ptr<IDXGIAdapter> Adapter;

	if (!init.use_software_emulation)
	{
		adapter::info adapter_info = adapter::find(init.virtual_desktop_position
			, init.min_driver_version, init.use_exact_driver_version);
		Adapter = dxgi::get_adapter(adapter_info.id);
	}

	uint Flags = 0;
	bool UsingDebug = false;
	if (init.driver_debug_level != gpu::debug_level::none)
	{
		Flags |= D3D11_CREATE_DEVICE_DEBUG;
		UsingDebug = true;
	}

	if (!init.multithreaded)
		Flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;

	intrusive_ptr<ID3D11Device> Device;
	D3D_FEATURE_LEVEL FeatureLevel;
	HRESULT hr = D3D11CreateDevice(
		Adapter
		, init.use_software_emulation ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_UNKNOWN
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
			, init.use_software_emulation ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_UNKNOWN
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
	if (D3DVersion < init.version)
	{
		sstring StrVer;
		oTHROW(not_supported, "Failed to create an ID3D11Device with a minimum feature set of D3D %s!", to_string(StrVer, init.version));
	}

	debug_name(Device, oSAFESTRN(init.debug_name));

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

	if (UsingDebug && init.driver_debug_level == gpu::debug_level::normal)
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

gpu::device_info get_info(Device* dev, bool is_software_emulation)
{
	gpu::device_info d;
	debug_name(d.debug_name, dev);

	intrusive_ptr<IDXGIAdapter> Adapter;
	{
		intrusive_ptr<IDXGIDevice> DXGIDevice;
		oV(dev->QueryInterface(__uuidof(IDXGIDevice), (void**)&DXGIDevice));
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
	d.is_software_emulation = is_software_emulation;
	d.debug_reporting_enabled = !!(dev->GetCreationFlags() & D3D11_CREATE_DEVICE_DEBUG);
	return d;
}

}}}
