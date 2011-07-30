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
#include <iostream>
#include "oVideoCache.h"

static const char* sTITLE = "OOOii Video Converter";

static const oOption sCmdLineOptions[] = 
{
	{ "input", 'i', "input", "specify the input file." },
	{ "output", 'o', "output", "specify the output file. If omitted, the input file name and path will be used, with a different extension" },
	{ "quality", 'q', "quality", "controls how long it will take to convert the file, and the quality of the image. set to speed/mixed/quality. defaults to quality." },
	{ "bitrate", 'b', "bitrate", "bitrate in kb/s. Controls how large the file is, and therefore the quality of the image. if missing a conservative bit rate will be used." },
	{ "preview", 'p', "preview", "Display a small version of the compressed video in a preview window as it is created. true/false. defaults to false" },
	{ "twopass", 't', "twopass", "Encode using two passes for higher quality, but longer compression time. true/false. Defaults to true" },
	{ "splitCount", 's', "splitCount", "Splits the encoded file into multiple files. Specify the number of output files you want Defaults to 1" },
	{ "splitDirection", 'd', "splitDirection", "What direction to split the file in. h or v Defaults to v" },
	{ "firstFrame", 'f', "firstFrame", "skip any frame before this one." },
	{ "lastFrame", 'l', "lastFrame", "skip any frame after this one." },
	{ "FrameRate", 'r', "FrameRate", "Must be used if converting from an image sequence. Uses shorthand, 24 means 24000/1001 60 means 60000/1001 " },
	{ 0,0,0,0 }
};

struct PARAMETERS
{
	PARAMETERS() : InputFile(NULL), EncodeQuality(oVideoEncodeCPU::BEST_QUALITY),
		BitRate(-1), ShowPreview(false), TwoPass(true), SplitCount(1), SplitVerticly(true),
		FirstFrame(-1), LastFrame(-1), FrameRate(-1) { OutputFile[0] = 0;}
	const char* InputFile;
	char OutputFile[_MAX_PATH];
	oVideoEncodeCPU::QUALITY EncodeQuality;
	int BitRate;
	bool ShowPreview;
	bool TwoPass;
	unsigned int SplitCount;
	bool SplitVerticly;
	int FirstFrame;
	int LastFrame;
	int FrameRate;
};

void ParseCommandLine(int _Argc, const char* _Argv[], PARAMETERS* _pParameters)
{
	const char* value = 0;
	char ch = oOptTok(&value, _Argc, _Argv, sCmdLineOptions);
	while (ch)
	{
		switch (ch)
		{
		case 'i':
			_pParameters->InputFile = value;
			break;

		case 'o':
			strcpy_s(_pParameters->OutputFile,_MAX_PATH,value);
			break;

		case 'q':
			if(strncmp(value,"speed",5) == 0)
				_pParameters->EncodeQuality = oVideoEncodeCPU::REALTIME;
			else if(strncmp(value,"mixed",5) == 0)
				_pParameters->EncodeQuality = oVideoEncodeCPU::GOOD_QUALITY;
			break;

		case 'b':
			_pParameters->BitRate = clamp(atoi(value),128,512000); //arbitrary, just keep it reasonable.
			break;

		case 'p':
			if(strncmp(value,"t",1) == 0)
				_pParameters->ShowPreview = true;
			break;

		case 't':
			if(strncmp(value,"f",1) == 0)
				_pParameters->TwoPass = false;
			break;

		case 's':
			_pParameters->SplitCount = clamp(atoi(value),1,64); //arbitrary, just keep it reasonable.
			break;

		case 'd':
			if(strncmp(value,"h",1) == 0)
				_pParameters->SplitVerticly = false;
			break;

		case 'f':
			_pParameters->FirstFrame = atoi(value);
			break;

		case 'l':
			_pParameters->LastFrame = atoi(value);
			break;

		case 'r':
			_pParameters->FrameRate = clamp(atoi(value),24,120); //arbitrary, just keep it reasonable.
			break;

		default:
			break;
		}

		ch = oOptTok(&value, 0, 0, 0);
	}
}

