// $(header)
#pragma once
#ifndef oWebmEncoderCodecRaw420_h
#define oWebmEncoderCodecRaw420_h

#include "oWebmEncoderCodec.h"
#include <oooii/oRefCount.h>

class oWebmEncoderCodecRaw420 : public oWebmEncoderCodec
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oWebmEncoderCodecRaw420>());
	oWebmEncoderCodecRaw420(const oVideoEncodeCPU::DESC &_Desc, bool* _pSuccess);
	~oWebmEncoderCodecRaw420();

	const std::vector<unsigned char>& GetWebmCodecEbml() const override { return CodecEbml; }
	const std::vector<unsigned char>& GetWebmCodecNameEbml() const override { return CodecNameEbml; }

	void Encode( const oSurface::YUV420& _Frame, void* _pDataStream, size_t _StreamSize, size_t* _pDataWrittenToStream, 
		unsigned int _frameStamp, unsigned int _frameDuration, bool _forceIFrame, FrameDesc &_desc ) override;
	void EncodeFirstPass(const oSurface::YUV420& _Frame, unsigned int _frameStamp, unsigned int _frameDuration, bool _forceIFrame /*= false*/) override;
	void StartSecondPass(unsigned int _frameStamp, unsigned int _frameDuration) override;

private:
	oVideoEncodeCPU::DESC Desc;
	
	oRefCount RefCount;
	std::vector<unsigned char> CodecEbml;
	std::vector<unsigned char> CodecNameEbml;
};

#endif