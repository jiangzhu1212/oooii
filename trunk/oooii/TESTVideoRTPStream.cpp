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
#include <oooii/oTest.h>
#include <oooii/oooii.h>
#include <oVideo/oVideoRTPStream.h>

struct TESTVideoRTPStream: public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		static const char* VidName = "..\\..\\Data\\Test\\Videos\\TestVideo_OOOii.webm";

		// Create a video to use in the test
		oRef<oVideoFile> LoadedFile;
		{
			char sequence[_MAX_PATH];
			oTESTB( oFindInSysPath( sequence, _MAX_PATH, oSYSPATH_APP, VidName, NULL, oFile::Exists ), "Failed to find %s", VidName );

			oVideoContainer::DESC MovDesc;
			MovDesc.AllowDroppedFrames = false;
			oTESTB( oVideoFile::Create( sequence, MovDesc, &LoadedFile ), oGetLastErrorDesc() );
		}

		oRef<oVideoRTPStream> Stream;
		
		oVideoRTPStream::DESC FailDesc;
		oFromString(&FailDesc.Address, "127.0.0.1:4");
		oTESTB( !oVideoRTPStream::Create( FailDesc, &Stream), "Create unexpectedly succeeded.  Only odd ports are legal ports." );

		oVideoRTPStream::DESC Desc;
		oFromString(&Desc.Address, "127.0.0.255:555"); // Broadcast only to localhost to prevent spamming the network.
		oTESTB( oVideoRTPStream::Create( Desc, &Stream), oGetLastErrorDesc() );

		oTESTB( !Stream->StreamFrame(), "No videos queued so shouldn't be able to stream" );
		oTESTB( oGetLastError() == ENFILE, "Wrong error code indicated");

		// obug_1643: try to read the header at this point and make sure the SSRC is 0

		// Test queuing video
		oTESTB( Stream->QueueVideo( LoadedFile, 2 ), oGetLastErrorDesc() );

		bool bContinue = true;

		// Stream all frames.
		while(bContinue)
		{
			bContinue = Stream->StreamFrame();
			oTESTB(bContinue || oGetLastError() == ENFILE, oGetLastErrorDesc());
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTVideoRTPStream);