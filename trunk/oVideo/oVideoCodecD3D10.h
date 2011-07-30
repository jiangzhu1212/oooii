// $(header)
#ifndef oVideoCodecD3D10_h
#define oVideoCodecD3D10_h
#include <oooii/oD3D10.h>
#include <oVideo/oVideoCodec.h>
#include <oooii/oMutex.h>
#include <map>
#include <oooii/oEvent.h>

template<size_t _Size>
struct oD3D10TextureList
{
	oRef<ID3D10Texture2D> Textures[_Size];
	oRef<ID3D10ShaderResourceView> SRVs[_Size];

	bool Create(size_t index, ID3D10Device* pDevice, unsigned int _Width, unsigned int _Height, DXGI_FORMAT _Format)
	{
		D3D10_TEXTURE2D_DESC TextureDesc;
		TextureDesc.Width = _Width;
		TextureDesc.Height = _Height;
		TextureDesc.Format = _Format;
		TextureDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		TextureDesc.ArraySize = 1;
		TextureDesc.MipLevels = 1;
		TextureDesc.MiscFlags = 0;
		TextureDesc.SampleDesc.Count = 1;
		TextureDesc.SampleDesc.Quality = 0;
		TextureDesc.Usage = D3D10_USAGE_DYNAMIC;
		TextureDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

		if( S_OK != pDevice->CreateTexture2D( &TextureDesc, NULL, &Textures[index] ) )
			return false;

		if( S_OK != pDevice->CreateShaderResourceView( Textures[index], NULL, &SRVs[index] ) )
			return false;

		return true;
	}
};

class oVideoDecodeD3D10SimpleYUV : public oVideoDecodeD3D10
{
	// This is is a very simple D3D10 video decoder that only
	// accelerates the YUV portion of the decode
public:
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_NOOP_QUERYINTERFACE();
	oVideoDecodeD3D10SimpleYUV(DESC _desc, std::vector<oRef<oVideoDecodeCPU>> &_CPUDecoders, bool* _pSuccess);
	~oVideoDecodeD3D10SimpleYUV();

	bool Register(interface ID3D10Texture2D* _pDestinationTexture ) override;
	void Unregister() override;
	bool Decode(size_t *_decodedFrameNumber) override;
	
private:
	void DecodePartitionVerticalStitch(size_t _index,std::vector<D3D10_MAPPED_TEXTURE2D> &_MappedTextures);
	void DecodePartitionHorizontalStitch(size_t _index,std::vector<D3D10_MAPPED_TEXTURE2D> &_MappedTextures);
	oFUNCTION<void (size_t, std::vector<D3D10_MAPPED_TEXTURE2D>&)> DecodePartition;
	
	bool CreateTextureSet(unsigned int _index, ID3D10Device *_device);
	void DecodeFrameTask();

	oRef<ID3D10RenderTargetView> RTV;
	D3D10_VIEWPORT VP;
	oRef<ID3D10SamplerState> BilinearSampler;
	oD3D10TextureList<3> TextureList[2]; // double buffered
	oD3D10ScreenQuad Quad;

	std::vector<size_t> DecodedFrameNumbers;

	std::vector<oVideoContainer::DESC> ConDescs;
	oRefCount Refcount;
	oSurface::YUV420 YUVTexture;
	std::vector<oRef<oVideoDecodeCPU>> CPUDecoders;

	std::vector<unsigned char> LuminancePlane;
	std::vector<unsigned char> UChromQuarterPlane;
	std::vector<unsigned char> VChromQuarterPlane;
	
	DESC Desc;
	double FrameTime;
	double LastFrameTime;

	unsigned int StitchedWidth;
	unsigned int StitchedHeight;

	oEvent DecodeEvent;
	size_t LastDecodedFrame;
	volatile unsigned int CurrentDisplayFrame;
	bool LastFrameInvalid;
};


#endif //oVideoCodecD3D10_h
