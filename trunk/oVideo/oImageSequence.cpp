// $(header)
#include "oImageSequence.h"
#include <oooii/oFile.h>
#include <oooii/oPath.h>
#include <oooii/oImage.h>
#include <oooii/oSize.h>
#include <oooii/oThreading.h>
#include <oooii/oAsyncFileIO.h>

oImageSequence::ImageFile::ImageFile(const char* _Filename) : ImageNumber(0)
{
	strcpy_s(Filename, _Filename);

	char* extStart = oGetFileExtension(Filename);

	char* numberStart = extStart - 1; // -1 for '.' that begins extension
	while (numberStart >= Filename && isdigit(*numberStart))
		--numberStart;
	++numberStart; // Move back to number

	char tmp = *extStart;
	*extStart = 0;
	ImageNumber = atoi(numberStart);
	*extStart = tmp;
}

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

bool oImageSequence::ListImageFiles(const char* _Path, const oFile::DESC& _Desc, const oImageSequence::DESC& _ISDesc, std::vector<oImageSequence::ImageFile>& _ImageFiles)
{
	oImageSequence::ImageFile file(_Path);
	if ((_ISDesc.StartingFrame == -1 || file.ImageNumber >= _ISDesc.StartingFrame) && (_ISDesc.EndingFrame == -1 || file.ImageNumber <= _ISDesc.EndingFrame))
	{
		oFile::DESC fileDesc;
		oFile::GetDesc(_Path, &fileDesc);
		file.FileSize = fileDesc.Size;
		if(file.FileSize == 0)
			printf_s("File %s skipped as its size was 0\n", _Path);
		else
			_ImageFiles.push_back(file);
	}
	return true;
}

oImageSequence::oImageSequence( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, CurrentFrame(0)
	, CurrentBuffer(-1)
{
	*_pSuccess = false;

	char WildCard[_MAX_PATH];
	strcpy_s(WildCard, _pFileName);
	oTrimFileExtension(WildCard);
	strcat_s(WildCard, "*");
	strcat_s(WildCard, oGetFileExtension(_pFileName));

	oFile::EnumFiles(WildCard, oBIND(ListImageFiles, oBIND1, oBIND2, oBINDREF(Desc), oBINDREF(ImageFiles)));

	if(ImageFiles.empty())
	{
		printf_s("There were no images to compress\n");
		return;
	}

	std::sort(ImageFiles.begin(), ImageFiles.end());

	printf_s("First frame %d Last Frame %d\n",ImageFiles[0].ImageNumber, (ImageFiles.end()-1)->ImageNumber);

	long long frameCheck = ImageFiles[0].ImageNumber;
	for (size_t i = 1;i < ImageFiles.size();++i)
	{		
		if(ImageFiles[i].ImageNumber != frameCheck+1)
			printf_s("missing frame %d\n", (frameCheck+1));
		frameCheck = ImageFiles[i].ImageNumber;
	}

	Desc.CodecType = OIMAGE;

	PrefetchTask(CurrentFrame);
	
	oVideoContainer::MAPPED mapped;
	if (oImageSequence::Map(&mapped))
	{
		oRef<oImage> image;
		oImage::Create(mapped.pFrameData, mapped.DataSize, oSurface::UNKNOWN, &image);
	
		oImage::DESC imDesc;
		image->GetDesc(&imDesc);
		Desc.Dimensions.x = imDesc.Width; 
		Desc.Dimensions.y = imDesc.Height;

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

void oImageSequence::Restart() 
{ 
	IOEvent.Wait();
	CurrentFrame = 0;
	PrefetchTask(CurrentFrame);
}

bool oImageSequence::Map(MAPPED* _pMapped)
{
	IOEvent.Wait();
	CurrentBuffer = NextBuffer();

	if (HasFinished())
		return oVideoReturnEndOfFile(_pMapped);

	_pMapped->pFrameData = &PNGBuffer[CurrentBuffer][0];
	_pMapped->DataSize = oSize32(PNGBuffer[CurrentBuffer].size());
	_pMapped->DecodedFrameNumber = CurrentFrame;

	++CurrentFrame;
	if (!HasFinished())
	{
		PrefetchTask(CurrentFrame);
	}
	return true;
}

void oImageSequence::Unmap()
{
}

void oImageSequence::PrefetchTask(unsigned int _frame)
{
	IOEvent.Reset();

	int buffer = NextBuffer();
	PNGBuffer[buffer].resize(ImageFiles[_frame].FileSize);

	oAsyncFileIO::COMMAND command;
	command.CommandType = oAsyncFileIO::READ;
	command.DataIsText = false;
	command.PathToFile = ImageFiles[_frame].Filename;
	command.pDataOut = &PNGBuffer[buffer][0];
	command.SizeOfDataBlob = PNGBuffer[buffer].size();
	command.OptionalContinuation = [&](oAsyncFileIO::COMMAND& _command){IOEvent.Set();};
	oAsyncFileIO::ScheduleIO(command);
}