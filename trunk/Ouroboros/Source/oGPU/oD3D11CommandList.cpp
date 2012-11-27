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
#include "oD3D11Buffer.h"
#include "oD3D11CommandList.h"
#include "oD3D11ComputeShader.h"
#include "oD3D11InstanceList.h"
#include "oD3D11LineList.h"
#include "oD3D11Mesh.h"
#include "oD3D11Pipeline.h"
#include "oD3D11Texture.h"
#include <oGPU/oGPUDrawConstants.h>

static ID3D11UnorderedAccessView* oD3D11GetUAV(const oGPUResource* _pResource, int _Miplevel, int _Slice, bool _UseAppendIfAvailable = false, bool _AssertHasCounter = false)
{
	switch (_pResource->GetType())
	{
		case oGPU_INSTANCE_LIST: return nullptr;
		case oGPU_LINE_LIST: return nullptr;
		case oGPU_BUFFER:
		{
			oGPUBuffer::DESC d;
			oD3D11Buffer* b = static_cast<oD3D11Buffer*>(const_cast<oGPUResource*>(_pResource));
			b->GetDesc(&d);
			oASSERT(!_AssertHasCounter || d.Type == oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND || d.Type == oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER, "Buffer does not have a counter");
			if (_UseAppendIfAvailable && d.Type == oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND)
				return b->UAVAppend;
			else
				return b->UAV;
		}

		case oGPU_MESH: return nullptr;
		case oGPU_TEXTURE_RGB: return static_cast<oD3D11Texture*>(const_cast<oGPUResource*>(_pResource))->UAV;
		oNODEFAULT;
	}
}

static void oD3D11GetUAVs(ID3D11UnorderedAccessView* (&_ppUAVs)[oGPU_MAX_NUM_UNORDERED_BUFFERS], int _NumUnorderedResources, oGPUResource** _ppUnorderedResources, int _Miplevel, int _Slice, bool _UseAppendIfAvailable = false, bool _AssertHaveCounters = false)
{
	if (!_ppUnorderedResources || _NumUnorderedResources == oInvalid || _NumUnorderedResources == 0)
		memset(_ppUAVs, 0, sizeof(_ppUAVs));
	else
	{
		for (int i = 0; i < _NumUnorderedResources; i++)
		{
			if (_ppUnorderedResources[i])
			{
				_ppUAVs[i] = oD3D11GetUAV(_ppUnorderedResources[i], 0, 0, _UseAppendIfAvailable, _AssertHaveCounters);
				oASSERT(_ppUAVs[i], "The specified resource %p %s does not have an unordered resource view", _ppUnorderedResources[i], _ppUnorderedResources[i]->GetName()); 
			}
			else 
				_ppUAVs[i] = nullptr;
		}
	}
}

static ID3D11ShaderResourceView* oD3D11GetSRV(const oGPUResource* _pResource, int _Miplevel, int _Slice)
{
	switch (_pResource->GetType())
	{
		case oGPU_INSTANCE_LIST: return nullptr;
		case oGPU_LINE_LIST: return nullptr;
		case oGPU_BUFFER: return static_cast<oD3D11Buffer*>(const_cast<oGPUResource*>(_pResource))->SRV;
		case oGPU_MESH: return nullptr;
		case oGPU_TEXTURE_RGB: return static_cast<oD3D11Texture*>(const_cast<oGPUResource*>(_pResource))->SRV;
		oNODEFAULT;
	}
}

ID3D11Resource* oD3D11GetSubresource(oGPUResource* _pResource, int _Subresource, int* _pD3DSubresourceIndex)
{
	*_pD3DSubresourceIndex = 0;
	switch (_pResource->GetType())
	{
		case oGPU_INSTANCE_LIST: return static_cast<oD3D11InstanceList*>(_pResource)->Instances;
		case oGPU_LINE_LIST: return static_cast<oD3D11LineList*>(_pResource)->Lines;
		case oGPU_BUFFER: return static_cast<oD3D11Buffer*>(_pResource)->Buffer;
		case oGPU_MESH: return static_cast<oD3D11Mesh*>(_pResource)->GetSubresource(_Subresource);
		case oGPU_TEXTURE_RGB: *_pD3DSubresourceIndex = _Subresource; return static_cast<oD3D11Texture*>(_pResource)->Texture;
		oNODEFAULT;
	}
}

