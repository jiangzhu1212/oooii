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
#pragma warning(disable:4505) // unreferenced local function has been removed
#include "oVP8DecodeCPU.h"
#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>

static const vpx_img_fmt VP8_FRAME_FORMAT = VPX_IMG_FMT_I420;

const oGUID& oGetGUID( threadsafe const oVP8DecodeCPU* threadsafe const * )
{
	// {5EE39410-0695-490a-BB1C-4531279D8093}
	static const oGUID oIIDVP8DecodeCPU = { 0x5ee39410, 0x695, 0x490a, { 0xbb, 0x1c, 0x45, 0x31, 0x27, 0x9d, 0x80, 0x93 } };
	return oIIDVP8DecodeCPU;
}

oVP8DecodeCPU::oVP8DecodeCPU( oVideoContainer* _pContainer,  bool* _pSuccess) : IsInitialized(false), Container(_pContainer)
{
	*_pSuccess = true;
}

oVP8DecodeCPU::~oVP8DecodeCPU()
{
	if( IsInitialized )
		vpx_codec_destroy(&Context);
}

void oVP8DecodeCPU::InitializeVP8()
{
	if(!IsInitialized)
	{
		oVideoContainer::DESC containerDesc;
		Container->GetDesc(&containerDesc);

		vpx_codec_dec_cfg cfg;
		cfg.h = containerDesc.Dimensions.y;
		cfg.w = containerDesc.Dimensions.x;
		cfg.threads = 1;

		switch( vpx_codec_dec_init( &Context, &vpx_codec_vp8_dx_algo, &cfg, 0 )  )
		{
		case VPX_CODEC_OK:
			IsInitialized = true;
			break;

		case VPX_CODEC_INCAPABLE:
			oSetLastError(EBADFD);
			return;

		case VPX_CODEC_INVALID_PARAM:
			oSetLastError(EINVAL);
			return;

		default:
			return;
		};
	}
}

bool oVP8DecodeCPU::Decode(oSurface::YUV420* _pFrame, size_t* _pDecodedFrameNumber)
{
	if (_pDecodedFrameNumber)
		*_pDecodedFrameNumber = oINVALID_SIZE_T;

	oVideoContainer::MAPPED mapped;
	if (Container->Map(&mapped))
	{
		if (_pDecodedFrameNumber)
			*_pDecodedFrameNumber = mapped.DecodedFrameNumber;

		// @oooii-tony can this be moved outside the map call so it doesn't have to 
		// call unmap if it fails?
		InitializeVP8();
		if(!IsInitialized)
		{
			Container->Unmap();
			return false;
		}

		vpx_codec_decode(&Context, (const uint8_t*)mapped.pFrameData, (unsigned int)mapped.DataSize, nullptr, 0 );

		// We don't need to loop over the iterator as we always drain the frame immediately after calling decode
		vpx_codec_iter_t  iter = NULL;
		vpx_image_t* pImage = vpx_codec_get_frame(&Context, &iter );
		oASSERT( pImage, "Failed to retrieve frame");
		oASSERT( VP8_FRAME_FORMAT == pImage->fmt, "Unknown frame format" );
		oASSERT( pImage->stride[VPX_PLANE_U] == pImage->stride[VPX_PLANE_V], "Expected U and V planes to have equal stride" );

		_pFrame->pY = pImage->planes[VPX_PLANE_Y];
		_pFrame->pU = pImage->planes[VPX_PLANE_U];
		_pFrame->pV = pImage->planes[VPX_PLANE_V];

		_pFrame->UVPitch = pImage->stride[VPX_PLANE_U];
		_pFrame->YPitch = pImage->stride[VPX_PLANE_Y];

		Container->Unmap();
		return true;
	}

	return false;
}

bool oVP8DecodeCPU::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	if( oGetGUID<oVP8DecodeCPU>() == _InterfaceID || oGetGUID<oVideoDecodeCPU>() == _InterfaceID )
	{
		Reference();
		*_ppInterface = this;
		return true;
	}
	if( oGetGUID<oVideoContainer>() == _InterfaceID )
	{
		Container->Reference();
		*_ppInterface = Container;
		return true;
	}

	return false;
}