// $(header)
#include "oWebmEncoder.h"
#include <oooii/oBit.h>
#include <oooii/oByte.h>
#include <oooii/oSTL.h>

const oGUID& oGetGUID( threadsafe const oWebmEncoder* threadsafe const * )
{
	// {CACF15AD-0771-4327-B740-8675B79DAEA0}
	static const oGUID oIIDWebmEncoder = { 0xcacf15ad, 0x771, 0x4327, { 0xb7, 0x40, 0x86, 0x75, 0xb7, 0x9d, 0xae, 0xa0 } };
	return oIIDWebmEncoder; 
}

struct EBMLElement
{
	static unsigned int GetNumBytesForSize(unsigned long long _size)
	{
		if(_size == 0)
			return 1;
		int index = firstbithigh(_size);
		return (index+7)/7; // can hold 7 bits for each byte used for size.
	}
	static void WriteSize(unsigned char *_stream,unsigned long long _size, unsigned int _numBytesOverride = -1)
	{
		unsigned int SizeBytes = (_numBytesOverride == -1) ? GetNumBytesForSize(_size) : _numBytesOverride; 
		oASSERT(SizeBytes <= 8,"Tried to write an EBML element with size greater than 2^56-2 bytes.");

		unsigned long long SizeMarker = 0x80 << ((SizeBytes-1)*7);
		_size |= SizeMarker;
		unsigned long long writeMask = 0xff << (SizeBytes-1)*8;
		unsigned int byteToWrite = (SizeBytes-1)*8;
		for (unsigned int i = 0;i < SizeBytes;++i)
		{
			_stream[i] = (static_cast<unsigned char>((_size&writeMask)>>byteToWrite));
			writeMask >>= 8;
			byteToWrite-=8;
		}
	}
	static void WriteSize(std::vector<unsigned char> &_stream,unsigned long long _size)
	{
		unsigned int SizeBytes = GetNumBytesForSize(_size); 
		unsigned int SizeOffset = static_cast<unsigned int>(_stream.size());
		_stream.resize(_stream.size()+SizeBytes);
		WriteSize(&_stream[SizeOffset],_size);
	}
	static void WriteBinary(std::vector<unsigned char> &_stream,const unsigned char *_data,unsigned int _size)
	{
		unsigned int startOffset = static_cast<unsigned int>(_stream.size());
		_stream.resize(_stream.size()+ _size);
		memcpy(&_stream[startOffset],_data,_size);
	}
	static void WritePrimitive(std::vector<unsigned char> &_stream,float _data)
	{
		unsigned int data = oByteSwap(*reinterpret_cast<unsigned int*>(&_data));
		WriteBinary(_stream,reinterpret_cast<unsigned char*>(&data),sizeof(data));
	}
	template<typename T>
	static void WritePrimitive(std::vector<unsigned char> &_stream,T _data)
	{
		T data = oByteSwap(_data);
		WriteBinary(_stream,reinterpret_cast<unsigned char*>(&data),sizeof(data));
	}
};

struct EBMLHeader : EBMLElement
{
	void Write(std::vector<unsigned char> &_stream)
	{
		const unsigned char EBMLHeaderID[] = {0x1a,0x45,0xdf,0xa3};
		const unsigned char DocType[] = {0x42,0x82,0x84,'w','e','b','m'};
		const unsigned char DocTypeVersion[] = {0x42,0x87,0x81,1};
		const unsigned char DocTypeReadVersion[] = {0x42,0x85,0x81,1};
		const unsigned int HeaderSize = sizeof(DocType)+sizeof(DocTypeVersion)+sizeof(DocTypeReadVersion);
		WriteBinary(_stream,EBMLHeaderID,sizeof(EBMLHeaderID));
		WriteSize(_stream,HeaderSize);
		WriteBinary(_stream,DocType,sizeof(DocType));
		WriteBinary(_stream,DocTypeVersion,sizeof(DocTypeVersion));
		WriteBinary(_stream,DocTypeReadVersion,sizeof(DocTypeReadVersion));
	}
};

