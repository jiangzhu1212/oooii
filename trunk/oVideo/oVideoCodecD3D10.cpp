// $(header)
#include "oVideoCodecD3D10.h"
#include <oooii/oMemory.h>
#include <oooii/oThreading.h>
#include <oooii/oStdio.h>
#include <oooii/oMathInternalHLSL.h>
#include <oooii/oMath.h>

#include <oHLSLYUVTORGBByteCode.h>

//Arbitrary, but if less than 3ms remaining, don't try and catch up any behind decoders.
//	If there aren't many splits then we will probably go over.
const static double MINIMUM_TIME_REMAINING_TO_TRY_CATCHUP = 0.003; 

void oVideoDecodeD3D10SimpleYUV::DecodePartitionVerticalStitch(size_t _index, std::vector<D3D10_MAPPED_TEXTURE2D> &_MappedTextures)
{
	oSurface::YUV420 YUVTexturePartition = YUVTexture;

	unsigned int startRow = static_cast<unsigned int>(ConDescs[0].Dimensions.y * _index);
	// @oooii-eric: TODO: This code is a waste for VP8, get rid of it when possible. VP8 doesn't use the supplied buffer, but some of our other codecs do. see comment in oVideoCodec.h
	YUVTexturePartition.pY += startRow*YUVTexture.YPitch;
	YUVTexturePartition.pU += (startRow/2)*YUVTexture.UVPitch;
	YUVTexturePartition.pV += (startRow/2)*YUVTexture.UVPitch;

	size_t decodedFrameNumber;
	if( !CPUDecoders[_index]->Decode(&YUVTexturePartition, &decodedFrameNumber) )
	{
		LastFrameInvalid = true;
		return;
	}

	unsigned int MappedYOffset = startRow*_MappedTextures[0].RowPitch;
	unsigned int MappedUVOffset = (startRow/2)*_MappedTextures[1].RowPitch;

	oMemcpy2d( oByteAdd(_MappedTextures[0].pData,MappedYOffset), _MappedTextures[0].RowPitch, YUVTexturePartition.pY, YUVTexturePartition.YPitch, ConDescs[_index].Dimensions.x, ConDescs[_index].Dimensions.y );
	oMemcpy2d( oByteAdd(_MappedTextures[1].pData,MappedUVOffset), _MappedTextures[1].RowPitch, YUVTexturePartition.pU, YUVTexturePartition.UVPitch, ConDescs[_index].Dimensions.x/2, ConDescs[_index].Dimensions.y / 2 );
	oMemcpy2d( oByteAdd( _MappedTextures[2].pData,MappedUVOffset), _MappedTextures[2].RowPitch, YUVTexturePartition.pV, YUVTexturePartition.UVPitch, ConDescs[_index].Dimensions.x/2, ConDescs[_index].Dimensions.y / 2 );

	DecodedFrameNumbers[_index] = decodedFrameNumber; //definitely some false sharing here, so do last which should keep it from costing any real performance.
}

void oVideoDecodeD3D10SimpleYUV::DecodePartitionHorizontalStitch(size_t _index, std::vector<D3D10_MAPPED_TEXTURE2D> &_MappedTextures)
{
	oSurface::YUV420 YUVTexturePartition = YUVTexture;

	unsigned int startColumn = static_cast<unsigned int>(ConDescs[0].Dimensions.x * _index);
	YUVTexturePartition.pY += startColumn;
	YUVTexturePartition.pU += (startColumn/2);
	YUVTexturePartition.pV += (startColumn/2);

	size_t decodedFrameNumber;
	if( !CPUDecoders[_index]->Decode(&YUVTexturePartition, &decodedFrameNumber) )
	{
		LastFrameInvalid = true;
		return;
	}

	unsigned int MappedYOffset = startColumn;
	unsigned int MappedUVOffset = startColumn/2;

	oMemcpy2d( oByteAdd(_MappedTextures[0].pData,MappedYOffset), _MappedTextures[0].RowPitch, YUVTexturePartition.pY, YUVTexturePartition.YPitch, ConDescs[_index].Dimensions.x, ConDescs[_index].Dimensions.y );
	oMemcpy2d( oByteAdd(_MappedTextures[1].pData,MappedUVOffset), _MappedTextures[1].RowPitch, YUVTexturePartition.pU, YUVTexturePartition.UVPitch, ConDescs[_index].Dimensions.x/2, ConDescs[_index].Dimensions.y / 2 );
	oMemcpy2d( oByteAdd( _MappedTextures[2].pData,MappedUVOffset), _MappedTextures[2].RowPitch, YUVTexturePartition.pV, YUVTexturePartition.UVPitch, ConDescs[_index].Dimensions.x/2, ConDescs[_index].Dimensions.y / 2 );

	DecodedFrameNumbers[_index] = decodedFrameNumber; //definitely some false sharing here, so do last which should keep it from costing any real performance.
}

