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
	template<typename T> void ReadRle(T& _item,unsigned int _numItems = 1)
	{
		unsigned int numBytes = _numItems*sizeof(T);
		if(numBytes+DataIndex >= DataSize)
			return;
		memcpy(&_item,oByteAdd(Data,DataIndex),numBytes);
		DataIndex += numBytes;
	}
	template<typename T> void ReadRleSwap(T& _item,unsigned int _numItems = 1)
	{
		ReadRle(_item,_numItems);
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
	void *Data;
	size_t DataSize;
	unsigned int DataIndex;
};

#endif //oVP8DecodeCPU_h