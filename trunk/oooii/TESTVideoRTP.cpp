// $(header)
#include <oooii/oTest.h>
#include <oooii/oooii.h>
#include <oVideo/oVideoRTPStream.h>

struct TESTVideoRTP: public oTest
{

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		bool DevMode = false; // Enable this to run with a window

		static const char* VidName = "..\\..\\Data\\Test\\Videos\\TestVideo_OOOii.webm";

		char sequence[_MAX_PATH];
		oTESTB( oFindInSysPath( sequence, _MAX_PATH, oSYSPATH_APP, VidName, NULL, oFile::Exists ), "Failed to find %s", VidName );

		oRef<oVideoFile> LoadedFile;
		oVideoContainer::DESC MovDesc;
		MovDesc.AllowDroppedFrames = false;
		oTESTB( oVideoFile::Create( sequence, MovDesc, &LoadedFile ), oGetLastErrorDesc() );
		LoadedFile->GetDesc(&MovDesc);

		oRef<oVideoRTP> VideoRTP;
		oVideoRTP::DESC RTPDesc;
		RTPDesc.VideoDesc = MovDesc;
		oTESTB( oVideoRTP::Create(RTPDesc, &VideoRTP), "Failed to create RTP with %s", oGetLastErrorDesc() );
		oVideoRTP::RTP_PACKET* pPacket = NULL;
		oTESTB( 0 == VideoRTP->DrainPacket(&pPacket), "Unknown packet drained");

		oRef<oVideoStream> VideoStream;
		oVideoContainer::DESC StreamDesc = MovDesc;
		StreamDesc.ContainerType = oVideoContainer::OOII_RTP_CONTAINER;
		oTESTB( oVideoStream::Create( StreamDesc, &VideoStream ), "Failed to create video stream" );

		oRef<oVideoDecodeCPU> Decoder;
		oTESTB( oVideoDecodeCPU::Create( VideoStream, &Decoder), "Failed to create decoder" );

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

		// Simulate packet loss
		size_t DropFrameOnFrame = 2;
		size_t DropPacketsOnFrame = 8;
		while(1)
		{
		
			int FramesPerIteration = 2;
			for( int i = 0; i < FramesPerIteration; ++i )
			{
				oVideoContainer::MAPPED Map;
				if( LoadedFile->Map(&Map) )
				{
					VideoRTP->PacketizeFrame(Map.pFrameData, oSize32(Map.DataSize) );

					oVideoRTP::RTP_PACKET* packet;

					std::vector<oRef<threadsafe oBuffer>> TempPackets;
					while( VideoRTP->DrainPacket(&packet))
					{
						if( (size_t)FrameCount == DropPacketsOnFrame && packet->Priority != packet->PRIORITY_HIGH )
							continue; 

						if( (size_t)FrameCount == DropFrameOnFrame && i == 0)
							continue;

						unsigned char* pTemp = new unsigned char[packet->Size];
						memcpy(pTemp, packet->pData, packet->Size);

						oRef<threadsafe oBuffer> tempPacket;
						oTESTB( oBuffer::Create("Temp packet", pTemp, packet->Size, oBuffer::Delete, &tempPacket), "Failed to create temp packet buffer" );
						TempPackets.push_back(tempPacket);
					}
					
					// Randomize the packets to simulate network unreliability
					std::random_shuffle(TempPackets.begin(), TempPackets.end() );

					oFOREACH(oRef<threadsafe oBuffer>& packet, TempPackets )
					{
						oLockedPointer<oBuffer> lockedPacket(packet);
						
						VideoStream->PushByteStream(lockedPacket->GetData(), lockedPacket->GetSize() );
					}

					LoadedFile->Unmap();
				}
			}

			if( LoadedFile->HasFinished())
				break;

			for( int i = 0; i < FramesPerIteration; ++i )
			{
				oTESTB(Decoder->Decode(&EncoderFrame, &DecodedFrameCount), "Failed to decode frame %i", FrameCount );
				oTESTB( DropFrameOnFrame != DecodedFrameCount, "Frame 2 is supposed to be artificially dropped it should not be decoded" );
				
				if( (DropFrameOnFrame + 1) == DecodedFrameCount )
				{
					oTESTB(2 == FrameCount, "Expected FrameCount to be %i", DropFrameOnFrame);
					FrameCount = (int)DecodedFrameCount;
					i = FramesPerIteration; // Early out of loop since we dropped a frame
				}
				else
					oTESTB(DecodedFrameCount == (size_t)FrameCount, "decoded frame count was not the expected value");

				oSurface::convert_YUV420_to_B8G8R8A8_UNORM( MovDesc.Dimensions.x, MovDesc.Dimensions.y, EncoderFrame, pFrame, RGBFramestride );

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

		}
		delete [] pFrame;

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTVideoRTP);