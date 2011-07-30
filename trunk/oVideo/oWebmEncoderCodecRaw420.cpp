// $(header)
#include "oWebmEncoderCodecRaw420.h"
#include <oooii/oByte.h>

const oGUID& oGetGUID( threadsafe const oWebmEncoderCodecRaw420* threadsafe const * )
{
	// {9318ce96-9a49-4aed-ad54-046f70ef75fd}
	static const oGUID oIIDoWebmEncoderCodecRaw420 = { 0x9318ce96, 0x9a49, 0x4aed, { 0xad, 0x54, 0x04, 0x6f, 0x70, 0xef, 0x75, 0xfd } };
	return oIIDoWebmEncoderCodecRaw420; 
}

oWebmEncoderCodecRaw420::oWebmEncoderCodecRaw420(const oVideoEncodeCPU::DESC &_Desc, bool* _pSuccess) : Desc(_Desc)
{
	*_pSuccess = false;
	
	CodecEbml.push_back(0x86);
	CodecEbml.push_back(0x8c);
	CodecEbml.push_back('o');
	CodecEbml.push_back('o');
	CodecEbml.push_back('o');
	CodecEbml.push_back('i');
	CodecEbml.push_back('i');
	CodecEbml.push_back('_');
	CodecEbml.push_back('r');
	CodecEbml.push_back('a');
	CodecEbml.push_back('w');
	CodecEbml.push_back('4');
	CodecEbml.push_back('2');
	CodecEbml.push_back('0');

	CodecNameEbml.push_back(0x25);
	CodecNameEbml.push_back(0x86);
	CodecNameEbml.push_back(0x88);
	CodecNameEbml.push_back(0x8c);
	CodecNameEbml.push_back('o');
	CodecNameEbml.push_back('o');
	CodecNameEbml.push_back('o');
	CodecNameEbml.push_back('i');
	CodecNameEbml.push_back('i');
	CodecNameEbml.push_back('_');
	CodecNameEbml.push_back('r');
	CodecNameEbml.push_back('a');
	CodecNameEbml.push_back('w');
	CodecNameEbml.push_back('4');
	CodecNameEbml.push_back('2');
	CodecNameEbml.push_back('0');
		
	*_pSuccess = true;
}

oWebmEncoderCodecRaw420::~oWebmEncoderCodecRaw420()
{
}

void oWebmEncoderCodecRaw420::Encode( const oSurface::YUV420& _Frame, void* _pDataStream, size_t _StreamSize, size_t* _pDataWrittenToStream, 
	unsigned int _frameStamp, unsigned int _frameDuration, bool _forceIFrame, FrameDesc &_desc )
{
	size_t ySize = _Frame.YPitch*Desc.Dimensions.y;
	size_t uvSize = _Frame.UVPitch*Desc.Dimensions.y/2;
	oASSERT(_StreamSize >= (ySize + 2*uvSize), "Buffer supplied to webm raw 420 codec wasn't large enough");

	memcpy( _pDataStream, _Frame.pY,ySize);
	memcpy( oByteAdd(_pDataStream, ySize), _Frame.pU, uvSize);
	memcpy( oByteAdd(_pDataStream, ySize + uvSize), _Frame.pV, uvSize);

	*_pDataWrittenToStream = ySize + 2*uvSize;

	_desc.Invisible = false;
	_desc.Keyframe = true; //every frame is a keyframe in raw format
	_desc.CanDiscard = true; //since frames are never dependent on each other
}

void oWebmEncoderCodecRaw420::EncodeFirstPass(const oSurface::YUV420& _Frame, unsigned int _frameStamp, unsigned int _frameDuration, bool _forceIFrame /*= false*/)
{
	oASSERT(Desc.TwoPass,"Tried to encode first pass when encoder is not set up for 2 pass encoding");

}

void oWebmEncoderCodecRaw420::StartSecondPass(unsigned int _frameStamp, unsigned int _frameDuration)
{
	
}