bool oVideoDecodeD3D10SimpleYUV::Decode(size_t *_decodedFrameNumber)
{
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
	RTV->GetDevice(&Device);

	if(decodeFrame)
	{
		DecodeEvent.Wait();
		if(!LastFrameInvalid)
			oSWAP(&CurrentDisplayFrame,(CurrentDisplayFrame+1)&1);

		DecodeEvent.Reset();
		oIssueAsyncTask(oBIND(&oVideoDecodeD3D10SimpleYUV::DecodeFrameTask, this));
	}

	if(_decodedFrameNumber)
		*_decodedFrameNumber = LastDecodedFrame;

	Device->PSSetSamplers(0, 1, BilinearSampler.address() );
	Device->PSSetShaderResources(0, 3, TextureList[CurrentDisplayFrame].SRVs[0].address() );

	Device->RSSetViewports(1, &VP);
	
	static int frame = 0;
	++frame;
	float Clear[4] = { frame % 2 ? 1.0f : 0.0f,  frame % 2 ? 0.0f : 1.0f, 0.0f, 1.0f };
	Device->ClearRenderTargetView(RTV, Clear );
	Device->OMSetRenderTargets(1, RTV.address(), NULL );

	if(Desc.SourceRects[0].IsEmpty() || Desc.DestRects[0].IsEmpty())
	{
		Quad.Draw(Device, oRECTF(float2(-1,-1), float2(1,1)), oRECTF(float2(0,0), float2(1,1)));
	}
	else
	{
		for (int i = 0; i < oCOUNTOF(Desc.SourceRects); ++i)
		{
			if(Desc.SourceRects[i].IsEmpty() || Desc.DestRects[i].IsEmpty())
				break;

			oRECTF source;
			float x1 = Desc.SourceRects[i].GetMin().x / (float)StitchedWidth;
			float y1 = Desc.SourceRects[i].GetMin().y / (float)StitchedHeight;
			float x2 = Desc.SourceRects[i].GetMax().x / (float)StitchedWidth;
			float y2 = Desc.SourceRects[i].GetMax().y / (float)StitchedHeight;
			source.SetMin(float2(x1, y1));
			source.SetMax(float2(x2, y2));

			oRECTF dest;
			x1 = (Desc.DestRects[i].GetMin().x / (float)VP.Width)*2 - 1;
			y1 = (Desc.DestRects[i].GetMin().y / (float)VP.Height)*-2 + 1;
			x2 = (Desc.DestRects[i].GetMax().x / (float)VP.Width)*2 - 1;
			y2 = (Desc.DestRects[i].GetMax().y / (float)VP.Height)*-2 + 1;
			dest.ExtendBy(float2(x1, y1));
			dest.ExtendBy(float2(x2, y2));

			Quad.Draw(Device, dest, source);
		}
	}

	return true;
}

oVideoDecodeD3D10SimpleYUV::oVideoDecodeD3D10SimpleYUV(DESC _desc, std::vector<oRef<oVideoDecodeCPU>> &_CPUDecoders, bool* _pSuccess )
	: CPUDecoders(_CPUDecoders), Desc(_desc), StitchedWidth(0), StitchedHeight(0), LastDecodedFrame(0), CurrentDisplayFrame(0)
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
			if(ConDescs[i].Dimensions.x != ConDescs[0].Dimensions.x)
			{
				oSetLastError(EINVAL, "If stitching vertically all videos must have the same width");
				return;
			}
			StitchedHeight += ConDescs[i].Dimensions.y;
		}
		else
		{
			if(ConDescs[i].Dimensions.y != ConDescs[0].Dimensions.y)
			{
				oSetLastError(EINVAL, "If stitching vertically all videos must have the same width");
				return;
			}
			StitchedWidth += ConDescs[i].Dimensions.x;
		}
	}

	if(Desc.StitchVertically)
		StitchedWidth = ConDescs[0].Dimensions.x;
	else
		StitchedHeight = ConDescs[0].Dimensions.y;
	
	//Note that for some codecs like VP8 this buffer will not get used. This space is just in case a decoder doesn't provide its own space.
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

	if(Desc.StitchVertically)
		DecodePartition = oBIND(&oVideoDecodeD3D10SimpleYUV::DecodePartitionVerticalStitch, this, oBIND1, oBIND2);
	if(!Desc.StitchVertically)
		DecodePartition = oBIND(&oVideoDecodeD3D10SimpleYUV::DecodePartitionHorizontalStitch, this, oBIND1, oBIND2);

	*_pSuccess = true;
}

oVideoDecodeD3D10SimpleYUV::~oVideoDecodeD3D10SimpleYUV()
{
	DecodeEvent.Wait();
}

bool oVideoDecodeD3D10SimpleYUV::Register(interface ID3D10Texture2D* _pDestinationTexture)
{
	oRef<ID3D10Device> Device;
	_pDestinationTexture->GetDevice(&Device);

	D3D10_RENDER_TARGET_VIEW_DESC renderDesc;
	renderDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	renderDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	renderDesc.Texture2D.MipSlice = 0;

	if( S_OK != Device->CreateRenderTargetView( _pDestinationTexture, &renderDesc, &RTV ) )
	{
		oSetLastError(EINVAL, "Failed to create render target view");
		return false;
	}

	D3D10_SAMPLER_DESC SamplerDesc;
	SamplerDesc.AddressU = D3D10_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D10_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D10_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.Filter = D3D10_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.ComparisonFunc = D3D10_COMPARISON_ALWAYS;
	SamplerDesc.MaxAnisotropy = 0;
	SamplerDesc.MipLODBias = 0;

	oV( Device->CreateSamplerState( &SamplerDesc, &BilinearSampler ) );

	Quad.Create( Device, oHLSLYUVTORGBByteCode, oCOUNTOF(oHLSLYUVTORGBByteCode) );
	
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

	if(!CreateTextureSet(0, Device))
		return false;
	if(!CreateTextureSet(1, Device))
		return false;
	
	DecodeEvent.Reset();
	oIssueAsyncTask(oBIND(&oVideoDecodeD3D10SimpleYUV::DecodeFrameTask, this));

	return true;
}

void oVideoDecodeD3D10SimpleYUV::Unregister()
{
	RTV = nullptr;
}

bool oVideoDecodeD3D10SimpleYUV::CreateTextureSet(unsigned int _index, ID3D10Device *_device)
{
	oD3D10TextureList<3>& YUVTextures = TextureList[_index];
	
	// Create the luminance texture
	if( !YUVTextures.Create(0, _device, StitchedWidth, StitchedHeight, DXGI_FORMAT_R8_UNORM ) )
	{
		oSetLastError(EINVAL, "Failed to create D3D10 texture!");
		return false;
	}

	// Create the two chrominance textures
	if( !YUVTextures.Create(1, _device, StitchedWidth / 2, StitchedHeight / 2, DXGI_FORMAT_R8_UNORM ) )
	{
		oSetLastError(EINVAL, "Failed to create D3D10 texture!");
		return false;
	}

	if( !YUVTextures.Create(2, _device, StitchedWidth / 2, StitchedHeight / 2, DXGI_FORMAT_R8_UNORM ) )
	{
		oSetLastError(EINVAL, "Failed to create D3D10 texture!");
		return false;
	}

	return true;
};

void oVideoDecodeD3D10SimpleYUV::DecodeFrameTask()
{
	double startTaskTime = oTimer();

	unsigned int NextDisplayFrame = (CurrentDisplayFrame+1)&1;
	ID3D10Texture2D* pYTexture = TextureList[NextDisplayFrame].Textures[0];
	ID3D10Texture2D* pUTexture = TextureList[NextDisplayFrame].Textures[1];
	ID3D10Texture2D* pVTexture = TextureList[NextDisplayFrame].Textures[2];
	std::vector<D3D10_MAPPED_TEXTURE2D> MappedTextures;
	MappedTextures.resize(3);
	oV( pYTexture->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &(MappedTextures[0]) ) );
	oV( pUTexture->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &(MappedTextures[1]) ) );
	oV( pVTexture->Map(0, D3D10_MAP_WRITE_DISCARD, 0, &(MappedTextures[2]) ) );

	size_t numDecoders = CPUDecoders.size();
	DecodedFrameNumbers.resize(numDecoders);
	
	LastFrameInvalid = false;
	oParallelFor(oBIND(DecodePartition, oBIND1, MappedTextures), 0, numDecoders);

	LastDecodedFrame = DecodedFrameNumbers[0];
	for(unsigned int i = 1; i < numDecoders; ++i)
	{
		LastDecodedFrame = __max(LastDecodedFrame, DecodedFrameNumbers[i]);
	}

	double catchUpTimeLeft = FrameTime - (oTimer() - startTaskTime);
	bool CatchUpNeeded = true;
	unsigned int catchUpIndex = 0;
	while(CatchUpNeeded && catchUpTimeLeft > MINIMUM_TIME_REMAINING_TO_TRY_CATCHUP)
	{	
		CatchUpNeeded = false;
		for(unsigned int i = 0; i < numDecoders; ++i)
		{
			catchUpIndex = (catchUpIndex+1)%numDecoders; // to avoid preferential catch up treatment to early decoders.
			if(DecodedFrameNumbers[catchUpIndex] < LastDecodedFrame)
			{
				CatchUpNeeded = true;
				DecodePartition(catchUpIndex, MappedTextures);
				break;
			}
		}
		catchUpTimeLeft = FrameTime - (oTimer() - startTaskTime);
	}

	pYTexture->Unmap(0);
	pUTexture->Unmap(0);
	pVTexture->Unmap(0);

	if(!Desc.AllowCatchUp)
	{
		for(unsigned int i = 1; i < numDecoders; ++i)
		{
			oASSERT(DecodedFrameNumbers[i] == DecodedFrameNumbers[0], "A partition in split vp8 decoding mode, decoded a diffrent frame number than other partitions.");
		}
	}
	
	DecodeEvent.Set();
}
