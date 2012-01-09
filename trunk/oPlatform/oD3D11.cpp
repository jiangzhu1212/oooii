// $(header)
#include <oPlatform/oD3D11.h>
#include <oBasis/oAssert.h>
#include <oBasis/oByte.h>
#include <oPlatform/oDXGI.h>
#include <oPlatform/oImage.h>
#include <oBasis/oMemory.h>
#include <cerrno>

// {13BA565C-4766-49C4-8C1C-C1F459F00A65}
static const GUID oWKPDID_oD3DBufferTopology = { 0x13ba565c, 0x4766, 0x49c4, { 0x8c, 0x1c, 0xc1, 0xf4, 0x59, 0xf0, 0xa, 0x65 } };

// Value obtained from D3Dcommon.h and reproduced here because of soft-linking
static const GUID oWKPDID_D3DDebugObjectName = { 0x429b8c22, 0x9188, 0x4b0c, { 0x87, 0x42, 0xac, 0xb0, 0xbf, 0x85, 0xc2, 0x00} };

static const char* d3d11_dll_functions[] = 
{
	"D3D11CreateDevice",
};

oD3D11::oD3D11()
{
	hD3D11 = oModuleLink("d3d11.dll", d3d11_dll_functions, (void**)&D3D11CreateDevice, oCOUNTOF(d3d11_dll_functions));
	oASSERT(hD3D11, "");
}

oD3D11::~oD3D11()
{
	oModuleUnlink(hD3D11);
}

void oD3D11InitBufferDesc(D3D11_BUFFER_DESC* _pDesc, D3D11_BIND_FLAG _BindFlag, bool _CPUWritable, size_t _Size, size_t _Count)
{
	_pDesc->ByteWidth = static_cast<UINT>(_Size * _Count);
	_pDesc->Usage = _CPUWritable ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	_pDesc->BindFlags = _BindFlag;
	_pDesc->CPUAccessFlags = _CPUWritable ? D3D11_CPU_ACCESS_WRITE : 0;
	_pDesc->MiscFlags = 0;
	_pDesc->StructureByteStride = static_cast<UINT>(_Size);
}

#define oDEBUG_CHECK_BUFFER(fnName, ppOut) \
	if (FAILED(hr)) \
	{	oWinSetLastError(oDEFAULT, #fnName " failed: "); \
		return false; \
	} \
	else if (_DebugName && *_DebugName) \
	{	oD3D11SetDebugName(*ppOut, _DebugName); \
	}

bool oD3D11CreateConstantBuffer(ID3D11Device* _pDevice, const char* _DebugName, bool _CPUWrite, const void* _pBufferStruct, size_t _SizeofBufferStruct, size_t _StructCount, ID3D11Buffer** _ppConstantBuffer)
{
	oD3D_BUFFER_TOPOLOGY t;
	t.ElementCount = static_cast<UINT>(_StructCount);
	t.ElementStride = static_cast<UINT>(_SizeofBufferStruct);

	D3D11_BUFFER_DESC desc;
	oD3D11InitBufferDesc(&desc, D3D11_BIND_CONSTANT_BUFFER, _CPUWrite, oByteAlign(_SizeofBufferStruct, 16), _StructCount);
	D3D11_SUBRESOURCE_DATA SRD;
	SRD.pSysMem = _pBufferStruct;
	HRESULT hr = _pDevice->CreateBuffer(&desc, _pBufferStruct ? &SRD : 0, _ppConstantBuffer);
	oDEBUG_CHECK_BUFFER(oD3D11CreateConstantBuffer, _ppConstantBuffer);
	oVERIFY(oD3D11SetBufferDescription(*_ppConstantBuffer, t));
	return true;
}

bool oD3D11CreateIndexBuffer(ID3D11Device* _pDevice, const char* _DebugName, bool _CPUWrite, const void* _pIndices, size_t _NumIndices, bool _Use16BitIndices, ID3D11Buffer** _ppIndexBuffer)
{
	oD3D_BUFFER_TOPOLOGY t;
	t.ElementCount = static_cast<UINT>(_NumIndices);
	t.ElementStride = _Use16BitIndices ? sizeof(unsigned short) : sizeof(unsigned int);

	D3D11_BUFFER_DESC desc;
	oD3D11InitBufferDesc(&desc, D3D11_BIND_INDEX_BUFFER, _CPUWrite, t.ElementStride * t.ElementCount, 1);
	D3D11_SUBRESOURCE_DATA SRD;
	SRD.pSysMem = _pIndices;
	HRESULT hr = _pDevice->CreateBuffer(&desc, _pIndices ? &SRD : 0, _ppIndexBuffer);
	oDEBUG_CHECK_BUFFER(oD3D11CreateIndexBuffer, _ppIndexBuffer);
	oVERIFY(oD3D11SetBufferDescription(*_ppIndexBuffer, t));
	return true;
}

bool oD3D11CreateVertexBuffer(ID3D11Device* _pDevice, const char* _DebugName, bool _CPUWrite, const void* _pVertices, size_t _NumVertices, size_t _VertexStride, ID3D11Buffer** _ppVertexBuffer)
{
	oD3D_BUFFER_TOPOLOGY t;
	t.ElementCount = static_cast<UINT>(_NumVertices);
	t.ElementStride = static_cast<UINT>(_VertexStride);

	D3D11_BUFFER_DESC desc;
	oD3D11InitBufferDesc(&desc, D3D11_BIND_VERTEX_BUFFER, _CPUWrite, t.ElementStride * t.ElementCount, 1);
	D3D11_SUBRESOURCE_DATA SRD;
	SRD.pSysMem = _pVertices;
	HRESULT hr = _pDevice->CreateBuffer(&desc, _pVertices ? &SRD : 0, _ppVertexBuffer);
	oDEBUG_CHECK_BUFFER(oD3D11CreateVertexBuffer, _ppVertexBuffer);
	oVERIFY(oD3D11SetBufferDescription(*_ppVertexBuffer, t));
	return true;
}

bool oD3D11CreateShaderResourceView(const char* _DebugName, ID3D11Resource* _pTexture, ID3D11ShaderResourceView** _ppShaderResourceView)
{
	oD3D11_TEXTURE_DESC desc;
	oD3D11GetTextureDesc(_pTexture, &desc);

	// If a depth-stencil resource is specified we have to convert the specified 
	// format to a compatible color one
	D3D11_SHADER_RESOURCE_VIEW_DESC srv;
	D3D11_SHADER_RESOURCE_VIEW_DESC* pSRV = (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL) ? &srv : 0;
	if (pSRV)
	{
		srv.Format = oDXGIGetColorCompatibleFormat(desc.Format);

		if (desc.Depth > 1)
		{
			srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			srv.Texture3D.MostDetailedMip = 0;
			srv.Texture3D.MipLevels = desc.MipLevels;
		}

		else if (desc.ArraySize == 1)
		{
			srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv.Texture2D.MostDetailedMip = 0;
			srv.Texture2D.MipLevels = desc.MipLevels;
		}

		else
		{
			srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srv.Texture2DArray.MostDetailedMip = 0;
			srv.Texture2DArray.MipLevels = desc.MipLevels;
			srv.Texture2DArray.FirstArraySlice = 0;
			srv.Texture2DArray.ArraySize = desc.ArraySize;
		}
	}

	oRef<ID3D11Device> D3DDevice;
	_pTexture->GetDevice(&D3DDevice);

	HRESULT hr = D3DDevice->CreateShaderResourceView(_pTexture, pSRV, _ppShaderResourceView);
	oDEBUG_CHECK_BUFFER(oD3D11CreateShaderResourceView, _ppShaderResourceView);

	return true;
}

bool oD3D11CreateRenderTargetView(const char* _DebugName, ID3D11Resource* _pTexture, ID3D11View** _ppView)
{
	oD3D11_TEXTURE_DESC desc;
	oD3D11GetTextureDesc(_pTexture, &desc);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv;
	D3D11_DEPTH_STENCIL_VIEW_DESC* pDSV = (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL) ? &dsv : 0;

	HRESULT hr = S_OK;

	oRef<ID3D11Device> D3DDevice;
	_pTexture->GetDevice(&D3DDevice);

	if (pDSV)
	{
		dsv.Format = oDXGIGetDepthCompatibleFormat(desc.Format);
		dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		dsv.Flags = 0;
		hr = D3DDevice->CreateDepthStencilView(_pTexture, &dsv, (ID3D11DepthStencilView**)_ppView);
	}
	else
		hr = D3DDevice->CreateRenderTargetView(_pTexture, 0, (ID3D11RenderTargetView**)_ppView);

	oDEBUG_CHECK_BUFFER(oD3D11CreateRenderTargetView, _ppView);

	return true;
}

bool oD3D11CreateRenderTarget(ID3D11Device* _pDevice, const char* _DebugName, unsigned int _Width, unsigned int _Height, unsigned int _ArraySize, DXGI_FORMAT _Format, ID3D11Texture2D** _ppRenderTarget, ID3D11View** _ppRenderTargetView, ID3D11ShaderResourceView** _ppShaderResourceView)
{
	*_ppRenderTarget = 0;

	if (_ppRenderTargetView)
		*_ppRenderTargetView = 0;

	if (_ppShaderResourceView)
		*_ppShaderResourceView = 0;

	if (!oD3D11CreateTexture2D(_pDevice, _DebugName, _Width, _Height, _ArraySize, _Format, oD3D11_RENDER_TARGET, _ppRenderTarget, _ppShaderResourceView))
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "oD3D11CreateTexture2D failed, check DX debug output");
		return false;
	}

	if (_ppRenderTargetView && !oD3D11CreateRenderTargetView(_DebugName, *_ppRenderTarget, _ppRenderTargetView))
	{
		(*_ppRenderTarget)->Release();
		*_ppRenderTarget = 0;

		if (_ppShaderResourceView && *_ppShaderResourceView)
		{
			(*_ppShaderResourceView)->Release();
			*_ppShaderResourceView = 0;
		}

		oErrorSetLast(oERROR_INVALID_PARAMETER, "oD3D11CreateRenderTargetView failed, check DX debug output");
		return false;
	}

	return true;
}

bool oD3D11SetDebugName(ID3D11DeviceChild* _pDeviceChild, const char* _Name)
{
	oRef<ID3D11Device> D3DDevice;
	_pDeviceChild->GetDevice(&D3DDevice);
	UINT CreationFlags = D3DDevice->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		return S_OK == _pDeviceChild->SetPrivateData(oWKPDID_D3DDebugObjectName, static_cast<UINT>(strlen(_Name) + 1), _Name);
	return true;
}

bool oD3D11GetDebugName(char* _StrDestination, size_t _SizeofStrDestination, const ID3D11DeviceChild* _pDeviceChild)
{
	UINT size = static_cast<UINT>(_SizeofStrDestination);
	oRef<ID3D11Device> D3DDevice;
	const_cast<ID3D11DeviceChild*>(_pDeviceChild)->GetDevice(&D3DDevice);
	UINT CreationFlags = D3DDevice->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		return S_OK == const_cast<ID3D11DeviceChild*>(_pDeviceChild)->GetPrivateData(oWKPDID_D3DDebugObjectName, &size, _StrDestination);
	else
		strcpy_s(_StrDestination, _SizeofStrDestination, "non-debug device child");
	return true;
}

bool oD3D11SetBufferDescription(ID3D11Buffer* _pBuffer, const oD3D_BUFFER_TOPOLOGY& _Topology)
{
	return S_OK == _pBuffer->SetPrivateData(oWKPDID_oD3DBufferTopology, sizeof(_Topology), &_Topology);
}

