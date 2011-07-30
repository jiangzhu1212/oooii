// $(header)
#pragma once
#ifndef oVideoCodec_h
#define oVideoCodec_h
 
#include <oooii/oInterface.h>
#include <oooii/oSurface.h>
#include <vector>
#include <oooii/oMathTypes.h>

interface oVideoContainer : oInterface
{
	// A video container is a file or stream that contains
	// frames of video and audio.  The container is separate
	// from the underlying codec.
	enum CONTAINER_TYPE
	{
		INVALID_CONTAINER = -1,
		MOV_CONTAINER,
		WEBM_CONTAINER,
		OOII_RTP_CONTAINER,
		IMAGE_SEQUENCE_CONTAINER,
		COUNT_CONTAINER
	};

	enum CODEC_TYPE
	{
		INVALID_CODEC = -1,
		VP8_CODEC,
		RLE_CODEC,
		RAW_420,
		OIMAGE, //Note that this codec type does not provide FrameTime info in the DESC. The value must be supplied instead.
		COUNT_CODEC
	};

	struct DESC
	{
		DESC()
			: ContainerType(INVALID_CONTAINER)
			, CodecType(INVALID_CODEC)
			, Dimensions(-1, -1)
			, FrameTimeNumerator(0)
			, FrameTimeDenominator(1)
			, AllowDroppedFrames(true)
			, StartingFrame(-1) 
			, EndingFrame(-1) 
		{}
		CONTAINER_TYPE ContainerType;
		CODEC_TYPE CodecType;
		int2 Dimensions;
		unsigned int FrameTimeNumerator; 
		unsigned int FrameTimeDenominator;
		bool AllowDroppedFrames;
		int StartingFrame; // Note that StartingFrame and Ending Frame are not supported by all codecs. 
		int EndingFrame; //In particular this feature doesn't work with the webm codecs currently.

		bool operator == (const DESC& _other) const
		{
			// Parameters that affect if a decoder can switch seamlessly between
			// containers are considered
			if( _other.ContainerType != ContainerType )
				return false;
			if( _other.CodecType != CodecType )
				return false;
			if( _other.Dimensions.x != Dimensions.x )
				return false;
			if( _other.Dimensions.y != Dimensions.y )
				return false;
			if( _other.FrameTimeNumerator != FrameTimeNumerator )
				return false;
			if( _other.FrameTimeDenominator != FrameTimeDenominator )
				return false;
			return true;
		}

		bool operator != (const DESC& _other) const
		{
			return !(_other == *this);
		}
	};

	struct MAPPED
	{
		void* pFrameData;
		size_t DataSize;
		size_t DecodedFrameNumber; // @oooii-tony: todo replace this+Valid with oIndex
	};

	// Maps the next frame of data.  If no data is available _szData is 0
	// if a failure occurs or no more data will ever arrive _pValid 
	// is set to false.
	// Will block if there a frame is already mapped.
	virtual bool Map(MAPPED* _pMapped) = 0;

	// Ensure Unmap is only called after a successful map.
	virtual void Unmap() = 0;

	// If the video file has completely played through 
	// (i.e. no more frames) HasFinished is True
	virtual bool HasFinished() const = 0;

	virtual void GetDesc(DESC* _pDesc) const = 0;
};

interface oVideoFile : oVideoContainer
{
	// A video that is complete and saved to disk.  These types
	// of containers often have to access random parts of the
	// file so they behave differently than streams.

	virtual void Restart() = 0;
	
	// The description used by create can be left empty or partially empty 
	// in which create will attempt to infer the data from the file itself.
	oAPI static bool Create(const char* _pFileName, const oVideoContainer::DESC& _Desc, oVideoFile** _pVideoFile);
};

interface oVideoStream : oVideoContainer
{
	// A video stream is never complete in that data can continue
	// to be sent to it.  The method of data transportation
	// (i.e. TCP IP/UDP) is separated from the actual data consumption.
	// Therefore the user needs to call PushByteStream to add video data.
	// A streamer is always consuming data as fast as it can (i.e. it ignores framerate)
	// if the user wants to artificially control framerate they should slow calls to PushByteStream.

	//Streams never finish
	bool HasFinished() const override {return false;}

	virtual void PushByteStream(const void* _pData, size_t _SzData) = 0;
	oAPI static bool Create(const oVideoContainer::DESC& _Desc, oVideoStream** _ppVideoStream);
};