const oGUID& oGetGUID(threadsafe const oD3D11CommandList* threadsafe const *)
{
	// {2D6106C4-7741-41CD-93DE-2C2A9BCD9163}
	static const oGUID oIID_D3D11CommandList = { 0x2d6106c4, 0x7741, 0x41cd, { 0x93, 0xde, 0x2c, 0x2a, 0x9b, 0xcd, 0x91, 0x63 } };
	return oIID_D3D11CommandList;
}

oDEFINE_GPUDEVICE_CREATE(oD3D11, CommandList);
oBEGIN_DEFINE_GPUDEVICECHILD_CTOR(oD3D11, CommandList)
	, Desc(_Desc)
{
	*_pSuccess = false;
	oD3D11DEVICE();

	if (Desc.DrawOrder == oInvalid)
		D3DDevice->GetImmediateContext(&Context);
	else
	{
		D3D11_FEATURE_DATA_THREADING threadingCaps = { FALSE, FALSE };
		oV(D3DDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingCaps, sizeof(threadingCaps)));

		if (!threadingCaps.DriverCommandLists)
		{
			// Is this just a thing of the past? Hopefully...
			oErrorSetLast(oERROR_REFUSED, "Code requires driver workaround: http://msdn.microsoft.com/en-us/library/windows/desktop/ff476486(v=vs.85).aspx, but we haven't implemented it.");
			return;
		}

		HRESULT hr = D3DDevice->CreateDeferredContext(0, &Context);
		if (FAILED(hr))
		{
			oStringM err;

			UINT DeviceCreationFlags = D3DDevice->GetCreationFlags();

			if (DeviceCreationFlags & D3D11_CREATE_DEVICE_SINGLETHREADED)
				oPrintf(err, "oGPUCommandLists cannot be created on an oGPUDevice created as single-threaded: ");
			else
				oPrintf(err, "Failed to create oGPUDeviceContext %u: ", _Desc.DrawOrder);

			oWinSetLastError(hr, err);
			return;
		}
		oDEVICE_REGISTER_THIS();
	}

	*_pSuccess = true;
}

oD3D11CommandList::~oD3D11CommandList()
{
	if (Desc.DrawOrder != oInvalid)
		oDEVICE_UNREGISTER_THIS();
}

bool oD3D11CommandList::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	if (MIXINQueryInterface(_InterfaceID, _ppInterface))
		return true;

	else if (_InterfaceID == (const oGUID&)__uuidof(ID3D11DeviceContext))
	{
		Context->AddRef();
		*_ppInterface = Context;
	}

	return !!*_ppInterface;
}

static void SetViewports(ID3D11DeviceContext* _pDeviceContext, const int2& _TargetDimensions, int _NumViewports, const oAABoxf* _pViewports)
{
	D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	if (_NumViewports && _pViewports)
	{
		for (int i = 0; i < _NumViewports; i++)
			oD3D11ToViewport(_pViewports[i], &Viewports[i]);
		_pDeviceContext->RSSetViewports(static_cast<uint>(_NumViewports), Viewports);
	}

	else
	{
		_NumViewports = 1;
		oD3D11ToViewport(_TargetDimensions, &Viewports[0]);
	}

	_pDeviceContext->RSSetViewports(oUInt(_NumViewports), Viewports);
}

void oD3D11CommandList::Begin()
{
	if (Desc.DrawOrder != oInvalid) // ignore for immediate
		oDEVICE_LOCK_SUBMIT();
}

void oD3D11CommandList::End()
{
	if (Desc.DrawOrder != oInvalid) // ignore for immediate
	{
		Context->FinishCommandList(FALSE, &CommandList);
		oDEVICE_UNLOCK_SUBMIT();
	}
}

void oD3D11CommandList::Flush()
{
	Context->Flush();
}

void oD3D11CommandList::Reserve(oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource)
{
	return static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->MEMReserve(Context, _pResource, _Subresource, _pMappedSubresource);
}

