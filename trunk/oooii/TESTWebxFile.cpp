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