// $(header)
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