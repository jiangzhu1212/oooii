/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oPlatform/oTest.h>
#include <oBasis/oRef.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oStreamUtil.h>

struct PLATFORM_FileSync : public oTest
{
	char TempFileBlob[1024 * 32];

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oStd::path_string testFilePath;
		oTESTB0(FindInputFile(testFilePath, "oooii.ico"));

		oRef<threadsafe oSchemeHandler> FileSchemeHandler;
		oTESTB(oFindSchemeHandler("file", &FileSchemeHandler), "There is no handler for files registered");
		oTESTB0(FileSchemeHandler);

		// Test file reading
		{
			static const unsigned int NUM_READS = 5;

			oRef<threadsafe oStreamReader> ReadFile;
			oTESTB( oStreamReaderCreate(testFilePath, &ReadFile), oErrorGetLastString() );

			oSTREAM_DESC FileDesc;
			ReadFile->GetDesc(&FileDesc), oErrorGetLastString();

			size_t BytesPerRead = static_cast<size_t>( FileDesc.Size ) / NUM_READS;
			int ActualReadCount = NUM_READS + ( ( ( FileDesc.Size % NUM_READS ) > 0 )? 1 : 0 );

			oSTREAM_READ StreamRead;
			StreamRead.Range.Offset = 0;
			StreamRead.Range.Size = BytesPerRead;

			void* pHead = TempFileBlob;
			bool TestSuccess = false;

			size_t r = 0;
			for(int i = 0; i < NUM_READS; ++i, r += BytesPerRead )
			{
				StreamRead.pData = pHead;
				TestSuccess = ReadFile->Read(StreamRead);
				
				pHead = oStd::byte_add(pHead, BytesPerRead);
				StreamRead.Range.Offset += BytesPerRead;
			}
			auto RemainingBytes = FileDesc.Size - r;
			if( RemainingBytes > 0)
			{
				StreamRead.Range.Size = static_cast<size_t>( RemainingBytes );
				StreamRead.pData = pHead;
				TestSuccess = ReadFile->Read(StreamRead);
			}

			static const uint128 ExpectedFileHash(3650274822346168237, 9475904461222329612);
			oTESTB(TestSuccess, "Test failed, ReadFile->Read returned false");
			oTESTB( oHash_murmur3_x64_128( TempFileBlob, oUInt( FileDesc.Size ) ) == ExpectedFileHash, "Test failed to compute correct hash" );
		}

		// Now test writing data out then reading it back
		oStd::path_string TempFilePath;
		oTESTB0(BuildPath(TempFilePath, "", oTest::TEMP));
		oTESTB0(oFileEnsureParentFolderExists(TempFilePath));
		oStrAppendf(TempFilePath, "/TESTAsyncFileIO.bin");

		{
			oRef<threadsafe oStreamReader> ReadFile;
			oTESTB( !oStreamReaderCreate(TempFilePath, &ReadFile), "Should not be able to create a FileReader for a file that does not exist." );
		}

		static const oGUID TestGUID = { 0x9aab7fc7, 0x6ad8, 0x4260, { 0x98, 0xef, 0xfd, 0x93, 0xda, 0x8e, 0xdc, 0x3c } };
		// Now test write the test file
		{
			oRef<threadsafe oStreamWriter> WriteFile;
			oTESTB( oStreamWriterCreate(TempFilePath, &WriteFile), oErrorGetLastString() );
		
			oSTREAM_WRITE StreamWrite;
			StreamWrite.pData = &TestGUID;
			StreamWrite.Range.Offset = 0;
			StreamWrite.Range.Size = sizeof(TestGUID);

			oTESTB(WriteFile->Write(StreamWrite), "Test Failed: Write failed");
		}
		oGUID LoadWrite;
		oTESTB( oStreamLoadPartial(&LoadWrite, sizeof(oGUID), TempFilePath), oErrorGetLastString() );
		oTESTB( TestGUID == LoadWrite, "Write failed to write correct GUID");		
		
		return SUCCESS;
	};
};

oTEST_REGISTER(PLATFORM_FileSync);