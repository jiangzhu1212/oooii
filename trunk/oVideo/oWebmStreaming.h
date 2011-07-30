// $(header)
#pragma once
#ifndef oWebmStreaming_h
#define oWebmStreaming_h

#include <oVideo/oVideoCodec.h>
#include <oooii/oRefCount.h>
#include <oooii/oMutex.h>
#include <oooii/oSize.h>
#include <oooii/oStddef.h>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include "oVideoHelpers.h"

struct WebmTrack
{
	WebmTrack() : CodecID(NULL) {}
	~WebmTrack() {delete [] CodecID;}
	unsigned int	TrackNumber;
	unsigned char	TrackType;
	char*			CodecID;
	int2			Dimensions;
	unsigned int	FrameTimeNumerator; 
	unsigned int	FrameTimeDenominator;
};

class oWebmStreaming : public oVideoStream
{
public:
	struct HEADER
	{
		unsigned int ID;
		oSize64 Size;
	};

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oWebmStreaming>());
	oDECLARE_VIDEO_MAP_INTERFACE();
	oWebmStreaming( const oVideoContainer::DESC& _Desc, bool* _pSuccess);
	~oWebmStreaming();

	//If we haven't decoded the header yet, then we don't have anything meaningful to return.
	virtual void GetDesc(DESC* _pDesc) const override
	{
		if(VideoTrack != -1) *_pDesc = Desc;
	}

	virtual void PushByteStream(const void* _pData, size_t _SzData) ;

	//If true then PushByteStream will parse the webm data until it has one 
	//	full frame of data ready to be decoded, and then will save how much
	//	data it consumed, and return that in GetDataConsumed.
	void SetParseOneFrame(bool _parseOneFrame) {ParseOneFrame = _parseOneFrame;}
	unsigned int GetDataConsumed() const {return PacketIndex;}
	bool HasEncodedFrame() const {return !EncodedVideoFrames.empty();}
	unsigned long long GetLastFrameIndex() const {return LastClusterIndex;}
private:

	//Don't drop frames when getting a new iframe unless we are at least this number of frames ahead
	const static unsigned int MIN_FRAMES_AHEAD_TO_DISCARD = 2; 

	void CheckForHeaderInfo();
	void PushByteStreamInternal(const void* _pData, size_t _SzData);

	bool ReadEBML(unsigned long long& _Data, bool _StripSize);
	bool ReadEBMLHeader(HEADER& _Header);
	unsigned int ReadInt(unsigned int _numBytes,unsigned int _offset = 0);

	void ParseDocType(const HEADER& _Header);
	void ParseTrack(const HEADER& _Header);
	void ParseTrackNumber(const HEADER& _Header);
	void ParseTrackType(const HEADER& _Header);
	void ParseTrackCodecID(const HEADER& _Header);
	void ParseTrackWidth(const HEADER& _Header);
	void ParseTrackHeight(const HEADER& _Header);
	void ParseFrameDuration(const HEADER& _Header);
	void ParseSimpleBlock(const HEADER& _Header);
	void ParseCluster(const HEADER& _Header);
 
	oVideoContainer::DESC Desc;
	oRefCount RefCount;
	std::tr1::unordered_set<unsigned int> TagsToReadInto;
	typedef oFUNCTION<void (const HEADER&)> ParseFunctionType;
	typedef std::tr1::unordered_map<unsigned int,ParseFunctionType> TagsToParseType;
	TagsToParseType TagsToParse;
	
	std::vector<WebmTrack> Tracks;

	unsigned char *PacketData;
	unsigned int PacketSize;
	unsigned int PacketIndex;
	bool InvalidStream;
	bool IsWebmStream;
	int VideoTrack;

	std::deque<std::vector<unsigned char>> EncodedVideoFrames;
	oMutex VideoFrameLock;
	oMutex PushBytesLock;

	bool ParseOneFrame;
	unsigned long long LastClusterIndex;
	unsigned long long LastParseIndex;

	size_t CurrentFrame;
};

#endif
