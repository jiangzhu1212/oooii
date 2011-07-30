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
#include "oWebmStreaming.h"
#include <oooii/oLimits.h>
#include <oooii/oMath.h>
#include <oooii/oSTL.h>

const oGUID& oGetGUID( threadsafe const oWebmStreaming* threadsafe const * )
{
	// {4EA1C1D7-51FB-4aa3-81FA-EE7389668CE6}
	static const oGUID oIIDWebmStreaming = { 0x4ea1c1d7, 0x51fb, 0x4aa3, { 0x81, 0xfa, 0xee, 0x73, 0x89, 0x66, 0x8c, 0xe6 } };
	return oIIDWebmStreaming;
}

oWebmStreaming::oWebmStreaming( const oVideoContainer::DESC& _Desc, bool* _pSuccess) : Desc(_Desc), InvalidStream(false), VideoTrack(-1),
	IsWebmStream(false), ParseOneFrame(false), CurrentFrame(0)
{
	//HEADER
	TagsToReadInto.insert(0x1a45dfa3);
		//doctype
		TagsToParse[0x4282] = ParseFunctionType(oBIND(&oWebmStreaming::ParseDocType,this,oBIND1));

	//segment
	TagsToReadInto.insert(0x18538067);
		//tracks
		TagsToReadInto.insert(0x1654ae6b);
			//track
			TagsToReadInto.insert(0xae);
			TagsToParse[0xae] = ParseFunctionType(oBIND(&oWebmStreaming::ParseTrack,this,oBIND1));
			TagsToParse[0xd7] = ParseFunctionType(oBIND(&oWebmStreaming::ParseTrackNumber,this,oBIND1));
			TagsToParse[0x83] = ParseFunctionType(oBIND(&oWebmStreaming::ParseTrackType,this,oBIND1));
			TagsToParse[0x86] = ParseFunctionType(oBIND(&oWebmStreaming::ParseTrackCodecID,this,oBIND1));
			TagsToParse[0x23e383] = ParseFunctionType(oBIND(&oWebmStreaming::ParseFrameDuration,this,oBIND1));
				//video settings
				TagsToReadInto.insert(0xe0);
				TagsToParse[0xb0] = ParseFunctionType(oBIND(&oWebmStreaming::ParseTrackWidth,this,oBIND1));
				TagsToParse[0xba] = ParseFunctionType(oBIND(&oWebmStreaming::ParseTrackHeight,this,oBIND1));
		//cluster
		TagsToReadInto.insert(0x1f43b675);
		TagsToParse[0x1f43b675] = ParseFunctionType(oBIND(&oWebmStreaming::ParseCluster,this,oBIND1));
		//TagsToParse[0xe7] = ParseFunctionType(oBIND(&oWebmStreaming::ParseTimeStamp,this,oBIND1)); //we dont care about time, decode as fast as we can
			//simple block
			TagsToParse[0xa3] = ParseFunctionType(oBIND(&oWebmStreaming::ParseSimpleBlock,this,oBIND1));

	*_pSuccess = true;
}

oWebmStreaming::~oWebmStreaming()
{

}

bool oWebmStreaming::ReadEBML(unsigned long long& _Data, bool _StripSize)
{
	_Data = PacketData[PacketIndex++];

	int bitIndex = firstbithigh(_Data);
	if(-1 == bitIndex)
	{
		oSetLastError(EINVAL,"Invalid ebml encountered, will skip the byte and attempt to recover.");
		return false;
	}
	unsigned int numBytesLeft = 7-bitIndex;
	if(_StripSize)
		_Data &= ~(0x01<<bitIndex); //get rid size bit

	for (unsigned int i = 0;i < numBytesLeft;++i)
	{
		_Data <<= 8;
		_Data |= PacketData[PacketIndex++];
	}
	return true;
}

bool oWebmStreaming::ReadEBMLHeader(HEADER& _Header)
{
	unsigned long long data = 0;
	if(!ReadEBML(data,false))
		return false;
	_Header.ID = static_cast<unsigned int>(data);
	if(!ReadEBML(data,true))
		return false;
	_Header.Size = data;
	return true;
}

unsigned int oWebmStreaming::ReadInt(unsigned int _numBytes, unsigned int _offset /*= 0*/)
{
	unsigned int result = 0;
	for (unsigned int i = 0;i < _numBytes;++i)
	{
		result <<= 8;
		result |= PacketData[PacketIndex+i];
	}	
	return result;
}

void oWebmStreaming::ParseDocType(const HEADER& _Header)
{
	if (_Header.Size != 4 || strncmp((char*)PacketData+PacketIndex,"webm",4) != 0)
	{
		oSetLastError(EINVAL,"webm stream had invalid doc type");
		InvalidStream = true;
	}
	else
	{
		IsWebmStream = true;
	}
}

void oWebmStreaming::ParseTrack(const HEADER& _Header)
{
	Tracks.push_back(WebmTrack());
}

void oWebmStreaming::ParseTrackNumber(const HEADER& _Header)
{
	Tracks[Tracks.size()-1].TrackNumber = ReadInt(static_cast<unsigned int>(_Header.Size));
}

void oWebmStreaming::ParseTrackType(const HEADER& _Header)
{
	Tracks[Tracks.size()-1].TrackType = static_cast<unsigned char>(ReadInt(static_cast<unsigned int>(_Header.Size)));
}

void oWebmStreaming::ParseTrackCodecID(const HEADER& _Header)
{
	 oASSERT( _Header.Size < oNumericLimits<size_t>::GetMax(), "Size is to large " );
	 Tracks[Tracks.size()-1].CodecID = new char[static_cast<size_t>(_Header.Size)];
	 memcpy(Tracks[Tracks.size()-1].CodecID,PacketData+PacketIndex, static_cast<size_t>( _Header.Size ));
}

