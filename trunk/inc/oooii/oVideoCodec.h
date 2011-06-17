// $(header)
#pragma once
#ifndef oVideoCodec_h
#define oVideoCodec_h
 
#include <oooii/oInterface.h>
#include <oooii/oSurface.h>
#include <vector>

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
		COUNT_CONTAINER
	};

	enum CODEC_TYPE
	{
		INVALID_CODEC = -1,
		VP8_CODEC,
		RLE_CODEC,
		RAW_420,
		COUNT_CODEC
	};

	struct DESC
	{
		DESC()
			: ContainerType(INVALID_CONTAINER)
			, CodecType(INVALID_CODEC)
			, Width((unsigned int)-1)
			, Height((unsigned int)-1)
			, FrameTimeNumerator(0)
			, FrameTimeDenominator(1)
			, AllowDroppedFrames(true)
		{}
		CONTAINER_TYPE ContainerType;
		CODEC_TYPE CodecType;
		unsigned int Width;
		unsigned int Height;
		unsigned int FrameTimeNumerator; 
		unsigned int FrameTimeDenominator;
		bool AllowDroppedFrames;
	};

	// Maps the next frame of data.  If no data is available _szData is 0
	// if a failure occurs or no more data will ever arrive _pValid 
	// is set to false.
	// Will block if there a frame is already mapped.
	virtual void MapFrame(void** _ppFrameData, size_t* _szData, bool* _pValid, size_t *_decodedFrameNumber) threadsafe = 0;
	virtual void UnmapFrame() threadsafe = 0;

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oVideoFile : oVideoContainer
{
	// A video that is complete and saved to disk.  These types
	// of containers often have to access random parts of the
	// file so they behave differently than streams.

	// If the video file has completely played through 
	// (i.e. no more frames) HasFinished is True
	virtual bool HasFinished() const  = 0;
	virtual void Restart() threadsafe = 0;
	
	// The description used by create can be left empty or partially empty 
	// in which create will attempt to infer the data from the file itself.
	static bool Create(const char* _pFileName, const oVideoContainer::DESC& _Desc, oVideoFile** _pVideoFile);
};

interface oVideoStream : oVideoContainer
{
	// A video stream is never complete in that data can continue
	// to be sent to it.  The method of data transportation
	// (i.e. TCP IP/UDP) is separated from the actual data consumption.
	// Therefore the user needs to call PushByteStream to add video data.
	// A streamer is always consuming data as fast as it can (i.e. it ignores framerate)
	// if the user wants to artificially control framerate they should slow calls to PushByteStream.

	virtual void PushByteStream(void* _pData, size_t _SzData) threadsafe = 0;
	static bool Create(const oVideoContainer::DESC& _Desc, threadsafe oVideoStream** _ppVideoStream);
};

interface oVideoDecodeCPU : oInterface
{
	virtual bool Decode(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber = nullptr) threadsafe = 0;
	static bool Create(threadsafe oVideoContainer* _pContainer, threadsafe oVideoDecodeCPU** _ppDecoder);
};

interface oVideoDecodeD3D10: oInterface
{
	struct DESC
	{
		DESC()
			: UseFrameTime(false),
			StitchVertically(true)
		{}

		bool UseFrameTime; //if false the video will play as fast as possible, otherwise the frame time of the video will be honored.
		bool StitchVertically; //If there is more than one container, stitch the videos together vertically if this is true, otherwise stitch horizontally.
		std::vector<threadsafe oVideoContainer*> pContainers; //The frame time from the first container will be used
	};

	// A D3D10 decoder utilized the GPU to assist in the decode process.  Since with
	// most codecs there is a good amount of CPU work in addition to GPU work a single
	// decoder services multiple GPUs, hence the Register/Unregister API
	oDECLARE_HANDLE(HDECODE_CONTEXT);

	virtual HDECODE_CONTEXT Register(interface ID3D10Texture2D* _pDestinationTexture) threadsafe = 0;
	virtual void Unregister(HDECODE_CONTEXT) threadsafe = 0;

	virtual bool Decode(HDECODE_CONTEXT, size_t *_decodedFrameNumber = nullptr) threadsafe = 0;

	static bool Create(DESC _desc, threadsafe oVideoDecodeD3D10** _ppDecoder);
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
			, Width( 256 )
			, Height( 256 )
			, BitRate( (unsigned int )-1 )
			, FrameTimeNumerator(1001)
			, FrameTimeDenominator(30000)
			, Quality(REALTIME)
			, TwoPass(false)
		{}

		CONTAINER_TYPE ContainerType;
		CODEC_TYPE CodecType;

		// Dimensions of the stream
		unsigned int Width;
		unsigned int Height;

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

	static bool Create(const DESC &_desc, oVideoEncodeCPU** _ppVideoEncoder);
};

#endif //oVideoCodec_h
