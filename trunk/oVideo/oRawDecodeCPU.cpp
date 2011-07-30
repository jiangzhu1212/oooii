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
#include "oRawDecodeCPU.h"
#include <oooii/oByte.h>

const oGUID& oGetGUID( threadsafe const oRawDecodeCPU* threadsafe const * )
{
	// {917d6a97-ad93-402e-aae5-9daa022cd114}
	static const oGUID oIIDoRawDecodeCPU = { 0x917d6a97, 0xad93, 0x402e, { 0xaa, 0xe5, 0x9d, 0xaa, 0x02, 0x2c, 0xd1, 0x14 } };
	return oIIDoRawDecodeCPU; 
}

oRawDecodeCPU::oRawDecodeCPU( oVideoContainer* _pContainer,  bool* _pSuccess) : Container(_pContainer)
{
	*_pSuccess = true;
}

oRawDecodeCPU::~oRawDecodeCPU()
{
}

bool oRawDecodeCPU::Decode(oSurface::YUV420* _pFrame, size_t* _pDecodedFrameNumber)
{
	oVideoContainer::MAPPED mapped;
	bool result = Container->Map(&mapped);
	if (result)
	{
		if (_pDecodedFrameNumber)
			*_pDecodedFrameNumber = mapped.DecodedFrameNumber;

		oVideoContainer::DESC desc;
		Container->GetDesc(&desc);

		size_t yPixelCount = desc.Dimensions.x * desc.Dimensions.y;
		size_t uvPixelCount = yPixelCount / 4;
		WorkBuffer.resize(yPixelCount + 2*uvPixelCount);
		_pFrame->pY = &WorkBuffer[0];
		_pFrame->pU = &WorkBuffer[yPixelCount];
		_pFrame->pV = &WorkBuffer[yPixelCount+uvPixelCount];
		memcpy(_pFrame->pY, mapped.pFrameData, yPixelCount);
		memcpy(_pFrame->pU, oByteAdd(mapped.pFrameData, yPixelCount), uvPixelCount);
		memcpy(_pFrame->pV, oByteAdd(mapped.pFrameData, yPixelCount+uvPixelCount), uvPixelCount);
		_pFrame->UVPitch = desc.Dimensions.x/2;
		_pFrame->YPitch = desc.Dimensions.x;

		Container->Unmap();
	}

	return result;
}

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