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
#include <oooii/oGPU.h>
#include <oooii/oStddef.h>
#include <oooii/oString.h>
#include <oooii/oWindows.h>
#include <oooii/oRef.h>

bool oGPU::GetDesc(unsigned int _GPUIndex, oGPU::DESC* _pDesc)
{
	#if oDXVER >= oDXVER_10
	oRef<IDXGIFactory1> pFactory;
	oCreateDXGIFactory(&pFactory);
	if (pFactory)
	{
		IDXGIAdapter* pAdapter = 0;
		if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters(_GPUIndex, &pAdapter))
			return false;

		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(&desc);
		oStrConvert(_pDesc->Description, oCOUNTOF(_pDesc->Description), desc.Description);
		_pDesc->VRAM = desc.DedicatedVideoMemory;
		_pDesc->DedicatedSystemMemory = desc.DedicatedSystemMemory;
		_pDesc->SharedSystemMemory = desc.SharedSystemMemory;
		_pDesc->Index = _GPUIndex;
		_pDesc->D3DVersion = oDXGIGetD3DVersion(pAdapter);
		pAdapter->Release();
		return true;
	}
	#else
		oTRACE("oGPU::GetDesc not yet implemented on pre-DX10 Windows");
	#endif
	return false;
}
