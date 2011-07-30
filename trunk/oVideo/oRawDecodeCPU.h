// $(header)
#pragma once
#ifndef oRawDecodeCPU_h
#define oRawDecodeCPU_h

#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oMutex.h>
#include <oVideo/oVideoCodec.h>

class oRawDecodeCPU : public oVideoDecodeCPU
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	virtual bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe;

	oRawDecodeCPU( oVideoContainer* _pContainer,  bool* _pSuccess);
	~oRawDecodeCPU();

	virtual bool Decode(oSurface::YUV420* _pFrame, size_t* _pDecodedFrameNumber);

private:
	oRefCount RefCount;

	oRef<oVideoContainer> Container;
	std::vector<unsigned char> WorkBuffer;
};

#endif
