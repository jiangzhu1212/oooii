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
#ifndef oWebmFile_h
#define oWebmFile_h
#include <oVideo/oVideoCodec.h>
#include <oooii/oRefCount.h>
#include <oooii/oThreading.h>
#include <oooii/oRef.h>
#include <oooii/oMemoryMappedFile.h>
#include "oWebmStreaming.h"
#include "oVideoHelpers.h"

class oWebmFile : public oVideoFile
{
public:
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oGetGUID<oVideoFile>(), oGetGUID<oVideoContainer>() );
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDECLARE_VIDEO_MAP_INTERFACE();
	oWebmFile( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess);
	~oWebmFile();

	virtual void GetDesc(DESC* _pDesc) const override
	{
		WebmStreaming->GetDesc(_pDesc);
	}

	bool HasFinished() const override { return Finished; }
	void Restart() override { FileIndex = DataStartIndex; Finished = false;}

private:
	oRefCount RefCount;
	char FileName[_MAX_PATH];

	bool Finished;

	oRef<threadsafe oMemoryMappedFile> MemoryMappedFrame;
	oRef<oWebmStreaming> WebmStreaming;
	unsigned long long FileIndex;
	unsigned long long DataStartIndex;
};


#endif //oWebmFile_h