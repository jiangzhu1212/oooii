// $(header)
#include "oRawDecodeCPU.h"
#include <oooii/oByte.h>

const oGUID& oGetGUID( threadsafe const oRawDecodeCPU* threadsafe const * )
{
	// {917d6a97-ad93-402e-aae5-9daa022cd114}
	static const oGUID oIIDoRawDecodeCPU = { 0x917d6a97, 0xad93, 0x402e, { 0xaa, 0xe5, 0x9d, 0xaa, 0x02, 0x2c, 0xd1, 0x14 } };
	return oIIDoRawDecodeCPU; 
}

oRawDecodeCPU::oRawDecodeCPU( threadsafe oVideoContainer* _pContainer,  bool* _pSuccess) : Container(_pContainer)
{
	*_pSuccess = true;
}

oRawDecodeCPU::~oRawDecodeCPU()
{
}

bool oRawDecodeCPU::DecodeInternal(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber)
{
	void *data;
	size_t dataSize;
	bool valid;
	Container->MapFrame(&data, &dataSize, &valid, _decodedFrameNumber);
	if(!valid || dataSize == 0)
	{
		Container->UnmapFrame();
		return false;
	}

	oVideoContainer::DESC desc;
	Container->GetDesc(&desc);

	size_t yPixelCount = desc.Width * desc.Height;
	size_t uvPixelCount = yPixelCount / 4;
	WorkBuffer.resize(yPixelCount + 2*uvPixelCount);
	_pFrame->pY = &WorkBuffer[0];
	_pFrame->pU = &WorkBuffer[yPixelCount];
	_pFrame->pV = &WorkBuffer[yPixelCount+uvPixelCount];
	memcpy(_pFrame->pY, data, yPixelCount);
	memcpy(_pFrame->pU, oByteAdd(data, yPixelCount), uvPixelCount);
	memcpy(_pFrame->pV, oByteAdd(data, yPixelCount+uvPixelCount), uvPixelCount);
	_pFrame->UVPitch = desc.Width/2;
	_pFrame->YPitch = desc.Width;

	Container->UnmapFrame();

	return true;
}

bool oRawDecodeCPU::Decode(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber) threadsafe
{
	oMutex::ScopedLock lock(DecodeLock);
	return thread_cast<oRawDecodeCPU*>(this)->DecodeInternal(_pFrame, _decodedFrameNumber);
};

bool oRawDecodeCPU::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	if( oGetGUID<oRawDecodeCPU>() == _InterfaceID || oGetGUID<oVideoDecodeCPU>() == _InterfaceID )
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