struct EBMLSegment : EBMLElement
{
	void Write(std::vector<unsigned char> &_stream)
	{
		const unsigned char EBMLSegment[] = {0x18,0x53,0x80,0x67,0xff};
		WriteBinary(_stream,EBMLSegment,sizeof(EBMLSegment));
	}
};

struct EBMLVideoTrack : EBMLElement
{
	unsigned int FrameDuration; //in nanoseconds;
	//Width and Height have to be from 1-16382. Not a limitation of webm. Its a limitation of this code.
	unsigned short Width; 
	unsigned short Height;     
	oRef<oWebmEncoderCodec> Codec;
	void Write(std::vector<unsigned char> &_stream)
	{
		const std::vector<unsigned char> CodecEbml = Codec->GetWebmCodecEbml();
		const std::vector<unsigned char> CodecNameEbml = Codec->GetWebmCodecNameEbml();

		const unsigned char ID[] = {0xae};
		const unsigned char TrackNumber[] = {0xd7,0x81,0x01};
		const unsigned char TrackUID[] = {0x73,0xc5,0x84,0x01,0x01,0x01,0x01};
		const unsigned char TrackType[] = {0x83,0x81,0x01};
		const unsigned char FlagDefault[] = {0x88,0x81,0x00};
		const unsigned char DefaultDuration[] = {0x23,0xe3,0x83,0x84};
		const unsigned char TrackName[] = {0x53,0x6e,0x85,'V','i','d','e','o'};

		const unsigned char VideoSettingsID[] = {0xe0};
		const unsigned char PixelWidth[] = {0xb0,0x82};
		const unsigned char PixelHeight[] = {0xba,0x82};
		const unsigned char DisplayWidth[] = {0x54,0xb0,0x82};
		const unsigned char DisplayHeight[] = {0x54,0xba,0x82};
		unsigned int sizeOfVideoSettings = sizeof(PixelWidth)+
			sizeof(PixelHeight)+sizeof(DisplayWidth)+sizeof(DisplayHeight)+
			sizeof(Width)+sizeof(Height)+sizeof(Width)+sizeof(Height);

		unsigned int sizeOfVideoTrack = sizeOfVideoSettings+
			sizeof(TrackNumber)+sizeof(TrackUID)+sizeof(TrackType)+
			sizeof(DefaultDuration)+sizeof(TrackName)+static_cast<unsigned int>(CodecEbml.size())+
			static_cast<unsigned int>(CodecNameEbml.size())+sizeof(FrameDuration)+sizeof(VideoSettingsID)+
			sizeof(FlagDefault);

		WriteBinary(_stream,ID,sizeof(ID));
		WriteSize(_stream,sizeOfVideoTrack);
		WriteBinary(_stream,TrackNumber,sizeof(TrackNumber));
		WriteBinary(_stream,TrackUID,sizeof(TrackUID));
		WriteBinary(_stream,TrackType,sizeof(TrackType));
		WriteBinary(_stream,FlagDefault,sizeof(FlagDefault));
		WriteBinary(_stream,DefaultDuration,sizeof(DefaultDuration));
		WritePrimitive(_stream,FrameDuration);
		WriteBinary(_stream,TrackName,sizeof(TrackName));
		WriteBinary(_stream,&CodecEbml[0],static_cast<unsigned int>(CodecEbml.size()));
		WriteBinary(_stream,&CodecNameEbml[0],static_cast<unsigned int>(CodecNameEbml.size()));
		WriteBinary(_stream,VideoSettingsID,sizeof(VideoSettingsID));
		WriteSize(_stream,sizeOfVideoSettings);
		WriteBinary(_stream,PixelWidth,sizeof(PixelWidth));
		WritePrimitive(_stream,Width);
		WriteBinary(_stream,PixelHeight,sizeof(PixelHeight));
		WritePrimitive(_stream,Height);
		WriteBinary(_stream,DisplayWidth,sizeof(DisplayWidth));
		WritePrimitive(_stream,Width);
		WriteBinary(_stream,DisplayHeight,sizeof(DisplayHeight));
		WritePrimitive(_stream,Height);
	}
};