void oD3D11CommandList::Commit(oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE& _Source, const oGPU_BOX& _Subregion)
{
	static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->MEMCommit(Context, _pResource, _Subresource, _Source, _Subregion);
}

void oD3D11CommandList::Copy(oGPUBuffer* _pDestination, int _DestinationOffsetBytes, oGPUBuffer* _pSource, int _SourceOffsetBytes, int _SizeBytes)
{
	int D3DSubresourceIndex = 0;
	ID3D11Resource* d = oD3D11GetSubresource(_pDestination, 0, &D3DSubresourceIndex);
	ID3D11Resource* s = oD3D11GetSubresource(_pSource, 0, &D3DSubresourceIndex);

	D3D11_BOX CopyBox;
	CopyBox.left = _SourceOffsetBytes;
	CopyBox.top = 0;
	CopyBox.right = _SourceOffsetBytes + _SizeBytes;
	CopyBox.bottom = 1;
	CopyBox.front = 0;
	CopyBox.back = 1;

	Context->CopySubresourceRegion(d, 0, _DestinationOffsetBytes, 0, 0, s, 0, &CopyBox);
}

void oD3D11CommandList::Copy(oGPUResource* _pDestination, oGPUResource* _pSource)
{
	oASSERT(_pDestination && _pSource && _pDestination->GetType() == _pSource->GetType(), "Copy(%s, %s) can only occur between two same-typed objects", _pDestination ? oAsString(_pDestination->GetType()) : "(null)", _pSource ? oAsString(_pSource->GetType()) : "(null)");
	int D3DSubresourceIndex = 0;
	switch (_pDestination->GetType())
	{
		case oGPU_MESH:
		{
			for (int i = 0; i < oGPU_MESH_NUM_SUBRESOURCES; i++)
			{
				ID3D11Resource* d = oD3D11GetSubresource(_pDestination, i, &D3DSubresourceIndex);
				ID3D11Resource* s = oD3D11GetSubresource(_pSource, i, &D3DSubresourceIndex);
				Context->CopyResource(d, s);
			}
			break;
		}

		default:
		{
			ID3D11Resource* d = oD3D11GetSubresource(_pDestination, 0, &D3DSubresourceIndex);
			ID3D11Resource* s = oD3D11GetSubresource(_pSource, 0, &D3DSubresourceIndex);
			Context->CopyResource(d, s);
			break;
		}
	}
}

void oD3D11CommandList::CopyCounter(oGPUBuffer* _pDestination, uint _DestinationAlignedOffset, oGPUBuffer* _pUnorderedSource)
{
	#ifdef _DEBUG
		oGPUBuffer::DESC d;
		static_cast<oGPUBuffer*>(_pUnorderedSource)->GetDesc(&d);
		oASSERT(d.Type == oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND || d.Type == oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER, "Source must be an unordered structured buffer with APPEND or COUNTER modifiers");
	#endif

	oASSERT(oIsByteAligned(_DestinationAlignedOffset, sizeof(uint)), "_DestinationAlignedOffset must be sizeof(uint)-aligned");
	Context->CopyStructureCount(static_cast<oD3D11Buffer*>(_pDestination)->Buffer, _DestinationAlignedOffset, oD3D11GetUAV(_pUnorderedSource, 0, 0, true));
}

void oD3D11CommandList::SetCounters(int _NumUnorderedResources, oGPUResource** _ppUnorderedResources, uint* _pValues)
{
	ID3D11UnorderedAccessView* UAVs[oGPU_MAX_NUM_UNORDERED_BUFFERS];
	oD3D11GetUAVs(UAVs, _NumUnorderedResources, _ppUnorderedResources, 0, 0, true);

	// Executing a noop only seems to apply the initial counts if done through
	// OMSetRenderTargetsAndUnorderedAccessViews, so set things up that way with
	// a false setting here, and then flush it with a dispatch of a noop.
	Context->CSSetUnorderedAccessViews(0, oGPU_MAX_NUM_UNORDERED_BUFFERS, D3DDevice()->NullUAVs, D3DDevice()->NoopUAVInitialCounts); // clear any conflicting binding
	Context->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, _NumUnorderedResources, UAVs, _pValues); // set up binding
	Context->CSSetShader(D3DDevice()->NoopCS, nullptr, 0); // 
	Context->Dispatch(1, 1, 1);
}

