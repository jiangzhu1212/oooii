// $(header)
#include "oRleDecodeCPU.h"
#include <oooii/oMemory.h>
#include <oooii/oStdio.h>

const oGUID& oGetGUID( threadsafe const oRleDecodeCPU* threadsafe const * )
{
	// {13F1AD65-1CA2-41b6-A17F-B934DCBFA69C}
	static const oGUID oIIDRleDecodeCPU = { 0x13f1ad65, 0x1ca2, 0x41b6, { 0xa1, 0x7f, 0xb9, 0x34, 0xdc, 0xbf, 0xa6, 0x9c } };
	return oIIDRleDecodeCPU;
}

oRleDecodeCPU::oRleDecodeCPU( threadsafe oVideoContainer* _pContainer,  bool* _pSuccess) : IsInitialized(false), Container(_pContainer)
{
	oVideoContainer::DESC Desc;
	Container->GetDesc(&Desc);
	BGRADecodeFrame.resize(Desc.Width*Desc.Height);
	*_pSuccess = true;
}

oRleDecodeCPU::~oRleDecodeCPU()
{
	
}

bool oRleDecodeCPU::DecodeInternal(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber)
{
	oVideoContainer::DESC Desc;
	Container->GetDesc(&Desc);

	oASSERT(Desc.ContainerType == oVideoContainer::MOV_CONTAINER && Desc.CodecType == oVideoContainer::RLE_CODEC,"Tried to use the RLE decoder with a container that doesn't contain rle data");

	bool valid;
	Container->MapFrame(&Data, &DataSize, &valid, _decodedFrameNumber);
	if(!valid || DataSize == 0)
	{
		return false;
	}
	DataIndex = 0;
	 
	unsigned short Header = 0;
	ReadRleSwap(Header);
	 
 	unsigned short SkipLines = 0;
 	unsigned short NumLines = static_cast<unsigned short>(Desc.Height);
 	unsigned int Line = 0;
 	if (Header == 0x08)
 	{
 		ReadRleSwap(SkipLines);
 		DataIndex += 2; //unused data
 
 		ReadRleSwap(NumLines);
		DataIndex += 2; //unused data
 	}
 	if(DataIndex >= DataSize)
 	{
 		oSetLastError(EINVAL,"Failed decoding Rle. Failed read header and/or skip lines");
 		return false;
 	}
 
 	unsigned int DecodeOffset = Desc.Width*SkipLines;
 
 	struct RGBColor
 	{
 		unsigned char Red;
 		unsigned char Green;
 		unsigned char Blue;
 	};
 	RGBColor* oRESTRICT pixelRun = (RGBColor*)oSTACK_ALLOC(128*sizeof(RGBColor),4);
 
 	unsigned char SkipCode = 0;
 	ReadRle(SkipCode);
 	while (SkipCode != 0 && Line < NumLines)
 	{
 		DecodeOffset += SkipCode-1;
 
 		char RleCode = 0;
 		ReadRle(RleCode);
 		if(RleCode == 0 || RleCode == -1)
 		{
 			ReadRle(SkipCode);
 			if(RleCode == -1)
 				Line++;
 		}
 		else if(RleCode > 0)
 		{
			if(DataIndex+sizeof(RGBColor)*RleCode < DataSize)
			{
				memcpy(pixelRun,oByteAdd(Data,DataIndex),sizeof(RGBColor)*RleCode);
				DataIndex += sizeof(RGBColor)*RleCode;
				for (int i = 0;i < RleCode;++i)
				{
					BGRADecodeFrame[DecodeOffset] = (pixelRun[i].Blue) | (pixelRun[i].Green<<8) | (pixelRun[i].Red<<16) | 0xFF000000;
					DecodeOffset++;
				}
			}
 		}
 		else
 		{
 			RGBColor pixel;
 			ReadRle(pixel);
 			unsigned int BGRAColor = (pixel.Blue) | (pixel.Green<<8) | (pixel.Red<<16) | 0xFF000000;
 			oMemset4(&BGRADecodeFrame[DecodeOffset],BGRAColor,(-RleCode)*sizeof(unsigned int));
 			DecodeOffset += -RleCode;
 		}
 		if(DataIndex >= DataSize)
 		{
 			oSetLastError(EINVAL,"Failed decoding Rle. failed while decoding rle data");
 			return false;
 		}
 	}
 
 	// @oooii-eric would probably be faster to convert to yuv for each line as its decoded, for better cache locality. but early testing didn't show any improvement.
 	oSurface::convert_B8G8R8A8_UNORM_to_YUV420(Desc.Width, Desc.Height, reinterpret_cast<unsigned char*>(&BGRADecodeFrame[0]), Desc.Width*sizeof(unsigned int), _pFrame );

	Container->UnmapFrame();

	return true;
}

bool oRleDecodeCPU::Decode(oSurface::YUV420* _pFrame, size_t *_decodedFrameNumber) threadsafe
{
	oMutex::ScopedLock lock(DecodeLock);
	return thread_cast<oRleDecodeCPU*>(this)->DecodeInternal(_pFrame, _decodedFrameNumber);
};

bool oRleDecodeCPU::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	if( oGetGUID<oRleDecodeCPU>() == _InterfaceID || oGetGUID<oVideoDecodeCPU>() == _InterfaceID )
	{
		Reference();
		*_ppInterface = this;
		return true;
	}
	if( oGetGUID<oVideoContainer>() == _InterfaceID )
	{
		Container->Reference();
		*_ppInterface = Container;
		return true;
	}

	return false;
}