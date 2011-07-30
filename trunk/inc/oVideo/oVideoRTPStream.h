// $(header)
#pragma once
#ifndef oVideoRTPStream_h
#define oVideoRTPStream_h

#include <oVideo/oVideoCodec.h>
#include <oooii/oSocket.h>

namespace oVideoRTPConnection
{
	//////////////////////////////////////////////////////////////////////////
	// OOOii RTP Streaming Connection Protocol 
	// These structures are used to define the streaming protocol
	// that is used to setup connections. 
	//////////////////////////////////////////////////////////////////////////
	// Step 1:
	// The client sends a TUNE_CHANNELS_REQUEST
	// with an optional name that describes what channel
	// the client is responsible for.  If the client doesn't know
	// it sets OptionalName to NULL which causes the server to look up the channel
	// based on the client's IP
	struct TUNE_CHANNELS_REQUEST
	{
		unsigned int MagicNumber; // Should be 'oTCR'
		char OptionalName[64]; // Optional name that is used to identify the client when the server does not identify by IP
	};

	// Step 2:
	// The server returns a CHANNEL_HEADER to the client that describes 
	// the channel information needed to properly stream the channel.  This
	// information is sent to the client in 2 subsequent message blobs, if
	// there is not a valid channel the blob sizes will be 0
	struct CHANNEL_HEADER
	{
		unsigned int MagicNumber; // Should be 'oTCR'
		unsigned int StreamStiching; // 0 Means streams are stitched horizontally, 1 means they are stiched vertically
		unsigned int StreamBlobSize; // The total size in bytes of the stream blob that is sent immediately after this header.
		unsigned int RectangleBlobSize; // The total size in bytes of the rectangle blob that is sent immediately after the stream blob.
	};

	// Step 3:
	// The channel blob is made up of multiple CHANNEL_STREAM_BLOB as described by StreamBlobSize
	struct CHANNEL_STREAM_BLOB
	{
		unsigned short Port;  // The address and port of to listen for the RTPC_HEADER (see below).
	};

	// Step 4:
	// The rectangle blob is made up of multiple CHANNEL_RECTANGLE_BLOB as described by RectangleBlobSize
	struct CHANNEL_RECTANGLE_BLOB
	{
		oRECT SourceRect; // The region of the stream this client should decode
		oRECT DestRect; // Where on screen this client should place the decoded values
	};
}

interface oVideoRTP : oInterface
{
	struct DESC
	{
		DESC()
			: MaxPacketSize(1472)
			, StartingPictureID(0)
		{}
		oVideoContainer::DESC VideoDesc;
		unsigned int MaxPacketSize;
		unsigned StartingPictureID : 7;
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

	// Returns the SSRC of the stream which can be used to validate packets are from the correct stream
	virtual unsigned int GetSSRC() = 0;

	static bool ValidatePacket(const void* _pPacketData, unsigned int _SSRC);

	oAPI static bool Create(const DESC &_desc, oVideoRTP** _ppVideoRTP);
};


interface oVideoRTPStream : oInterface
{
	struct RTCP_HEADER
	{
		enum STATUS
		{
			STREAMING, // VidoeDesc validly describes the data
		};
		unsigned int MagicNumber; // Should be 'oRTP'
		unsigned int SSRC; // If this is 0 it indicates that currently no videos are streaming
		oVideoContainer::DESC VideoDesc;
	};

	struct DESC
	{
		enum PROTOCOL
		{
			UDP
		};
		DESC()
			: MaxPacketSize	(1472) // 1500 (Ethernet MTU) minus IP packet overhead (20) minus UDP packet overhead (8).
			, Protocol		(UDP)
		{}
		// This should be set to the MTU of the network
		unsigned int MaxPacketSize; 

		// This is always UDP.
		PROTOCOL Protocol;

		// Address that the RTCP data is transmitted to. The port must be odd-numbered.
		// The actual RTP stream is always broadcast on the even port that is one less
		// than the RTCPPort.
		oNetAddr Address;
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// This is called to initiate the streaming of the next frame.  If there are no more
	// videos to stream or an error occurs this can fail.  Check oGetLastError to handle failure.
	virtual bool StreamFrame() = 0;

	// This should be called regularly to send the RTCP_HEADER on the RTCP port.
	virtual bool SendControlPacket() = 0;

	// Queues the next video for streaming and plays it the supplied number of times, with 0 indicating 
	// that the video should be looped indefinitely.  Videos are streamed in the order they are queued.
	virtual bool QueueVideo(oVideoContainer* _pVideo, unsigned int _PlayCount = 0 ) = 0;

	oAPI static bool Create(const DESC& _Desc, oVideoRTPStream** _ppStream);
};

#endif //oVideoRTPStream_h
