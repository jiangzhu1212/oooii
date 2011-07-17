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
#include <oooii/oStdio.h>
#include <oooii/oMath.h>
#include <oooii/oString.h>
#include <oooii/oSTL.h>
#include <oooii/oCPU.h>
#include "oWebmFile.h"

const unsigned int VIEW_SIZE = oMB(8); //frame should never be larger than this in the webm file, or even close. typically a frame is around 512K a frame for a 120Mb/s file.

oWebmFile::oWebmFile( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess) : Finished(false)
{
	*_pSuccess = false;
	bool success = false;
	oWebmStreaming *tempWebmStreaming;
	oCONSTRUCT(&tempWebmStreaming, oWebmStreaming(_Desc, &success));
	*(&WebmStreaming) = tempWebmStreaming;
	*_pSuccess = success;
	if(!success)
		return;

	WebmStreaming->SetParseOneFrame(true);	

	if(!oMemoryMappedFile::Create( _pFileName, &MemoryMappedFrame))
		return;

	unsigned int sizeToMap = static_cast<unsigned int>(__min(MemoryMappedFrame->GetFileSize(),VIEW_SIZE));
	void* view = MemoryMappedFrame->Map(0, sizeToMap);
	WebmStreaming->PushByteStream(view, sizeToMap);
	MemoryMappedFrame->Unmap();
	FileIndex = WebmStreaming->GetDataConsumed();

	DataStartIndex = WebmStreaming->GetLastFrameIndex();

	*_pSuccess = true;
}

oWebmFile::~oWebmFile()
{

}

bool oWebmFile::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	if (_InterfaceID == oGetGUID<oVideoContainer>())
	{
		Reference();
		*_ppInterface = this;
		return true;
	}

	oSetLastError(ENOENT);
	return false;
}

bool oWebmFile::MapNOLOCK(MAPPED* _pMapped)
{
	if (FileIndex >= MemoryMappedFrame->GetFileSize())
	{
		Finished = true;
		return oVideoReturnEndOfFile(_pMapped);
	}

	if (!WebmStreaming->HasEncodedFrame())
	{
		unsigned int sizeToMap = static_cast<unsigned int>(__min(MemoryMappedFrame->GetFileSize()-FileIndex,VIEW_SIZE));
		void* view = MemoryMappedFrame->Map(FileIndex,sizeToMap);
		WebmStreaming->PushByteStream(view,sizeToMap);
		FileIndex += WebmStreaming->GetDataConsumed();
		MemoryMappedFrame->Unmap();
	}
	WebmStreaming->Map(_pMapped);
	return true;
}

void oWebmFile::UnmapNOLOCK()
{
	WebmStreaming->Unmap();
	if (FileIndex >= MemoryMappedFrame->GetFileSize() && !WebmStreaming->HasEncodedFrame()) // end of file
	{
		Finished = true;
	}
}