struct EBMLTracks : EBMLElement
{
	EBMLVideoTrack VideoTrack;
	void Write(std::vector<unsigned char> &_stream)
	{
		const unsigned char ID[] = {0x16,0x54,0xae,0x6b};
		std::vector<unsigned char> videoTrackStream;
		VideoTrack.Write(videoTrackStream);

		WriteBinary(_stream,ID,sizeof(ID));
		WriteSize(_stream,videoTrackStream.size());
		WriteBinary(_stream,&videoTrackStream[0],static_cast<unsigned int>(videoTrackStream.size()));
	}
};

struct EBMLCluster : EBMLElement
{
	unsigned int TimeStamp;
	unsigned int MaxPayloadSize;
	void Write(EBMLClusterCache &_cache)
	{
		std::vector<unsigned char> Stream;
		const unsigned char ID[] = {0x1f,0x43,0xb6,0x75};
		const unsigned char Timecode[] = {0xe7,0x84};
		const unsigned char SimpleBlock[] = {0xa3};
		const unsigned char SimpleBlockHeader[] = {0x81,0x00,0x00,0x00};

		_cache.SimpleblockSizeBase = sizeof(SimpleBlockHeader);
		unsigned int SimpleblockSize = _cache.SimpleblockSizeBase+MaxPayloadSize;
		_cache.ClusterSizeBase = sizeof(SimpleBlock)+sizeof(Timecode)+
			sizeof(TimeStamp)+GetNumBytesForSize(SimpleblockSize);
		unsigned int ClusterSize = _cache.ClusterSizeBase+SimpleblockSize;

		WriteBinary(Stream,ID,sizeof(ID));
		_cache.ClusterSizeOffset = static_cast<unsigned int>(Stream.size());
		_cache.ClusterSizeNumBytes = GetNumBytesForSize(ClusterSize);
		WriteSize(Stream,ClusterSize);
		WriteBinary(Stream,Timecode,sizeof(Timecode));
		_cache.TimeStampOffset = static_cast<unsigned int>(Stream.size());
		WritePrimitive(Stream,TimeStamp);
		WriteBinary(Stream,SimpleBlock,sizeof(SimpleBlock));
		_cache.SimpleBlockSizeOffset = static_cast<unsigned int>(Stream.size());
		_cache.SimpleSizeNumBytes = GetNumBytesForSize(SimpleblockSize);
		WriteSize(Stream,SimpleblockSize);
		_cache.FlagsOffset = static_cast<unsigned int>(Stream.size())+3; //flags are last byte in SimpleBlockHeader
		WriteBinary(Stream,SimpleBlockHeader,sizeof(SimpleBlockHeader));

		_cache.Cache.resize(Stream.size());
		memcpy(oGetData(_cache.Cache),&Stream[0],Stream.size());
	}
};

void oWebmEncoder::SetClusterHeader(void *_buffer,const oWebmEncoderCodec::FrameDesc &_desc)
{
	unsigned int SimpleblockSize = ClusterCache.SimpleblockSizeBase+_desc.PayloadSize;
	unsigned int ClusterSize = ClusterCache.ClusterSizeBase+SimpleblockSize;
	EBMLElement::WriteSize((unsigned char*)oByteAdd(_buffer,ClusterCache.SimpleBlockSizeOffset),SimpleblockSize,ClusterCache.SimpleSizeNumBytes);
	EBMLElement::WriteSize((unsigned char*)oByteAdd(_buffer,ClusterCache.ClusterSizeOffset),ClusterSize,ClusterCache.ClusterSizeNumBytes);
	unsigned int beTimeStamp = oByteSwap(_desc.TimeStamp);
	memcpy(oByteAdd(_buffer,ClusterCache.TimeStampOffset),&beTimeStamp,sizeof(beTimeStamp));

	unsigned char* flags = (unsigned char*)oByteAdd(_buffer,ClusterCache.FlagsOffset);
	if(_desc.Keyframe)
		*flags |= 0x80;
	if(_desc.Invisible)
		*flags |= 0x08;
	if(_desc.CanDiscard)
		*flags |= 0x01;
}

