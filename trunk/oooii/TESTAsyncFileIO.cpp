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
#include <oooii/oTest.h>
#include <oooii/oAsyncFileIO.h>
#include <oooii/oEvent.h>
#include <oooii/oFile.h>
#include <oooii/oHash.h>
#include <oooii/oSize.h>
#include <oooii/oPath.h>

void TESTAsyncFileIORead_Continuation(oAsyncFileIO::COMMAND* _pResult, uint128_t* _pFileHash, threadsafe oEvent* pEvent)
{
	if( _pResult->Success )
	{
		*_pFileHash = oHash_murmur3_x64_128( _pResult->pDataOut, oSize32( _pResult->SizeOfDataBlob ) );
	}
	pEvent->Set();
}

void TESTAsyncFileIO_NOP_Continuation(oAsyncFileIO::COMMAND* _pResult)
{

}

void ValidateIOWrite_Continuation(oAsyncFileIO::COMMAND* _pResult, threadsafe oEvent* pEvent)
{
	pEvent->Set();
}

struct TESTAsyncFileIO : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		const uint128_t ExpextedFileHash = {3650274822346168237,9475904461222329612};
		char TempFileBlob[1024 * 32];
		char testFilePath[_MAX_PATH];
		oTESTB(oTestManager::Singleton()->FindFullPath(testFilePath, "oooii.ico"), "Failed to find %s", "oooii.ico");

		oFile::DESC FileDesc;
		oTESTB( oFile::GetDesc(testFilePath, &FileDesc), "Failed to get file desc" );
		oTESTB(FileDesc.Size < oCOUNTOF(TempFileBlob), "Not enough stack memory to load file");
		threadsafe oEvent ReadComplete;
		uint128_t FileHash;


		oAsyncFileIO::COMMAND Command;
		Command.PathToFile = testFilePath;
		Command.DataIsText = false;
		Command.CommandType = oAsyncFileIO::READ;
		Command.pDataOut = TempFileBlob;
		Command.SizeOfDataBlob = oSize64( FileDesc.Size );
		Command.Continuation = oBIND( TESTAsyncFileIORead_Continuation, oBIND1, &FileHash, &ReadComplete );

		oAsyncFileIO::ScheduleIO(&Command);

		oTESTB(ReadComplete.Wait(5000), "ASyncFileIO read timed out");

		oTESTB(FileHash == ExpextedFileHash, "Read file failed to compute correct hash");
		oTESTB(Command.Success, "Errors loading file %s", testFilePath);


		// Now test writing data out then reading it back
		char TempFilePath[_MAX_PATH];
		oFile::CreateTempFolder(TempFilePath);
		oStrAppend(TempFilePath, "/TESTAsyncFileIO.bin");

		oAsyncFileIO::COMMAND Write0;
		Write0.PathToFile = TempFilePath;
		Write0.CommandType = oAsyncFileIO::WRITE;
		Write0.pDataIn = &ExpextedFileHash.data[0];
		Write0.SizeOfDataBlob = sizeof(long long);
		Write0.Continuation = &TESTAsyncFileIO_NOP_Continuation;
		
		// Write the first long long
		oAsyncFileIO::ScheduleIO(&Write0);

		// Write the second long long
		oAsyncFileIO::COMMAND Write1 = Write0;
		Write1.CommandType = oAsyncFileIO::WRITE_APPEND;
		Write1.pDataIn = &ExpextedFileHash.data[1];
		oAsyncFileIO::ScheduleIO(&Write1);

		// Read back the written file
		oAsyncFileIO::COMMAND ReadBack = Write0;
		memset( TempFileBlob, NULL, oCOUNTOF(TempFileBlob));
		ReadBack.CommandType = oAsyncFileIO::READ;
		ReadBack.pDataOut = TempFileBlob;
		ReadBack.SizeOfDataBlob = sizeof(uint128_t);
		ReadBack.Continuation = oBIND( &ValidateIOWrite_Continuation, oBIND1, &ReadComplete );

		ReadComplete.Reset();
		oAsyncFileIO::ScheduleIO(&ReadBack);
		oTESTB(ReadComplete.Wait(5000), "ASyncFileIO read timed out");
		oTESTB( *(uint128_t*)ReadBack.pDataOut == ExpextedFileHash, "Failed to write out and readback file hash" );
		oFile::Delete(TempFilePath);

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTAsyncFileIO);
