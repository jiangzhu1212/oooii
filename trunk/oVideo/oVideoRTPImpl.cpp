// $(header)
#include <oooii/oooii.h>
#include <oVideo/oVideoRTPStream.h>
#include "oVideoRTPImpl.h"

class oVideoRTPImpl : public oVideoRTP
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oVideoRTPImpl(const DESC &_desc, bool* _pSuccess);

	virtual bool PacketizeFrame(const void* _pFrame, unsigned int _szFrame) override;

	virtual bool DrainPacket(RTP_PACKET** _ppPacket) override;

	virtual unsigned int GetSSRC() override
	{
		return StandardDescriptor.SSRC;
	}


private:

	static unsigned int PacketOverhead()
	{
		return sizeof( oVP8PayloadDescriptor ) + sizeof( RTP_PACKET ) /*bookeeping*/;
	}
	DESC Desc;
	oRefCount RefCount;

	std::vector<unsigned char> RTPPackets;
	std::vector<unsigned char>::iterator WriteHead;
	std::vector<unsigned char>::iterator ReadHead;

	oVP8PayloadDescriptor StandardDescriptor;
};

bool oVideoRTP::Create( const DESC &_desc, oVideoRTP** _ppVideoRTP )
{
	bool success = false;
	oCONSTRUCT( _ppVideoRTP, oVideoRTPImpl( _desc, &success) );
	return success;
}

oVideoRTPImpl::oVideoRTPImpl( const DESC &_desc, bool* _pSuccess )
	: Desc(_desc)
{
	if( Desc.VideoDesc.CodecType != oVideoContainer::VP8_CODEC )
	{
		oSetLastError(EINVAL, "None supported codec type");
		return;
	}
	if( Desc.MaxPacketSize < PacketOverhead() + 4 )
	{
		oSetLastError(EINVAL, "MaxPacketSize is too small");
		return;
	}
	// Calculate the max number of packets per-frame based on raw uncompressed frame size divided by the max packet size (minus packet overhead)
	unsigned int MaxPacketsPerFrame = ( Desc.VideoDesc.Dimensions.x * Desc.VideoDesc.Dimensions.y * 4 ) / ( Desc.MaxPacketSize - PacketOverhead() );
	RTPPackets.resize(MaxPacketsPerFrame * Desc.MaxPacketSize);
	WriteHead = ReadHead = RTPPackets.begin();

	// Fill in the initial data and data that doesn't change
	StandardDescriptor.Version = 2;
	StandardDescriptor.Padding = 0;
	StandardDescriptor.Extension = 0;

	StandardDescriptor.CSRCCount = 0;

	StandardDescriptor.Marker = 0;
	StandardDescriptor.PayloadType = 0;

	// Per the RTP standard the packet sequence starts at a random number and increased from there
	StandardDescriptor.Sequence = rand(); 
	
	StandardDescriptor.Timestamp = 0;

	// Per the RTP standard this should be a random number per RTP stream (it should not change during the lifetime of the stream)
	StandardDescriptor.SSRC = rand();

	StandardDescriptor.Reserved = 0;
	StandardDescriptor.UsingPictureID = 1;
	StandardDescriptor.NonReferenceFrame = 0;
	StandardDescriptor.Fragmentation = 0;
	StandardDescriptor.BeginningFrame = 0;
	
	StandardDescriptor.PictureIDExtention = 0;
	StandardDescriptor.PictureID = Desc.StartingPictureID;

	*_pSuccess = true;
}

bool oVideoRTPImpl::PacketizeFrame(const void* _pFrame, unsigned int _szFrame)
{
	int SpaceNeeded = Desc.MaxPacketSize * ( _szFrame / ( Desc.MaxPacketSize - PacketOverhead() ) );
	// static_cast as our packet storage is never over 2 gb
	SpaceNeeded -= static_cast<int>( std::distance( WriteHead, RTPPackets.end() ) );
	SpaceNeeded -= static_cast<int>( std::distance( RTPPackets.begin(), ReadHead ) );
	if( SpaceNeeded > 0 )
		return false;

	unsigned int Timestamp = oTimerMS();

	bool FragmentedPartition = false;
	const unsigned char* pFrameBegin = (unsigned char*)(_pFrame);
	const unsigned char* pFrameEnd = pFrameBegin + _szFrame;
	const unsigned char* pFrameCurrent = pFrameBegin;
	const unsigned char* pFirstPartitionEnd = pFrameBegin;

	while(pFrameCurrent < pFrameEnd)
	{
		// This is the size of the data that will be delivered in this packet, it can be adjusted
		// depending on fragmentation
		int Payload = __min( static_cast<int>( Desc.MaxPacketSize - static_cast<unsigned int>( sizeof(oVP8PayloadDescriptor) )),static_cast<int>( pFrameEnd - pFrameCurrent ));
		if( Payload > std::distance(WriteHead, RTPPackets.end() ) )
		{
			oASSERT(false, "FIXME: Kevin, just want to see when this happens");
			// Push the write-head to the beginning
			((RTP_PACKET*)&WriteHead[0])->Size = (unsigned int)-1;
			WriteHead = RTPPackets.begin();
		}

		RTP_PACKET* pPacketDescription = (RTP_PACKET*)&WriteHead[0];
		WriteHead += sizeof(RTP_PACKET);

		oVP8PayloadDescriptor* pPayDesc = (oVP8PayloadDescriptor*)(&WriteHead[0]);
		WriteHead += sizeof(oVP8PayloadDescriptor);

		*pPayDesc = StandardDescriptor;
		// Increment sequence for next packet
		++StandardDescriptor.Sequence;

		pPayDesc->Timestamp = Timestamp;

		if( pFrameCurrent == pFrameBegin )
		{
			// This is the first packet so try to place the entire partition in it's own packet
			pPayDesc->BeginningFrame = 1;

			oVP8PayloadHeader* pPayHeader = (oVP8PayloadHeader*)(pFrameCurrent);
			int FirstPayload = pPayHeader->FirstPartitionSize + sizeof( oVP8PayloadHeader );
			pPayDesc->Fragmentation = FirstPayload > Payload ? oVP8PayloadDescriptor::BEGIN_FRAG : oVP8PayloadDescriptor::NO_FRAG;
			
			pFirstPartitionEnd = pFrameBegin + pPayHeader->FirstPartitionSize;
			if(oVP8PayloadDescriptor::NO_FRAG == pPayDesc->Fragmentation)
			{
				Payload = FirstPayload;
			}
			else
				FragmentedPartition = true;
		}
		else if( pFrameCurrent + Payload == pFrameEnd )
		{
			// This will be the last packet
			pPayDesc->Fragmentation = FragmentedPartition ? oVP8PayloadDescriptor::END_FRAG : oVP8PayloadDescriptor::NO_FRAG;
		}
		else
		{
			pPayDesc->Fragmentation = FragmentedPartition ? oVP8PayloadDescriptor::PARTIAL_FRAG : oVP8PayloadDescriptor::BEGIN_FRAG;
			FragmentedPartition = true;
		}

		// Copy payload
		memcpy(&WriteHead[0], pFrameCurrent, Payload );

		// Increment pointers and store bookeeping
		pPacketDescription->Priority = pFirstPartitionEnd > pFrameCurrent ? RTP_PACKET::PRIORITY_HIGH : RTP_PACKET::PRIORITY_NORMAL;
		pPacketDescription->Size = Payload + sizeof(oVP8PayloadDescriptor);
		pPacketDescription->pData = pPayDesc;

		WriteHead += Payload;
		pFrameCurrent += Payload;
	}

	// Increment the PictureID
	++StandardDescriptor.PictureID;

	return true;
}

bool oVideoRTPImpl::DrainPacket(RTP_PACKET** _ppPacket)
{
	if( WriteHead == ReadHead )
		return false;

	*_ppPacket = (RTP_PACKET*)&ReadHead[0];
	ReadHead += sizeof(RTP_PACKET) + (*_ppPacket)->Size;

	// Check for buffer wrap
	if( (unsigned int)-1 == ((RTP_PACKET*)&ReadHead[0])->Size )
		ReadHead = RTPPackets.begin();

	return true;
}

