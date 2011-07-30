// $(header)
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

oImageDecodeCPU::oImageDecodeCPU( oVideoContainer* _pContainer,  bool* _pSuccess) : Container(_pContainer)
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

	if(Container.c_ptr()->HasFinished())
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

bool oImageDecodeCPU::Decode(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber)
{
	ReadImageEvent.Wait();

	if(ContainerWasFinished)
	{
		if (Container.c_ptr()->HasFinished())
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

	if(static_cast<int>( imgDesc.Width ) != conDesc.Dimensions.x || static_cast<int>( imgDesc.Height ) != conDesc.Dimensions.y)
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
