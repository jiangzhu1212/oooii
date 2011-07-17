// $(header)
#pragma once
#ifndef oWebmEncoder_h
#define oWebmEncoder_h
#pragma warning(disable:4505)
#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"

#include <oooii/oVideoCodec.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oThreading.h>
#include <vector>
#include "oWebmEncoderCodec.h"

struct EBMLClusterCache
{
	std::vector<unsigned char> Cache;
	unsigned int ClusterSizeOffset;
	unsigned int ClusterSizeBase;
	unsigned int ClusterSizeNumBytes;
	unsigned int SimpleBlockSizeOffset;
	unsigned int SimpleblockSizeBase;
	unsigned int SimpleSizeNumBytes;
	unsigned int TimeStampOffset;
	unsigned int FlagsOffset;
};

//Currently this does not populate all webm fields. Only the ones necessary for stream webm files. This means if you generate
//	a file using this, seeking will be very slow or perhaps not even work depending on your player. In particular the segment size
//	is set to infinite, the duration is not set to anything meaningful, and there is no queue section.
//
// A segment info section is also missing. It doesn't add anything useful for our purposes, but some players may not like that its missing.
//
// For m ore info on the ebml format and webm see these links.
// http://www.matroska.org/technical/specs/index.html
// http://www.webmproject.org/code/specs/container/
class oWebmEncoder : public oVideoEncodeCPU
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oWebmEncoder>());
	oWebmEncoder( const oVideoEncodeCPU::DESC& _Desc, bool* _pSuccess);
	~oWebmEncoder();

	unsigned int GetMaxHeaderSize() const override {return 128;} //could be more precise here if we wanted.
	void GetHeader(void* _pDataStream, size_t _StreamSize, size_t* _pDataWrittenToStream) override;
	void Encode(const oSurface::YUV420& _Frame, void* _pDataStream, size_t _StreamSize, size_t* _pDataWrittenToStream, bool _forceIFrame = false) override;
	void EncodeFirstPass(const oSurface::YUV420& _Frame, bool _forceIFrame = false) override;
	void StartSecondPass() override;

private:
	void SetClusterHeader(void *_buffer,const oWebmEncoderCodec::FrameDesc &_desc);
	unsigned int GetFrameStamp() const { return static_cast<unsigned int>((FrameCount*Desc.FrameTimeNumerator*1000)/Desc.FrameTimeDenominator);} //in ms
	unsigned int GetFrameDuration() const { return static_cast<unsigned int>((Desc.FrameTimeNumerator*1000)/Desc.FrameTimeDenominator);} //in ms

	oRefCount RefCount;
	DESC Desc;
	std::vector<unsigned char> Header;
	int64_t FrameCount;
	unsigned int MaxPayloadSize;
	EBMLClusterCache ClusterCache;      
	oRef<oWebmEncoderCodec> Codec;
};

#endif // oWebmEncoder_h