bool oD3D11GetBufferDescription(const ID3D11Buffer* _pBuffer, oD3D_BUFFER_TOPOLOGY* _pTopology)
{
	UINT size = sizeof(oD3D_BUFFER_TOPOLOGY);
	return S_OK == const_cast<ID3D11Buffer*>(_pBuffer)->GetPrivateData(oWKPDID_oD3DBufferTopology, &size, _pTopology);
}

template<> const char* oAsString(const oD3D11_PIPELINE_STAGE& _Stage)
{
	switch (_Stage)
	{
		case oD3D11_PIPELINE_STAGE_VERTEX: return "oD3D11_PIPELINE_STAGE_VERTEX";
		case oD3D11_PIPELINE_STAGE_HULL: return "oD3D11_PIPELINE_STAGE_HULL";
		case oD3D11_PIPELINE_STAGE_DOMAIN: return "oD3D11_PIPELINE_STAGE_DOMAIN";
		case oD3D11_PIPELINE_STAGE_GEOMETRY: return "oD3D11_PIPELINE_STAGE_GEOMETRY";
		case oD3D11_PIPELINE_STAGE_PIXEL: return "oD3D11_PIPELINE_STAGE_PIXEL";
		oNODEFAULT;
	}
}

const char* oD3D11GetShaderProfile(ID3D11Device* _pDevice, oD3D11_PIPELINE_STAGE _Stage)
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
	switch (_pDevice->GetFeatureLevel())
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
		oErrorSetLast(oERROR_NOT_FOUND, "Shader profile does not exist for D3D%.2f's stage %s", oGetD3DVersion(_pDevice->GetFeatureLevel()), oAsString(_Stage));

	return profile;
}

bool oD3D11ConvertCompileErrorBuffer(char* _OutErrorMessageString, size_t _SizeofOutErrorMessageString, ID3DBlob* _pErrorMessages)
{
	if (!_OutErrorMessageString)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	if (_pErrorMessages)
	{
		errno_t err = oReplace(_OutErrorMessageString, _SizeofOutErrorMessageString, (const char*)_pErrorMessages->GetBufferPointer(), "%", "%%");
		if (err)
		{
			oErrorSetLast(err == STRUNCATE ? oERROR_AT_CAPACITY : oERROR_INVALID_PARAMETER);
			return false;
		}
	}

	else
		*_OutErrorMessageString = 0;

	return true;
}

size_t oD3D11GetEncodedByteCodeSize(const BYTE* _pByteCode)
{
	// Discovered empirically
	return _pByteCode ? ((const unsigned int*)_pByteCode)[6] : 0;
}

void oD3D11SetConstantBuffers(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot, size_t _NumBuffers, const ID3D11Buffer* const* _ppConstantBuffers)
{
	// COM-base APIs like DirectX have legitimate const-correctness issues: how 
	// can you call AddRef() or Release() on a const interface? But that's 
	// somewhat pedantic for the most case. Especially in DX11 where the device/
	// context no longer refs the set objects. So in this wrapper encapsulate the 
	// weirdness and expose a more expected API.
	ID3D11Buffer* const* ppConstantBuffers = const_cast<ID3D11Buffer* const*>(_ppConstantBuffers);
	UINT s = static_cast<UINT>(_StartSlot);
	UINT n = static_cast<UINT>(_NumBuffers);
	_pDeviceContext->VSSetConstantBuffers(s, n, ppConstantBuffers);
	_pDeviceContext->HSSetConstantBuffers(s, n, ppConstantBuffers);
	_pDeviceContext->DSSetConstantBuffers(s, n, ppConstantBuffers);
	_pDeviceContext->GSSetConstantBuffers(s, n, ppConstantBuffers);
	_pDeviceContext->PSSetConstantBuffers(s, n, ppConstantBuffers);
}

void oD3D11SetSamplers(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot, size_t _NumSamplers, const ID3D11SamplerState* const* _ppSamplers)
{
	// See oD3D11SetConstantBuffers for an explanation of this cast
	ID3D11SamplerState* const* ppSamplers = const_cast<ID3D11SamplerState* const*>(_ppSamplers);
	UINT s = static_cast<UINT>(_StartSlot);
	UINT n = static_cast<UINT>(_NumSamplers);
	_pDeviceContext->VSSetSamplers(s, n, ppSamplers);
	_pDeviceContext->HSSetSamplers(s, n, ppSamplers);
	_pDeviceContext->DSSetSamplers(s, n, ppSamplers);
	_pDeviceContext->GSSetSamplers(s, n, ppSamplers);
	_pDeviceContext->PSSetSamplers(s, n, ppSamplers);
}

void oD3D11SetShaderResourceViews(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot, size_t _NumShaderResourceViews, const ID3D11ShaderResourceView* const* _ppViews)
{
	// See oD3D11SetConstantBuffers for an explanation of this cast
	ID3D11ShaderResourceView* const* ppViews = const_cast<ID3D11ShaderResourceView* const*>(_ppViews);
	UINT s = static_cast<UINT>(_StartSlot);
	UINT n = static_cast<UINT>(_NumShaderResourceViews);
	_pDeviceContext->VSSetShaderResources(s, n, ppViews);
	_pDeviceContext->HSSetShaderResources(s, n, ppViews);
	_pDeviceContext->DSSetShaderResources(s, n, ppViews);
	_pDeviceContext->GSSetShaderResources(s, n, ppViews);
	_pDeviceContext->PSSetShaderResources(s, n, ppViews);
}

