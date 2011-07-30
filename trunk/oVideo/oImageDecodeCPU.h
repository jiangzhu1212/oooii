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
