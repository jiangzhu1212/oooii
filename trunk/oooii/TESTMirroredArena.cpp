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
#include "pch.h"
#include <oooii/oAllocatorTLSF.h>
#include <oooii/oByte.h>
#include <oooii/oErrno.h>
#include <oooii/oMirroredArena.h>
#include <oooii/oPath.h>
#include <oooii/oProcess.h>
#include <oooii/oRef.h>
#include <oooii/oSocket.h>
#include <oooii/oStdio.h>
#include <oooii/oTest.h>

void* BASE_ADDRESS = (void*)(oMirroredArena::GetRequiredAlignment() * 10);
const static size_t ARENA_SIZE = 512 * 1024;

static const unsigned int TEST1[] = { 0, 1, 2, 3, 4, 5, 6, };
static const char* TEST2[] = { "This is a test", "This is only a test", "We now return you to " };

static oTest::RESULT RunTest(char* _StrStatus, size_t _SizeofStrStatus, oMirroredArena::USAGE _Usage)
{
	threadsafe oRef<oProcess> Client;
	{
		int exitcode = 0;
		char msg[512];
		oTESTB(oTestRunSpecialTest("TESTMirroredArenaClient", msg, oCOUNTOF(msg), &exitcode, &Client), "%s", msg);
	}

	oRef<oMirroredArena> MirroredArenaServer;
	{
		oMirroredArena::DESC desc;
		desc.BaseAddress = BASE_ADDRESS;
		desc.Usage = _Usage;
		desc.Size = ARENA_SIZE;
		oTESTB(oMirroredArena::Create(&desc, &MirroredArenaServer), "Failed to create mirrored arena for server");
	}

	// Mark all memory as dirty and make it so we can debug a bit better by writting 
	// a known pattern to the whole arena.
	oMemset4(BASE_ADDRESS, 0xdeadbeef, ARENA_SIZE);

	oRef<oAllocator> AllocatorServer;
	{
		oAllocator::DESC desc;
		desc.pArena = BASE_ADDRESS;
		desc.ArenaSize = ARENA_SIZE;
		oTESTB(oAllocatorTLSF::Create("MirroredArenaServer", &desc, &AllocatorServer), "Failed to create allocator for server");
	}

	// Copy some test data into the server heap

	unsigned int* test1 = static_cast<unsigned int*>(AllocatorServer->Allocate(sizeof(TEST1)));
	oTESTB(test1, "test1 allocation failed");
	memcpy(test1, TEST1, sizeof(TEST1));

	char** test2Strings = static_cast<char**>(AllocatorServer->Allocate(oCOUNTOF(TEST2) * sizeof(char*)));
	oTESTB(test2Strings, "test2Strings allocation failed");

	for (size_t i = 0; i < oCOUNTOF(TEST2); i++)
	{
		size_t bufferSize = 1 + strlen(TEST2[i]);
		test2Strings[i] = static_cast<char*>(AllocatorServer->Allocate(sizeof(char) * bufferSize));
		oTESTB(test2Strings[i], "test2Strings[%u] allocation failed", i);
		strcpy_s(test2Strings[i], bufferSize, TEST2[i]);
	}

	size_t sizeRequired = 0;
	oTESTB(MirroredArenaServer->RetrieveChanges(0, 0, &sizeRequired), "Failed to get size required for changes.");
	oTESTB(sizeRequired, "Nothing was written to sizeRequired");

	oTestScopedArray<char> transitBuffer(sizeRequired);

	size_t changeSize = 0;
	oTESTB(MirroredArenaServer->RetrieveChanges(transitBuffer.GetPointer(), transitBuffer.GetCount(), &changeSize) && changeSize == sizeRequired, "RetreiveChanges failed");

	// Set up a socket to communicate with other process

	threadsafe oRef<oSocketBlocking> ClientSocket;
	{
		oSocketBlocking::DESC desc;
		desc.Peername = "127.0.0.1:1234";
		desc.ConnectionTimeoutMS = 1000;
		desc.MaxSendSize = 2 * ARENA_SIZE;
		desc.MaxReceiveSize = 2 * ARENA_SIZE;
		oTESTB(oSocketBlocking::Create("MirroredArena Server's Client", desc, &ClientSocket), "Failed to create client");
	}

	oTRACE("test1: 0x%p", test1);
	oTRACE("test2Strings: 0x%p", test2Strings);
	oTESTB(ClientSocket->Send(&test1, sizeof(test1)), "Failed to send test1");
	oTESTB(ClientSocket->Send(&test2Strings, sizeof(test2Strings)), "Failed to send test2Strings");
	oTESTB(ClientSocket->Send(transitBuffer.GetPointer(), (oSocket::size_t)changeSize), "Failed to send memory diffs");

	AllocatorServer->Reset(); // blow away the memory, buffer is in flight

	oTESTB(Client->Wait(10000), "Client did not close cleanly");

	int exitcode = 0;
	oTESTB(Client->GetExitCode(&exitcode), "Failed to get final exit code");

	if (exitcode)
	{
		char msg[4096];
		size_t bytes = Client->ReadFromStdout(msg, oCOUNTOF(msg));
		msg[bytes] = 0;
		sprintf_s(_StrStatus, _SizeofStrStatus, "%s", msg);
		return oTest::FAILURE;
	}

	oTESTB(exitcode == 0, "Exitcode: %d", exitcode);

	return oTest::SUCCESS;
}

