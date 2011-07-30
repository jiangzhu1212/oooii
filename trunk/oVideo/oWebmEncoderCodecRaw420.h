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