// $(header)
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