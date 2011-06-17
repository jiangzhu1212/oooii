// $(header)
#include "oVideoCodecD3D10.h"
#include <oooii/oMemory.h>
#include <oooii/oThreading.h>
#include <oooii/oStdio.h>

#include <oHLSLYUVTORGBByteCode.h>

void oVideoDecodeD3D10SimpleYUV::DecodePartition(size_t _index,std::vector<D3D10_MAPPED_TEXTURE2D> &_MappedTextures)
{
	oSurface::YUV420 YUVTexturePartition = YUVTexture;
	unsigned int MappedYOffset = 0;
	unsigned int MappedUVOffset = 0;

	if(Desc.StitchVertically)
	{
		unsigned int startRow = 0;
		for (unsigned int i = 0;i < _index;++i)
		{
			startRow += ConDescs[i].Height;
		}
		YUVTexturePartition.pY += startRow*YUVTexture.YPitch;
		YUVTexturePartition.pU += (startRow/2)*YUVTexture.UVPitch;
		YUVTexturePartition.pV += (startRow/2)*YUVTexture.UVPitch;
		MappedYOffset = startRow*_MappedTextures[0].RowPitch;
		MappedUVOffset = (startRow/2)*_MappedTextures[1].RowPitch;
	}
	else
	{
		unsigned int startColumn = 0;
		for (unsigned int i = 0;i < _index;++i)
		{
			startColumn += ConDescs[i].Width;
		}
		YUVTexturePartition.pY += startColumn;
		YUVTexturePartition.pU += (startColumn/2);
		YUVTexturePartition.pV += (startColumn/2);
		MappedYOffset = startColumn;
		MappedUVOffset = (startColumn/2);
	}

	size_t decodedFrameNumber;
	if( !CPUDecoders[_index]->Decode(thread_cast<oSurface::YUV420*>( &YUVTexturePartition ), &decodedFrameNumber) )
		return;
	
	oMemcpy2d( oByteAdd(_MappedTextures[0].pData,MappedYOffset), _MappedTextures[0].RowPitch, YUVTexturePartition.pY, YUVTexturePartition.YPitch, ConDescs[_index].Width, ConDescs[_index].Height );
	oMemcpy2d( oByteAdd(_MappedTextures[1].pData,MappedUVOffset), _MappedTextures[1].RowPitch, YUVTexturePartition.pU, YUVTexturePartition.UVPitch, ConDescs[_index].Width/2, ConDescs[_index].Height / 2 );
	oMemcpy2d(oByteAdd( _MappedTextures[2].pData,MappedUVOffset), _MappedTextures[2].RowPitch, YUVTexturePartition.pV, YUVTexturePartition.UVPitch, ConDescs[_index].Width/2, ConDescs[_index].Height / 2 );
	
	DecodedFrameNumbers[_index] = decodedFrameNumber; //definitely some false sharing here, so do last which should keep it from costing any real performance.
}

