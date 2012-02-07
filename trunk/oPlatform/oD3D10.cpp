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
#include <oPlatform/oD3D10.h>
#include <oBasis/oFor.h>
#include <oBasis/oMemory.h>
#include <oPlatform/oDXGI.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oModule.h>
#include <oHLSLFSQuadByteCode.h>
#include <oHLSLPassthroughByteCode.h>

static const char* d3d10_dll_functions[] = 
{
	"D3D10CreateDevice1",
};

oD3D10::oD3D10()
{
	hD3D10 = oModuleLinkSafe("d3d10_1.dll", d3d10_dll_functions, (void**)&D3D10CreateDevice1, oCOUNTOF(d3d10_dll_functions));
	oASSERT(hD3D10, "");
}

oD3D10::~oD3D10()
{
	oModuleUnlink(hD3D10);
}

bool oD3D10CreateDevice(const int2& _VirtualDesktopPosition, ID3D10Device1** _ppDevice)
{
	if (!_ppDevice)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oRef<IDXGIFactory1> Factory;
	if (!oDXGICreateFactory(&Factory))
		return false;

	oRef<IDXGIOutput> Output;
	if (!oDXGIFindOutput(Factory, _VirtualDesktopPosition, &Output))
		return false;

	oRef<IDXGIAdapter1> Adapter;
	oVB_RETURN2(Output->GetParent(__uuidof(IDXGIAdapter1), (void**)&Adapter));

	HRESULT hr = oD3D10::Singleton()->D3D10CreateDevice1(Adapter, D3D10_DRIVER_TYPE_HARDWARE, nullptr, D3D10_CREATE_DEVICE_BGRA_SUPPORT, D3D10_FEATURE_LEVEL_10_1, D3D10_1_SDK_VERSION, _ppDevice);
	switch (hr)
	{
		case S_OK:
			break;

		case E_NOINTERFACE:
		{
			oTRACE("Failed to create D3D 10.1 device, falling back to D3D 10.0.");
			if (FAILED(oD3D10::Singleton()->D3D10CreateDevice1(Adapter, D3D10_DRIVER_TYPE_HARDWARE, nullptr, D3D10_CREATE_DEVICE_BGRA_SUPPORT, D3D10_FEATURE_LEVEL_10_0, D3D10_1_SDK_VERSION, _ppDevice)))
				return oErrorSetLast(oERROR_NOT_FOUND, "No D3D 10 or 10.1 device could be found");
			break;
		}
		default:
			return oWinSetLastError(hr);
	}

	return true;
}

bool oD3D10CreateImage(ID3D10Texture2D* _pSourceTexture, oImage** _ppImage)
{
	D3D10_TEXTURE2D_DESC d;
	_pSourceTexture->GetDesc(&d);

	oImage::FORMAT ImageFormat = oImageFormatFromSurfaceFormat(oDXGIToSurfaceFormat(d.Format));
	if (ImageFormat == oImage::UNKNOWN)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "The specified texture's format %s is not supported by oImage", oAsString(d.Format));

	oImage::DESC idesc;
	idesc.RowPitch = oImageCalcRowSize(ImageFormat, d.Width);
	idesc.Dimensions.x = d.Width;
	idesc.Dimensions.y = d.Height;
	idesc.Format = oImage::BGRA32;
	oVERIFY(oImageCreate("Snapshot image", idesc, _ppImage));
	void* pData = (*_ppImage)->GetData();

	D3D10_MAPPED_TEXTURE2D mapped;
	oV(_pSourceTexture->Map(0, D3D10_MAP_READ, 0, &mapped));
	(*_ppImage)->CopyData(mapped.pData, mapped.RowPitch, oImage::FlipVertical);
	_pSourceTexture->Unmap(0);
	return true;
}