void oD3D11GetTextureDesc(ID3D11Resource* _pResource, oD3D11_TEXTURE_DESC* _pDesc)
{
	D3D11_RESOURCE_DIMENSION type = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	_pResource->GetType(&type);
	switch (type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			D3D11_TEXTURE1D_DESC desc;
			static_cast<ID3D11Texture1D*>(_pResource)->GetDesc(&desc);
			_pDesc->Width = desc.Width;
			_pDesc->Height = 1;
			_pDesc->Depth = 1;
			_pDesc->ArraySize = desc.ArraySize;
			_pDesc->MipLevels = desc.MipLevels;
			_pDesc->Format = desc.Format;
			_pDesc->SampleDesc.Count = 0;
			_pDesc->SampleDesc.Quality = 0;
			_pDesc->Usage = desc.Usage;
			_pDesc->BindFlags = desc.BindFlags;
			_pDesc->CPUAccessFlags = desc.CPUAccessFlags;
			_pDesc->MiscFlags = desc.MiscFlags;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			D3D11_TEXTURE2D_DESC desc;
			static_cast<ID3D11Texture2D*>(_pResource)->GetDesc(&desc);
			_pDesc->Width = desc.Width;
			_pDesc->Height = desc.Height;
			_pDesc->Depth = 1;
			_pDesc->ArraySize = desc.ArraySize;
			_pDesc->MipLevels = desc.MipLevels;
			_pDesc->Format = desc.Format;
			_pDesc->SampleDesc = desc.SampleDesc;
			_pDesc->Usage = desc.Usage;
			_pDesc->BindFlags = desc.BindFlags;
			_pDesc->CPUAccessFlags = desc.CPUAccessFlags;
			_pDesc->MiscFlags = desc.MiscFlags;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			static_cast<ID3D11Texture3D*>(_pResource)->GetDesc(&desc);
			_pDesc->Width = desc.Width;
			_pDesc->Height = desc.Height;
			_pDesc->Depth = desc.Depth;
			_pDesc->ArraySize = 1;
			_pDesc->MipLevels = desc.MipLevels;
			_pDesc->Format = desc.Format;
			_pDesc->SampleDesc.Count = 0;
			_pDesc->SampleDesc.Quality = 0;
			_pDesc->Usage = desc.Usage;
			_pDesc->BindFlags = desc.BindFlags;
			_pDesc->CPUAccessFlags = desc.CPUAccessFlags;
			_pDesc->MiscFlags = desc.MiscFlags;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			D3D11_BUFFER_DESC desc;
			static_cast<ID3D11Buffer*>(_pResource)->GetDesc(&desc);
			_pDesc->Width = desc.ByteWidth;
			_pDesc->Height = 1;
			_pDesc->Depth = 1;
			_pDesc->ArraySize = desc.ByteWidth / desc.StructureByteStride;
			_pDesc->Format = DXGI_FORMAT_UNKNOWN;
			_pDesc->MipLevels = 1;
			_pDesc->SampleDesc.Count = 0;
			_pDesc->SampleDesc.Quality = 0;
			_pDesc->Usage = desc.Usage;
			_pDesc->BindFlags = desc.BindFlags;
			_pDesc->CPUAccessFlags = desc.CPUAccessFlags;
			_pDesc->MiscFlags = desc.MiscFlags;
			break;
		};

		oNODEFAULT;
	}
}

bool oD3D11CreateTexture2D(ID3D11Device* _pDevice, const char* _DebugName, UINT _Width, UINT _Height, UINT _ArraySize, DXGI_FORMAT _Format, oD3D11_TEXTURE_CREATION_TYPE _CreationType, ID3D11Texture2D** _ppTexture, ID3D11ShaderResourceView** _ppShaderResourceView)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = _Width;
	desc.Height = _Height;
	desc.MipLevels = (_CreationType == oD3D11_MIPPED_TEXTURE || _CreationType == oD3D11_MIPPED_RENDER_TARGET) ? 0 : 1;
	desc.ArraySize = __max(1, _ArraySize);
	desc.Format = _Format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = (_CreationType == oD3D11_DYNAMIC_TEXTURE) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	if (oDXGIIsDepthFormat(_Format))
		desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	else if (_CreationType == oD3D11_RENDER_TARGET || _CreationType == oD3D11_MIPPED_RENDER_TARGET)
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

	desc.CPUAccessFlags = (_CreationType == oD3D11_DYNAMIC_TEXTURE) ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.MiscFlags = (_CreationType == oD3D11_MIPPED_RENDER_TARGET) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	HRESULT hr = _pDevice->CreateTexture2D(&desc, 0, _ppTexture);
	oDEBUG_CHECK_BUFFER(oD3D11CreateTexture2D, _ppTexture);

	if (_ppShaderResourceView && !oD3D11CreateShaderResourceView(_DebugName, *_ppTexture, _ppShaderResourceView))
		return false;

	return true;
}

bool oD3D11CreateSnapshot(ID3D11Texture2D* _pRenderTarget, interface oImage** _ppImage)
{
	if (!_pRenderTarget || !_ppImage)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	oRef<ID3D11Device> D3D11Device;
	_pRenderTarget->GetDevice(&D3D11Device);
	oRef<ID3D11DeviceContext> D3D11DeviceContext;
	D3D11Device->GetImmediateContext(&D3D11DeviceContext);

	D3D11_TEXTURE2D_DESC d;
	_pRenderTarget->GetDesc(&d);
	d.BindFlags = 0;
	d.Usage = D3D11_USAGE_STAGING;
	d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	oRef<ID3D11Texture2D> CPUTexture;
	oV(D3D11Device->CreateTexture2D(&d, nullptr, &CPUTexture));
	D3D11DeviceContext->CopyResource(CPUTexture, _pRenderTarget);
	D3D11DeviceContext->Flush();

	const unsigned int RowSize = static_cast<unsigned int>(d.Width * oSurfaceGetSize(oSURFACE_B8G8R8A8_UNORM));
	oImage::DESC idesc;
	idesc.Pitch = RowSize;
	idesc.Dimensions.x = d.Width;
	idesc.Dimensions.y = d.Height;
	idesc.Format = oSURFACE_B8G8R8A8_UNORM;
	oVERIFY(oImageCreate("Temp Image", &idesc, _ppImage));
	void* pIData = (*_ppImage)->GetData();
	D3D11_MAPPED_SUBRESOURCE mapped;
	oV(D3D11DeviceContext->Map(CPUTexture, 0, D3D11_MAP_READ, 0, &mapped));
	
	if (mapped.RowPitch != RowSize || idesc.Pitch != RowSize)
		oMemcpy2d(pIData, idesc.Pitch, mapped.pData, mapped.RowPitch, RowSize, d.Height);
	else
		memcpy(pIData, mapped.pData, RowSize * d.Height);

	D3D11DeviceContext->Unmap(CPUTexture, 0);

	(*_ppImage)->FlipVertical();

	return true;
}

