// $(header)
#pragma once
#ifndef oImageDecodeCPU_h
#define oImageDecodeCPU_h

#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oMutex.h>
#include <oVideo/oVideoCodec.h>
#include <oooii/oEvent.h>

struct oImage;

class oImageDecodeCPU : public oVideoDecodeCPU
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	virtual bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe;

	oImageDecodeCPU( oVideoContainer* _pContainer,  bool* _pSuccess);
	~oImageDecodeCPU();

	virtual bool Decode(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber);

private:
	void ReadImage();

	oRefCount RefCount;
	oEvent ReadImageEvent;
	bool LastImageReadResult;
	bool ContainerWasFinished;
	size_t LastReadFrameNumber;

	oRef<oImage> Image;
	oRef<oVideoContainer> Container;
	oMutex DecodeLock;
	std::vector<unsigned char> RawBuffer;
	std::vector<unsigned char> YuvBuffer;
};

#endif
