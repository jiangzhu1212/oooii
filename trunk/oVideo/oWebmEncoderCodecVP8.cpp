// $(header)
#include "oWebmEncoderCodecVP8.h"

static const vpx_img_fmt VP8_FRAME_FORMAT = VPX_IMG_FMT_I420;

const oGUID& oGetGUID( threadsafe const oWebmEncoderCodecVP8* threadsafe const * )
{
	// {b26feaab-c29d-418f-be85-07e2d3c4b8cc}
	static const oGUID oIIDoWebmEncoderCodecVP8 = { 0xb26feaab, 0xc29d, 0x418f, { 0xbe, 0x85, 0x07, 0xe2, 0xd3, 0xc4, 0xb8, 0xcc } };
	return oIIDoWebmEncoderCodecVP8; 
}

oWebmEncoderCodecVP8::oWebmEncoderCodecVP8(const oVideoEncodeCPU::DESC &_Desc, bool* _pSuccess) : Desc(_Desc), pImage(NULL)
{
	*_pSuccess = false;

	FirstPassStats.buf = NULL;
	FirstPassStats.sz = 0;

	CodecEbml.push_back(0x86);
	CodecEbml.push_back(0x85);
	CodecEbml.push_back('V');
	CodecEbml.push_back('_');
	CodecEbml.push_back('V');
	CodecEbml.push_back('P');
	CodecEbml.push_back('8');

	CodecNameEbml.push_back(0x25);
	CodecNameEbml.push_back(0x86);
	CodecNameEbml.push_back(0x88);
	CodecNameEbml.push_back(0x83);
	CodecNameEbml.push_back('V');
	CodecNameEbml.push_back('P');
	CodecNameEbml.push_back('8');

	if( VPX_CODEC_OK != vpx_codec_enc_config_default( &vpx_codec_vp8_cx_algo, &VP8Cfg, 0 ) )
	{
		oSetLastError(EINVAL,"failed to get VP8 default params");
		return;
	}

	unsigned int BitRate = _Desc.BitRate;
	if( (unsigned int )-1 == _Desc.BitRate )
		BitRate = _Desc.Dimensions.x * _Desc.Dimensions.y * VP8Cfg.rc_target_bitrate / VP8Cfg.g_w / VP8Cfg.g_h; 

	//@ooii-eric: Hack, for some reason over 35Mb/s doesnt work for single pass. remove this clamp once that is fixed. Note that even in 2 pass,
	//	bit rates above about 70mb/s don't have much of an impact, but it doesn't break.
	if(!_Desc.TwoPass)
		BitRate = __min(_Desc.BitRate,35000);

	VP8Cfg.rc_target_bitrate = BitRate;
	VP8Cfg.g_w = _Desc.Dimensions.x;
	VP8Cfg.g_h = _Desc.Dimensions.y;
	VP8Cfg.g_timebase.num = 1; // webm time stamps are in milliseconds, set up vp8 to match. All timestamps are a multiple of this.
	VP8Cfg.g_timebase.den = 1000;

	if(_Desc.TwoPass)
	{
		VP8Cfg.g_pass = VPX_RC_FIRST_PASS;
	}
	else
	{
		VP8Cfg.g_pass = VPX_RC_ONE_PASS;
	}

	if( (unsigned int)-1 == GetQuality( _Desc.Quality ) )
	{
		oSetLastError(EINVAL,"Invalid quality");
		return;
	}

	switch( vpx_codec_enc_init(&Context, &vpx_codec_vp8_cx_algo, &VP8Cfg, 0 ) )
	{
	case VPX_CODEC_OK:
		break;

	case VPX_CODEC_INVALID_PARAM:
		oSetLastError(EINVAL,"VP8 init error invalid param");
		return;

	default:
		oSetLastError(EINVAL,"VP8 init error unknown");
		return;
	}

	pImage = vpx_img_alloc(NULL, VP8_FRAME_FORMAT, VP8Cfg.g_w, VP8Cfg.g_h, 0 );
	if( !pImage )
	{
		oSetLastError(EINVAL,"failed to alloc VP8 image");
		return;
	}

	*_pSuccess = true;
}

oWebmEncoderCodecVP8::~oWebmEncoderCodecVP8()
{
	if(pImage)
	{
		vpx_img_free(pImage);
		vpx_codec_destroy(&Context);
	}
	free(FirstPassStats.buf);
}

unsigned int oWebmEncoderCodecVP8::GetQuality( oVideoEncodeCPU::QUALITY quality )
{
	switch( quality )
	{
	case oVideoEncodeCPU::REALTIME:
		return VPX_DL_REALTIME;

	case oVideoEncodeCPU::GOOD_QUALITY:
		return VPX_DL_GOOD_QUALITY;

	case oVideoEncodeCPU::BEST_QUALITY:
		return VPX_DL_BEST_QUALITY;
	}

	return (unsigned int)-1;
}

