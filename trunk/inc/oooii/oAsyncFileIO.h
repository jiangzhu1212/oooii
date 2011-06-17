// $(header)
#pragma once
#ifndef oAsyncFileIO_h
#define oAsyncFileIO_h

#include <oooii/oStddef.h>

namespace oAsyncFileIO 
{
	enum COMMAND_TYPE
	{
		READ,
		WRITE,
		WRITE_APPEND
	};

	struct COMMAND
	{	
		// Input
		COMMAND_TYPE CommandType; 
		bool DataIsText; 
		const char* PathToFile;
		union
		{
			void* pDataOut;
			const void* pDataIn;
		};
		size_t SizeOfDataBlob;

		// Output
		bool Success;

		// Function to call on the IO thread once the file is done. 
		// Success will indicate what happened, a call to oGetLastError()
		// will return more info.
		oFUNCTION< void(oAsyncFileIO::COMMAND* _pCommand) > Continuation;
	};

	void ScheduleIO(COMMAND* _pCommand);
};

#endif
