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
#ifndef oVideoHelpers_h
#define oVideoHelpers_h

#include <oVideo/oVideoCodec.h>
#include <oooii/oErrno.h>

// @oooii-eric: deprecated for now. video is kind of inherently not threadsafe. what can happen is
//	if 2 threads are reading frames from a container, the mutex would just serialize them. then the 
//	first thread in would get frame 0, the iframe and be happy. the second thread would get frame 1 though
//	which is not an iframe, and since that thread had not seen an iframe yet, it would not be able to decode
//	the frame. If we really wanted to have multithreaded calls to decode we would need each thread to get the same frame,
//	and have another way to indicate it was time to go to the next frame. for now just no longer claim 
//	that video decoders are threadsafe. They still use many threads internally.
/*
#define oDEFINE_VIDEO_MUTEXED_MAP_INTERFACE(_Type, _Mutex) \
	bool Map(MAPPED* _pMapped) threadsafe override \
	{ (_Mutex).Lock(); \
	bool result = thread_cast<_Type*>(this)->MapNOLOCK(_pMapped); \
	if (!result) (_Mutex).Unlock(); \
	return result && (_pMapped->DataSize > 0); \
	} \
	void Unmap() threadsafe override \
	{	thread_cast<_Type*>(this)->UnmapNOLOCK(); \
		(_Mutex).Unlock(); \
	}

#define oDECLARE_VIDEO_MAP_INTERFACE() \
	bool MapNOLOCK(MAPPED* _pMapped); \
	void UnmapNOLOCK()
	*/

#define oDECLARE_VIDEO_MAP_INTERFACE() \
	bool Map(MAPPED* _pMapped) override; \
	void Unmap() override

inline void oVideoNullMapped(oVideoContainer::MAPPED* _pMapped)
{
	_pMapped->pFrameData = nullptr;
	_pMapped->DataSize = 0;
	_pMapped->DecodedFrameNumber = oINVALID_SIZE_T;
}

inline bool oVideoReturnEndOfFile(oVideoContainer::MAPPED* _pMapped)
{
	oSetLastError(ENOENT); // @oooii-tony: I kinda wish there were an EOF, but is it work adding EEOF? or ESTREAMEND? or EENUMEND for FindFiles/EnumFiles/EnumGPUs/etc.?
	oVideoNullMapped(_pMapped);
	return false;
}

#endif
