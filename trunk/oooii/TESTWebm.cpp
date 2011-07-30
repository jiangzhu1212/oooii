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

struct TESTWebm: public oTest
{

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		bool DevMode = false; // Enable this to run with a window
		bool GenerateWebmFile = false; //Enable to output a webm file to the location below.

		static const char* OutputVidName = "..\\..\\Data\\Test\\Videos\\test.webm";

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
		MovDesc.AllowDroppedFrames = false;
		oTESTB( oVideoFile::Create( sequence, MovDesc, &VideoFile ), oGetLastErrorDesc() );
		VideoFile->GetDesc(&MovDesc);

		oRef<oVideoDecodeCPU> Decoder;
		oTESTB( oVideoDecodeCPU::Create( VideoFile, &Decoder), "Failed to create decoder" );

		std::vector<unsigned char> LuminancePlane;
		std::vector<unsigned char> UChromQuarterPlane;
		std::vector<unsigned char> VChromQuarterPlane;

		unsigned int FullPixels = MovDesc.Dimensions.x*MovDesc.Dimensions.y;
		LuminancePlane.resize( FullPixels );
		UChromQuarterPlane.resize( FullPixels / 4 );
		VChromQuarterPlane.resize( FullPixels / 4 );

		oSurface::YUV420 EncoderFrame;
		EncoderFrame.pY = &LuminancePlane[0];
		EncoderFrame.YPitch = MovDesc.Dimensions.x;

		EncoderFrame.pU = &UChromQuarterPlane[0];
		EncoderFrame.pV = &VChromQuarterPlane[0];
		EncoderFrame.UVPitch = MovDesc.Dimensions.x / 2;

		unsigned char* pFrame = new unsigned char[MovDesc.Dimensions.x*MovDesc.Dimensions.y*sizeof(unsigned int)];

		oRef<oVideoEncodeCPU> WebmEncoder;
		oVideoEncodeCPU::DESC WebmDesc;
		WebmDesc.ContainerType = oVideoEncodeCPU::WEBM_CONTAINER;
		WebmDesc.CodecType = oVideoEncodeCPU::VP8_CODEC;
		WebmDesc.Quality = oVideoEncodeCPU::REALTIME;
		WebmDesc.Dimensions = MovDesc.Dimensions;
		WebmDesc.FrameTimeNumerator = 1001; //this should get pulled from the mov decoder, but it doesnt give this info yet.
		WebmDesc.FrameTimeDenominator = 24000;
		WebmDesc.BitRate = 4000;
		oTESTB( oVideoEncodeCPU::Create( WebmDesc, &WebmEncoder ), oGetLastErrorDesc() );
		unsigned char PacketStream[1024 * 256];

		size_t packetSz = 0;
		WebmEncoder->GetHeader(PacketStream,oCOUNTOF(PacketStream),&packetSz);
		if(GenerateWebmFile)
			fwrite(PacketStream,1,packetSz,File);

		oRef<oVideoStream> StreamDecoder;
		oVideoStream::DESC streamDesc;
		streamDesc.ContainerType = oVideoStream::WEBM_CONTAINER;
		streamDesc.AllowDroppedFrames = false;
		oTESTB( oVideoStream::Create( streamDesc, &StreamDecoder ), oGetLastErrorDesc() );

		StreamDecoder->PushByteStream(PacketStream,packetSz);

		oRef<oVideoDecodeCPU> VP8Decoder;
		oTESTB( oVideoDecodeCPU::Create( StreamDecoder, &VP8Decoder ), oGetLastErrorDesc() );


		// Create a test window to visualize things
		oRef<oWindow> Window;
		oRef<threadsafe oWindow::Picture> Picture;
		
		if(DevMode)
		{
			oWindow::DESC desc;
			desc.ClientX = oWindow::DEFAULT;
			desc.ClientY = oWindow::DEFAULT;
			desc.ClientWidth = MovDesc.Dimensions.x;
			desc.ClientHeight = MovDesc.Dimensions.y;
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
			picDesc.SurfaceDesc.Width = MovDesc.Dimensions.x;
			picDesc.SurfaceDesc.Height = MovDesc.Dimensions.y;
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

		size_t RGBFramestride = MovDesc.Dimensions.x * 4;
		int FrameCount = 0;
		size_t DecodedFrameCount = 0;
		while(!VideoFile->HasFinished())
		{
			oTESTB(Decoder->Decode(&EncoderFrame, &DecodedFrameCount), "Failed to decode a frame" );
			oTESTB(DecodedFrameCount == (size_t)FrameCount, "decoded frame count was not the expected value");

			bool forceIFrame = false;
			if(!(FrameCount%48)) //force every 48th frame to be an iframe.
				forceIFrame = true;
			WebmEncoder->Encode(EncoderFrame,PacketStream,oCOUNTOF( PacketStream ), &packetSz,forceIFrame);
			if(GenerateWebmFile)
				fwrite(PacketStream,1,packetSz,File);

			StreamDecoder->PushByteStream(PacketStream,packetSz);

			oSurface::YUV420 DecoderFrame;
			VP8Decoder->Decode( &DecoderFrame, &DecodedFrameCount );
			oTESTB(DecodedFrameCount == (size_t)FrameCount, "decoded frame count was not the expected value");

			oSurface::convert_YUV420_to_B8G8R8A8_UNORM( MovDesc.Dimensions.x, MovDesc.Dimensions.y, DecoderFrame, pFrame, RGBFramestride );

			if( ++FrameCount == 10 )
			{
				oRef<oImage> snapshot;
				oImage::DESC ImageDesc;
				ImageDesc.Format = oSurface::R8G8B8A8_UNORM;
				ImageDesc.Height = MovDesc.Dimensions.y;
				ImageDesc.Width = MovDesc.Dimensions.x;
				ImageDesc.Pitch = oSurface::CalcRowPitch(oSurface::R8G8B8A8_UNORM, MovDesc.Dimensions.x );
				ImageDesc.Size = oSurface::CalcLevelPitch(oSurface::R8G8B8A8_UNORM, MovDesc.Dimensions.x, MovDesc.Dimensions.y);

				oTESTB(oImage::Create(&ImageDesc, &snapshot), "Failed to create snapshot");
				memcpy(snapshot->Map(), pFrame, RGBFramestride * MovDesc.Dimensions.y);
				snapshot->Unmap();
				snapshot->FlipVertical();

				oTESTB( TestImage(snapshot, FrameCount), "Image compare frame %i failed", FrameCount );
			}

			if(DevMode)
			{
				Picture->Copy( &pFrame[0], MovDesc.Dimensions.x * sizeof( int ) );
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

oTEST_REGISTER(TESTWebm);