oVideoRTPSequencerImpl::oVideoRTPSequencerImpl( const DESC& _Desc, bool* _pSuccess )
	: Desc(_Desc)
	, Mapped(false)
{
	if( Desc.CodecType != oVideoContainer::VP8_CODEC )
	{
		oSetLastError(EINVAL, "None supported codec type");
		return;
	}

	size_t DecodedFrameSize = Desc.Dimensions.x * Desc.Dimensions.y * 4;

	SeqData.Data.resize( DecodedFrameSize * MAX_FRAMES );
	SeqData.WriteHead = SeqData.Data.begin();
	
	FrameData.resize(DecodedFrameSize);

	*_pSuccess = true;
}

bool oVideoRTPSequencerImpl::Map(MAPPED* _pMapped) 
{
	if( Mapped )
		return false;

	if( !FindFrame( &_pMapped->pFrameData, &_pMapped->DataSize, &_pMapped->DecodedFrameNumber ) )
		return false;

	Mapped = true;
	return true;
}
void oVideoRTPSequencerImpl::Unmap() 
{
	Mapped = false;
}

bool oVideoRTPSequencerImpl::FindFrame( void** _pFrameData, size_t* _pSzData, size_t* _pFrameIndex )
{
	if( OpenSequences.empty() )
		return false;

	SEQUENCE* pFront = OpenSequences.front();
	if(0 == ( SEQUENCE::VALID_FIRST_PARTITION & pFront->Status ))
		return false;

	unsigned short Start = pFront->Start;
	unsigned short End = SEQUENCE::VALID_FINAL_PARTITION == ( SEQUENCE::VALID_FINAL_PARTITION & pFront->Status ) ? pFront->EndFinalPartition : pFront->EndFirstPartition;

	SequenceFrame(pFront->Packets, Start, End, _pFrameData,  _pSzData);

	*_pFrameIndex = pFront->PictureID;

	pFront->Reset();
	OpenSequences.pop_front();

	return true;
}

void oVideoRTPSequencerImpl::PushByteStream( const void* _pData, size_t _SzData )
{
	oASSERT( _SzData < SeqData.Data.size(), "One single packet is larger than entire sequence buffer" );

	const oVP8PayloadDescriptor* pPayDesc = static_cast<const oVP8PayloadDescriptor*>(_pData);
	unsigned char picID = pPayDesc->PictureID;
	unsigned short seqID = pPayDesc->Sequence;
	SEQUENCE& seq = Sequences[ picID % MAX_FRAMES];

	// Search for duplicates so we can early out
	oFOREACH( const PACKET& other, seq.Packets )
	{
		if( other.seqID == seqID )
			return; // Duplicate packet
	}

	if( 1 == pPayDesc->BeginningFrame )
	{
		seq.Start = seqID;
		seq.Status |= SEQUENCE::VALID_START;
	}

	if( oVP8PayloadDescriptor::END_FRAG == pPayDesc->Fragmentation || oVP8PayloadDescriptor::NO_FRAG == pPayDesc->Fragmentation )
	{
		if( !(seq.Status & (SEQUENCE::VALID_END_FIRST_PARTITION | SEQUENCE::VALID_FIRST_PARTITION) ) )
		{
			seq.Status |= SEQUENCE::VALID_END_FIRST_PARTITION;
			seq.EndFirstPartition = seqID;
		}
		else
		{
			// Start by assuming this will be the end then check for swap conditions
			bool Swap = false;

			if( ( ( seqID < seq.Start ) ^ ( seq.EndFirstPartition < seq.Start ) ) == 1 )
			{ 
				// Only one of the ends wrapped
				Swap = seq.EndFirstPartition < seq.Start;
			}
			else
				Swap = seqID < seq.EndFirstPartition;

			if( Swap )
			{
				seq.EndFinalPartition = seq.EndFirstPartition;
				seq.EndFirstPartition = seqID;
			}
			else
				seq.EndFinalPartition = seqID;

			seq.Status |= SEQUENCE::VALID_END_FINAL_PARTITION;
		}
	}

	if( static_cast<int>( _SzData ) > std::distance( SeqData.WriteHead, SeqData.Data.end() ) )
		SeqData.WriteHead = SeqData.Data.begin();

	PACKET pac;
	pac.seqID = seqID;
	pac.pData = &SeqData.WriteHead[0];
	pac.size = _SzData - sizeof( oVP8PayloadDescriptor );
	SeqData.WriteHead += pac.size;

	memcpy( pac.pData, oByteAdd( _pData, sizeof( oVP8PayloadDescriptor ) ), pac.size );

	if( seq.Packets.empty() )
	{
		seq.PictureID = picID;
		OpenSequences.push_back(&seq);
		
		if( OpenSequences.size() > FrameKill )
		{
			// Drop the front if it doesn't at least have a valid first partition
			SEQUENCE* pFront = OpenSequences.front();
			if( pFront->Status & SEQUENCE::VALID_FIRST_PARTITION )
			{
				pFront->Reset();
				OpenSequences.pop_front();
			}
		}
	}

	seq.Packets.push_back(pac);

	if( SEQUENCE::ACTION_CHECK_FIRST_PARTITION == (SEQUENCE::ACTION_CHECK_FIRST_PARTITION & seq.Status) )
	{
		// See if we have a complete sequence for partial decode
		if( AllPacketsPresent(seq.Packets, seq.Start, seq.EndFirstPartition ) )
		{
			seq.Status = SEQUENCE::VALID_FIRST_PARTITION | (seq.Status & SEQUENCE::VALID_END_FINAL_PARTITION);  // Preserve the end of the final partition if it has been detected
		}
	}
	if( SEQUENCE::ACTION_CHECK_FINAL_PARTITION == (SEQUENCE::ACTION_CHECK_FINAL_PARTITION & seq.Status) )
	{
		// See if we can complete the sequence (+1 as all indices are inclusive)
		unsigned short PacketCount = 1 + seq.EndFinalPartition - seq.Start;
		if( seq.Packets.size() == PacketCount )
			seq.Status = SEQUENCE::VALID_FINAL_PARTITION;
	}
}

// Returns whether the value is between the _Start and the _End (inclusive of _Start and _End) taking into account integer rollover
template<typename T>
bool oBetweenInclusive( T _Start, T _End, T _Value )
{
	// @oooii-kevin: This cast is necessary because the MS compiler is implicitly converting the intermediate computation to 32-bit (which is probably a compiler bug)
	return (T)(_Value - _Start)  <= (T)(_End - _Start);
}

bool oVideoRTPSequencerImpl::AllPacketsPresent( const std::vector<PACKET>& _Packets, unsigned short _StartIndex, unsigned short _EndIndex )
{
	// Assuming unique packets and that the start and end index exist in the array
	int NumPackets = _EndIndex - _StartIndex;
	if( NumPackets < 2 )
		return true;

	oFOREACH( const PACKET& Packet, _Packets )
	{
		if( oBetweenInclusive( _StartIndex, _EndIndex, Packet.seqID ) )
			if( --NumPackets == 0 )
				return true;
	}

	return false;
}

void oVideoRTPSequencerImpl::SequenceFrame( const std::vector<PACKET>& _Packets, unsigned short _StartIndex, unsigned short _EndIndex, void** _ppFrameData, size_t* _pSzData )
{
	std::vector<unsigned char>::iterator writeHead = FrameData.begin();

	unsigned short p = _StartIndex - 1;
	while(p != _EndIndex) 
	{
		++p;
		oFOREACH(const PACKET& packet, _Packets)
		{
			if( packet.seqID == p )
			{
				memcpy(&writeHead[0], packet.pData, packet.size );
				writeHead += packet.size;
				break;
			}
		}
	}

	*_pSzData = std::distance(FrameData.begin(), writeHead );
	*_ppFrameData = &FrameData[0];
}

