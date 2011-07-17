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
#ifndef oImageSequence_h
#define oImageSequence_h
#include <oooii/oVideoCodec.h>
#include <oooii/oRefCount.h>
#include <oooii/oMemoryMappedFile.h>
#include <oooii/oMutex.h>
#include <oooii/oRef.h>
#include "oVideoHelpers.h"

class oImageSequence : public oVideoFile
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,threadsafe);
	oDEFINE_VIDEO_MUTEXED_MAP_INTERFACE(oImageSequence, Mutex);
	oImageSequence( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess);
	~oImageSequence();

	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;
	virtual bool HasFinished() const override { return CurrentFrame >= ImageFiles.size();}
	void Restart() threadsafe override { oMutex::ScopedLock lock(Mutex); CurrentFrame = 0;}

private:
	oDECLARE_VIDEO_MAP_INTERFACE();

	struct ImageFile
	{
		ImageFile(char *_pFileName); //will modify the supplied string
		bool operator<(const ImageFile &_other)
		{
			return ImageNumber < _other.ImageNumber;
		}
		char FileName[_MAX_PATH];
		long long ImageNumber;
	};

	oRefCount RefCount;
	oVideoContainer::DESC Desc;
	unsigned int CurrentFrame; //The frame that will get decoded on the next call to Decode.
	oMutex Mutex;

	oRef<threadsafe oMemoryMappedFile> MemoryMappedFrame;
	std::vector<ImageFile> ImageFiles;
};


#endif //oImageSequence_h
