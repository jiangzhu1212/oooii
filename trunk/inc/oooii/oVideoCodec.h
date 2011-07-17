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
			, Width((unsigned int)-1)
			, Height((unsigned int)-1)
			, FrameTimeNumerator(0)
			, FrameTimeDenominator(1)
			, AllowDroppedFrames(true)
			, StartingFrame(-1) 
			, EndingFrame(-1) 
		{}
		CONTAINER_TYPE ContainerType;
		CODEC_TYPE CodecType;
		unsigned int Width;
		unsigned int Height;
		unsigned int FrameTimeNumerator; 
		unsigned int FrameTimeDenominator;
		bool AllowDroppedFrames;
		int StartingFrame; // Note that StartingFrame and Ending Frame are not supported by all codecs. 
		int EndingFrame; //In particular this feature doesn't work with the webm codecs currently.
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
	virtual bool Map(MAPPED* _pMapped) threadsafe = 0;

	// Ensure Unmap is only called after a successful map.
	virtual void Unmap() threadsafe = 0;

	// If the video file has completely played through 
	// (i.e. no more frames) HasFinished is True
	virtual bool HasFinished() const  = 0;

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oVideoFile : oVideoContainer
{
	// A video that is complete and saved to disk.  These types
	// of containers often have to access random parts of the
	// file so they behave differently than streams.

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

	//Streams never finish
	bool HasFinished() const override {return false;}

	virtual void PushByteStream(const void* _pData, size_t _SzData) threadsafe = 0;
	static bool Create(const oVideoContainer::DESC& _Desc, threadsafe oVideoStream** _ppVideoStream);
};

interface oVideoDecodeCPU : oInterface
{
	// @oooii-eric: TODO: There is an inconsistency in behavior here. Some codecs require _pFrame to already
	//	have valid pointers assigned. Other codecs like VP8 use its own memory and the values supplied in _pFrame 
	//	are replaced. In this (more common) case the user allocated space for _pFrame is a waste. Since codecs
	//	like vp8 have to allocate there own space (because of hidden padding, and retention of previous frame
	//	for intra frame decoding) should just make all codecs behave that way.
	virtual bool Decode(oSurface::YUV420* _pFrame, size_t* _pDecodedFrameNumber = nullptr) threadsafe = 0;
	static bool Create(threadsafe oVideoContainer* _pContainer, threadsafe oVideoDecodeCPU** _ppDecoder);
};

interface oVideoDecodeD3D10: oInterface
{
	struct DESC
	{
		DESC()
			: UseFrameTime(false),
			StitchVertically(true),
			NVIDIASurround(false)
		{}

		bool UseFrameTime; //if false the video will play as fast as possible, otherwise the frame time of the video will be honored.
		bool StitchVertically; //If there is more than one container, stitch the videos together vertically if this is true, otherwise stitch horizontally.
		bool NVIDIASurround; // If true assume 3 monitors arranged horizontally.
		std::vector<threadsafe oVideoContainer*> pContainers; //The frame time from the first container will be used
	};

	// A D3D10 decoder utilized the GPU to assist in the decode process.  Since with
	// most codecs there is a good amount of CPU work in addition to GPU work a single
	// decoder services multiple GPUs, hence the Register/Unregister API
	oDECLARE_HANDLE(HDECODE_CONTEXT);

	virtual HDECODE_CONTEXT Register(interface ID3D10Texture2D* _pDestinationTexture) threadsafe = 0;
	virtual void Unregister(HDECODE_CONTEXT) threadsafe = 0;

	virtual bool Decode(HDECODE_CONTEXT, size_t* _pDecodedFrameNumber = nullptr) threadsafe = 0;

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

interface oVideoRTP : oInterface
{
	struct DESC
	{
		DESC()
			: MaxPacketSize(1500)
		{}
		oVideoContainer::DESC VideoDesc;
		unsigned int MaxPacketSize;
	};

	struct RTP_PACKET
	{
		enum PRIORITY
		{
			PRIORITY_HIGH, // These packets should be given higher protection by the transport mechanism as NORMAL packets are useless without them
			PRIORITY_NORMAL // These are packets that can be lost without losing an entire frame
		};
		PRIORITY Priority;
		unsigned int Size;
		const void* pData;
	};

	// Can fail if there is no room to store the packets.  If this occurs the user must call DrainPacket to make room
	virtual bool PacketizeFrame(const void* _pFrame, unsigned int _szFrame) = 0;
	
	// Returns the size of the packet or 0 if no more packets are available.  The pointer is valid until another
	// call to DrainPacket
	virtual bool DrainPacket(RTP_PACKET** _ppPacket) = 0;

	static bool Create(const DESC &_desc, oVideoRTP** _ppVideoRTP);
};

#endif //oVideoCodec_h