bool oVideoDecodeD3D10SimpleYUV::Decode(HDECODE_CONTEXT _ctx, size_t *_decodedFrameNumber) threadsafe
{
	GPU_CONTEXT* pGPUContext = reinterpret_cast<GPU_CONTEXT*>(_ctx);

	bool decodeFrame = false;
	if(!Desc.UseFrameTime)
		decodeFrame = true;
	else
	{
		double currentTime = oTimer();
		if((currentTime - LastFrameTime) >= FrameTime)
		{
			decodeFrame = true;
			LastFrameTime += FrameTime;
		}
	}

	oRef<ID3D10Device> Device;
	pGPUContext->RTV->GetDevice(&Device);

	if(decodeFrame)
	{
		ID3D10Texture2D* pYTexture = pGPUContext->TextureList.Textures[0];
		ID3D10Texture2D* pUTexture = pGPUContext->TextureList.Textures[1];
		ID3D10Texture2D* pVTexture = pGPUContext->TextureList.Textures[2];
		std::vector<D3D10_MAPPED_TEXTURE2D> MappedTextures;
		MappedTextures.resize(3);
		oV( pYTexture->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &(MappedTextures[0]) ) );
		oV( pUTexture->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &(MappedTextures[1]) ) );
		oV( pVTexture->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &(MappedTextures[2]) ) );

		size_t numDecoders = thread_cast<oVideoDecodeD3D10SimpleYUV*>(this)->CPUDecoders.size();
		thread_cast<oVideoDecodeD3D10SimpleYUV*>(this)->DecodedFrameNumbers.resize(numDecoders);
		oParallelFor(oBIND(&oVideoDecodeD3D10SimpleYUV::DecodePartition, thread_cast<oVideoDecodeD3D10SimpleYUV*>(this), oBIND1, MappedTextures), 0, numDecoders);

		pYTexture->Unmap(0);
		pUTexture->Unmap(0);
		pVTexture->Unmap(0);

		if(_decodedFrameNumber)
			*_decodedFrameNumber = thread_cast<oVideoDecodeD3D10SimpleYUV*>(this)->DecodedFrameNumbers[0];
		for(unsigned int i = 1; i < numDecoders; ++i)
		{
			oASSERT(thread_cast<oVideoDecodeD3D10SimpleYUV*>(this)->DecodedFrameNumbers[i] == thread_cast<oVideoDecodeD3D10SimpleYUV*>(this)->DecodedFrameNumbers[0], "A partition in split vp8 decoding mode, decoded a diffrent frame number than other partitions.");
		}
	}

	Device->PSSetSamplers(0, 1, pGPUContext->BilinearSampler.address() );
	Device->PSSetShaderResources(0, 3, pGPUContext->TextureList.SRVs[0].address() );

	Device->RSSetViewports(1, &pGPUContext->VP);
	
	static int frame = 0;
	++frame;
	float Clear[4] = { frame % 2 ? 1.0f : 0.0f,  frame % 2 ? 0.0f : 1.0f, 0.0f, 1.0f };
	Device->ClearRenderTargetView(pGPUContext->RTV, Clear );
	Device->OMSetRenderTargets(1, pGPUContext->RTV.address(), NULL );

	pGPUContext->Quad.Draw(Device);
	
	return true;
}

oVideoDecodeD3D10SimpleYUV::oVideoDecodeD3D10SimpleYUV(DESC _desc, std::vector<oRef<threadsafe oVideoDecodeCPU>> &_CPUDecoders, bool* _pSuccess )
	: CPUDecoders(_CPUDecoders), Desc(_desc), StitchedWidth(0), StitchedHeight(0)
{
	*_pSuccess = false;

	if(CPUDecoders.empty())
	{
		oSetLastError(EINVAL, "No containers were given to the decoder. Must be at least 1");
		return;
	}

	oRef<oVideoContainer> Container;
	ConDescs.resize(CPUDecoders.size());
	for (unsigned int i = 0;i < CPUDecoders.size();++i)
	{
		if( !CPUDecoders[i]->QueryInterface(&Container) )
		{
			oSetLastError(E_NOINTERFACE, "Not a oVideoContainer");
			return;
		}

		Container->GetDesc(&(ConDescs[i]));
		//allowing dropped frames can cause partitions to get out of sync.
		// @oooii-eric: TODO: If we want to support dropped frames with split vp8, need some way to make sure all partitions drop equally.
		oASSERT(CPUDecoders.size() == 1 || ConDescs[i].AllowDroppedFrames == false, "Because of split vp8 decoding, oVideoDecodeD3D10SimpleYUV shouldn't allow dropped frames");

		if(Desc.StitchVertically)
		{
			if(ConDescs[i].Width != ConDescs[0].Width)
			{
				oSetLastError(EINVAL, "If stitching vertically all videos must have the same width");
				return;
			}
			StitchedHeight += ConDescs[i].Height;
		}
		else
		{
			if(ConDescs[i].Height != ConDescs[0].Height)
			{
				oSetLastError(EINVAL, "If stitching vertically all videos must have the same width");
				return;
			}
			StitchedWidth += ConDescs[i].Width;
		}
	}

	if(Desc.StitchVertically)
		StitchedWidth = ConDescs[0].Width;
	else
		StitchedHeight = ConDescs[0].Height;

	unsigned int FullPixels = StitchedWidth * StitchedHeight;
	LuminancePlane.resize( FullPixels );
	UChromQuarterPlane.resize( FullPixels / 4 );
	VChromQuarterPlane.resize( FullPixels / 4 );

	YUVTexture.pY = &LuminancePlane[0];
	YUVTexture.YPitch = StitchedWidth;

	YUVTexture.pU = &UChromQuarterPlane[0];
	YUVTexture.pV = &VChromQuarterPlane[0];
	YUVTexture.UVPitch = StitchedWidth / 2;

	FrameTime = ConDescs[0].FrameTimeNumerator/static_cast<double>(ConDescs[0].FrameTimeDenominator);
	LastFrameTime = oTimer() - FrameTime;

	*_pSuccess = true;
}

