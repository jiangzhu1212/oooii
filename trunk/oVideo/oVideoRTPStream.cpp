// $(header)
#include <oVideo/oVideoRTPStream.h>
#include <oooii/oSocket.h>

class oVideoRTPStreamImpl : public oVideoRTPStream
{
	DESC Desc;
	oRefCount RefCount;
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oVideoRTPStreamImpl(const DESC &_desc, bool* _pSuccess);

	virtual void GetDesc(DESC* _pDesc) const threadsafe;

	// This is called to initiate the streaming of the next frame.  If there are no more
	// videos to stream or an error occurs this can fail.  Check oGetLastError to handle failure.
	virtual bool StreamFrame();

	// This should be called regularly to send the RTCP_HEADER on the RTCP port.
	virtual bool SendControlPacket();

	// Queues the next video for streaming and loops it the supplied number of times, with -1 indicating 
	// that the video should be looped indefinitely.  Videos are streamed in the order they are queued.
	virtual bool QueueVideo(oVideoContainer* _pVideo, unsigned int _LoopCount = (unsigned int)-1 );

	void SendCallback(void* _pData, oSocket::size_t _Size, const oNetAddr& _ToAddr);

private:
	struct QueueNode
	{
		QueueNode() : pVideo(NULL), PlayCount(0), Frame(0) { }

		bool operator==( const QueueNode& rhs ) { return (rhs.pVideo == pVideo && rhs.PlayCount == PlayCount && rhs.Frame == Frame); }

		oVideoContainer*	pVideo;
		unsigned int		PlayCount;
		unsigned int		Frame;
	};
	typedef std::vector<QueueNode> tVideoQueue;

	oRef<threadsafe oSocketAsyncUDP> Socket;

	oRef<oVideoRTP> VideoRTP;

	tVideoQueue		VideoQueue;
	size_t			VideoQueuePos;

	oNetAddr		RTPDestAddr;
	oNetAddr		RTCPDestAddr;

	RTCP_HEADER		RTCPHeader;
};

oVideoRTPStreamImpl::oVideoRTPStreamImpl(const DESC &_desc, bool* _pSuccess)
	: Desc(_desc)
{
	*_pSuccess = false;

	if(_desc.Protocol != DESC::UDP)
	{
		oSetLastError(EINVAL, "Protocol must be UDP.");
		return;
	}

	RTCPDestAddr = Desc.Address;

	{
		oSocketAsyncUDP::DESC desc;
		desc.SendCallback = oBIND(&oVideoRTPStreamImpl::SendCallback, this, oBIND1, oBIND2, oBIND3);
		if(!oSocketAsyncUDP::Create("oVideoRTPStreamSocket", &desc, &Socket))
		{
			return; // Pass-through error.
		}
	}

	{
		static const size_t BufSize = 256;
		char destaddrstr[BufSize];
		oToString(destaddrstr, Desc.Address);
		char* seperator = strstr(destaddrstr, ":");
		if(!seperator)
		{
			oSetLastError(EINVAL, "Invalid Address.");
			return;
		}

		unsigned short port = (unsigned short)atoi(seperator+1) - 1;

		if(port & 0x1)
		{
			oSetLastError(EINVAL, "RTCP Port (%u) must be odd.", port + 1);
			return;
		}

		sprintf_s(seperator, BufSize - std::distance(destaddrstr, seperator), ":%u", port);

		if(0 != oFromString(&RTPDestAddr, destaddrstr))
		{
			oSetLastError(EINVAL, "Invalid Address.");
			return;
		}
	}

	RTCPHeader.MagicNumber = oHash_fourcc("oRTP");
	VideoQueuePos = 0;

	*_pSuccess = true;
}

void oVideoRTPStreamImpl::GetDesc(DESC* _pDesc) const threadsafe
{
	const oVideoRTPStreamImpl* pThis = thread_cast<const oVideoRTPStreamImpl*>(this); // Safe because Desc doesn't change.
	*_pDesc = pThis->Desc;
}

bool oVideoRTPStreamImpl::StreamFrame()
{
	if(VideoQueue.empty())
	{
		RTCPHeader.SSRC = 0;

		oSetLastError(ENFILE, "No video queued.");
		return false;
	}

	if(VideoQueuePos >= VideoQueue.size())
		VideoQueuePos = 0;

	QueueNode& node = VideoQueue[VideoQueuePos];

	oASSERT(node.pVideo, "NULL VideoContainer.");

	// If this is the first frame, initialize the VideoRTP for this video.
	if(0 == node.Frame)
	{
		VideoRTP = 0;

		oVideoContainer::DESC VCDesc;
		node.pVideo->GetDesc(&VCDesc);
		
		oVideoRTP::DESC VRTPDesc;
		VRTPDesc.VideoDesc = VCDesc;
		VRTPDesc.MaxPacketSize = Desc.MaxPacketSize;
		if(!oVideoRTP::Create(VRTPDesc, &VideoRTP))
		{
			return false; // Pass-through error.
		}

		RTCPHeader.SSRC = VideoRTP->GetSSRC();
		RTCPHeader.VideoDesc = VCDesc;
	}

	oVideoContainer::MAPPED mapped;
	if(node.pVideo->Map(&mapped))
	{
		VideoRTP->PacketizeFrame(mapped.pFrameData, oSize32(mapped.DataSize));

		oVideoRTP::RTP_PACKET* pPacket = NULL;
		while(VideoRTP->DrainPacket(&pPacket))
		{
			Socket->Send(pPacket->pData, pPacket->Size, RTPDestAddr);
		}

		node.pVideo->Unmap();
		node.Frame++;
	}

	if(node.pVideo->HasFinished())
	{
		if(node.PlayCount)
		{
			if(!--node.PlayCount)
				VideoQueuePos++;
		}

		// Only oVideoFiles can be restarted at the moment.
		oRef<oVideoFile> VideoFile;
		if( node.pVideo->QueryInterface(&VideoFile) )
		{
			VideoFile->Restart();
			node.Frame = 0;
		}
		else
		{
			// Can't restart the video, so remove it from the queue.
			VideoQueue.erase(std::find(VideoQueue.begin(), VideoQueue.end(), node));
			VideoQueuePos--;
		}
	}

	return true;
}

bool oVideoRTPStreamImpl::SendControlPacket()
{
	Socket->Send(&RTCPHeader, sizeof(RTCPHeader), RTCPDestAddr);
	return true;
}

bool oVideoRTPStreamImpl::QueueVideo(oVideoContainer* _pVideo, unsigned int _PlayCount)
{
	if(!_pVideo)
	{
		oSetLastError(EINVAL, "NULL VideoContainer.");
		return false;
	}

	QueueNode node;
	node.pVideo = _pVideo;
	node.PlayCount = _PlayCount;

	VideoQueue.push_back(node);

	return true;
}

bool oVideoRTPStream::Create( const DESC& _Desc, oVideoRTPStream** _ppStream )
{
	bool success = false;
	oCONSTRUCT( _ppStream, oVideoRTPStreamImpl( _Desc, &success) );
	return success;
}

void oVideoRTPStreamImpl::SendCallback(void* _pData, oSocket::size_t _Size, const oNetAddr& _ToAddr)
{

}
