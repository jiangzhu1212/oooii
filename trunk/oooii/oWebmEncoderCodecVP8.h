// $(header)
#pragma once
#ifndef oWebmEncoderCodecVP8_h
#define oWebmEncoderCodecVP8_h

#include "oWebmEncoderCodec.h"
#include <oooii/oRefCount.h>
#pragma warning(disable:4505)
#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"

class oWebmEncoderCodecVP8 : public oWebmEncoderCodec
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oWebmEncoderCodecVP8>());
	oWebmEncoderCodecVP8(const oVideoEncodeCPU::DESC &_Desc, bool* _pSuccess);
	~oWebmEncoderCodecVP8();
	
 	const std::vector<unsigned char>& GetWebmCodecEbml() const override { return CodecEbml; }
	const std::vector<unsigned char>& GetWebmCodecNameEbml() const override { return CodecNameEbml; }

	void Encode( const oSurface::YUV420& _Frame, void* _pDataStream, size_t _StreamSize, size_t* _pDataWrittenToStream, 
		unsigned int _frameStamp, unsigned int _frameDuration, bool _forceIFrame, FrameDesc &_desc ) override;
	void EncodeFirstPass(const oSurface::YUV420& _Frame, unsigned int _frameStamp, unsigned int _frameDuration, bool _forceIFrame /*= false*/) override;
	void StartSecondPass(unsigned int _frameStamp, unsigned int _frameDuration) override;
private:
	oVideoEncodeCPU::DESC Desc;

	unsigned int GetQuality( oVideoEncodeCPU::QUALITY quality );

	oRefCount RefCount;
	std::vector<unsigned char> CodecEbml;
	std::vector<unsigned char> CodecNameEbml;

	vpx_codec_ctx_t Context;
	vpx_image_t* pImage;
	vpx_codec_enc_cfg VP8Cfg;
	vpx_fixed_buf_t FirstPassStats;   
};

#endif