void oD3D11CommandList::SetRenderTargetAndUnorderedResources(oGPURenderTarget* _pRenderTarget, int _NumViewports, const oAABoxf* _pViewports, bool _SetForDispatch, int _UnorderedResourcesStartSlot, int _NumUnorderedResources, oGPUResource** _ppUnorderedResources, uint* _pInitialCounts)
{
	oASSERT(!_SetForDispatch || !_pRenderTarget, "If _SetForDispatch is true, then _pRenderTarget must be nullptr");

	oD3D11RenderTarget* RT = static_cast<oD3D11RenderTarget*>(_pRenderTarget);

	UINT StartSlot = _UnorderedResourcesStartSlot;
	if (StartSlot == oInvalid)
		StartSlot = RT ? RT->Desc.MRTCount : 0;

	ID3D11UnorderedAccessView* UAVs[oGPU_MAX_NUM_UNORDERED_BUFFERS];
	oD3D11GetUAVs(UAVs, _NumUnorderedResources, _ppUnorderedResources, 0, 0, !_SetForDispatch);

	UINT NumUAVs = _NumUnorderedResources;
	if (_NumUnorderedResources == oInvalid)
		NumUAVs = oGPU_MAX_NUM_UNORDERED_BUFFERS - StartSlot;

	UINT* pInitialCounts = _pInitialCounts;
	if (!pInitialCounts)
		pInitialCounts = D3DDevice()->NoopUAVInitialCounts;

	if (_SetForDispatch)
	{
		// buffers can't be bound in both places at once, so ensure that state
		Context->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, oGPU_MAX_NUM_UNORDERED_BUFFERS, D3DDevice()->NullUAVs, D3DDevice()->NoopUAVInitialCounts);
		Context->CSSetUnorderedAccessViews(StartSlot, NumUAVs, UAVs, pInitialCounts);
	}

	else
	{
		// buffers can't be bound in both places at once, so ensure that state
		Context->CSSetUnorderedAccessViews(0, oGPU_MAX_NUM_UNORDERED_BUFFERS, D3DDevice()->NullUAVs, D3DDevice()->NoopUAVInitialCounts);

		if (RT)
		{
			Context->OMSetRenderTargetsAndUnorderedAccessViews(RT->Desc.MRTCount, (ID3D11RenderTargetView* const*)RT->RTVs, RT->DSV, StartSlot, NumUAVs, UAVs, pInitialCounts);
			oGPURenderTarget::DESC d;
			_pRenderTarget->GetDesc(&d);
			SetViewports(Context, d.Dimensions.xy, _NumViewports, _pViewports);
		}

		else
		{
			oASSERT(NumUAVs >= 0 && NumUAVs <= oGPU_MAX_NUM_UNORDERED_BUFFERS, "Invalid _NumUnorderedResources");
			Context->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, StartSlot, NumUAVs, UAVs, pInitialCounts);
			SetViewports(Context, 1, _NumViewports, _pViewports);
		}	
	}
}

void oD3D11CommandList::SetPipeline(const oGPUPipeline* _pPipeline)
{
	if (_pPipeline)
	{
		oD3D11Pipeline* p = const_cast<oD3D11Pipeline*>(static_cast<const oD3D11Pipeline*>(_pPipeline));
		Context->IASetInputLayout(p->InputLayout);
		Context->VSSetShader(p->VertexShader, 0, 0);
		Context->HSSetShader(p->HullShader, 0, 0);
		Context->DSSetShader(p->DomainShader, 0, 0);
		Context->GSSetShader(p->GeometryShader, 0, 0);
		Context->PSSetShader(p->PixelShader, 0, 0);
	}

	else
	{
		Context->IASetInputLayout(nullptr);
		Context->VSSetShader(nullptr, nullptr, 0);
		Context->HSSetShader(nullptr, nullptr, 0);
		Context->DSSetShader(nullptr, nullptr, 0);
		Context->GSSetShader(nullptr, nullptr, 0);
		Context->PSSetShader(nullptr, nullptr, 0);
	}
}

