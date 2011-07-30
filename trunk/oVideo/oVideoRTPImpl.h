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
#pragma once
#ifndef oVideoRTImpl_h
#define oVideoRTImpl_h

#include <oVideo/oVideoCodec.h>

// Based on "Real-time Transport Protocol" http://en.wikipedia.org/wiki/Real-time_Transport_Protocol
struct oRTPHeader
{
	unsigned Version : 2;
	unsigned Padding : 1;
	unsigned Extension : 1;

	unsigned CSRCCount : 4;

	unsigned Marker : 1;
	unsigned PayloadType : 7;

	unsigned Sequence : 16;

	unsigned Timestamp : 32;

	unsigned SSRC : 32;
};

// Based on "RTP Payload Format for VP8 Video" http://tools.ietf.org/html/draft-westin-payload-vp8-00
struct oVP8PayloadDescriptor : public oRTPHeader
{
	enum FRAGMENTATION_TYPE
	{
		NO_FRAG = 0,
		BEGIN_FRAG = 1,
		PARTIAL_FRAG = 2,
		END_FRAG = 3
	};
	unsigned Reserved : 3;
	unsigned UsingPictureID : 1;

	unsigned NonReferenceFrame : 1;
	unsigned Fragmentation : 2;
	unsigned BeginningFrame : 1; 
	
	// @oooii-kevin: This is an interpretation of the spec, we are limiting ourselves to a 7-bit pictureID (MSB is 0 as there's no extension) since the spec doesn't state we can't
	unsigned PictureIDExtention : 1; 
	unsigned PictureID : 7; 
};

// Based on "RTP Payload Format for VP8 Video" http://tools.ietf.org/html/draft-westin-payload-vp8-00
struct oVP8PayloadHeader
{
	unsigned InverseKeyFrame : 1;
	unsigned Version : 3;
	unsigned ShowFrame : 1;
	unsigned FirstPartitionSize : 19;
};


class oVideoRTPSequencerImpl : public oVideoStream
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, );
	oVideoRTPSequencerImpl(const DESC& _Desc, bool* _pSuccess);
	
	// If this ends up using a mutex for threadsafety, this should move to using
	// oDEFINE_VIDEO_MUTEXED_MAP_INTERFACE
	bool Map(MAPPED* _pMapped) override;
	void Unmap() override;
	void PushByteStream(const void* _pData, size_t _SzData) override;

private:

	oRefCount RefCount;
	DESC Desc;

	struct PACKET
	{
		void* pData;
		unsigned short seqID;
		size_t size;
	};

	struct SEQUENCE
	{
		// Status flags
		static const unsigned int INVALID = 0;
		static const unsigned int VALID_START = 1;
		static const unsigned int VALID_END_FIRST_PARTITION = 2;
		static const unsigned int VALID_FIRST_PARTITION = 4;
		static const unsigned int VALID_END_FINAL_PARTITION = 8;
		static const unsigned int VALID_FINAL_PARTITION = 16 | VALID_FIRST_PARTITION;

		static const unsigned int ACTION_CHECK_FIRST_PARTITION = VALID_END_FIRST_PARTITION | VALID_START;
		static const unsigned int ACTION_CHECK_FINAL_PARTITION = VALID_FIRST_PARTITION | VALID_END_FINAL_PARTITION;


		SEQUENCE()
		{
			Reset();
			Packets.reserve(128);
		}
		unsigned int Status;
		unsigned short Start;
		unsigned short EndFirstPartition;
		unsigned short EndFinalPartition;

		unsigned char PictureID;

		void Reset()
		{
			Status = INVALID;
			Packets.clear();
		}

		std::vector<PACKET> Packets;
	};

	static const unsigned int FrameKill = 3; // If we receiving packets from more than this number of frames we start dropping frames
	static const unsigned int MAX_FRAMES = FrameKill * 2;
	SEQUENCE Sequences[MAX_FRAMES];
	std::list<SEQUENCE*> OpenSequences;
	struct  
	{
		std::vector<unsigned char> Data;
		std::vector<unsigned char>::iterator WriteHead;
	} SeqData;
	
	bool Mapped;
	std::vector<unsigned char> FrameData;

	bool FindFrame(void** _ppFrameData, size_t* _pSzData, size_t* _pFrameIndex);
	void SequenceFrame(const std::vector<PACKET>& _Packets, unsigned short _StartIndex, unsigned short _EndIndex, void** _ppFrameData, size_t* _pSzData);
	static bool AllPacketsPresent(const std::vector<PACKET>& _Packets, unsigned short _StartIndex, unsigned short _EndIndex);
};

#endif // oVideoRTImpl_h