size_t oD3D11GetNumElements(D3D11_PRIMITIVE_TOPOLOGY _PrimitiveTopology, size_t _NumPrimitives)
{
	switch (_PrimitiveTopology)
	{
		case D3D11_PRIMITIVE_TOPOLOGY_POINTLIST: return _NumPrimitives;
		case D3D11_PRIMITIVE_TOPOLOGY_LINELIST: return _NumPrimitives * 2;
		case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP: return _NumPrimitives + 1;
		case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST: return _NumPrimitives * 3;
		case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return _NumPrimitives + 2;
		oNODEFAULT;
	}
}

void oD3D11Draw(ID3D11DeviceContext* _pDeviceContext
							 , size_t _NumElements
							 , size_t _NumVertexBuffers
							 , const ID3D11Buffer* const* _ppVertexBuffers
							 , const unsigned int* _VertexStrides
							 , size_t _IndexOfFirstVertexToDraw
							 , size_t _OffsetToAddToEachVertexIndex
							 , const ID3D11Buffer* _IndexBuffer
							 , bool _32BitIndexBuffer
							 , size_t _IndexOfFirstIndexToDraw
							 , size_t _NumInstances
							 , size_t _IndexOfFirstInstanceIndexToDraw)
{
	static UINT sOffsets[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	// DirectX as an API has a funny definition for const, probably because of 
	// ref-counting, so consider it a platform-quirk and keep const correctness
	// above this API, but cast it away as DirectX requires here...

	_pDeviceContext->IASetVertexBuffers(0, static_cast<UINT>(_NumVertexBuffers), const_cast<ID3D11Buffer* const*>(_ppVertexBuffers), _VertexStrides, sOffsets);

	if (_IndexBuffer)
	{
		_pDeviceContext->IASetIndexBuffer(const_cast<ID3D11Buffer*>(_IndexBuffer), _32BitIndexBuffer ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, static_cast<UINT>(_IndexOfFirstIndexToDraw * (_32BitIndexBuffer ? 4 : 2)));

		if (_NumInstances)
			_pDeviceContext->DrawIndexedInstanced(static_cast<UINT>(_NumElements), static_cast<UINT>(_NumInstances), static_cast<UINT>(_IndexOfFirstIndexToDraw), static_cast<UINT>(_OffsetToAddToEachVertexIndex), static_cast<UINT>(_IndexOfFirstInstanceIndexToDraw));
		else
			_pDeviceContext->DrawIndexed(static_cast<UINT>(_NumElements), 0, static_cast<UINT>(_OffsetToAddToEachVertexIndex));
	}

	else
	{
		if (_NumInstances)
			_pDeviceContext->DrawInstanced(static_cast<UINT>(_NumElements), static_cast<UINT>(_NumInstances), static_cast<UINT>(_IndexOfFirstVertexToDraw), static_cast<UINT>(_IndexOfFirstInstanceIndexToDraw));
		else
			_pDeviceContext->Draw(static_cast<UINT>(_NumElements), static_cast<UINT>(_IndexOfFirstVertexToDraw));
	}
}

void oD3D11DrawSVQuad(ID3D11DeviceContext* _pDeviceContext, size_t _NumInstances)
{
	ID3D11Buffer* pVertexBuffers[] = { 0, 0 };
	UINT pStrides[] = { 0, 0 };
	UINT pOffsets[] = { 0, 0 };
	_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffers, pStrides, pOffsets);
	_pDeviceContext->DrawInstanced(4, static_cast<UINT>(_NumInstances), 0, 0);
}

void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11Texture2D* _pRenderTargetResource, float _MinDepth, float _MaxDepth)
{
	D3D11_TEXTURE2D_DESC desc;
	_pRenderTargetResource->GetDesc(&desc);

	D3D11_VIEWPORT v;
	v.TopLeftX = 0.0f;
	v.TopLeftY = 0.0f;
	v.Width = static_cast<float>(desc.Width);
	v.Height = static_cast<float>(desc.Height);
	v.MinDepth = 0.0f;
	v.MaxDepth = 1.0f;
	_pDeviceContext->RSSetViewports(1, &v);
}

void oD3D11SetFullTargetViewport(ID3D11DeviceContext* _pDeviceContext, ID3D11RenderTargetView* _pRenderTargetView, float _MinDepth, float _MaxDepth)
{
	oRef<ID3D11Texture2D> texture;
	_pRenderTargetView->GetResource((ID3D11Resource**)&texture);
#ifdef _DEBUG
	D3D11_RESOURCE_DIMENSION dimension;
	texture->GetType(&dimension);
	oASSERT(dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D, "Unexpected resource type");
#endif
	oD3D11SetFullTargetViewport(_pDeviceContext, texture, _MinDepth, _MaxDepth);
}

