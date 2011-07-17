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
#include "oWebmEncoderCodec.h"
#include "oWebmEncoderCodecVP8.h"
#include "oWebmEncoderCodecRaw420.h"

bool oWebmEncoderCodec::Create( const oVideoEncodeCPU::DESC &_Desc, oWebmEncoderCodec** _pCodec )
{
	bool success = false;
	switch( _Desc.CodecType )
	{
	case oVideoEncodeCPU::VP8_CODEC:
		{
			oCONSTRUCT( _pCodec, oWebmEncoderCodecVP8( _Desc, &success) );
		}
		break;

	case oVideoEncodeCPU::RAW_420:
		{
			oCONSTRUCT( _pCodec, oWebmEncoderCodecRaw420( _Desc, &success) );
		}
		break;
	}

	return success;
}