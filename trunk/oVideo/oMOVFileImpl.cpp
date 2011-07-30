// $(header)
#include "oMOVFileImpl.h"
#include <oooii/oMemory.h>
#include <oooii/oMath.h>
#include <oooii/oString.h>
#include <oooii/oSTL.h>
#include <oooii/oCPU.h>
#include <oooii/oFile.h>

struct oAtomMediaHandler
{
	char Version;
	char Flags[3];
	char ComponentType[4];
	char ComponentSubType[4];
	void FixupEndian() {}
	//There are more fields in this atom, but no need to read them as they are all reserved for future use for now.
};

struct oAtomMediaHeader
{
	char Version;
	char Flags[3];
	unsigned int CreationTime;
	unsigned int ModificationTime;
	unsigned int TimeScale;
	unsigned int Duration;
	char LanguageCode[2];
	unsigned short Quality;
	void FixupEndian()
	{
		CreationTime = oByteSwap(CreationTime);
		ModificationTime = oByteSwap(ModificationTime);
		TimeScale = oByteSwap(TimeScale);
		Duration = oByteSwap(Duration);
		Quality = oByteSwap(Quality);
	}
};

struct oAtomVideoSampleDescription
{
	unsigned int Size;
	char DataFormat[4];
	char Reserved[6];
	unsigned short DataReferenceIndex;
	unsigned short Version;
	unsigned short RevisionLevel;
	char Vendor[4];
	unsigned int TemporalQuality;
	unsigned int SpatialQuality;
	unsigned short Width;
	unsigned short Height;
	unsigned int HorizontalResolution;
	unsigned int VerticalResolution;
	unsigned int DataSize;
	unsigned short FrameCount;
	char CompressorName[32];
	unsigned short Depth;
	unsigned short ColorTableID;
	void FixupEndian()
	{
		Size = oByteSwap(Size);
		DataReferenceIndex = oByteSwap(DataReferenceIndex);
		Version = oByteSwap(Version);
		RevisionLevel = oByteSwap(RevisionLevel);
		TemporalQuality = oByteSwap(TemporalQuality);
		SpatialQuality = oByteSwap(SpatialQuality);
		Width = oByteSwap(Width);
		Height = oByteSwap(Height);
		HorizontalResolution = oByteSwap(HorizontalResolution);
		VerticalResolution = oByteSwap(VerticalResolution);
		DataSize = oByteSwap(DataSize);
		FrameCount = oByteSwap(FrameCount);
		Depth = oByteSwap(Depth);
		ColorTableID = oByteSwap(ColorTableID);
	}
};

struct oAtomVideoSampleDescriptions
{
	char Version;
	char Flags[3];
	unsigned int NumberOfEntries;
	void FixupEndian()
	{
		NumberOfEntries = oByteSwap(NumberOfEntries);
	}
};

struct oAtomSampleToChunkEntry
{
	unsigned int FirstChunk;
	unsigned int SamplesPerChunk;
	unsigned int SampleDescriptionID;
	void FixupEndian()
	{
		FirstChunk = oByteSwap(FirstChunk);
		SamplesPerChunk = oByteSwap(SamplesPerChunk);
		SampleDescriptionID = oByteSwap(SampleDescriptionID);
	}
};

struct oAtomSampleToChunk
{
	char Version;
	char Flags[3];
	unsigned int NumberOfEntries;
	void FixupEndian()
	{
		NumberOfEntries = oByteSwap(NumberOfEntries);
	}
};

struct oAtomTimeToSampleEntry
{
	unsigned int SampleCount;
	unsigned int SampleDuration;
	void FixupEndian()
	{
		SampleCount = oByteSwap(SampleCount);
		SampleDuration = oByteSwap(SampleDuration);
	}
};

struct oAtomTimeToSample
{
	char Version;
	char Flags[3];
	unsigned int NumberOfEntries;
	void FixupEndian()
	{
		NumberOfEntries = oByteSwap(NumberOfEntries);
	}
};

