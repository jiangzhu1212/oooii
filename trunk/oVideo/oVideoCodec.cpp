// $(header)
#include <oVideo/oVideoCodec.h>
#include <oooii/oPath.h>
#include "oVideoCodecD3D10.h"
#include "oMOVFileImpl.h"
#include "oWebmEncoder.h"
#include "oWebmStreaming.h"
#include "oVP8DecodeCPU.h"
#include "oRleDecodeCPU.h"
#include "oWebmFile.h"
#include "oRawDecodeCPU.h"  
#include "oImageDecodeCPU.h"
#include "oImageSequence.h"
#include "oVideoRTPImpl.h"
#include <oooii/oImage.h>

oAPI const oGUID& oGetGUID(threadsafe const oVideoContainer* threadsafe const * /* = 0 */)
{
	// {10666508-6D62-44ae-94AB-DD8587592191}
	static const oGUID guid = 
	{ 0x10666508, 0x6d62, 0x44ae, { 0x94, 0xab, 0xdd, 0x85, 0x87, 0x59, 0x21, 0x91 } };
	return guid;
}

oAPI const oGUID& oGetGUID(threadsafe const oVideoFile* threadsafe const * /* = 0 */)
{
	// {726C8F56-E021-4862-99B9-1F2617A56FDF}
	static const oGUID guid = 
	{ 0x726c8f56, 0xe021, 0x4862, { 0x99, 0xb9, 0x1f, 0x26, 0x17, 0xa5, 0x6f, 0xdf } };
	return guid;
}

oAPI const oGUID& oGetGUID(threadsafe const oVideoDecodeCPU* threadsafe const * )
{
	// {D1091F01-B364-4e6c-963F-ACD4E69866B1}
	static const oGUID guid = 
	{ 0xd1091f01, 0xb364, 0x4e6c, { 0x96, 0x3f, 0xac, 0xd4, 0xe6, 0x98, 0x66, 0xb1 } };
	return guid;
}

bool oVideoDecodeCPU::Create( oVideoContainer* _pContainer, oVideoDecodeCPU** _ppFrameDecoder )
{
	oVideoContainer::DESC ContainerDesc;
		_pContainer->GetDesc(&ContainerDesc);

	bool success = false;
	switch( ContainerDesc.CodecType )
	{
	case oVideoContainer::RLE_CODEC:
		{
			oCONSTRUCT( _ppFrameDecoder, oRleDecodeCPU( _pContainer, &success) );
		}
		break;

	case oVideoContainer::VP8_CODEC:
		{
			oCONSTRUCT( _ppFrameDecoder, oVP8DecodeCPU( _pContainer, &success) );
		}
		break;

	case oVideoContainer::RAW_420:
		{
			oCONSTRUCT( _ppFrameDecoder, oRawDecodeCPU( _pContainer, &success) );
		}
		break;

	case oVideoContainer::OIMAGE:
		{
			oCONSTRUCT( _ppFrameDecoder, oImageDecodeCPU( _pContainer, &success) );
		}
		break;
	}

	return success;
}

bool oVideoDecodeD3D10::Create(DESC _desc, oVideoContainer** _ppContainers, size_t _NumContainers, oVideoDecodeD3D10** _ppDecoder)
{
	// @oooii-kevin: oVideoDecodeD3D10 simply wraps a oVideoDecodeCPU with GPU accelerated YUV conversion
	std::vector<oRef<oVideoDecodeCPU>> CPUDecoders;
	CPUDecoders.resize(_NumContainers);
	for (unsigned int i = 0;i < _NumContainers; ++i)
	{
		if( !oVideoDecodeCPU::Create(_ppContainers[i], &(CPUDecoders[i]) ) )
			return false; //oVideoDecodeCPU will set last error
	}

	bool success = false;
	oCONSTRUCT( _ppDecoder, oVideoDecodeD3D10SimpleYUV( _desc, CPUDecoders, &success ) );
	return success;
}

bool oVideoFile::Create( const char* _pFileName, const oVideoContainer::DESC& _Desc, oVideoFile** _ppVideoFile )
{
	oVideoContainer::DESC Desc = _Desc;
	if( oVideoContainer::INVALID_CONTAINER == Desc.ContainerType)
	{
		const char* pFileExt = oGetFileExtension( _pFileName );
		if( 0 == _stricmp( pFileExt, ".mov") )
		{
			Desc.ContainerType = oVideoContainer::MOV_CONTAINER;
		}
		else if( 0 == _stricmp( pFileExt, ".webm") )
		{
			Desc.ContainerType = oVideoContainer::WEBM_CONTAINER;
		}
		else if( oImage::IsSupportedFileType(_pFileName))
		{
			Desc.ContainerType = oVideoContainer::IMAGE_SEQUENCE_CONTAINER;
		}
		else
		{
			oSetLastError(EBADFD, "Unknown file extension");
			return false;
		}
	}

	bool success = false;
	switch( Desc.ContainerType )
	{
	case oVideoContainer::MOV_CONTAINER:
		{
			oCONSTRUCT( _ppVideoFile, oMOVFileImpl( _pFileName, Desc, &success) );
		}
		break;
	case oVideoContainer::WEBM_CONTAINER:
		{
			oCONSTRUCT( _ppVideoFile, oWebmFile( _pFileName, Desc, &success) );
		}
		break;
	case oVideoContainer::IMAGE_SEQUENCE_CONTAINER:
		{
			oCONSTRUCT( _ppVideoFile, oImageSequence( _pFileName, Desc, &success) );
		}
		break;
	}

	return success;
}

bool oVideoEncodeCPU::Create( const DESC &_desc, oVideoEncodeCPU** _ppVideoEncoder )
{
	bool success = false;
	switch( _desc.ContainerType )
	{
	case oVideoEncodeCPU::WEBM_CONTAINER:
		{
			oCONSTRUCT( _ppVideoEncoder, oWebmEncoder( _desc, &success) );
		}
		break;
	default:
		oSetLastError(EINVAL,"Unsupported container format requested for oVideoEncodeCPU. Only webm is supported currently.");
		break;
	}

	return success;
}

bool oVideoStream::Create( const oVideoContainer::DESC& _Desc, oVideoStream** _ppVideoStream )
{
	bool success = false;
	switch( _Desc.ContainerType )
	{
		case oVideoContainer::WEBM_CONTAINER:
		{
			oCONSTRUCT( _ppVideoStream, oWebmStreaming( _Desc, &success) );
		}
		break;

		case oVideoContainer::OOII_RTP_CONTAINER:
		{
			oCONSTRUCT( _ppVideoStream, oVideoRTPSequencerImpl( _Desc, &success ) );
		}
		break;
		
		default:
			oSetLastError(EINVAL,"Unsupported container format requested for oVideoStream");
			break;
	}

	return success;
}