void oD3D11CopyStructs(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pBuffer, const void* _pStructs)
{
	D3D11_BUFFER_DESC desc;
	_pBuffer->GetDesc(&desc);

	switch (desc.Usage)
	{
		case D3D11_USAGE_DYNAMIC:
		{
			D3D11_MAPPED_SUBRESOURCE MSR;
			_pDeviceContext->Map(_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MSR);
			memcpy(MSR.pData, _pStructs, desc.ByteWidth);
			_pDeviceContext->Unmap(_pBuffer, 0);
			break;
		}

		case D3D11_USAGE_DEFAULT:
			_pDeviceContext->UpdateSubresource(_pBuffer, 0, 0, _pStructs, desc.ByteWidth, 0);
			break;

		oNODEFAULT;
	}
}

oD3D11RasterizerState::oD3D11RasterizerState(ID3D11Device* _pDevice)
{
	static const D3D11_FILL_MODE sFills[] = 
	{
		D3D11_FILL_SOLID,
		D3D11_FILL_SOLID,
		D3D11_FILL_SOLID,
		D3D11_FILL_WIREFRAME,
		D3D11_FILL_WIREFRAME,
		D3D11_FILL_WIREFRAME,
		D3D11_FILL_SOLID,
		D3D11_FILL_SOLID,
		D3D11_FILL_SOLID,
	};
	static_assert(oCOUNTOF(sFills) == NUM_STATES, "");

	static const D3D11_CULL_MODE sCulls[] = 
	{
		D3D11_CULL_BACK,
		D3D11_CULL_FRONT,
		D3D11_CULL_NONE,
		D3D11_CULL_BACK,
		D3D11_CULL_FRONT,
		D3D11_CULL_NONE,
		D3D11_CULL_BACK,
		D3D11_CULL_FRONT,
		D3D11_CULL_NONE,
	};
	static_assert(oCOUNTOF(sCulls) == NUM_STATES, "");

	D3D11_RASTERIZER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.DepthClipEnable = TRUE;

	for (size_t i = 0; i < oCOUNTOF(States); i++)
	{
		desc.FillMode = sFills[i];
		desc.CullMode = sCulls[i];
		oV(_pDevice->CreateRasterizerState(&desc, &States[i]));
	}
}