struct oAtomChunkOffset
{
	char Version;
	char Flags[3];
	unsigned int NumberOfEntries;
	void FixupEndian()
	{
		NumberOfEntries = oByteSwap(NumberOfEntries);
	}
};

oMOVFileImpl::oMOVFileImpl(const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess)
	:Desc(_Desc), File(NULL)
{
	*_pSuccess = false;
	strcpy_s(FileName, _pFileName);
	
	if(!oFile::Open(FileName, false, false, &File)) 
	{
		oSetLastError(EINVAL,"Could not open file %s",FileName);
		return;
	}

	//All MOV files are required to have a ftyp atom.
	std::vector<oAtomHeader> searchAtoms;
	FindAtoms(searchAtoms,"ftyp",NULL);
	if(searchAtoms.size() != 1)
	{
		oSetLastError(EINVAL,"MOV file %s missing required ftyp atom",FileName);
		return;
	}

	//Find the 'trak' atom that has the video info.
	searchAtoms.clear();
	FindAtoms(searchAtoms,"moov.trak",NULL);

	oAtomHeader VideoTrak;
	bool foundVideoTrak = false;
	oFOREACH(const oAtomHeader &trak,searchAtoms)
	{
		std::vector<oAtomHeader> mediaHandlerAtoms;
		FindAtoms(mediaHandlerAtoms,"mdia.hdlr",&trak);
		oAtomMediaHandler mediaHandlerAtom;
		ReadAtom(mediaHandlerAtoms[0],mediaHandlerAtom);

		if((strncmp(mediaHandlerAtom.ComponentType,"mhlr",4) == 0) && (strncmp(mediaHandlerAtom.ComponentSubType,"vide",4) == 0))
		{
			foundVideoTrak = true; 
			VideoTrak = trak;
		}
	}

	if(!foundVideoTrak)
	{
		oSetLastError(EINVAL,"MOV file %s missing video trak, only video mov's are currently supported",FileName);
		return;
	}

	//get the time scale and duration from the media atom
	searchAtoms.clear();
	FindAtoms(searchAtoms,"mdia.mdhd",&VideoTrak);
	if(searchAtoms.size() != 1)
	{
		oSetLastError(EINVAL,"MOV file %s missing required media header atom",FileName);
		return;
	}
	oAtomMediaHeader mediaHeader;
	if(!ReadAtom(searchAtoms[0],mediaHeader))
	{
		oSetLastError(EINVAL,"MOV file %s: Couldn't read media header atom",FileName);
		return;
	}
	TimeScale = mediaHeader.TimeScale;
	Duration = mediaHeader.Duration;
	Desc.FrameTimeDenominator = TimeScale;

	//Look at sample descriptions. mostly to verify that the samples are in a format 
	//	the Decode function can understand. Also the the Width and Height from these atoms.
	searchAtoms.clear();
	FindAtoms(searchAtoms,"mdia.minf.stbl.stsd",&VideoTrak);
	if(searchAtoms.size() != 1)
	{
		oSetLastError(EINVAL,"MOV file %s missing sample descriptions",FileName);
		return;
	}
	 
	oAtomVideoSampleDescriptions sampleDescriptions;
	if(!ReadAtom(searchAtoms[0],sampleDescriptions))
	{
		oSetLastError(EINVAL,"MOV file %s: Couldn't read sample descriptions atom",FileName);
		return;
	}
	if(sampleDescriptions.NumberOfEntries != 1)
	{
		oSetLastError(EINVAL,"MOV file %s: There should be exactly one sample description for a Video track",FileName);
		return;
	}

	oAtomHeader sampleDescriptionHeader;
	sampleDescriptionHeader.DataOffset = oFile::Tell(File);
	oAtomVideoSampleDescription sampleDescription;
	if(!ReadAtom(sampleDescriptionHeader,sampleDescription))
	{
		oSetLastError(EINVAL,"MOV file %s: Couldn't read ample description atom",FileName);
		return;
	}
	if(strncmp(sampleDescription.DataFormat,"rle ",4) != 0)
	{
		oSetLastError(EINVAL,"MOV file %s: Wrong compression type, only rle compression is supported",FileName);
		return;
	}
	// Set the type
	Desc.CodecType = RLE_CODEC;

	if(sampleDescription.Depth != 24)
	{
		oSetLastError(EINVAL,"MOV file %s: Only 24bit Video is currently supported",FileName);
		return;
	}
	if(sampleDescription.FrameCount != 1)
	{
		oSetLastError(EINVAL,"MOV file %s: Currently MOV files must be uniform. i.e. entire file must be the same format, compressions, and frame rate",FileName);
		return;
	}

	Desc.Dimensions.x = sampleDescription.Width;
	Desc.Dimensions.y = sampleDescription.Height;

	//Get stsc atom just for verification. We only support mov's that have one sample per chunk.
	searchAtoms.clear();
	FindAtoms(searchAtoms,"mdia.minf.stbl.stsc",&VideoTrak);
	if(searchAtoms.size() != 1)
	{
		oSetLastError(EINVAL,"MOV file %s: Video track is missing required Sample To Chunk table",FileName);
		return;
	}
	oAtomSampleToChunk sampleToChunk;
	if(!ReadAtom(searchAtoms[0],sampleToChunk))
	{
		oSetLastError(EINVAL,"MOV file %s: Failed to read Sample to Chunk table location",FileName);
		return;
	}
	if(sampleToChunk.NumberOfEntries != 1)
		return;
	oAtomHeader sampleToChunkEntryHeader;
	sampleToChunkEntryHeader.DataOffset = oFile::Tell(File);
	oAtomSampleToChunkEntry sampleToChunkEntry;
	if(!ReadAtom(sampleToChunkEntryHeader,sampleToChunkEntry))
	{
		oSetLastError(EINVAL,"MOV file %s: Failed to read Sample to Chunk table location",FileName);
		return;
	}
	if(sampleToChunkEntry.FirstChunk != 1 || sampleToChunkEntry.SamplesPerChunk != 1)
	{
		oSetLastError(EINVAL,"MOV file %s: Currently MOV files must be uniform. i.e. entire file must be the same format, compressions, and frame rate",FileName);
		return;
	}

	//Get timing info from the stts atom. Also more verification
	searchAtoms.clear();
	FindAtoms(searchAtoms,"mdia.minf.stbl.stts",&VideoTrak);
	if(searchAtoms.size() != 1)
	{
		oSetLastError(EINVAL,"MOV file %s: Could not find required Time to Sample atom",FileName);
		return;
	}
	oAtomTimeToSample timeToSample;
	if(!ReadAtom(searchAtoms[0],timeToSample))
	{
		oSetLastError(EINVAL,"MOV file %s: Failed to read Time to Sample atom",FileName);
		return;
	}
	if(timeToSample.NumberOfEntries != 1) 
	{
		oSetLastError(EINVAL,"MOV file %s: Currently MOV files must be uniform. i.e. entire file must be the same format, compressions, and frame rate",FileName);
		return;
	}
	oAtomHeader timeToSampleEntryHeader;
	timeToSampleEntryHeader.DataOffset = oFile::Tell(File);
	oAtomTimeToSampleEntry timeToSampleEntry;
	if(!ReadAtom(timeToSampleEntryHeader,timeToSampleEntry))
	{
		oSetLastError(EINVAL,"MOV file %s: Failed to read Time to Sample atom",FileName);
		return;
	}
	NumberOfFrames = timeToSampleEntry.SampleCount;
	FrameDuration = timeToSampleEntry.SampleDuration;
	Desc.FrameTimeNumerator = FrameDuration;

	//Get the table that gives the location in the file for the data for each frame. Table can be a list of 32 bit or 64 bit offsets.
	searchAtoms.clear();
	FindAtoms(searchAtoms,"mdia.minf.stbl.stco",&VideoTrak);
	if(searchAtoms.size() == 1)
		Is64BitFile = false;
	else
	{
		searchAtoms.clear();
		FindAtoms(searchAtoms,"mdia.minf.stbl.co64",&VideoTrak);
		if(searchAtoms.size() == 1)
			Is64BitFile = true;
		else
		{
			oSetLastError(EINVAL,"MOV file %s: Failed to find required stco(smaller than 4G files) or co64(larger than 4G files) atom",FileName);
			return;
		}
	}
	oAtomChunkOffset chunkOffsets;
	if(!ReadAtom(searchAtoms[0],chunkOffsets))
	{
		oSetLastError(EINVAL,"MOV file %s: Failed to read Chunk offsets atom",FileName);
		return;
	}
	if(chunkOffsets.NumberOfEntries != NumberOfFrames)
	{
		oSetLastError(EINVAL,"MOV file %s: chunk offsets atom did not have the correct number of entries for the number of frames in this video",FileName);
		return;
	}
	FrameFileOffsets = oFile::Tell(File);

	if(Desc.StartingFrame == -1)
		CurrentFrame = 0;
	else
		CurrentFrame = Desc.StartingFrame;

	if(!oMemoryMappedFile::Create(FileName,&MemoryMappedFrame))
	{
		oSetLastError(EINVAL,"MOV file %s: Couldn't create a memory mapping to the file",FileName);
		return;
	}

	bool succeeded = true;

	unsigned long long FrameDataOffset;
	for(unsigned int i = 0;i < NumberOfFrames;++i)
	{
		if(Is64BitFile)
		{
			succeeded &= oFile::Seek(File, FrameFileOffsets+i*sizeof(unsigned long long));
			succeeded &= FRead(FrameDataOffset);
			FrameDataOffset = oByteSwap(FrameDataOffset);
		}
		else
		{
			succeeded &= oFile::Seek(File, FrameFileOffsets+i*sizeof(unsigned int));
			unsigned int FrameDataOffset32;
			succeeded &= FRead(FrameDataOffset32);
			FrameDataOffset32 = oByteSwap(FrameDataOffset32);
			FrameDataOffset = FrameDataOffset32;
		}
		FileOffsetTable.push_back(FrameDataOffset);
	}

	if(!succeeded)
	{
		oSetLastError(EINVAL,"MOV file %s: failed to read file offset table",FileName);
		return;
	}

	SortedFileOffsetTable = FileOffsetTable;
	SortedFileOffsetTable.push_back(MemoryMappedFrame->GetFileSize()); //one extra element which should get sorted last, for getting size of last frame.
	std::sort(SortedFileOffsetTable.begin(),SortedFileOffsetTable.end());

	*_pSuccess = true;
}