struct TESTMirroredArena : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		char subStatus[1024];

		if ( SUCCESS != RunTest(subStatus, oCOUNTOF(subStatus), oMirroredArena::READ_WRITE))
		{
			sprintf_s(_StrStatus, _SizeofStrStatus, "READ_WRITE: %s", subStatus);
			return FAILURE;
		}

		if ( SUCCESS != RunTest(subStatus, oCOUNTOF(subStatus), oMirroredArena::READ_WRITE_DIFF))
		{
			sprintf_s(_StrStatus, _SizeofStrStatus, "READ_WRITE_DIFF: %s", subStatus);
			return FAILURE;
		}

		return SUCCESS;
	}
};

struct TESTMirroredArenaClient : public oSpecialTest
{
	static const unsigned int WAIT_FOR_CONNECTION_TIMEOUT = 5000;
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oRef<oMirroredArena> MirroredArenaClient;
		{
			oMirroredArena::DESC desc;
			desc.BaseAddress = BASE_ADDRESS;
			desc.Usage = oMirroredArena::READ;
			desc.Size = ARENA_SIZE;
			oTESTB(oMirroredArena::Create(&desc, &MirroredArenaClient), "Failed to create mirrored arena for client");
		}

		// Listen for a connection
		threadsafe oRef<oSocketServer> server;
		{
			oSocketServer::DESC desc;
			desc.ListenPort = 1234;
			desc.MaxNumConnections = 1;
			oTESTB(oSocketServer::Create("MirroredArena Client's connection Server", desc, &server), "Failed to create server");
		}

		threadsafe oRef<oSocketBlocking> client;
		oTESTB(server->WaitForConnection(&client, WAIT_FOR_CONNECTION_TIMEOUT ), "WaitForConnection failed");

		oTestScopedArray<char> transitBuffer(ARENA_SIZE + 1024);

		unsigned int* test1 = 0;
		const char** test2Strings = 0;
		void* diffs = 0;

		while (!test1 || !test2Strings || !diffs)
		{
			size_t received = client->Receive(transitBuffer.GetPointer(), (oSocket::size_t)transitBuffer.GetCount());
			oTESTB(received, "CLIENT: Failed to receive data from server %s", oGetLastErrorDesc() );

			// Strange look to accommodate Nagel's algorithm
			void* p = transitBuffer.GetPointer();
			while (received)
			{
				if (!test1)
				{
					test1 = *(unsigned int**)p;
					received -= sizeof(unsigned int*);
					p = oByteAdd(p, sizeof(unsigned int*));
				}

				if (received && !test2Strings)
				{
					test2Strings = *(const char***)p;
					received -= sizeof(const char**);
					p = oByteAdd(p, sizeof(const char**));
				}

				if (received && !diffs)
				{
					diffs = p;
					received = 0;
				}
			}
		}

		// Ok, we just sent pointers across a socket... do they hold up?
		oTESTB(MirroredArenaClient->ApplyChanges(diffs), "ApplyChanges failed");
		oTESTB(!memcmp(test1, TEST1, sizeof(TEST1)), "memcmp of TEST1 failed");
		for (size_t i = 0; i < oCOUNTOF(TEST2); i++)
			oTESTB(!strcmp(test2Strings[i], TEST2[i]), "memcmp of test2Strings[%u] failed", i);

		return SUCCESS;
	}
};

TESTMirroredArena TestMirroredArena;
TESTMirroredArenaClient TESTMirroredArenaClient;

