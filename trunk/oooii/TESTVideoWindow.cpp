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
#include <oooii/oWindow.h>

struct TESTVideoWindow : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		static const char* VidName = "..\\..\\Data\\Test\\Videos\\TestVideo_OOOii.mov";
		char sequence[_MAX_PATH];
		oTESTB( oFindInSysPath( sequence, _MAX_PATH, oSYSPATH_APP, VidName, NULL, oFile::Exists ), "Failed to find %s", VidName );

		oRef<oVideoFile> VideoFile;
		oTESTB( oVideoFile::Create( sequence, oVideoContainer::DESC(), &VideoFile), oGetLastErrorDesc() );

		oVideoFile::DESC Desc;
		VideoFile->GetDesc( &Desc );
		
		oWindow::DESC VDDesc;
		VDDesc.ClientWidth = Desc.Width;
		VDDesc.ClientHeight =  Desc.Height;
		VDDesc.HasFocus = false;
		VDDesc.MSSleepWhenNoFocus = 0;

		oRef<oWindow> Window;
		oWindow::Create( &VDDesc, NULL, "TestVideoPlayer", 0, &Window );
		
		oRef<threadsafe oWindow::Font> Font;
		{
			oWindow::Font::DESC desc;
			strcpy_s(desc.FontName, "Comic Sans");
			desc.Style = oWindow::Font::ITALIC;
			desc.PointSize = 24.0f;
			desc.ShadowOffset = 1.0f;
			oTESTB(Window->CreateFont(&desc, &Font), "Failed to create font");
		}

		oRef<threadsafe oWindow::Text> Text;
		{
			oWindow::Text::DESC desc;
			desc.X = oWindow::DEFAULT;
			desc.Y = oWindow::DEFAULT;
			desc.Width = 320;
			desc.Height = 240;
			desc.Anchor = oWindow::MIDDLE_CENTER;
			desc.Alignment = oWindow::MIDDLE_CENTER;
			desc.Color = std::OOOiiGreen;
			desc.ShadowColor = std::Black;
			strcpy_s(desc.String, "OOOii!");
			desc.MultiLine = true;
			oTESTB(Window->CreateText(&desc, Font, &Text), "Failed to create text");
		}

		//be sure to flush any paints before creating the video. each paint decodes a frame. don't want to throw off the test image
		Window->Begin();
		Window->End(true, true);

		oRef<threadsafe oWindow::Video> Video;
		{
			oWindow::Video::DESC desc;
			std::vector<threadsafe oVideoContainer*> containers;
			containers.push_back(VideoFile);
			Window->CreateVideo(&desc, containers, &Video );
		}

		int Frame = 0;
		while( !VideoFile->HasFinished() )
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

 oTEST_REGISTER(TESTVideoWindow);