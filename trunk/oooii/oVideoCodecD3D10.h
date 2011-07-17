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
#ifndef oVideoCodecD3D10_h
#define oVideoCodecD3D10_h
#include <oooii/oD3D10.h>
#include <oooii/oVideoCodec.h>
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
	oVideoDecodeD3D10SimpleYUV(DESC _desc, std::vector<oRef<threadsafe oVideoDecodeCPU>> &_CPUDecoders, bool* _pSuccess);
	~oVideoDecodeD3D10SimpleYUV();

	virtual HDECODE_CONTEXT Register(interface ID3D10Texture2D* _pDestinationTexture ) threadsafe;
	virtual void Unregister(HDECODE_CONTEXT) threadsafe;
	virtual bool Decode(HDECODE_CONTEXT, size_t *_decodedFrameNumber) threadsafe;

private:
	void DecodePartitionVerticalStitch(size_t _index,std::vector<D3D10_MAPPED_TEXTURE2D> &_MappedTextures);
	void DecodePartitionHorizontalStitch(size_t _index,std::vector<D3D10_MAPPED_TEXTURE2D> &_MappedTextures);
	void DecodePartitionVerticalStitchSurround(size_t _index,std::vector<D3D10_MAPPED_TEXTURE2D> &_MappedTextures);
	void DecodePartitionHorizontalStitchSurround(size_t _index,std::vector<D3D10_MAPPED_TEXTURE2D> &_MappedTextures);
	oFUNCTION<void (size_t, std::vector<D3D10_MAPPED_TEXTURE2D>&)> DecodePartition;
	
	struct GPU_CONTEXT
	{
		oRef<ID3D10RenderTargetView> RTV;
		D3D10_VIEWPORT VP;
		oRef<ID3D10SamplerState> BilinearSampler;
		oD3D10TextureList<3> TextureList[2]; // double buffered
		oD3D10FullscreenQuad Quad;
	};

	bool CreateTextureSet(unsigned int _index, GPU_CONTEXT &_context, ID3D10Device *_device) threadsafe;
	void DecodeFrameTask(GPU_CONTEXT* _pGPUContext);

	std::vector<size_t> DecodedFrameNumbers;

	std::vector<oVideoContainer::DESC> ConDescs;
	oRefCount Refcount;
	oSurface::YUV420 YUVTexture;
	std::vector<oRef<threadsafe oVideoDecodeCPU>> CPUDecoders;

	std::vector<unsigned char> LuminancePlane;
	std::vector<unsigned char> UChromQuarterPlane;
	std::vector<unsigned char> VChromQuarterPlane;
	
	oRWMutex YUVMapMutex;
	typedef std::map<void*, GPU_CONTEXT> yuv_map_t;
	yuv_map_t YUVMap;
	DESC Desc;
	double FrameTime;
	double LastFrameTime;

	unsigned int StitchedWidth;
	unsigned int StitchedHeight;

	oEvent DecodeEvent;
	size_t LastDecodedFrame;
	volatile unsigned int CurrentDisplayFrame;
};


#endif //oVideoCodecD3D10_h
