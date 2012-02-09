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
#include <oPlatform/oGPU.h>
#include <oBasis/oAssert.h>
#include <oPlatform/oDXGI.h>
#include <oBasis/oRef.h>

const char* oAsString(const oGPU_VENDOR& _Vendor)
{
	switch (_Vendor)
	{
		case oGPU_VENDOR_UNKNOWN: return "Unknown Vendor";
		case oGPU_VENDOR_NVIDIA: return "NVIDIA";
		case oGPU_VENDOR_AMD: return "AMD";
		oNODEFAULT;
	}
}

const char* oAsString(const oGPU_API& _API)
{
	switch (_API)
	{
		case oGPU_API_D3D: return "Direct3D";
		case oGPU_API_OGL: return "OpenGL";
		oNODEFAULT;
	}
}

bool oGPUEnum(unsigned int _Index, oGPU_DESC* _pDesc)
{
	#if oDXVER >= oDXVER_10
		oRef<IDXGIFactory1> pFactory;
		oDXGICreateFactory(&pFactory);
		if (pFactory)
		{
			oRef<IDXGIAdapter> Adapter;
			if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters(_Index, &Adapter))
				return oErrorSetLast(oERROR_END_OF_FILE, "Index %u not found", _Index);

			DXGI_ADAPTER_DESC desc;
			Adapter->GetDesc(&desc);

			oWINDOWS_VIDEO_DRIVER_DESC VDDesc;
			oWinGetVideoDriverDesc(&VDDesc);

			_pDesc->GPUDescription = desc.Description;
			_pDesc->DriverDescription = VDDesc.Desc;
			_pDesc->VRAM = desc.DedicatedVideoMemory;
			_pDesc->DedicatedSystemMemory = desc.DedicatedSystemMemory;
			_pDesc->SharedSystemMemory = desc.SharedSystemMemory;
			_pDesc->Index = _Index;
			_pDesc->Vendor = VDDesc.Vendor;
			_pDesc->API = oGPU_API_D3D; // @oooii-tony: No work toward OGL support has been started
			_pDesc->DriverVersion = VDDesc.Version;
			_pDesc->FeatureVersion = oDXGIGetFeatureLevel(Adapter);
			_pDesc->InterfaceVersion = oDXGIGetInterfaceVersion(Adapter);

			return true;
		}
	#else
		oTRACE("oGPUEnum not yet implemented on pre-DX10 Windows");
	#endif

	return oErrorSetLast(oERROR_NOT_FOUND, "oGPUEnum not yet implemented on pre-DX10 Windows");
}

bool oGPUFindD3DCapable(unsigned int _NthMatch, const oVersion& _MinimumFeatureLevel, oGPU_DESC* _pDesc)
{
	unsigned int UserGPUIndex = oInvalid;
	unsigned int D3DGPUIndex = oInvalid;
	while (oGPUEnum(++D3DGPUIndex, _pDesc))
	{
		if (_pDesc->FeatureVersion >= _MinimumFeatureLevel)
		{
			if (++UserGPUIndex == _NthMatch)
				return true;
		}
	}

	memset(_pDesc, 0, sizeof(oGPU_DESC));
	return oErrorSetLast(oERROR_END_OF_FILE, "Index _NthMatch=%u is greater than the number of GPUs on the current system (%u)", _NthMatch, D3DGPUIndex);
}
