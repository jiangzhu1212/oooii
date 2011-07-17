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
#include "oImageDecodeCPU.h"
#include <oooii/oByte.h>
#include <oooii/oImage.h>
#include <oooii/oThreading.h>
#include <oooii/oSTL.h>

const oGUID& oGetGUID( threadsafe const oImageDecodeCPU* threadsafe const * )
{
	// {ffa3178b-cf5a-4846-a3f6-0274ae647cc9}
	static const oGUID oIIDoImageDecodeCPU = { 0xffa3178b, 0xcf5a, 0x4846, { 0xa3, 0xf6, 0x02, 0x74, 0xae, 0x64, 0x7c, 0xc9 } };
	return oIIDoImageDecodeCPU; 
}

oImageDecodeCPU::oImageDecodeCPU( threadsafe oVideoContainer* _pContainer,  bool* _pSuccess) : Container(_pContainer)
{
	ReadImageEvent.Reset();
	oIssueAsyncTask(oBIND(&oImageDecodeCPU::ReadImage, this));

	*_pSuccess = true;
}

oImageDecodeCPU::~oImageDecodeCPU()
{
	oVERIFY(ReadImageEvent.Wait(20000)); //should only block for if there is a frame getting decoded, and there should never be more than 1. So this should never take long.
}

void oImageDecodeCPU::ReadImage()
{
	oRAII setEvent([&] {ReadImageEvent.Set();});

	//ReadImageEvent is used to make this cast safe.
	if(thread_cast<oVideoContainer*>(Container.c_ptr())->HasFinished())
	{
		ContainerWasFinished = true;
		return;
	}
	else
		ContainerWasFinished = false;

	oVideoContainer::MAPPED mapped;
	if (Container->Map(&mapped))
	{
		LastReadFrameNumber = mapped.DecodedFrameNumber;

		if (!Image)
			oImage::Create(mapped.pFrameData, mapped.DataSize, oSurface::YUV420_UNORM, &Image);
		else
			Image->Update(mapped.pFrameData, mapped.DataSize);

		LastImageReadResult = true;
		Container->Unmap();
	}
}

bool oImageDecodeCPU::DecodeInternal(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber)
{
	ReadImageEvent.Wait();

	if(ContainerWasFinished)
	{
		//ReadImageEvent is used to make this cast safe.
		if (thread_cast<oVideoContainer*>(Container.c_ptr())->HasFinished())
		{
			return false;
		}
		else //video was restarted, so restart decoding.
		{
			ReadImageEvent.Reset();
			oIssueAsyncTask(oBIND(&oImageDecodeCPU::ReadImage, this));
			ReadImageEvent.Wait();
		}
	}

	if(!LastImageReadResult)
		return false;

	if(_decodedFrameNumber)
		*_decodedFrameNumber = LastReadFrameNumber;
	
	oVideoContainer::DESC conDesc;
	Container->GetDesc(&conDesc);

	oImage::DESC imgDesc;
	Image->GetDesc(&imgDesc);

	if(imgDesc.Width != conDesc.Width || imgDesc.Height != conDesc.Height)
	{
		oSetLastError(EINVAL, "A png image was the wrong size.");
		return false;
	}

	unsigned char* yuvdata = (unsigned char*)Image->Map();
	_pFrame->YPitch = imgDesc.Pitch;
	_pFrame->UVPitch = imgDesc.Pitch/2;
	_pFrame->pY = yuvdata;
	_pFrame->pU = yuvdata + _pFrame->YPitch * imgDesc.Height;
	_pFrame->pV = _pFrame->pU + _pFrame->UVPitch * imgDesc.Height/2;
	Image->Unmap();

	ReadImageEvent.Reset();
	oIssueAsyncTask(oBIND(&oImageDecodeCPU::ReadImage, this));

	return true;
}

bool oImageDecodeCPU::Decode(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber) threadsafe
{
	oMutex::ScopedLock lock(DecodeLock);
	return thread_cast<oImageDecodeCPU*>(this)->DecodeInternal(_pFrame, _decodedFrameNumber);
};

bool oImageDecodeCPU::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	if( oGetGUID<oImageDecodeCPU>() == _InterfaceID || oGetGUID<oVideoDecodeCPU>() == _InterfaceID )
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