oVideoDecodeD3D10::HDECODE_CONTEXT oVideoDecodeD3D10SimpleYUV::Register(interface ID3D10Texture2D* _pDestinationTexture) threadsafe
{
	oRWMutex::ScopedLock Lock( YUVMapMutex );
	yuv_map_t* pRawMap = thread_cast<yuv_map_t*>( &YUVMap );

	yuv_map_t::iterator iter = pRawMap->find(_pDestinationTexture);
	if( iter != pRawMap->end() )
	{
		return reinterpret_cast<oVideoDecodeD3D10::HDECODE_CONTEXT>(&iter->second); // Already registered
	}

	GPU_CONTEXT GPU_Context;

	oRef<ID3D10Device> Device;
	_pDestinationTexture->GetDevice(&Device);

	D3D10_RENDER_TARGET_VIEW_DESC renderDesc;
	renderDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	renderDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	renderDesc.Texture2D.MipSlice = 0;

	if( S_OK != Device->CreateRenderTargetView( _pDestinationTexture, &renderDesc, &GPU_Context.RTV ) )
	{
		oSetLastError(EINVAL, "Failed to create render target view");
		return NULL;
	}

	D3D10_SAMPLER_DESC SamplerDesc;
	SamplerDesc.AddressU = D3D10_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D10_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D10_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.Filter = D3D10_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.ComparisonFunc = D3D10_COMPARISON_ALWAYS;
	SamplerDesc.MaxAnisotropy = 0;
	SamplerDesc.MipLODBias = 0;

	oV( Device->CreateSamplerState( &SamplerDesc, &GPU_Context.BilinearSampler ) );

	GPU_Context.Quad.Create( Device, oHLSLYUVTORGBByteCode, oCOUNTOF(oHLSLYUVTORGBByteCode) );


	D3D10_VIEWPORT& VP = GPU_Context.VP;
	{
		D3D10_TEXTURE2D_DESC TexDesc;
		_pDestinationTexture->GetDesc(&TexDesc);

		VP.TopLeftX = 0;
		VP.TopLeftY = 0;
		VP.MinDepth = 0.0f;
		VP.MaxDepth = 1.0f;
		VP.Height = TexDesc.Height;
		VP.Width = TexDesc.Width;
	}

	oD3D10TextureList<3>& YUVTextures = GPU_Context.TextureList;

	// Create the luminance texture
	if( !YUVTextures.Create(0, Device, StitchedWidth, StitchedHeight, DXGI_FORMAT_R8_UNORM ) )
	{
		oSetLastError(EINVAL, "Failed to create D3D10 texture!");
		return NULL;
	}

	// Create the two chrominance textures
	if( !YUVTextures.Create(1, Device, StitchedWidth / 2, StitchedHeight / 2, DXGI_FORMAT_R8_UNORM ) )
	{
		oSetLastError(EINVAL, "Failed to create D3D10 texture!");
		return NULL;
	}

	if( !YUVTextures.Create(2, Device, StitchedWidth / 2, StitchedHeight / 2, DXGI_FORMAT_R8_UNORM ) )
	{
		oSetLastError(EINVAL, "Failed to create D3D10 texture!");
		return NULL;
	}
	(*pRawMap)[_pDestinationTexture] = GPU_Context;

	return reinterpret_cast<oVideoDecodeD3D10::HDECODE_CONTEXT>(&(*pRawMap)[_pDestinationTexture]);
}

void oVideoDecodeD3D10SimpleYUV::Unregister(HDECODE_CONTEXT _Context) threadsafe
{
	oRWMutex::ScopedLock Lock( YUVMapMutex );
	yuv_map_t* pRawMap = thread_cast<yuv_map_t*>( &YUVMap );
	GPU_CONTEXT* pGPUContext = reinterpret_cast<GPU_CONTEXT*>(_Context);

	for( yuv_map_t::iterator iter = pRawMap->begin(); iter != pRawMap->end(); ++iter )
	{
		if( iter->second.RTV == pGPUContext->RTV )
		{
			pRawMap->erase(iter);
			return;
		}
	}

}