void oD3D11CommandList::SetSurfaceState(oGPU_SURFACE_STATE _State)
{
	Context->IASetPrimitiveTopology(_State >= oGPU_FRONT_POINTS ? D3D11_PRIMITIVE_TOPOLOGY_POINTLIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->RSSetState(D3DDevice()->SurfaceStates[_State]);
}

void oD3D11CommandList::SetBlendState(oGPU_BLEND_STATE _State)
{
	static const FLOAT sBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Context->OMSetBlendState(D3DDevice()->BlendStates[_State], sBlendFactor, 0xffffffff);
}

void oD3D11CommandList::SetDepthStencilState(oGPU_DEPTH_STENCIL_STATE _State)
{
	Context->OMSetDepthStencilState(D3DDevice()->DepthStencilStates[_State], 0);
}

void oD3D11CommandList::SetSamplers(int _StartSlot, int _NumStates, const oGPU_SAMPLER_STATE* _pSamplerState)
{
	ID3D11SamplerState* Samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
	oASSERT(oCOUNTOF(Samplers) >= _NumStates, "Too many samplers specified");
	for (int i = 0; i < _NumStates; i++)
		Samplers[i] = D3DDevice()->SamplerStates[_pSamplerState[i]];
	
	oD3D11SetSamplers(Context, oUInt(_StartSlot), oUInt(_NumStates), Samplers);
}

void oD3D11CommandList::SetShaderResources(int _StartSlot, int _NumResources, const oGPUResource* const* _ppResources)
{
	const ID3D11ShaderResourceView* SRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];

	if (!_NumResources || !_ppResources)
	{
		memset(SRVs, 0, sizeof(ID3D11ShaderResourceView*) * D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		_NumResources = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
	}
	else
	{
		oASSERT(oCOUNTOF(SRVs) >= _NumResources, "Too many resources specified");
		for (int i = 0; i < _NumResources; i++)
			SRVs[i] = _ppResources[i] ? oD3D11GetSRV(_ppResources[i], 0, 0) : nullptr;
	}

	oD3D11SetShaderResourceViews(Context, _StartSlot, _NumResources, SRVs);
}

void oD3D11CommandList::SetBuffers(int _StartSlot, int _NumBuffers, const oGPUBuffer* const* _ppBuffers)
{
	const ID3D11Buffer* CBs[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	oASSERT(oCOUNTOF(CBs) > _NumBuffers, "Too many buffers specified");
	for (int i = 0; i < _NumBuffers; i++)
		CBs[i] = const_cast<ID3D11Buffer*>(static_cast<const oD3D11Buffer*>(_ppBuffers[i])->Buffer.c_ptr());

	oD3D11SetConstantBuffers(Context, _StartSlot, _NumBuffers, CBs);
}

void oD3D11CommandList::Clear(oGPURenderTarget* _pRenderTarget, oGPU_CLEAR _Clear)
{
	oD3D11RenderTarget* pRT = static_cast<oD3D11RenderTarget*>(_pRenderTarget);

	if (_Clear >= oGPU_CLEAR_COLOR)
	{
		FLOAT c[4];
		for (int i = 0; i < pRT->Desc.MRTCount; i++)
		{
			oColorDecompose(pRT->Desc.ClearDesc.ClearColor[i], c);
			Context->ClearRenderTargetView(pRT->RTVs[i], c);
		}
	}

	static const UINT sClearFlags[] = 
	{
		D3D11_CLEAR_DEPTH,
		D3D11_CLEAR_STENCIL,
		D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,
		0,
		D3D11_CLEAR_DEPTH,
		D3D11_CLEAR_STENCIL,
		D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,
	};
	static_assert(oCOUNTOF(sClearFlags) == oGPU_NUM_CLEARS, "# clears mismatch");

	if (pRT->DSV && _Clear != oGPU_CLEAR_COLOR)
		Context->ClearDepthStencilView(pRT->DSV, sClearFlags[_Clear], pRT->Desc.ClearDesc.DepthClearValue, pRT->Desc.ClearDesc.StencilClearValue);
}

void oD3D11CommandList::Draw(const oGPUMesh* _pMesh, int _RangeIndex, const oGPUInstanceList* _pInstanceList)
{
	const oD3D11Mesh* M = static_cast<const oD3D11Mesh*>(_pMesh);
	oGPUMesh::DESC desc;
	M->GetDesc(&desc);

	uint StartIndex = 0;
	uint NumTriangles = 0;
	uint MinVertex = 0;

	if (_RangeIndex == oInvalid)
		NumTriangles = desc.NumIndices / 3;
	else
	{
		oASSERT(_RangeIndex < oUInt(M->Ranges.size()), "");
		const oGPU_RANGE& r = M->Ranges[_RangeIndex];
		StartIndex = r.StartTriangle * 3;
		NumTriangles = r.NumTriangles;
		MinVertex = r.MinVertex;
	}

	#ifdef _DEBUG
	{
		oRef<ID3D11InputLayout> InputLayout = 0;
		Context->IAGetInputLayout(&InputLayout);
		oASSERT(InputLayout, "No InputLayout specified");
	}
	#endif
	oASSERT(M->Vertices[0], "No geometry vertices specified");

	const ID3D11Buffer* pVertices[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	UINT Strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	uint nVertexBuffers = 0;
	for (uint i = 0; i < oCOUNTOF(M->Vertices); i++)
	{
		if (M->Vertices[i])
		{
			pVertices[i] = M->Vertices[i];
			Strides[i] = M->VertexStrides[i];
			nVertexBuffers = __max(nVertexBuffers, i+1);
		}
	}

	uint NumInstances = 0;
	if (_pInstanceList)
	{
		oGPUInstanceList::DESC ILDesc;
		_pInstanceList->GetDesc(&ILDesc);
		oASSERT(ILDesc.InputSlot >= nVertexBuffers, "Mesh defines vertices in the instance input slot");
		pVertices[ILDesc.InputSlot] = static_cast<const oD3D11InstanceList*>(_pInstanceList)->Instances;
		Strides[ILDesc.InputSlot] = static_cast<const oD3D11InstanceList*>(_pInstanceList)->InstanceStride;
		nVertexBuffers = __max(nVertexBuffers, ILDesc.InputSlot+1);
		NumInstances = ILDesc.NumInstances;
	}

	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	Context->IAGetPrimitiveTopology(&topology);

	oD3D11Draw(Context
		, topology
		, NumTriangles
		, nVertexBuffers
		, pVertices
		, Strides
		, MinVertex
		, 0
		, M->Indices
		, StartIndex
		, NumInstances);
}

void oD3D11CommandList::Draw(const oGPULineList* _pLineList)
{
	oGPULineList::DESC d;
	_pLineList->GetDesc(&d);

	const ID3D11Buffer* pLines = static_cast<const oD3D11LineList*>(_pLineList)->Lines;
	UINT VertexStride = sizeof(oGPU_LINE) / 2; // div by 2 because each point on a line is what is actually stored/drawn
	
	D3D11_PRIMITIVE_TOPOLOGY old;
	Context->IAGetPrimitiveTopology(&old);

	oD3D11Draw(Context
		, D3D11_PRIMITIVE_TOPOLOGY_LINELIST
		, d.NumLines
		, 1
		, &pLines
		, &VertexStride
		, 0
		, 0
		, nullptr);

	Context->IASetPrimitiveTopology(old);
}

void oD3D11CommandList::Draw(uint _VertexCount)
{
	D3D11_PRIMITIVE_TOPOLOGY old;
	Context->IAGetPrimitiveTopology(&old);
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	Context->Draw(_VertexCount, 0);
	Context->IASetPrimitiveTopology(old);
}

void oD3D11CommandList::Draw(oGPUBuffer* _pDrawArgs, int _AlignedByteOffsetForArgs)
{
	D3D11_PRIMITIVE_TOPOLOGY old;
	Context->IAGetPrimitiveTopology(&old);
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	Context->DrawInstancedIndirect(static_cast<oD3D11Buffer*>(_pDrawArgs)->Buffer, _AlignedByteOffsetForArgs);
	Context->IASetPrimitiveTopology(old);
}

void oD3D11CommandList::DrawSVQuad(uint _NumInstances)
{
	ID3D11Buffer* pVertexBuffers[] = { nullptr, nullptr };
	uint pStrides[] = { 0, 0 };
	uint pOffsets[] = { 0, 0 };
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	Context->IASetVertexBuffers(0, 1, pVertexBuffers, pStrides, pOffsets);
	Context->DrawInstanced(4, _NumInstances, 0, 0);
}

bool oD3D11CommandList::GenerateMips(oGPURenderTarget* _pRenderTarget)
{
	oGPURenderTarget::DESC desc;
	_pRenderTarget->GetDesc(&desc);
	if (!oGPUTextureTypeHasMips(desc.Type))
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Cannot generate mips if the type doesn't contain oGPU_TRAIT_TEXTURE_MIPS");

	oRef<oGPUTexture> texture;
	_pRenderTarget->GetTexture(0, &texture);
	oD3D11Texture* d3dTexture = static_cast<oD3D11Texture*>(texture.c_ptr());
	Context->GenerateMips(d3dTexture->SRV);
	return true;
}

void oD3D11CommandList::ClearI(oGPUResource* _pUnorderedResource, const uint4 _Values)
{
	ID3D11UnorderedAccessView* UAV = oD3D11GetUAV(_pUnorderedResource, 0, 0, false);
	oASSERT(UAV, "The specified resource %p %s does not an unordered resource view", _pUnorderedResource, _pUnorderedResource->GetName());
	Context->ClearUnorderedAccessViewUint(UAV, (const uint*)&_Values);
}

void oD3D11CommandList::ClearF(oGPUResource* _pUnorderedResource, const float4 _Values)
{
	ID3D11UnorderedAccessView* UAV = oD3D11GetUAV(_pUnorderedResource, 0, 0, false);
	oASSERT(UAV, "The specified resource %p %s does not an unordered resource view", _pUnorderedResource, _pUnorderedResource->GetName());
	Context->ClearUnorderedAccessViewFloat(UAV, (const float*)&_Values);
}

void oD3D11CommandList::Dispatch(oGPUComputeShader* _pComputeShader, const int3& _ThreadGroupCount)
{
	oASSERT(less_than_equal(_ThreadGroupCount, int3(D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION)), "_ThreadGroupCount cannot have a dimension greater than %u", D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION);
	Context->CSSetShader(static_cast<oD3D11ComputeShader*>(_pComputeShader)->ComputeShader, nullptr, 0);
	Context->Dispatch(_ThreadGroupCount.x, _ThreadGroupCount.y, _ThreadGroupCount.z);
}

void oD3D11CommandList::Dispatch(oGPUComputeShader* _pComputeShader, oGPUBuffer* _pThreadGroupCountBuffer, int _AlignedByteOffsetToThreadGroupCount)
{
	#ifdef _DEBUG
		oGPUBuffer::DESC d;
		_pThreadGroupCountBuffer->GetDesc(&d);
		oASSERT(d.Type == oGPU_BUFFER_UNORDERED_RAW, "Parameters for dispatch must come from an oGPUBuffer of type oGPU_BUFFER_UNORDERED_RAW");

		// Found this out the hard way... if a UAV is bound as a target, then it 
		// won't be flushed such that values are ready for this indirect dispatch.
		// D3D is quiet about it, so do the check here...

		ID3D11Buffer* pBuffer = static_cast<oD3D11Buffer*>(_pThreadGroupCountBuffer)->Buffer;
		oD3D11CheckBoundRTAndUAV(Context, 1, &pBuffer);
		oD3D11CheckBoundCSSetUAV(Context, 1, &pBuffer);
	#endif
	Context->CSSetShader(static_cast<oD3D11ComputeShader*>(_pComputeShader)->ComputeShader, nullptr, 0);
	Context->DispatchIndirect(static_cast<oD3D11Buffer*>(_pThreadGroupCountBuffer)->Buffer, _AlignedByteOffsetToThreadGroupCount);
}