interface oVideoDecodeCPU : oInterface
{
	// @oooii-eric: TODO: There is an inconsistency in behavior here. Some codecs require _pFrame to already
	//	have valid pointers assigned. Other codecs like VP8 use its own memory and the values supplied in _pFrame 
	//	are replaced. In this (more common) case the user allocated space for _pFrame is a waste. Since codecs
	//	like vp8 have to allocate there own space (because of hidden padding, and retention of previous frame
	//	for intra frame decoding) should just make all codecs behave that way.
	virtual bool Decode(oSurface::YUV420* _pFrame, size_t* _pDecodedFrameNumber = nullptr) = 0;
	oAPI static bool Create(oVideoContainer* _pContainer, oVideoDecodeCPU** _ppDecoder);
};

interface oVideoDecodeD3D10: oInterface
{
	struct DESC
	{
		DESC()
			: UseFrameTime(false),
			StitchVertically(true),
			AllowCatchUp(true)
		{}

		bool UseFrameTime; //if false the video will play as fast as possible, otherwise the frame time of the video will be honored.
		bool StitchVertically; //If there is more than one container, stitch the videos together vertically if this is true, otherwise stitch horizontally.
		bool AllowCatchUp; // If true allow any containers that have fallen behind, to catch up. If false an assert will be thrown if containers don't alll decode the same frame.
		oRECT SourceRects[24]; // video outside this rect will get discarded. You still have to pay the performance cost for decoding that video though.
		oRECT DestRects[24]; // video outside this rect will get discarded. You still have to pay the performance cost for decoding that video though.
	};
	
	virtual bool Register(interface ID3D10Texture2D* _pDestinationTexture) = 0;
	virtual void Unregister() = 0;

	virtual bool Decode(size_t* _pDecodedFrameNumber = nullptr) = 0;

	//The frame time from the first container will be used
	oAPI static bool Create(DESC _desc, oVideoContainer** _ppContainers, size_t _NumContainers, oVideoDecodeD3D10** _ppDecoder);
};

interface oVideoEncodeCPU : oInterface
{
	static const unsigned int MAX_FRAME_EXTRA = 1024; //All codecs must promise to not need more than Width*Height + (Width*Height)/2 + this variable for its frame data. This variable acocunts for space needed for frame header/ect.
	enum QUALITY
	{
		REALTIME,
		GOOD_QUALITY,
		BEST_QUALITY,
	};

	enum CONTAINER_TYPE
	{
		INVALID_CONTAINER = -1,
		WEBM_CONTAINER,
		COUNT_CONTAINER
	};

	enum CODEC_TYPE
	{
		INVALID_CODEC = -1,
		VP8_CODEC,
		RAW_420,
		COUNT_CODEC
	};

	struct DESC
	{
		DESC()
			: ContainerType(WEBM_CONTAINER)
			, CodecType(VP8_CODEC)
			, Dimensions( 256, 256 )
			, BitRate( (unsigned int )-1 )
			, FrameTimeNumerator(1001)
			, FrameTimeDenominator(30000)
			, Quality(REALTIME)
			, TwoPass(false)
		{}

		CONTAINER_TYPE ContainerType;
		CODEC_TYPE CodecType;

		// Dimensions of the stream
		int2 Dimensions;

		// Average BitRate per second. For reference, DVD is about 6Mbits and blu-ray is about 20-40Mbits
		unsigned int BitRate;

		// Rational to describe the time for a single frame in seconds
		// (ex: 1001/30000 for 29.970 Hz NTSC)
		unsigned int FrameTimeNumerator; 
		unsigned int FrameTimeDenominator;

		// Quality to encode at
		QUALITY Quality;
		bool TwoPass;
	};

	//Get the header that must be at the beginning of any video stream. 
	virtual unsigned int GetMaxHeaderSize() const = 0;
	virtual void GetHeader(void* _pDataStream, size_t _StreamSize, size_t* _pDataWrittenToStream) = 0;
	virtual void Encode(const oSurface::YUV420& _Frame, void* _pDataStream, size_t _StreamSize, size_t* _pDataWrittenToStream, bool _forceIFrame = false) = 0; //also used for second pass in 2 pass encoding
	virtual void EncodeFirstPass(const oSurface::YUV420& _Frame, bool _forceIFrame = false) = 0;
	virtual void StartSecondPass() = 0;

	oAPI static bool Create(const DESC &_desc, oVideoEncodeCPU** _ppVideoEncoder);
};

namespace oVideo
{
	oAPI bool CreateWebXFile(const char** _SourcePaths, unsigned int _NumSourcePaths, const char *_DestPath, bool _StitchVertically, bool _DeleteSourceFiles = true);
	oAPI bool CreateWebXContainers( const char *_SourcePath, const oVideoContainer::DESC& _Desc, oFUNCTION<void (oVideoFile *_container)> _functor, bool *_pStitchVertically);
}

#endif //oVideoCodec_h
