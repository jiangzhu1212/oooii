// $(header)
#include <oooii/oTest.h>
#include <oooii/oooii.h>
#include <oooii/oVideoCodec.h>

struct TESTWebmRaw: public oTest
{

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		bool DevMode = false; // Enable this to run with a window
		bool GenerateWebmFile = false; //Enable to output a webm file to the location below.

		static const char* OutputVidName = "..\\..\\Data\\Test\\Videos\\testraw.webm";

		char FullOutputVidName[_MAX_PATH];
		oGetSysPath( FullOutputVidName, _MAX_PATH, oSYSPATH_APP);
		strcat_s(FullOutputVidName,_MAX_PATH,OutputVidName);

		//create mov file decoder to read test movie.
		FILE *File = NULL;
		if(GenerateWebmFile)
			fopen_s(&File,FullOutputVidName,"wb");

		static const char* VidName = "..\\..\\Data\\Test\\Videos\\TestVideo_OOOii.mov";

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

		unsigned int FullPixels = MovDesc.Width*MovDesc.Height;
		LuminancePlane.resize( FullPixels );
		UChromQuarterPlane.resize( FullPixels / 4 );
		VChromQuarterPlane.resize( FullPixels / 4 );

		oSurface::YUV420 EncoderFrame;
		EncoderFrame.pY = &LuminancePlane[0];
		EncoderFrame.YPitch = MovDesc.Width;

		EncoderFrame.pU = &UChromQuarterPlane[0];
		EncoderFrame.pV = &VChromQuarterPlane[0];
		EncoderFrame.UVPitch = MovDesc.Width / 2;

		unsigned char* pFrame = new unsigned char[MovDesc.Width*MovDesc.Height*sizeof(unsigned int)];

		oRef<oVideoEncodeCPU> WebmEncoder;
		oVideoEncodeCPU::DESC WebmDesc;
		WebmDesc.ContainerType = oVideoEncodeCPU::WEBM_CONTAINER;
		WebmDesc.CodecType = oVideoEncodeCPU::RAW_420;
		WebmDesc.Quality = oVideoEncodeCPU::REALTIME;
		WebmDesc.Width = MovDesc.Width;
		WebmDesc.Height = MovDesc.Height;
		WebmDesc.FrameTimeNumerator = 1001; //this should get pulled from the mov decoder, but it doesnt give this info yet.
		WebmDesc.FrameTimeDenominator = 24000;
		WebmDesc.BitRate = 4000;
		oTESTB( oVideoEncodeCPU::Create( WebmDesc, &WebmEncoder ), oGetLastErrorDesc() );
		std::vector<unsigned char> PacketStream;
		PacketStream.resize(oMB(16));

		size_t packetSz = 0;
		WebmEncoder->GetHeader(&PacketStream[0],PacketStream.size(),&packetSz);
		if(GenerateWebmFile)
			fwrite(&PacketStream[0],1,packetSz,File);

 		oRef<threadsafe oVideoStream> StreamDecoder;
 		oVideoStream::DESC streamDesc;
 		streamDesc.ContainerType = oVideoStream::WEBM_CONTAINER;
		streamDesc.AllowDroppedFrames = false;
 		oTESTB( oVideoStream::Create( streamDesc, &StreamDecoder ), oGetLastErrorDesc() );
 
 		StreamDecoder->PushByteStream(&PacketStream[0],packetSz);
 
 		oRef<threadsafe oVideoDecodeCPU> VP8Decoder;
 		oTESTB( oVideoDecodeCPU::Create( StreamDecoder, &VP8Decoder ), oGetLastErrorDesc() );


		// Create a test window to visualize things
		oRef<oWindow> Window;
		oRef<threadsafe oWindow::Picture> Picture;

		if(DevMode)
		{
			oWindow::DESC desc;
			desc.ClientX = oWindow::DEFAULT;
			desc.ClientY = oWindow::DEFAULT;
			desc.ClientWidth = MovDesc.Width;
			desc.ClientHeight = MovDesc.Height;
			desc.State = oWindow::RESTORED;
			desc.Style = oWindow::FIXED;
			desc.UseAntialiasing = false;
			desc.Enabled = true; // control is handled by this test and a timeout
			desc.HasFocus = true;
			desc.AlwaysOnTop = false;
			desc.EnableCloseButton = true;
			desc.MSSleepWhenNoFocus = 0;
			oTESTB(oWindow::Create(&desc, NULL,  "OOOii oWindow", 0, &Window), "Failed to create window");


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

		size_t RGBFramestride = MovDesc.Width * 4;
		int FrameCount = 0;
		size_t DecodedFrameCount = 0;
		while(!VideoFile->HasFinished())
		{
			oTESTB(Decoder->Decode(&EncoderFrame), "Failed to decode a frame" );

			bool forceIFrame = false;
			if(!(FrameCount%48)) //force every 48th frame to be an iframe.
				forceIFrame = true;
			WebmEncoder->Encode(EncoderFrame,&PacketStream[0], PacketStream.size(), &packetSz,forceIFrame);
			if(GenerateWebmFile)
				fwrite(&PacketStream[0],1,packetSz,File);

			StreamDecoder->PushByteStream(&PacketStream[0], packetSz);

			oSurface::YUV420 DecoderFrame;
			VP8Decoder->Decode( &DecoderFrame, &DecodedFrameCount );
			oTESTB(DecodedFrameCount == (size_t)FrameCount, "decoded frame count was not the expected value");

			oSurface::convert_YUV420_to_B8G8R8A8_UNORM( MovDesc.Width, MovDesc.Height, DecoderFrame, pFrame, RGBFramestride );

			if( ++FrameCount == 10 )
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

				oTESTB( TestImage(snapshot, FrameCount), "Image compare frame %i failed", FrameCount );
			}

			if(DevMode)
			{
				Picture->Copy( &pFrame[0], MovDesc.Width * sizeof( int ) );
				Window->Begin();
				Window->End();
			}
		}
		delete [] pFrame;

		if(GenerateWebmFile)
			fclose(File);

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTWebmRaw);