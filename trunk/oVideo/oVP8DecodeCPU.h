// $(header)
#pragma once
#ifndef oVP8DecodeCPU_h
#define oVP8DecodeCPU_h

#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oMutex.h>
#include <oVideo/oVideoCodec.h>
#include <vpx/vpx_codec.h>

class oVP8DecodeCPU : public oVideoDecodeCPU
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	virtual bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe;

	oVP8DecodeCPU( oVideoContainer* _pContainer,  bool* _pSuccess);
	~oVP8DecodeCPU();

	virtual bool Decode(oSurface::YUV420* _pFrame, size_t* _pDecodedFrameNumber) ;

private:
	void InitializeVP8();

	oRefCount RefCount;
	vpx_codec_ctx_t Context;
	bool IsInitialized;

	oRef<oVideoContainer> Container;
};

#endif