oWebmEncoder::oWebmEncoder( const oVideoEncodeCPU::DESC& _Desc, bool* _pSuccess) : Desc(_Desc)
{
	*_pSuccess = false;
	
	if(_Desc.ContainerType != WEBM_CONTAINER || !(_Desc.CodecType == VP8_CODEC || _Desc.CodecType == RAW_420))
	{
		oSetLastError(EINVAL,"Only VP8  and RAW_420 are currently supported with the webm container format");
		return;
	}
	// @oooii-eric: FIXME If you need ranges outside of this, will have to make the code just a hair smarter. Its currently set to use 2 bytes fixed for width and height.
	if(Desc.Width < 1 || Desc.Width > 16382 || Desc.Height < 1 || Desc.Height > 16382)
	{
		oSetLastError(EINVAL,"Webm encoder currently requires Width and Height to be between 1 and 16382. This is not a webm limitation");
		return;
	}

	oWebmEncoderCodec::Create(_Desc, &Codec);

	const unsigned long long secondsToNanoSeconds = 1000000000; //webm stores the length of one frame in nanoseconds.
	EBMLHeader header;
	header.Write(Header);
	EBMLSegment segment;
	segment.Write(Header);
	EBMLTracks tracks;
	tracks.VideoTrack.FrameDuration = static_cast<unsigned int>((_Desc.FrameTimeNumerator*secondsToNanoSeconds)/_Desc.FrameTimeDenominator);
	tracks.VideoTrack.Width = static_cast<unsigned short>(_Desc.Width);
	tracks.VideoTrack.Height = static_cast<unsigned short>(_Desc.Height);
	tracks.VideoTrack.Codec = Codec;
	tracks.Write(Header);

	FrameCount = 0;
	MaxPayloadSize = Desc.Width*Desc.Height;
	MaxPayloadSize = MaxPayloadSize+MaxPayloadSize/2; //one full rez y + 2 half rez chroma

	EBMLCluster cluster;
	cluster.MaxPayloadSize = MaxPayloadSize;
	cluster.Write(ClusterCache);

	*_pSuccess = true;
}

oWebmEncoder::~oWebmEncoder()
{
}

void oWebmEncoder::GetHeader(void* _pDataStream, size_t _StreamSize, size_t* _pDataWrittenToStream)
{
	if(_StreamSize < Header.size())
	{
		_pDataWrittenToStream = 0;
		oSetLastError(EINVAL,"Buffer provided to oWebmEncoder::GetHeader did not have enough space. Needed %d bytes, buffer had %d bytes",Header.size(),_StreamSize);
		return;
	}
	memcpy(_pDataStream,&Header[0],Header.size());
	*_pDataWrittenToStream = Header.size();
};

void oWebmEncoder::Encode(const oSurface::YUV420& _Frame, void* _pDataStream, size_t _StreamSize, size_t* _pDataWrittenToStream, bool _forceIFrame/* = false*/)
{
	memcpy(_pDataStream,oGetData(ClusterCache.Cache),oGetDataSize(ClusterCache.Cache));
	
	size_t packetSz = 0;
	void *data = oByteAdd(_pDataStream,oGetDataSize(ClusterCache.Cache));
	oWebmEncoderCodec::FrameDesc frameDesc;
	Codec->Encode( _Frame, data,_StreamSize-oGetDataSize(ClusterCache.Cache), &packetSz, GetFrameStamp(), GetFrameDuration(), _forceIFrame, frameDesc);

	frameDesc.TimeStamp = GetFrameStamp();
	frameDesc.PayloadSize = static_cast<unsigned int>(packetSz);
	SetClusterHeader(_pDataStream,frameDesc);

	*_pDataWrittenToStream = packetSz+oGetDataSize(ClusterCache.Cache);

	++FrameCount;
};

void oWebmEncoder::EncodeFirstPass(const oSurface::YUV420& _Frame, bool _forceIFrame /*= false*/)
{
	Codec->EncodeFirstPass(_Frame, GetFrameStamp(), GetFrameDuration(), _forceIFrame);
}

void oWebmEncoder::StartSecondPass()
{
	Codec->StartSecondPass(GetFrameStamp(), GetFrameDuration());
}