void oD3D11RasterizerState::SetState(ID3D11DeviceContext* _pDeviceContext, STATE _State)
{
	_pDeviceContext->IASetPrimitiveTopology(_State >= FRONT_POINTS ? D3D11_PRIMITIVE_TOPOLOGY_POINTLIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_pDeviceContext->RSSetState(States[_State]);
}

oD3D11RasterizerState::~oD3D11RasterizerState()
{
	for (size_t i = 0; i < oCOUNTOF(States); i++)
		States[i]->Release();
}

oD3D11BlendState::oD3D11BlendState(ID3D11Device* _pDevice)
{
	static const D3D11_RENDER_TARGET_BLEND_DESC sBlends[] =
	{
		{ FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
	};
	static_assert(oCOUNTOF(sBlends) == NUM_STATES, "");

	D3D11_BLEND_DESC desc = {0};
	for (size_t i = 0; i < oCOUNTOF(States); i++)
	{
		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = FALSE;
		desc.RenderTarget[0] = sBlends[i];
		oV(_pDevice->CreateBlendState(&desc, &States[i]));
	}
}

oD3D11BlendState::~oD3D11BlendState()
{
	for (size_t i = 0; i < oCOUNTOF(States); i++)
		States[i]->Release();
}

void oD3D11BlendState::SetState(ID3D11DeviceContext* _pDeviceContext, STATE _State)
{
	static const FLOAT sBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_pDeviceContext->OMSetBlendState(States[_State], sBlendFactor, 0xffffffff);
}

void oD3D11BlendState::SetDefaultState(ID3D11DeviceContext* _pDeviceContext)
{
	static const FLOAT sBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_pDeviceContext->OMSetBlendState(0, sBlendFactor, 0xffffffff);
}

oD3D11DepthStencilState::oD3D11DepthStencilState(ID3D11Device* _pDevice)
{
	D3D11_DEPTH_STENCIL_DESC desc = {0};

	oV(_pDevice->CreateDepthStencilState(&desc, &States[DEPTH_STENCIL_OFF]));

	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	oV(_pDevice->CreateDepthStencilState(&desc, &States[DEPTH_TEST_AND_WRITE]));

	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	oV(_pDevice->CreateDepthStencilState(&desc, &States[DEPTH_TEST]));
}

oD3D11DepthStencilState::~oD3D11DepthStencilState()
{
	for (size_t i = 0; i < oCOUNTOF(States); i++)
		States[i]->Release();
}

oD3D11SamplerState::oD3D11SamplerState(ID3D11Device* _pDevice)
{
	static const D3D11_FILTER sFilters[] = 
	{
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_COMPARISON_ANISOTROPIC,
		D3D11_FILTER_COMPARISON_ANISOTROPIC,
	};
	static_assert(oCOUNTOF(sFilters) == NUM_SAMPLER_STATES, "");

	static const D3D11_TEXTURE_ADDRESS_MODE sAddresses[] = 
	{
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
	};
	static_assert(oCOUNTOF(sAddresses) == NUM_SAMPLER_STATES, "");

	static const FLOAT sBiases[] =
	{
		0.0f,
		-1.0f,
		-2.0f,
		1.0f,
		2.0f,
	};
	static_assert(oCOUNTOF(sBiases) == NUM_MIP_BIAS_LEVELS, "");

	D3D11_SAMPLER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	for (size_t state = 0; state < NUM_SAMPLER_STATES; state++)
	{
		desc.Filter = sFilters[state];
		desc.AddressU = desc.AddressV = desc.AddressW = sAddresses[state];
		desc.MaxLOD = FLT_MAX; // documented default
		desc.MaxAnisotropy = 16; // documented default
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;  // documented default

		for (size_t bias = 0; bias < NUM_MIP_BIAS_LEVELS; bias++)
		{
			desc.MipLODBias = sBiases[bias];
			_pDevice->CreateSamplerState(&desc, &States[state][bias]);
		}
	}
}

oD3D11SamplerState::~oD3D11SamplerState()
{
	for (size_t state = 0; state < NUM_SAMPLER_STATES; state++)
		for (size_t bias = 0; bias < NUM_MIP_BIAS_LEVELS; bias++)
			States[state][bias]->Release();
}

void oD3D11SamplerState::SetState(ID3D11DeviceContext* _pDeviceContext, PIPELINE_STAGE _Stage, size_t _StartSlot, const SAMPLER_STATE* _States, const MIP_BIAS_LEVEL* _Levels, size_t _NumSamplers)
{
	ID3D11SamplerState* Samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
	oASSERT(oCOUNTOF(Samplers) > _NumSamplers, "Too many samplers specified");

	for (size_t i = 0; i < _NumSamplers; i++)
		Samplers[i] = States[_States[i]][_Levels[i]];

	switch (_Stage)
	{
		case VERTEX_SHADER: _pDeviceContext->VSSetSamplers(static_cast<UINT>(_StartSlot), static_cast<UINT>(_NumSamplers), Samplers); break;
		case HULL_SHADER: _pDeviceContext->HSSetSamplers(static_cast<UINT>(_StartSlot), static_cast<UINT>(_NumSamplers), Samplers); break;
		case DOMAIN_SHADER: _pDeviceContext->DSSetSamplers(static_cast<UINT>(_StartSlot), static_cast<UINT>(_NumSamplers), Samplers); break;
		case GEOMETRY_SHADER: _pDeviceContext->GSSetSamplers(static_cast<UINT>(_StartSlot), static_cast<UINT>(_NumSamplers), Samplers); break;
		case PIXEL_SHADER: _pDeviceContext->PSSetSamplers(static_cast<UINT>(_StartSlot), static_cast<UINT>(_NumSamplers), Samplers); break;
		oNODEFAULT;
	}
}

void oD3D11SamplerState::SetState(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot, const SAMPLER_STATE* _States, const MIP_BIAS_LEVEL* _Levels, size_t _NumSamplers)
{
	ID3D11SamplerState* Samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
	oASSERT(oCOUNTOF(Samplers) > _NumSamplers, "Too many samplers specified");

	for (size_t i = 0; i < _NumSamplers; i++)
		Samplers[i] = States[_States[i]][_Levels[i]];

	oD3D11SetSamplers(_pDeviceContext, static_cast<UINT>(_StartSlot), static_cast<UINT>(_NumSamplers), Samplers);
}

void oD3D11ShaderState::SetState(ID3D11DeviceContext* _pDeviceContext, size_t _Index)
{
	oASSERT(_Index < States.size(), "Invalid shader state %u specified.", _Index);
	STATE& s = States[_Index];
	_pDeviceContext->IASetInputLayout(s.InputLayout);
	_pDeviceContext->VSSetShader(s.VertexShader, 0, 0);
	_pDeviceContext->HSSetShader(s.HullShader, 0, 0);
	_pDeviceContext->DSSetShader(s.DomainShader, 0, 0);
	_pDeviceContext->GSSetShader(s.GeometryShader, 0, 0);
	_pDeviceContext->PSSetShader(s.PixelShader, 0, 0);
}

void oD3D11ShaderState::ClearState(ID3D11DeviceContext* _pDeviceContext)
{
	_pDeviceContext->IASetInputLayout(0);
	_pDeviceContext->VSSetShader(0, 0, 0);
	_pDeviceContext->HSSetShader(0, 0, 0);
	_pDeviceContext->DSSetShader(0, 0, 0);
	_pDeviceContext->GSSetShader(0, 0, 0);
	_pDeviceContext->PSSetShader(0, 0, 0);
}

void oD3D11ShaderState::RegisterState(size_t _Index, STATE& _State)
{
	oSafeSet(States, _Index, _State);
}

void oD3D11ShaderState::RegisterStates(ID3D11Device* _pDevice, const STATE_BYTE_CODE* _pStates, size_t _NumStates)
{
	// @oooii-tony: TODO: Optimize this so that shaders get shared

	oD3D11ShaderState::STATE s;
	for (unsigned int i = 0; i < _NumStates; i++)
	{
		s.Clear();
		oD3D11ShaderState::CreateState(_pDevice, &s, _pStates[i].InputLayoutDesc, _pStates[i].NumInputElements, _pStates[i].pVertexShader, _pStates[i].pHullShader, _pStates[i].pDomainShader, _pStates[i].pGeometryShader, _pStates[i].pPixelShader);
		RegisterState(i, s);
	}
}

void oD3D11ShaderState::CreateState(ID3D11Device* _pDevice
																		 , oD3D11ShaderState::STATE* _pState
																		 , const D3D11_INPUT_ELEMENT_DESC* _pElements, size_t _NumElements
																		 , const BYTE* _pByteCodeVS, size_t _SizeofByteCodeVS
																		 , const BYTE* _pByteCodeHS, size_t _SizeofByteCodeHS
																		 , const BYTE* _pByteCodeDS, size_t _SizeofByteCodeDS
																		 , const BYTE* _pByteCodeGS, size_t _SizeofByteCodeGS
																		 , const BYTE* _pByteCodePS, size_t _SizeofByteCodePS)
{
	if (_pElements && _pByteCodeVS)
		oV(_pDevice->CreateInputLayout(_pElements, static_cast<UINT>(_NumElements), _pByteCodeVS, _SizeofByteCodeVS, &_pState->InputLayout));
	if (_pByteCodeVS)
		oV(_pDevice->CreateVertexShader(_pByteCodeVS, _SizeofByteCodeVS, 0, &_pState->VertexShader));
	if (_pByteCodeHS)
		oV(_pDevice->CreateHullShader(_pByteCodeHS, _SizeofByteCodeHS, 0, &_pState->HullShader));
	if (_pByteCodeDS)
		oV(_pDevice->CreateDomainShader(_pByteCodeDS, _SizeofByteCodeDS, 0, &_pState->DomainShader));
	if (_pByteCodeGS)
		oV(_pDevice->CreateGeometryShader(_pByteCodeGS, _SizeofByteCodeGS, 0, &_pState->GeometryShader));
	if (_pByteCodePS)
		oV(_pDevice->CreatePixelShader(_pByteCodePS, _SizeofByteCodePS, 0, &_pState->PixelShader));
}

oD3D11RenderTarget::oD3D11RenderTarget(const char* _DebugName, ID3D11Device* _pDevice)
	: Device(_pDevice)
{
	strcpy_s(DebugName, _DebugName);
	memset(&Desc, 0, sizeof(Desc));
}

void oD3D11RenderTarget::GetDesc(DESC* _pDesc) const
{
	*_pDesc = Desc;
}

void oD3D11RenderTarget::SetDesc(const DESC& _Desc)
{
	bool needsResize = !!memcmp(&Desc, &_Desc, offsetof(DESC, ClearColor)) && _Desc.Width && _Desc.Height;
	Desc = _Desc;
	if (needsResize)
		Resize(Desc.Width, Desc.Height);
}

size_t oD3D11RenderTarget::GetNumSRVs() const
{
	return Desc.NumTargets + (SRVDepth ? 1 : 0);
}

void oD3D11RenderTarget::Resize(unsigned int _Width, unsigned int _Height)
{
	if (_Width != Desc.Width || _Height != Desc.Height)
	{
		oTRACE("oD3D11RenderTarget \"%s\" resizing %ux%u -> %ux%u", DebugName, Desc.Width, Desc.Height, _Width, _Height);

		Desc.Width = _Width;
		Desc.Height = _Height;

		if (!Desc.Width || !Desc.Height)
		{
			for (unsigned int i = 0; i < oCOUNTOF(Texture); i++)
			{
				Texture[i] = 0;
				RTVs[i] = 0;
				SRVs[i] = 0;
			}

			Depth = 0;
			DSV = 0;
			SRVDepth = 0;
		}

		else
		{
			char name[1024];

			for (unsigned int i = 0; i < Desc.NumTargets; i++)
			{
				sprintf_s(name, "%s%02u", oSAFESTRN(DebugName), i);
				oVERIFY(oD3D11CreateRenderTarget(Device, name, Desc.Width, Desc.Height, Desc.ArraySize, Desc.Format[i], &Texture[i], &RTVs[i], &SRVs[i]));
			}

			if (Desc.DepthStencilFormat != DXGI_FORMAT_UNKNOWN)
				oVERIFY(oD3D11CreateRenderTarget(Device, name, Desc.Width, Desc.Height, Desc.ArraySize, Desc.DepthStencilFormat, &Depth, &DSV, &SRVDepth));
		}
	}
}

void oD3D11RenderTarget::Clear(ID3D11DeviceContext* _pDeviceContext, CLEAR_TYPE _ClearType)
{
	if (_ClearType >= COLOR)
	{
		for (unsigned int i = 0; i < Desc.NumTargets; i++)
			_pDeviceContext->ClearRenderTargetView(RTVs[i], Desc.ClearColor[i]);
	}

	static const UINT clearFlags[] = 
	{
		D3D11_CLEAR_DEPTH,
		D3D11_CLEAR_STENCIL,
		D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,
		0,
		D3D11_CLEAR_DEPTH,
		D3D11_CLEAR_STENCIL,
		D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,
	};

	if (DSV && _ClearType != COLOR)
		_pDeviceContext->ClearDepthStencilView(DSV, clearFlags[_ClearType], Desc.DepthClearValue, Desc.StencilClearValue);
}

void oD3D11RenderTarget::SetRenderTargets(ID3D11DeviceContext* _pDeviceContext)
{
	_pDeviceContext->OMSetRenderTargets(Desc.NumTargets, (ID3D11RenderTargetView* const*)RTVs, DSV);
}

void oD3D11RenderTarget::SetDefaultViewport(ID3D11DeviceContext* _pDeviceContext)
{
	oD3D11SetFullTargetViewport(_pDeviceContext, RTVs[0]);
}

void oD3D11RenderTarget::SetShaderResources(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot)
{
	ID3D11ShaderResourceView* srvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	memcpy(srvs, SRVs, Desc.NumTargets * sizeof(ID3D11ShaderResourceView*));
	srvs[Desc.NumTargets] = SRVDepth;
	_pDeviceContext->PSSetShaderResources(static_cast<UINT>(_StartSlot), Desc.NumTargets + (SRVDepth ? 1 : 0), srvs);
}

void oD3D11RenderTarget::SetSingleShaderResource(ID3D11DeviceContext* _pDeviceContext, size_t _RenderTargetIndex, size_t _StartSlot)
{
	if (_RenderTargetIndex == DEPTH_STENCIL_INDEX)
		_pDeviceContext->PSSetShaderResources(static_cast<UINT>(_StartSlot), 1, &SRVDepth);
	else
	{
		ID3D11ShaderResourceView* SRV = SRVs[_RenderTargetIndex];
		_pDeviceContext->PSSetShaderResources(static_cast<UINT>(_StartSlot), 1, &SRV);
	}
}

void oD3D11RenderTarget::SetCSShaderResources(ID3D11DeviceContext* _pDeviceContext, size_t _StartSlot)
{
	ID3D11ShaderResourceView* srvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	memcpy(srvs, SRVs, Desc.NumTargets * sizeof(ID3D11ShaderResourceView*));
	srvs[Desc.NumTargets] = SRVDepth;
	_pDeviceContext->CSSetShaderResources(static_cast<UINT>(_StartSlot), Desc.NumTargets + (SRVDepth ? 1 : 0), srvs);
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
