#include "oImageSequence.h"
#include <oooii/oFile.h>
#include <oooii/oPath.h>
#include <oooii/oImage.h>
#include <oooii/oSize.h>

bool oImageSequence::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	if (_InterfaceID == oGetGUID<oVideoContainer>())
	{
		Reference();
		*_ppInterface = this;
		return true;
	}

	oSetLastError(ENOENT);
	return false;
}

oImageSequence::oImageSequence( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess) : Desc(_Desc), CurrentFrame(0)
{
	*_pSuccess = false;

	oFile::DESC fileDesc;
	void *fileSearchContext;
	char FilePath[_MAX_PATH];

	char WildCard[_MAX_PATH];
	strcpy_s(WildCard, _pFileName);
	oTrimFileExtension(WildCard);
	strcat_s(WildCard, "*");
	strcat_s(WildCard, oGetFileExtension(_pFileName));

	if(!oFile::FindFirst(&fileDesc, FilePath, WildCard, &fileSearchContext))
	{
		oSetLastError(EINVAL, "Could not find file matching wildcard %s", WildCard);
		return;
	}

	do 
	{
		ImageFile file(FilePath);
		if((Desc.StartingFrame == -1 || file.ImageNumber >= Desc.StartingFrame) && (Desc.EndingFrame == -1 || file.ImageNumber <= Desc.EndingFrame))
			ImageFiles.push_back(file);
	} while (oFile::FindNext(&fileDesc, FilePath, fileSearchContext));

	oFile::CloseFind(fileSearchContext);

	std::sort(ImageFiles.begin(), ImageFiles.end());

	long long frameCheck = ImageFiles[0].ImageNumber;
	for (int i = 1;i < ImageFiles.size();++i)
	{		
		if(ImageFiles[i].ImageNumber != frameCheck+1)
			printf_s("missing frame %d\n", (frameCheck+1));
		frameCheck = ImageFiles[i].ImageNumber;
	}

	Desc.CodecType = OIMAGE;

	oVideoContainer::MAPPED mapped;
	if (oImageSequence::Map(&mapped))
	{
		oRef<oImage> image;
		oImage::Create(mapped.pFrameData, mapped.DataSize, oSurface::UNKNOWN, &image);
	
		oImage::DESC imDesc;
		image->GetDesc(&imDesc);
		Desc.Width = imDesc.Width; 
		Desc.Height = imDesc.Height;

		Restart(); // only read a frame to get "header" info. so reset
		*_pSuccess = true;

		oImageSequence::Unmap();
	}

	else
	{
		oSetLastError(EINVAL, "Could not get image info from first frame");
		*_pSuccess = false;
	}
}

oImageSequence::~oImageSequence()
{

}

oImageSequence::ImageFile::ImageFile(char *_pFileName) : ImageNumber(0)
{
	strcpy_s(FileName, _pFileName);

	char *temp = oTrimFileExtension(_pFileName);
	char *numberStart = temp + strnlen_s(_pFileName, _MAX_PATH);
	if(numberStart == temp)
		return;
	--numberStart;

	while(isdigit(*numberStart))
		--numberStart;

	++numberStart;

	ImageNumber = atoi(numberStart);
}

bool oImageSequence::MapNOLOCK(MAPPED* _pMapped)
{
	oASSERT(!MemoryMappedFrame, "Tried to map an already mapped frame");

	if (HasFinished())
		return oVideoReturnEndOfFile(_pMapped);

	if(!oMemoryMappedFile::Create(ImageFiles[CurrentFrame].FileName, &MemoryMappedFrame ))
	{
		oSetLastError(EINVAL, "Could not map an image sequence file %s", ImageFiles[CurrentFrame].FileName);
		oVideoNullMapped(_pMapped);
		return false;
	}

	_pMapped->pFrameData = MemoryMappedFrame->Map(0, 0);
	_pMapped->DataSize = oSize32(MemoryMappedFrame->GetFileSize());
	_pMapped->DecodedFrameNumber = CurrentFrame;
	return true;
}

void oImageSequence::UnmapNOLOCK()
{
	if(MemoryMappedFrame)
	{
		MemoryMappedFrame->Unmap();
		MemoryMappedFrame = nullptr;
		++CurrentFrame;
	}
}
