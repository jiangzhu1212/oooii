// $(header)
#include <oooii/oTest.h>
#include <oooii/oooii.h>
#include <oVideo/oVideoCodec.h>

struct TESTWebxFile: public oTest
{

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		static const char* VidName = "..\\..\\Data\\Test\\Videos\\TestVideo_OOOii.webx";

		char sequence[_MAX_PATH];
		oTESTB( oFindInSysPath( sequence, _MAX_PATH, oSYSPATH_APP, VidName, NULL, oFile::Exists ), "Failed to find %s", VidName );

		std::vector<oRef<oVideoFile>> VideoFiles;
		oVideoContainer::DESC MovDesc;
		MovDesc.AllowDroppedFrames = false;
		oVideo::CreateWebXContainers(sequence, MovDesc, [&](oVideoFile* _file) {VideoFiles.push_back(oRef<oVideoFile>(_file, false));}, nullptr);
		oTESTB(VideoFiles.size(), "webx file %s didn't contain any video streams", sequence);
		VideoFiles[0]->GetDesc(&MovDesc);
		
		oWindow::DESC VDDesc;
		VDDesc.ClientWidth = MovDesc.Dimensions.x;
		VDDesc.ClientHeight = static_cast<int>(MovDesc.Dimensions.y * VideoFiles.size());
		VDDesc.HasFocus = false;
		VDDesc.MSSleepWhenNoFocus = 0;

		oRef<oWindow> Window;
		oWindow::Create( &VDDesc, NULL, "TestVideoPlayer", 0, &Window );

		//be sure to flush any paints before creating the video. each paint decodes a frame. don't want to throw off the test image
		Window->Begin();
		Window->End(true, true);

		oRef<threadsafe oWindow::Video> Video;
		{
			oWindow::Video::DESC desc;
			std::vector<oVideoContainer*> containers;
			containers.resize(VideoFiles.size());
			std::copy(VideoFiles.begin(), VideoFiles.end(), containers.begin());
			Window->CreateVideo(&desc, &containers[0], containers.size(), &Video );
		}

		int Frame = 0;
		while( !VideoFiles[0]->HasFinished() )
		{
			if( 10 == ++Frame )
			{
				oRef<oImage> snapshot;
				oTESTB(Window->CreateSnapshot(&snapshot), "Failed to create snapshot");
				oTESTB( TestImage(snapshot, Frame), "Image compare frame %i failed", Frame );
			}
			Window->Begin();
			Window->End(true, true);
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTWebxFile);