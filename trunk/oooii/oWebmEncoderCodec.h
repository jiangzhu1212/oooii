// $(header)
#pragma once
#ifndef oWebmEncoderCodec_h
#define oWebmEncoderCodec_h
#include <oooii/oVideoCodec.h>
#include <oooii/oStddef.h>

interface oWebmEncoderCodec : oInterface
{
	static bool Create(const oVideoEncodeCPU::DESC &_Desc, oWebmEncoderCodec** _pCodec);

	struct FrameDesc
	{
		unsigned int TimeStamp;
		unsigned int PayloadSize;
		bool Keyframe;
		bool Invisible;
		bool CanDiscard;
	};

	virtual const std::vector<unsigned char>& GetWebmCodecEbml() const = 0;
	virtual const std::vector<unsigned char>& GetWebmCodecNameEbml() const = 0;
	virtual void Encode( const oSurface::YUV420& _Frame, void* _pDataStream, size_t _StreamSize, size_t* _pDataWrittenToStream,
		unsigned int _frameStamp, unsigned int _frameDuration, bool _forceIFrame, FrameDesc &_desc ) = 0;
	virtual void EncodeFirstPass(const oSurface::YUV420& _Frame, unsigned int _frameStamp, unsigned int _frameDuration, bool _forceIFrame /*= false*/) = 0;
	virtual void StartSecondPass(unsigned int _frameStamp, unsigned int _frameDuration) = 0;
};

#endif