bool oD3D10CreateSnapshot(ID3D10Texture2D* _pRenderTarget, ID3D10Texture2D** _ppCPUTexture)
{
	if (!_pRenderTarget || !_ppCPUTexture)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oRef<ID3D10Device> D3D10Device;
	_pRenderTarget->GetDevice(&D3D10Device);

	D3D10_TEXTURE2D_DESC d;
	_pRenderTarget->GetDesc(&d);
	d.BindFlags = 0;
	d.Usage = D3D10_USAGE_STAGING;
	d.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
	oV(D3D10Device->CreateTexture2D(&d, nullptr, _ppCPUTexture));
	D3D10Device->CopyResource(*_ppCPUTexture, _pRenderTarget);
	D3D10Device->Flush();
	return true;
}

bool oD3D10CreateSnapshot(ID3D10Texture2D* _pRenderTarget, oImage** _ppImage)
{
	if (!_pRenderTarget || !_ppImage)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oRef<ID3D10Texture2D> CPUTexture;
	if (!oD3D10CreateSnapshot(_pRenderTarget, &CPUTexture))
		return false; // pass through error

	return oD3D10CreateImage(CPUTexture, _ppImage);
}

class oD3D10DeviceManagerImpl : public oD3D10DeviceManager
{
public:
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_NOOP_QUERYINTERFACE();
	oD3D10DeviceManagerImpl(const DESC& _Desc, bool* _pSuccess);

	bool EnumDevices(size_t n, ID3D10Device1** _ppDevice ) override;
	bool GetDevice(const RECT& _Rect, ID3D10Device1** _ppDevice) override;

private:
	oRefCount Refcount;
	DESC Desc;
	std::vector<oRef<ID3D10Device1>> Devices;
	std::vector<oRef<IDXGIAdapter1>> Adapters;
};

bool oD3D10DeviceManagerCreate(const oD3D10DeviceManager::DESC& _Desc, oD3D10DeviceManager** _ppDeviceManager)
{
	if (!_ppDeviceManager)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	bool success = false;
	oCONSTRUCT( _ppDeviceManager, oD3D10DeviceManagerImpl(_Desc, &success) );
	return success;
}

oD3D10DeviceManagerImpl::oD3D10DeviceManagerImpl(const DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
{
	*_pSuccess = false;
	oD3D10* pD3D10 = oD3D10::Singleton();
	if(!pD3D10)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed to load D3D10");
		return;
	}

	oRef<IDXGIFactory1> Factory;
	oDXGICreateFactory(&Factory);

	oRef<IDXGIAdapter1> Adapter;
	UINT a = 0;

#ifdef _DEBUG
	Desc.DeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

	while( S_OK == Factory->EnumAdapters1( a++, &Adapter) )
	{
		oRef<ID3D10Device1> Device;

		if( S_OK == pD3D10->D3D10CreateDevice1(Adapter, D3D10_DRIVER_TYPE_HARDWARE, nullptr, Desc.DeviceFlags, D3D10_FEATURE_LEVEL_10_1, D3D10_1_SDK_VERSION, &Device ) )
		{
			Devices.push_back(Device);
			Adapters.push_back(Adapter);
		}

		else if( S_OK == pD3D10->D3D10CreateDevice1(Adapter, D3D10_DRIVER_TYPE_HARDWARE, nullptr, Desc.DeviceFlags, D3D10_FEATURE_LEVEL_10_0, D3D10_1_SDK_VERSION, &Device ) )
		{
			Devices.push_back(Device);
			Adapters.push_back(Adapter);
		}
	}
	if( Adapters.empty() )
	{
		oErrorSetLast(oERROR_AT_CAPACITY, "Failed to find any D3D10 devices");
	}
	*_pSuccess = true;
}

bool oD3D10DeviceManagerImpl::EnumDevices( size_t n, ID3D10Device1** _ppDevice )
{
	if( n >= Devices.size() )
		return false;

	ID3D10Device1* pDevice = Devices[n];
	pDevice->AddRef();
	*_ppDevice = pDevice;
	return true;
}

