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