void oWebmStreaming::ParseTrackWidth(const HEADER& _Header)
{
	Tracks[Tracks.size()-1].Dimensions.x = ReadInt(static_cast<int>(_Header.Size));
}

void oWebmStreaming::ParseTrackHeight(const HEADER& _Header)
{
	Tracks[Tracks.size()-1].Dimensions.y = ReadInt(static_cast<int>(_Header.Size));
}

void oWebmStreaming::ParseFrameDuration(const HEADER& _Header)
{
	unsigned int duration = ReadInt(sizeof(duration));
	unsigned int fps = static_cast<unsigned int>(round(1000000000.0/duration)); //rounded to there nearest 1 fps
	Tracks[Tracks.size()-1].FrameTimeDenominator = fps*1000; //seems to be common to store these at this scale. i.e. denominater would be 24000 for a 24 fps movie
	Tracks[Tracks.size()-1].FrameTimeNumerator = static_cast<unsigned int>(round((duration/1000000000.0)*Tracks[Tracks.size()-1].FrameTimeDenominator));
}

void oWebmStreaming::CheckForHeaderInfo()
{
	if(VideoTrack == -1) 
	{
		oFOREACH(WebmTrack &track,Tracks)
		{
			if(track.TrackType == 1 && strncmp(track.CodecID,"V_VP8",5) == 0) //found a vp8 track!
			{
				Desc.CodecType = VP8_CODEC;
				VideoTrack = track.TrackNumber;
				Desc.Dimensions = track.Dimensions;
				Desc.FrameTimeDenominator = track.FrameTimeDenominator;
				Desc.FrameTimeNumerator = track.FrameTimeNumerator;
			}
			else if(track.TrackType == 1 && strncmp(track.CodecID,"oooii_raw420",12) == 0) //found a raw 420 track!
			{
				Desc.CodecType = RAW_420;
				VideoTrack = track.TrackNumber;
				Desc.Dimensions = track.Dimensions;
				Desc.FrameTimeDenominator = track.FrameTimeDenominator;
				Desc.FrameTimeNumerator = track.FrameTimeNumerator;
			}
		}
	}
}

void oWebmStreaming::ParseCluster(const HEADER& _Header)
{
	LastClusterIndex = LastParseIndex;
}

void oWebmStreaming::ParseSimpleBlock(const HEADER& _Header)
{
	CheckForHeaderInfo();//must be first simple block. need to find the video track if we havent already

	if(VideoTrack == -1)
	{
		oSetLastError(EINVAL,"Have not received all header info before receiving data blocks, or there was not a valid vp8 video track in this webm");
		return;
	}
	unsigned char BlockTrack = PacketData[PacketIndex]&0x7f;
	if(BlockTrack != VideoTrack) //this block wasn't for the video track
	{
		return;
	}
	
	unsigned int flags = PacketData[PacketIndex+3];
	bool isKeyFrame = (flags&0x80) != 0;
	if(flags&0x06)
	{
		oSetLastError(EINVAL,"we do not currently support webm lacing. the oooii encoder never laces frames");
		return;
	}

	oMutex::ScopedLock frameLock(VideoFrameLock);
	if(Desc.AllowDroppedFrames && isKeyFrame && EncodedVideoFrames.size() >= MIN_FRAMES_AHEAD_TO_DISCARD) // can discard previous frames.
	{
		CurrentFrame += EncodedVideoFrames.size();
		EncodedVideoFrames.clear();
	}
	oASSERT( _Header.Size < (unsigned int)oNumericLimits<size_t>::GetMax(), "Size is to large " );

	EncodedVideoFrames.push_back(std::vector<unsigned char>());
	EncodedVideoFrames.back().resize(static_cast<size_t>( _Header.Size-4 ));
	memcpy(oGetData(EncodedVideoFrames.back()),PacketData+PacketIndex+4,static_cast<size_t>( _Header.Size-4 ));
}

bool oWebmStreaming::Map(MAPPED* _pMapped)
{
	if (EncodedVideoFrames.empty())
		return oVideoReturnEndOfFile(_pMapped);

	_pMapped->pFrameData = oGetData(EncodedVideoFrames.front());
	_pMapped->DataSize = oGetDataSize(EncodedVideoFrames.front());
	_pMapped->DecodedFrameNumber = CurrentFrame;
	return true;
}

void oWebmStreaming::Unmap()
{
	if(!EncodedVideoFrames.empty())
	{
		EncodedVideoFrames.pop_front();
		++CurrentFrame;
	}
}

void oWebmStreaming::PushByteStream(const void* _pData, size_t _SzData)
{
	if(InvalidStream)
		return;

	PacketData = (unsigned char*)_pData;
	oASSERT(_SzData < oNumericLimits<unsigned int>::GetMax(),"webm stream reader only supports packets less than 4G currently (and actually probably a lot less than that practically)");
	PacketSize = static_cast<unsigned int>(_SzData);
	PacketIndex = 0;

	while(!(PacketIndex >= PacketSize) && (!ParseOneFrame || EncodedVideoFrames.empty()))
	{
		LastParseIndex = PacketIndex;
		HEADER header;
		ReadEBMLHeader(header);
		TagsToParseType::iterator parseFunction = TagsToParse.find(header.ID);
		if (parseFunction != TagsToParse.end())
		{
			(*parseFunction).second(header);
		}
		if(TagsToReadInto.find(header.ID) == TagsToReadInto.end())
			PacketIndex += static_cast<unsigned int>(header.Size); //a segment should be the only ebml element that is ever larger than 32 bits, and we read into that one
	}
	CheckForHeaderInfo(); //check and see if we have gotten the video track yet.
}
