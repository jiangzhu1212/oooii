// $(header)
#pragma once
#ifndef oRawDecodeCPU_h
#define oRawDecodeCPU_h

#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oMutex.h>
#include <oooii/oVideoCodec.h>

class oRawDecodeCPU : public oVideoDecodeCPU
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	virtual bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe;

	oRawDecodeCPU( threadsafe oVideoContainer* _pContainer,  bool* _pSuccess);
	~oRawDecodeCPU();

	virtual bool Decode(oSurface::YUV420* _pFrame, size_t* _pDecodedFrameNumber) threadsafe;

private:
	bool DecodeInternal(oSurface::YUV420* _pFrame, size_t* _pDecodedFrameNumber);

	oRefCount RefCount;

	oRef<threadsafe oVideoContainer> Container;
	oMutex DecodeLock;
	std::vector<unsigned char> WorkBuffer;
};

#endif
