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
#ifndef oRleDecodeCPU_h
#define oRleDecodeCPU_h

#include <oooii/oVideoCodec.h>
#include <oooii/oMutex.h>
#include <oooii/oRef.h> 
#include <oooii/oRefCount.h> 
#include <type_traits> 

class oRleDecodeCPU : public oVideoDecodeCPU
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oRleDecodeCPU( threadsafe oVideoContainer* _pContainer,  bool* _pSuccess);
	~oRleDecodeCPU();

	virtual bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe;
	virtual bool Decode(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber) threadsafe;

private:
	template<typename T> void ReadRle(T& _item, unsigned int* _pInOutDataIndex, const oVideoContainer::MAPPED& _Mapped, unsigned int _numItems = 1)
	{
		unsigned int numBytes = _numItems*sizeof(T);
		if(numBytes+(*_pInOutDataIndex) >= _Mapped.DataSize)
			return;
		memcpy(&_item,oByteAdd(_Mapped.pFrameData,*_pInOutDataIndex),numBytes);
		*_pInOutDataIndex += numBytes;
	}
	template<typename T> void ReadRleSwap(T& _item, unsigned int* _pInOutDataIndex, const oVideoContainer::MAPPED& _Mapped, unsigned int _numItems = 1)
	{
		ReadRle(_item,_pInOutDataIndex,_Mapped,_numItems);
		_item = oByteSwap(_item);
	}
	bool DecodeInternal(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber);

	oRefCount RefCount;
	bool IsInitialized;
	//For rle compression, if a pixel hasn't changed from the previous frame, it may not be included in the next frame's data. So you need to keep
	//	the last frame around.
	std::vector<unsigned int> BGRADecodeFrame;

	oRef<threadsafe oVideoContainer> Container;
	oMutex DecodeLock;
};

#endif //oVP8DecodeCPU_h