oMOVFileImpl::~oMOVFileImpl()
{
	if (File)
		oFile::Close(File);
}

bool oMOVFileImpl::ReadHeader(oAtomHeader &_header)
{
	unsigned int smallSize;
	unsigned int HeaderSize = sizeof(smallSize)+sizeof(_header.Type);

	if(!FRead(smallSize))
	{
		oSetLastError(EINVAL,"MOV file %s: Failed to read an atom header. Corrupted MOV file?",FileName);
		return false;
	}
	if(!FRead(_header.Type))
	{
		oSetLastError(EINVAL,"MOV file %s: Failed to read an atom header. Corrupted MOV file?",FileName);
		return false;
	}

	if(oFile::AtEndOfFile(File))
		return false;

	smallSize = oByteSwap(smallSize);
	if(smallSize == 1)
	{
		if(!FRead(_header.Size))
		{
			oSetLastError(EINVAL,"MOV file %s: Failed to read a 64 bit atom size. Corrupted MOV file?",FileName);
			return false;
		}
		_header.Size = oByteSwap(_header.Size);
		HeaderSize += sizeof(_header.Size);
	}
	else
		_header.Size = smallSize;

	_header.DataOffset = oFile::Tell(File);
	_header.EndDataOffset = _header.DataOffset+_header.Size-HeaderSize;

	return true;
}

template<typename T>
bool oMOVFileImpl:: ReadAtom(const oAtomHeader &_atom,T &_result)
{
	if(!oFile::Seek(File, _atom.DataOffset))
	{
		oSetLastError(EINVAL,"MOV file %s: Failed to seek to a data atom. Corrupted MOV File?",FileName);
		return false;
	}
	if(!FRead(_result))
	{
		oSetLastError(EINVAL,"MOV file %s: Failed to read a data atom. Corrupted MOV File?",FileName);
		return false;
	}
	_result.FixupEndian();
	return true;
}


void oMOVFileImpl::FindAtoms(std::vector<oAtomHeader> &_results,const std::vector<const char*> &_searchTokens,unsigned int _searchIndex)
{
	oAtomHeader nextAtomHeader;

	unsigned long long AtomStartOffset = oFile::Tell(File);
	while(ReadHeader(nextAtomHeader))
	{
		if(strncmp(nextAtomHeader.Type,_searchTokens[_searchIndex],4) == 0)
		{
			if(_searchIndex == _searchTokens.size()-1)
			{
				_results.push_back(nextAtomHeader);
			}
			else
			{
				FindAtoms(_results,_searchTokens,_searchIndex+1);
			}
		}

		oFile::Seek(File, AtomStartOffset+nextAtomHeader.Size);

		AtomStartOffset = oFile::Tell(File);
	}
}

void oMOVFileImpl::FindAtoms(std::vector<oAtomHeader> &_results,const char *_searchToken,const oAtomHeader * const _startAtom /*= NULL*/)
{
	char *tokContext;
	char *token;
	
	std::vector<const char*> _searchTokens;
	token = oStrTok(_searchToken,".",&tokContext);
	while(token != 0)
	{
		char *tokenCopy = (char*)oSTACK_ALLOC(4,1);
		memcpy(tokenCopy,token,4);
		_searchTokens.push_back(tokenCopy);
		token = oStrTok(NULL,".",&tokContext);
	}

	if(_startAtom)
	{
		if(!oFile::Seek(File, _startAtom->DataOffset))
		{
			oSetLastError(EINVAL,"MOV file %s: couldn't seek to a requested atom. Corrupted MOV file?",FileName);
			return;
		}
	}
	else
	{
		if(!oFile::Seek(File, 0))
		{
			oSetLastError(EINVAL,"MOV file %s: couldn't seek to the beginning of the file. Corrupted MOV file?",FileName);
			return;
		}
	}

	FindAtoms(_results,_searchTokens,0);
}

bool oMOVFileImpl::Map(MAPPED* _pMapped)
{
	if (HasFinished())
		return oVideoReturnEndOfFile(_pMapped);

	bool succeeded = true;

	oFile::Seek(File, FileOffsetTable[CurrentFrame]);
	std::vector<unsigned long long>::iterator iterator = lower_bound(SortedFileOffsetTable.begin(),SortedFileOffsetTable.end(),FileOffsetTable[CurrentFrame]);
	unsigned long long maxFrameSize = (*(++iterator))-FileOffsetTable[CurrentFrame]-sizeof(unsigned int); //extra int for the chunksize

	unsigned int RleChunkSize = 0;
	succeeded &= FRead(RleChunkSize);
	RleChunkSize = oByteSwap(RleChunkSize);
	RleChunkSize &= 0x0ffffff;
	RleChunkSize = static_cast<unsigned int>(__max(RleChunkSize,maxFrameSize));
	unsigned long long framePosition = oFile::Tell(File);

	_pMapped->DataSize = RleChunkSize;
	_pMapped->pFrameData = MemoryMappedFrame->Map(framePosition, RleChunkSize);
	_pMapped->DecodedFrameNumber = CurrentFrame;

	return true;
}

void oMOVFileImpl::Unmap()
{
	MemoryMappedFrame->Unmap();
	CurrentFrame++;
}