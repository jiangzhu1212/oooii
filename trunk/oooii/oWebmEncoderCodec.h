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