void oWebmEncoderCodecVP8::Encode( const oSurface::YUV420& _Frame, void* _pDataStream, size_t _StreamSize, size_t* _pDataWrittenToStream, 
	unsigned int _frameStamp, unsigned int _frameDuration, bool _forceIFrame, FrameDesc &_desc )
{
	// const_cast is necessary because the underlying API is not const
	// correct here (even though it doesn't modify the pointers)
	pImage->planes[VPX_PLANE_Y] = const_cast<unsigned char*>( _Frame.pY );
	pImage->planes[VPX_PLANE_U] = const_cast<unsigned char*>( _Frame.pU );
	pImage->planes[VPX_PLANE_V] = const_cast<unsigned char*>( _Frame.pV );
	pImage->stride[VPX_PLANE_Y] = static_cast<int>(_Frame.YPitch);
	pImage->stride[VPX_PLANE_U] = static_cast<int>(_Frame.UVPitch);
	pImage->stride[VPX_PLANE_V] = static_cast<int>(_Frame.UVPitch);

	vpx_fixed_buf_t buf;
	buf.buf = _pDataStream;
	buf.sz = _StreamSize;
	vpx_codec_set_cx_data_buf( &Context, &buf, 0, 0 );
	vpx_enc_frame_flags_t flags = _forceIFrame ? VPX_EFLAG_FORCE_KF : 0;

	vpx_codec_encode( &Context, pImage, _frameStamp, _frameDuration, flags, GetQuality( Desc.Quality ) );

	const vpx_codec_cx_pkt_t* pPacket = NULL;
	vpx_codec_iter_t iter = NULL;
	pPacket = vpx_codec_get_cx_data( &Context, &iter );
	{
		oASSERT( VPX_CODEC_CX_FRAME_PKT == pPacket->kind, "Check packet" );
		oASSERT( buf.buf == pPacket->data.frame.buf, "Frame was written to unexpected location" );

		// result should definitely not be larger than uncompressed size
		oASSERT( pPacket->data.frame.sz < _StreamSize, "compressed frame was larger than supplied stream" );
		*_pDataWrittenToStream = static_cast<unsigned int>( pPacket->data.frame.sz );
		_desc.Keyframe = (pPacket->data.frame.flags&VPX_FRAME_IS_KEY) ?  true : false;
		_desc.CanDiscard = (pPacket->data.frame.flags&VPX_FRAME_IS_DROPPABLE) ?  true : false;
		_desc.Invisible = (pPacket->data.frame.flags&VPX_FRAME_IS_INVISIBLE) ?  true : false;
	}
}

void oWebmEncoderCodecVP8::EncodeFirstPass(const oSurface::YUV420& _Frame, unsigned int _frameStamp, unsigned int _frameDuration, bool _forceIFrame /*= false*/)
{
	oASSERT(Desc.TwoPass,"Tried to encode first pass when encoder is not set up for 2 pass encoding");

	// const_cast is necessary because the underlying API is not const
	// correct here (even though it doesn't modify the pointers)
	pImage->planes[VPX_PLANE_Y] = const_cast<unsigned char*>( _Frame.pY );
	pImage->planes[VPX_PLANE_U] = const_cast<unsigned char*>( _Frame.pU );
	pImage->planes[VPX_PLANE_V] = const_cast<unsigned char*>( _Frame.pV );
	pImage->stride[VPX_PLANE_Y] = static_cast<int>(_Frame.YPitch);
	pImage->stride[VPX_PLANE_U] = static_cast<int>(_Frame.UVPitch);
	pImage->stride[VPX_PLANE_V] = static_cast<int>(_Frame.UVPitch);

	vpx_enc_frame_flags_t flags = _forceIFrame ? VPX_EFLAG_FORCE_KF : 0;

	vpx_codec_encode( &Context, pImage, _frameStamp, _frameDuration, flags, GetQuality( Desc.Quality ) );

	const vpx_codec_cx_pkt_t* pPacket = NULL;
	vpx_codec_iter_t iter = NULL;
	pPacket = vpx_codec_get_cx_data( &Context, &iter );
	{
		oASSERT( VPX_CODEC_STATS_PKT == pPacket->kind, "first pass should be only stats packets" );

		FirstPassStats.buf = realloc(FirstPassStats.buf, FirstPassStats.sz + pPacket->data.twopass_stats.sz);   
		memcpy((char*)FirstPassStats.buf + FirstPassStats.sz, pPacket->data.twopass_stats.buf, pPacket->data.twopass_stats.sz);
		FirstPassStats.sz +=  pPacket->data.twopass_stats.sz; 
	}
}

void oWebmEncoderCodecVP8::StartSecondPass(unsigned int _frameStamp, unsigned int _frameDuration)
{
	//need to flush any remaining stats before starting second pass
	vpx_codec_encode( &Context, NULL, _frameStamp, _frameDuration, 0, GetQuality( Desc.Quality ) );
	const vpx_codec_cx_pkt_t* pPacket = NULL;
	vpx_codec_iter_t iter = NULL;
	pPacket = vpx_codec_get_cx_data( &Context, &iter );
	while(pPacket)
	{
		oASSERT( VPX_CODEC_STATS_PKT == pPacket->kind, "first pass should be only stats packets" );

		FirstPassStats.buf = realloc(FirstPassStats.buf, FirstPassStats.sz + pPacket->data.twopass_stats.sz);   
		memcpy((char*)FirstPassStats.buf + FirstPassStats.sz, pPacket->data.twopass_stats.buf, pPacket->data.twopass_stats.sz);
		FirstPassStats.sz +=  pPacket->data.twopass_stats.sz; 
		pPacket = vpx_codec_get_cx_data( &Context, &iter );
	}

	vpx_codec_destroy(&Context);
	VP8Cfg.g_pass = VPX_RC_LAST_PASS;
	VP8Cfg.rc_twopass_stats_in = FirstPassStats;

	switch( vpx_codec_enc_init(&Context, &vpx_codec_vp8_cx_algo, &VP8Cfg, 0 ) )
	{
	case VPX_CODEC_OK:
		break;

	case VPX_CODEC_INVALID_PARAM:
		oSetLastError(EINVAL,"VP8 init error invalid param, failed trying to start second pass");
		return;

	default:
		oSetLastError(EINVAL,"VP8 init error unknown, failed trying to start second pass");
		return;
	}
}