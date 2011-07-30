// $(header)
#pragma once
#ifndef oRleDecodeCPU_h
#define oRleDecodeCPU_h

#include <oVideo/oVideoCodec.h>
#include <oooii/oMutex.h>
#include <oooii/oRef.h> 
#include <oooii/oRefCount.h> 
#include <type_traits> 

class oRleDecodeCPU : public oVideoDecodeCPU
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oRleDecodeCPU( oVideoContainer* _pContainer,  bool* _pSuccess);
	~oRleDecodeCPU();

	virtual bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe;
	virtual bool Decode(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber);

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

	oRefCount RefCount;
	bool IsInitialized;
	//For rle compression, if a pixel hasn't changed from the previous frame, it may not be included in the next frame's data. So you need to keep
	//	the last frame around.
	std::vector<unsigned int> BGRADecodeFrame;

	oRef<oVideoContainer> Container;
	oMutex DecodeLock;
};

#endif //oVP8DecodeCPU_h