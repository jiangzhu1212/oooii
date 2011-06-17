// $(header)
#include <oooii/oTest.h>
#include <oooii/oooii.h>
#include <oooii/oVideoCodec.h>

struct TESTMOVRleFileDecode: public oTest
{

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		static const char* VidName = "..\\..\\Data\\Test\\Videos\\TestVideo_OOOii.mov";
		bool DevMode = false; // Enable this to run with a window

		char sequence[_MAX_PATH];
		oTESTB( oFindInSysPath( sequence, _MAX_PATH, oSYSPATH_APP, VidName, NULL, oFile::Exists ), "Failed to find %s", VidName );

		oRef<oVideoFile> VideoFile;
		oVideoContainer::DESC MovDesc;
		oTESTB( oVideoFile::Create( sequence, MovDesc, &VideoFile ), oGetLastErrorDesc() );
		VideoFile->GetDesc(&MovDesc);

		oRef<threadsafe oVideoDecodeCPU> Decoder;
		oTESTB( oVideoDecodeCPU::Create( VideoFile, &Decoder), "Failed to create decoder" );
		
		std::vector<unsigned char> LuminancePlane;
		std::vector<unsigned char> UChromQuarterPlane;
		std::vector<unsigned char> VChromQuarterPlane;

		unsigned int FullPixels = MovDesc.Width * MovDesc.Height;
		LuminancePlane.resize( FullPixels );
		UChromQuarterPlane.resize( FullPixels / 4 );
		VChromQuarterPlane.resize( FullPixels / 4 );

		oSurface::YUV420 EncoderFrame;
		EncoderFrame.pY = &LuminancePlane[0];
		EncoderFrame.YPitch = MovDesc.Width;

		EncoderFrame.pU = &UChromQuarterPlane[0];
		EncoderFrame.pV = &VChromQuarterPlane[0];
		EncoderFrame.UVPitch = MovDesc.Width / 2;

		oRef<oWindow> Window;
		oRef<threadsafe oWindow::Picture> Picture;

		if( DevMode )
		{
			oWindow::DESC desc;
			desc.ClientX = oWindow::DEFAULT;
			desc.ClientY = oWindow::DEFAULT;
			desc.ClientWidth = MovDesc.Width/3;
			desc.ClientHeight = MovDesc.Height/3;
			desc.State = oWindow::RESTORED;
			desc.Style = oWindow::FIXED;
			desc.UseAntialiasing = false;
			desc.Enabled = true; // control is handled by this test and a timeout
			desc.HasFocus = true;
			desc.AlwaysOnTop = false;
			desc.EnableCloseButton = true;
			oTESTB(oWindow::Create(&desc, NULL, "OOOii oWindow", 0, &Window), "Failed to create window");


			oWindow::Picture::DESC picDesc;
			picDesc.SurfaceDesc.Width = MovDesc.Width;
			picDesc.SurfaceDesc.Height = MovDesc.Height;
			picDesc.SurfaceDesc.Format = oSurface::B8G8R8A8_UNORM;
			picDesc.SurfaceDesc.RowPitch = oSurface::GetSize(picDesc.SurfaceDesc.Format) * picDesc.SurfaceDesc.Width;
			picDesc.SurfaceDesc.NumMips = 1;
			picDesc.SurfaceDesc.NumSlices = 1;
			picDesc.SurfaceDesc.DepthPitch = 1;
			picDesc.Anchor = oWindow::MIDDLE_CENTER;

			picDesc.X = 0;
			picDesc.Y = 0;
			picDesc.Width = oWindow::DEFAULT;
			picDesc.Height = oWindow::DEFAULT;

			oTESTB(Window->CreatePicture( &picDesc, &Picture ), "Failed to create window picture" );
		}

		unsigned char* pFrame = new unsigned char[FullPixels*sizeof(unsigned int)];
		size_t RGBFramestride = MovDesc.Width * sizeof(unsigned int);
		int Frame = 0;
		while(!VideoFile->HasFinished())
		{
			oTESTB(Decoder->Decode(&EncoderFrame), "Failed to decode a frame" );
			oSurface::convert_YUV420_to_B8G8R8A8_UNORM( MovDesc.Width, MovDesc.Height, EncoderFrame, pFrame, RGBFramestride );

			if( DevMode )
			{
				Picture->Copy( &pFrame[0], MovDesc.Width * sizeof( int ) );

				Window->Begin();
				Window->End();
			}

			if( ++Frame == 10 )
			{
				oRef<oImage> snapshot;
				oImage::DESC ImageDesc;
				ImageDesc.Format = oSurface::R8G8B8A8_UNORM;
				ImageDesc.Height = MovDesc.Height;
				ImageDesc.Width = MovDesc.Width;
				ImageDesc.Pitch = oSurface::CalcRowPitch(oSurface::R8G8B8A8_UNORM, MovDesc.Width );
				ImageDesc.Size = oSurface::CalcLevelPitch(oSurface::R8G8B8A8_UNORM, MovDesc.Width, MovDesc.Height);

				oTESTB(oImage::Create(&ImageDesc, &snapshot), "Failed to create snapshot");
				memcpy(snapshot->Map(), pFrame, RGBFramestride * MovDesc.Height);
				snapshot->Unmap();
				snapshot->FlipVertical();

				oTESTB( TestImage(snapshot, Frame), "Image compare frame %i failed", Frame );
			}

		}
		delete [] pFrame;


		return SUCCESS;
	}
};

oTEST_REGISTER(TESTMOVRleFileDecode);