void EncodeFirstPass(size_t _index,std::vector<oRef<oVideoEncodeCPU>> &_WebmEncoders,std::vector<oSurface::YUV420> &_EncoderFrame, bool _forecIFrame)
{
	_WebmEncoders[_index]->EncodeFirstPass(_EncoderFrame[_index],_forecIFrame);
}

void EncodeSecondPass(size_t _index,std::vector<oRef<oVideoEncodeCPU>> &_WebmEncoders,std::vector<oSurface::YUV420> &_EncoderFrame, bool _forecIFrame,
					  unsigned int _PacketStreamSize, std::vector<FILE*> &_OutputFiles, std::vector<oRef<oVideoStream>> &_decoders)
{
	size_t packetSz = 0;
	unsigned char *PacketStream = new unsigned char[_PacketStreamSize];

	_WebmEncoders[_index]->Encode(_EncoderFrame[_index],PacketStream,_PacketStreamSize, &packetSz,_forecIFrame);
	fwrite(PacketStream,1,packetSz,_OutputFiles[_index]);

	if(_decoders.size())
		_decoders[_index]->PushByteStream(PacketStream,packetSz);

	delete[] PacketStream;
}

// @oooii-Eric: TODO: This was fine when it was 100 lines of a code, its getting a little long, please refactor me.
int main(int argc, const char* argv[])
{
	double startTime = oTimer();

	oConsole::SetTitle(sTITLE);

	PARAMETERS parameters;
	ParseCommandLine(argc, argv, &parameters);

	if(!parameters.InputFile)
	{
		std::cout << "oVideoConverter requires an input file" << std::endl;
		char buf[2048];
		printf("%s", oOptDoc(buf, oGetFilebase(argv[0]), sCmdLineOptions));
		return 1;
	}

	oRef<oVideoFile> VideoFile;
	oVideoContainer::DESC MovDesc;
	MovDesc.StartingFrame = parameters.FirstFrame;
	MovDesc.EndingFrame = parameters.LastFrame;
	oRef<oVideoDecodeCPU> Decoder;
	if(!oVideoFile::Create( parameters.InputFile, MovDesc, &VideoFile ))
	{
		std::cout << oGetLastErrorDesc() << std::endl;
		std::cout << "couldn't create input file decoder" << std::endl;
		return 1;
	}
	VideoFile->GetDesc(&MovDesc);
	if(!oVideoDecodeCPU::Create( VideoFile, &Decoder))
	{
		std::cout << oGetLastErrorDesc() << std::endl;
		std::cout << "couldn't create input file decoder" << std::endl;
		return 1;
	}
	
	if(!parameters.OutputFile[0])
	{
		strcpy_s(parameters.OutputFile,_MAX_PATH,parameters.InputFile);
		oReplaceFileExtension(parameters.OutputFile,".webx");
	}

	if(strcmp(parameters.InputFile,parameters.OutputFile) == 0)
	{
		std::cout << "input and output files must be different." << std::endl;
		char buf[2048];
		printf("%s", oOptDoc(buf, oGetFilebase(argv[0]), sCmdLineOptions));
		return 1;
	}

	std::vector<char*> OutputFiles;
	OutputFiles.resize(parameters.SplitCount);
	for(unsigned int i = 0;i < parameters.SplitCount;++i)
	{
		OutputFiles[i] = (char*)oSTACK_ALLOC(_MAX_PATH,1);
	}
	if(parameters.SplitCount > 1)
	{
		for(unsigned int i = 0;i < parameters.SplitCount;++i)
		{
			strcpy_s(OutputFiles[i],_MAX_PATH,parameters.OutputFile);
			char ext[_MAX_PATH];
			sprintf_s(ext,_MAX_PATH,"%d.webm",i+1);
			oReplaceFileExtension(OutputFiles[i],_MAX_PATH,ext);
		}
	}
	else
		strcpy_s(OutputFiles[0],_MAX_PATH,parameters.OutputFile);

	if(parameters.BitRate != -1)
		parameters.BitRate /= parameters.SplitCount;

	std::vector<unsigned char> LuminancePlane;
	std::vector<unsigned char> UChromQuarterPlane;
	std::vector<unsigned char> VChromQuarterPlane;

	std::vector<unsigned char> PaddedLuminancePlane;
	std::vector<unsigned char> PaddedUChromQuarterPlane;
	std::vector<unsigned char> PaddedVChromQuarterPlane;

	int2 PaddedDimensions = MovDesc.Dimensions;

	if(parameters.SplitVerticly)
	{
		if(PaddedDimensions.y%parameters.SplitCount != 0 || ((PaddedDimensions.y/parameters.SplitCount)&1) != 0)
			PaddedDimensions.y = (((PaddedDimensions.y/parameters.SplitCount)&~1) + 2)*parameters.SplitCount;
	}
	else
	{
		if(PaddedDimensions.x%parameters.SplitCount != 0 || ((PaddedDimensions.x/parameters.SplitCount)&1) != 0)
			PaddedDimensions.x = (((PaddedDimensions.x/parameters.SplitCount)&~1) + 2)*parameters.SplitCount;
	}

	unsigned int FullPixels = PaddedDimensions.x*PaddedDimensions.y;
	unsigned int YPartition = 0;
	unsigned int YOffset = 0;
	unsigned int UVPartition = 0;
	unsigned int UVOffset = 0;
	if(parameters.SplitVerticly)
	{
		YPartition = FullPixels/parameters.SplitCount;
		UVPartition = YPartition/4;
	}
	else
	{
		YOffset = PaddedDimensions.x/parameters.SplitCount;
		UVOffset = PaddedDimensions.x/(2*parameters.SplitCount);
	}

	LuminancePlane.resize( FullPixels );
	UChromQuarterPlane.resize( FullPixels / 4 );
	VChromQuarterPlane.resize( FullPixels / 4 );
	PaddedLuminancePlane.resize( FullPixels );
	PaddedUChromQuarterPlane.resize( FullPixels / 4 );
	PaddedVChromQuarterPlane.resize( FullPixels / 4 );

	memset(&PaddedLuminancePlane[0], 16, PaddedLuminancePlane.size());
	memset(&PaddedUChromQuarterPlane[0], 128, PaddedUChromQuarterPlane.size());
	memset(&PaddedVChromQuarterPlane[0], 128, PaddedVChromQuarterPlane.size());

	oSurface::YUV420 EncoderFrame;
	EncoderFrame.pY = &LuminancePlane[0];
	EncoderFrame.YPitch = PaddedDimensions.x;

	EncoderFrame.pU = &UChromQuarterPlane[0];
	EncoderFrame.pV = &VChromQuarterPlane[0];
	EncoderFrame.UVPitch = PaddedDimensions.x / 2;

	std::vector<oRef<oVideoEncodeCPU>> WebmEncoders;
	WebmEncoders.resize(parameters.SplitCount);
	oVideoEncodeCPU::DESC WebmDesc;
	
	unsigned int SplitWidth;
	unsigned int SplitHeight;

	if(parameters.SplitVerticly)
	{
		SplitWidth =  PaddedDimensions.x;
		SplitHeight = PaddedDimensions.y/parameters.SplitCount;
	}
	else
	{
		SplitWidth =  PaddedDimensions.x/parameters.SplitCount;
		SplitHeight = PaddedDimensions.y;
	}

	if(strncmp(oGetFileExtension(parameters.OutputFile),".webm",5) == 0 || strncmp(oGetFileExtension(parameters.OutputFile),".webx",5) == 0)
	{
		WebmDesc.ContainerType = oVideoEncodeCPU::WEBM_CONTAINER;
		WebmDesc.CodecType = oVideoEncodeCPU::VP8_CODEC;
		WebmDesc.Quality = parameters.EncodeQuality;
		WebmDesc.Dimensions.x =  SplitWidth;
		WebmDesc.Dimensions.y = SplitHeight;
		if(parameters.FrameRate == -1)
		{
			if(MovDesc.FrameTimeDenominator == 0 || MovDesc.FrameTimeNumerator == 0)
			{
				std::cout << "input movie doesn't have any frame rate information. You must specify this info on the commandline using -r" << std::endl;
				return 1;
			}
			WebmDesc.FrameTimeNumerator = MovDesc.FrameTimeNumerator; 
			WebmDesc.FrameTimeDenominator = MovDesc.FrameTimeDenominator;
		}
		else
		{
			WebmDesc.FrameTimeNumerator = 1001; 
			WebmDesc.FrameTimeDenominator = parameters.FrameRate * 1000;
		}
		WebmDesc.BitRate = parameters.BitRate;
		WebmDesc.TwoPass = parameters.TwoPass;
		for(unsigned int i = 0;i < parameters.SplitCount;++i)
		{
			if(!oVideoEncodeCPU::Create( WebmDesc, &(WebmEncoders[i]) ))
			{
				std::cout << oGetLastErrorDesc() << std::endl;
				std::cout << "couldn't create output file encoder" << std::endl;
				return 1;
			}
		}
	}
	else
	{
		std::cout << "oVideoConverter currently only outputs webm and webx files." << std::endl;
		char buf[2048];
		printf("%s", oOptDoc(buf, oGetFilebase(argv[0]), sCmdLineOptions));
		return 1;
	}

	std::vector<FILE*> OutputFileHandles;
	OutputFileHandles.resize(parameters.SplitCount);
	for(unsigned int i = 0;i < parameters.SplitCount;++i)
	{
		OutputFileHandles[i] = NULL;
		fopen_s(&OutputFileHandles[i],OutputFiles[i],"wb");
		if(!OutputFileHandles[i])
		{
			std::cout << "could not open output file for writing: " << parameters.OutputFile << std::endl;
			return 1;
		}
	}

	std::vector<oRef<oVideoStream>> StreamDecoders;
	if(parameters.ShowPreview)
	{
		StreamDecoders.resize(parameters.SplitCount);
		oVideoStream::DESC streamDesc;
		streamDesc.ContainerType = oVideoStream::WEBM_CONTAINER;
		for(unsigned int i = 0;i < parameters.SplitCount;++i)
		{
			if(!oVideoStream::Create( streamDesc, &(StreamDecoders[i]) ))
			{
				std::cout << oGetLastErrorDesc() << std::endl;
				std::cout << "failed to create preview decoder" << std::endl;
				parameters.ShowPreview = false;
			}
		}
	}

	unsigned int PacketStreamSize = PaddedDimensions.x*PaddedDimensions.y*4;
	unsigned char *PacketStream = new unsigned char[PacketStreamSize];

	size_t packetSz = 0;
	for(unsigned int i = 0;i < parameters.SplitCount;++i)
	{
		WebmEncoders[i]->GetHeader(PacketStream,PacketStreamSize,&packetSz);
		fwrite(PacketStream,1,packetSz,OutputFileHandles[i]);
		if(parameters.ShowPreview)
			StreamDecoders[i]->PushByteStream(PacketStream,packetSz);
	}

	std::vector<oRef<oVideoDecodeCPU>> VP8Decoders;
	if(parameters.ShowPreview)
	{
		VP8Decoders.resize(parameters.SplitCount);
		for(unsigned int i = 0;i < parameters.SplitCount;++i)
		{
			if(!oVideoDecodeCPU::Create( StreamDecoders[i], &(VP8Decoders[i]) ))
			{
				std::cout << oGetLastErrorDesc() << std::endl;
				std::cout << "failed to create preview decoder" << std::endl;
				parameters.ShowPreview = false;
			}
		}
	}

	// Create a test window to visualize things
	oRef<oWindow> Window;
	oRef<threadsafe oWindow::Picture> Picture;

	if(parameters.ShowPreview)
	{
		oWindow::DESC desc;
		desc.ClientX = oWindow::DEFAULT;
		desc.ClientY = oWindow::DEFAULT;
		desc.ClientWidth = MovDesc.Dimensions.x/4; //preview will show a smaller window than the video itself
		desc.ClientHeight = MovDesc.Dimensions.y/4;
		desc.State = oWindow::RESTORED;
		desc.Style = oWindow::FIXED;
		desc.UseAntialiasing = false;
		desc.Enabled = true; // control is handled by this test and a timeout
		desc.HasFocus = true;
		desc.AlwaysOnTop = false;
		desc.EnableCloseButton = true;
		if(!oWindow::Create(&desc, NULL,  "OOOii oWindow", 0, &Window))
		{
			std::cout << "failed to create preview window" << std::endl;
			parameters.ShowPreview = false;
		}

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

		if(!Window->CreatePicture( &picDesc, &Picture ))
		{
			std::cout << oGetLastErrorDesc() << std::endl;
			std::cout << "failed to create preview window" << std::endl;
			parameters.ShowPreview = false;
		}
	}

	int FrameCount = 0;

	std::vector<oSurface::YUV420> EncoderFrames;
	EncoderFrames.resize(parameters.SplitCount);

	oVideoCache FrameCache("frameCache.bin", PaddedDimensions.y);

	oSurface::YUV420 PaddedFrame;
	PaddedFrame.YPitch = PaddedDimensions.x;
	PaddedFrame.UVPitch = PaddedDimensions.x / 2;
	PaddedFrame.pY = &PaddedLuminancePlane[0];
	PaddedFrame.pU = &PaddedUChromQuarterPlane[0];
	PaddedFrame.pV = &PaddedVChromQuarterPlane[0];

	if(parameters.TwoPass)
	{
		while(!VideoFile->HasFinished())
		{
			if(!Decoder->Decode(&EncoderFrame))
			{
				std::cout << oGetLastErrorDesc() << std::endl;
				std::cout << "Failed to decode a frame. input file is probably corrupt" << std::endl;
				return 1;
			}

			oMemcpy2d( &PaddedLuminancePlane[0], PaddedDimensions.x, EncoderFrame.pY, EncoderFrame.YPitch, MovDesc.Dimensions.x, MovDesc.Dimensions.y );
			oMemcpy2d( &PaddedUChromQuarterPlane[0], PaddedDimensions.x / 2, EncoderFrame.pU, EncoderFrame.UVPitch, MovDesc.Dimensions.x / 2, MovDesc.Dimensions.y / 2 );
			oMemcpy2d( &PaddedVChromQuarterPlane[0], PaddedDimensions.x / 2, EncoderFrame.pV, EncoderFrame.UVPitch, MovDesc.Dimensions.x / 2, MovDesc.Dimensions.y / 2 );
			
			FrameCache.CacheFrame(PaddedFrame);

			//Note that for now a decoder may or may not provide its own data.
			for (unsigned int i = 0;i < parameters.SplitCount;++i)
			{
				EncoderFrames[i] = PaddedFrame;
				EncoderFrames[i].pY += YPartition*i+YOffset*i;
				EncoderFrames[i].pU += UVPartition*i+UVOffset*i;
				EncoderFrames[i].pV += UVPartition*i+UVOffset*i;
			}

			bool forceIFrame = false;
			if(!(FrameCount%256)) //force every 256th frame to be an iframe. arbitrary number
				forceIFrame = true;

			oParallelFor(oBIND(EncodeFirstPass,oBIND1,WebmEncoders,EncoderFrames,forceIFrame),0,parameters.SplitCount);
			++FrameCount;
			std::cout << "Pass 1: frame " << FrameCount << std::endl;
		}

		for (unsigned int i = 0;i < parameters.SplitCount;++i)
			WebmEncoders[i]->StartSecondPass();
		VideoFile->Restart();
		FrameCache.StartSecondPass();

		EncoderFrame.pY = &LuminancePlane[0];
		EncoderFrame.YPitch = PaddedDimensions.x;
		EncoderFrame.pU = &UChromQuarterPlane[0];
		EncoderFrame.pV = &VChromQuarterPlane[0];
		EncoderFrame.UVPitch = PaddedDimensions.x / 2;

		FrameCount = 0;
	}

	size_t RGBFramestride = MovDesc.Dimensions.x * 4;

	unsigned char* pFrame = new unsigned char[PaddedDimensions.x*PaddedDimensions.y*sizeof(unsigned int)];
	
	while(!VideoFile->HasFinished() && (!parameters.TwoPass || !FrameCache.HasFinished()))
	{
		if(parameters.TwoPass)
		{
			FrameCache.ReadBackFrame(PaddedFrame);
		}
		else
		{
			if(!Decoder->Decode(&EncoderFrame))
			{
				std::cout << oGetLastErrorDesc() << std::endl;
				std::cout << "Failed to decode a frame. input file is probably corrupt" << std::endl;
				return 1;
			}

			oMemcpy2d( &PaddedLuminancePlane[0], PaddedDimensions.x, EncoderFrame.pY, EncoderFrame.YPitch, MovDesc.Dimensions.x, MovDesc.Dimensions.y );
			oMemcpy2d( &PaddedUChromQuarterPlane[0], PaddedDimensions.x / 2, EncoderFrame.pU, EncoderFrame.UVPitch, MovDesc.Dimensions.x / 2, MovDesc.Dimensions.y / 2 );
			oMemcpy2d( &PaddedVChromQuarterPlane[0], PaddedDimensions.x / 2, EncoderFrame.pV, EncoderFrame.UVPitch, MovDesc.Dimensions.x / 2, MovDesc.Dimensions.y / 2 );
		}

		//Note that for now a decoder may or may not provide its own data.
		for (unsigned int i = 0;i < parameters.SplitCount;++i)
		{
			EncoderFrames[i] = PaddedFrame;
			EncoderFrames[i].pY += YPartition*i+YOffset*i;
			EncoderFrames[i].pU += UVPartition*i+UVOffset*i;
			EncoderFrames[i].pV += UVPartition*i+UVOffset*i;
		}

		bool forceIFrame = false;
		if(!(FrameCount%256)) //force every 256th frame to be an iframe. arbitrary number
			forceIFrame = true;
		oParallelFor(oBIND(EncodeSecondPass, oBIND1, WebmEncoders, EncoderFrames, forceIFrame, PacketStreamSize, OutputFileHandles, StreamDecoders), 0, parameters.SplitCount);

		if(parameters.ShowPreview)
		{
			for (unsigned int i = 0;i < parameters.SplitCount;++i)
			{
				oSurface::YUV420 DecoderFrame;
				VP8Decoders[i]->Decode( &DecoderFrame );

				unsigned char* pPartition = pFrame;
				if(parameters.SplitVerticly)
					pPartition += PaddedDimensions.x*SplitHeight*i*4;
				else
					pPartition += SplitWidth*i*4;
				oSurface::convert_YUV420_to_B8G8R8A8_UNORM( SplitWidth, SplitHeight, DecoderFrame, pPartition, RGBFramestride );
			}

			Picture->Copy( &pFrame[0], PaddedDimensions.x * sizeof( int ) );
			Window->Begin();
			Window->End();
		}

		++FrameCount;
		std::cout << "Pass 2: frame " << FrameCount << std::endl;
	}
	delete [] pFrame;
	delete [] PacketStream;

	for (unsigned int i = 0;i < parameters.SplitCount;++i)
		fclose(OutputFileHandles[i]);

	if(strncmp(oGetFileExtension(parameters.OutputFile),".webx",5) == 0)
	{
		oVideo::CreateWebXFile(((const char**)&OutputFiles[0]), oSize32(OutputFiles.size()), parameters.OutputFile, parameters.SplitVerticly);
	}

	double endTime = oTimer();

	unsigned int hours = static_cast<unsigned int>((endTime - startTime)/(60*60));
	unsigned int minutes = static_cast<unsigned int>((endTime - startTime)/(60)) - (hours*60);
	unsigned int seconds = static_cast<unsigned int>((endTime - startTime)) - (minutes*60) - (hours*60*60);
	printf("Total time to process : %d:%d:%d\n", hours, minutes, seconds);

	return 0;
}