bool oD3D10DeviceManagerImpl::GetDevice( const RECT& _Rect, ID3D10Device1** _ppDevice )
{
	oRef<ID3D10Device1> BestDevice;
	int BestDeviceCoverage = -1;
	oRECT TargetRect = oToRect( _Rect );

	unsigned int d = 0;
	oFOR( IDXGIAdapter1* Adapter, Adapters )
	{
		oRef<IDXGIOutput> Output;

		unsigned int o = 0;
		while (DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(o++, &Output) )
		{
			DXGI_OUTPUT_DESC desc;
			Output->GetDesc(&desc);

			oRECT ClippedRect = oClip(TargetRect, oToRect(desc.DesktopCoordinates));
			if( ClippedRect.IsEmpty() )
				continue;

			int2 dim = ClippedRect.GetDimensions();
			int coverage = dim.x * dim.y;
			if( coverage > BestDeviceCoverage )
			{
				BestDeviceCoverage = coverage;
				BestDevice = Devices[d];
			}
		}
		++d;
	}

	if( BestDevice )
	{
		BestDevice->AddRef();
		*_ppDevice = BestDevice;
		return true;
	}
	
	return false;
}

void oD3D10ShaderState::CreateShaders( ID3D10Device* _pDevice , oD3D10ShaderState::STATE* _pState , const BYTE* _pByteCodeVS, size_t _SizeofByteCodeVS , const BYTE* _pByteCodeGS, size_t _SizeofByteCodeGS , const BYTE* _pByteCodePS, size_t _SizeofByteCodePS )
{
	if (_pByteCodeVS)
		oV(_pDevice->CreateVertexShader(_pByteCodeVS, _SizeofByteCodeVS, &_pState->pVertexShader));
	if (_pByteCodeGS)
		oV(_pDevice->CreateGeometryShader(_pByteCodeGS, _SizeofByteCodeGS, &_pState->pGeometryShader));
	if (_pByteCodePS)
		oV(_pDevice->CreatePixelShader(_pByteCodePS, _SizeofByteCodePS, &_pState->pPixelShader));
}

void oD3D10ShaderState::RegisterState( unsigned int _Index, STATE& _State )
{
	oSafeSet(States, _Index, _State);
}

void oD3D10ShaderState::SetState( ID3D10Device* _pDevice, unsigned int _Index )
{
	oASSERT(_Index < States.size(), "Invalid shader state %u specified.", _Index);
	States[_Index].SetState(_pDevice);
}

void oD3D10ShaderState::STATE::SetState( ID3D10Device* _pDevice )
{
	_pDevice->IASetInputLayout(pInputLayout);
	_pDevice->VSSetShader(pVertexShader );
	_pDevice->GSSetShader(pGeometryShader );
	_pDevice->PSSetShader(pPixelShader );
}

void oD3D10FullscreenQuad::Create(ID3D10Device* _pDevice, const BYTE* _pPixelShaderByteCode, size_t _szBiteCode)
{
	CreateStates(_pDevice);

	oD3D10ShaderState::CreateShaders( _pDevice, &ShaderState, oHLSLFSQuadByteCode, oCOUNTOF( oHLSLFSQuadByteCode), nullptr, 0, _pPixelShaderByteCode, _szBiteCode );
}

void oD3D10FullscreenQuad::CreateStates(ID3D10Device* _pDevice)
{
	{ // Create our one and only raster state
		D3D10_RASTERIZER_DESC RasterDesc;
		memset(&RasterDesc, 0, sizeof( D3D10_RASTERIZER_DESC ) );
		RasterDesc.FillMode = D3D10_FILL_SOLID;
		RasterDesc.CullMode = D3D10_CULL_NONE;
		oV( _pDevice->CreateRasterizerState(&RasterDesc, &RasterState ) );
	}
	{ // Create our one and only depth state
		D3D10_DEPTH_STENCIL_DESC DepthDesc;
		memset( &DepthDesc, 0, sizeof( D3D10_DEPTH_STENCIL_DESC ) );
		oV( _pDevice->CreateDepthStencilState(&DepthDesc, &DepthState ) );
	}
	{ // Create our one and only blend state
		D3D10_BLEND_DESC BlendDesc;
		memset( &BlendDesc, 0, sizeof( D3D10_BLEND_DESC ) );
		BlendDesc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
		oV( _pDevice->CreateBlendState(&BlendDesc, &BlendState) );
	}
}

void oD3D10FullscreenQuad::SetStates(ID3D10Device* _pDevice)
{
	ShaderState.SetState(_pDevice);
	_pDevice->RSSetState(RasterState);
	_pDevice->OMSetDepthStencilState(DepthState, 0 );
	static const FLOAT sBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_pDevice->OMSetBlendState(BlendState, sBlendFactor, 0xffffffff);
	_pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void oD3D10FullscreenQuad::Draw( ID3D10Device* _pDevice )
{
	SetStates(_pDevice);

	ID3D10Buffer *pNullBuffer[] = { nullptr };
	unsigned pStrides[] = { 0 };
	unsigned pOffsets[] = { 0 };
	_pDevice->IASetVertexBuffers(0, 1, pNullBuffer, pStrides, pOffsets);

	_pDevice->Draw( 4, 0 );
}

void oD3D10ScreenQuad::Create(ID3D10Device* _pDevice, const BYTE* _pPixelShaderByteCode, size_t _szBiteCode)
{
	CreateStates(_pDevice);

	{ // Vertex Buffer
		D3D10_BUFFER_DESC BufferDesc;
		memset( &BufferDesc, 0, sizeof( D3D10_BUFFER_DESC ) );
		BufferDesc.ByteWidth = sizeof(VSIN) * 4;
		BufferDesc.Usage = D3D10_USAGE_DYNAMIC;
		BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		BufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		oV( _pDevice->CreateBuffer(&BufferDesc, nullptr, &VertexBuffer) );
	}

	{
		D3D10_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, 
			D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, 
			D3D10_INPUT_PER_VERTEX_DATA, 0 },
		};
		oV(_pDevice->CreateInputLayout(layout, 2, oHLSLPassthroughByteCode, oCOUNTOF( oHLSLPassthroughByteCode), &VertexLayout));
	}

	oD3D10ShaderState::CreateShaders( _pDevice, &ShaderState, oHLSLPassthroughByteCode, oCOUNTOF( oHLSLPassthroughByteCode), nullptr, 0, _pPixelShaderByteCode, _szBiteCode );
}

void oD3D10ScreenQuad::Draw( ID3D10Device* _pDevice , oRECTF _Destination, oRECTF _Source)
{
	SetStates(_pDevice);

	VSIN *buffer;
	VertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&buffer);
	buffer[0].Position = float2(_Destination.GetMin().x, _Destination.GetMax().y);
	buffer[0].Texcoord = _Source.GetMin();
	buffer[1].Position = _Destination.GetMax();
	buffer[1].Texcoord = float2(_Source.GetMax().x, _Source.GetMin().y);
	buffer[2].Position = _Destination.GetMin();
	buffer[2].Texcoord = float2(_Source.GetMin().x, _Source.GetMax().y);
	buffer[3].Position = float2(_Destination.GetMax().x, _Destination.GetMin().y);
	buffer[3].Texcoord = _Source.GetMax();
	VertexBuffer->Unmap();

	ID3D10Buffer *pNullBuffer[] = { VertexBuffer.c_ptr() };
	unsigned pStrides[] = { sizeof(VSIN) };
	unsigned pOffsets[] = { 0 };
	_pDevice->IASetInputLayout( VertexLayout );
	_pDevice->IASetVertexBuffers(0, 1, pNullBuffer, pStrides, pOffsets);
	
	_pDevice->Draw( 4, 0 );
}

