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
#include <oBasis/oAllocatorTLSF.h>
#include <oBasis/oByte.h>
#include <oBasis/oMemory.h>
#include <oBasis/oPath.h>
#include <oBasis/oRef.h>
#include <oBasis/oError.h>
#include <oPlatform/oMirroredArena.h>
#include <oPlatform/oPageAllocator.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oSocket.h>
#include <oPlatform/oTest.h>

void* BASE_ADDRESS = (void*)(oMirroredArena::GetRequiredAlignment() * 10);
const static size_t ARENA_SIZE = 512 * 1024;

static const unsigned int TEST1[] = { 0, 1, 2, 3, 4, 5, 6, };
static const char* TEST2[] = { "This is a test", "This is only a test", "We now return you to " };

#define oTESTB_MIR(fn, msg, ...) do { if (!(expr)) { oPrintf(_StrStatus, _SizeofStrStatus, format, ## __VA_ARGS__); oTRACE("FAILING: %s", _StrStatus); goto FailureLabel; } } while(false)

static oTest::RESULT RunTest(char* _StrStatus, size_t _SizeofStrStatus, oMirroredArena::USAGE _Usage)
{
	oRef<threadsafe oProcess> Client;
	{
		int exitcode = 0;
		char msg[512];
		*msg = 0;
		oTESTB(oSpecialTest::CreateProcess("TESTMirroredArenaClient", &Client), "");
		oTESTB(oSpecialTest::Start(Client, msg, oCOUNTOF(msg), &exitcode), "%s", msg);
	}

	oRef<oMirroredArena> MirroredArenaServer;

	oOnScopeExit OnScopeExit([&] { 
		oPageUnreserve(BASE_ADDRESS); 
		MirroredArenaServer = nullptr;
	});
	{
		oMirroredArena::DESC desc;
		desc.BaseAddress = oPageReserveAndCommit(BASE_ADDRESS, ARENA_SIZE);
		desc.Usage = _Usage;
		desc.Size = ARENA_SIZE;
		oTESTB0(oMirroredArenaCreate(desc, &MirroredArenaServer));
	}

	// Mark all memory as dirty and make it so we can debug a bit better by writing 
	// a known pattern to the whole arena.

	// NOTE: If you get a crash in here, it might be because you have break-on-
	// access-violation. In MSVC, go to Debug|Exceptions... and in Win32 Exceptions
	// uncheck the box beside Access Violation.
	*(int*)BASE_ADDRESS = 1;

	oMemset4(BASE_ADDRESS, 0xdeadbeef, ARENA_SIZE);

	oRef<oAllocator> AllocatorServer;
	{
		oAllocator::DESC desc;
		desc.pArena = BASE_ADDRESS;
		desc.ArenaSize = ARENA_SIZE;
		oTESTB(oAllocatorCreateTLSF("MirroredArenaServer", desc, &AllocatorServer), "Failed to create allocator for server");
	}

	// Copy some test data into the server heap

	unsigned int* test1 = static_cast<unsigned int*>(AllocatorServer->Allocate(sizeof(TEST1)));

	// Ignore asserts about leaving dangling memory because if that's so, it's 
	// probably because the test failed somewhere else, so have THAT error come
	// through, not the fact that we aborted before fully tidying up.
	oOnScopeExit OSEReset([&] { AllocatorServer->Reset(); } );

	oTESTB(test1, "test1 allocation failed");
	memcpy(test1, TEST1, sizeof(TEST1));

	// ensure some space so when we're testing for ranges below, there's some
	// gap.
	static const size_t kPad = oPageGetPageSize() * 2;

	char** test2Strings = static_cast<char**>(AllocatorServer->Allocate(kPad + oCOUNTOF(TEST2) * sizeof(char*)));
	oTESTB(test2Strings, "test2Strings allocation failed");
	test2Strings += kPad;

	for (size_t i = 0; i < oCOUNTOF(TEST2); i++)
	{
		size_t bufferSize = 1 + strlen(TEST2[i]);
		test2Strings[i] = static_cast<char*>(AllocatorServer->Allocate(sizeof(char) * bufferSize));
		oTESTB(test2Strings[i], "test2Strings[%u] allocation failed", i);
		oStrcpy(test2Strings[i], bufferSize, TEST2[i]);
	}

	size_t sizeRequired = 0;
	oTESTB(MirroredArenaServer->RetrieveChanges(0, 0, &sizeRequired), "Failed to get size required for changes: %s.", oErrorGetLastString());
	oTESTB(sizeRequired, "Nothing was written to sizeRequired");

	std::vector<char> transitBuffer(sizeRequired);

	size_t changeSize = 0;
	oTESTB(MirroredArenaServer->RetrieveChanges(oGetData(transitBuffer), oGetDataSize(transitBuffer), &changeSize) && changeSize == sizeRequired, "RetreiveChanges failed");
	oTESTB(MirroredArenaServer->IsInChanges(test1, sizeof(TEST1), oGetData(transitBuffer)), "test1 cannot be confirmed in the changes");
	
	// @oooii-tony: It'd be nice to test what happens if the pages are non-
	// contiguous but I think Allocate either writes a 0xdeadbeef type pattern to
	// memory or the allocator might dirty a portion of a page under these small
	// allocation conditions. Really more of this test should be expanded to ensure
	// this works, but I gotta get back to other things at the moment. oBug_1383
	//size_t extraSize = _Usage == oMirroredArena::READ_WRITE ? ARENA_SIZE : kPad-16;
	//oTESTB(!MirroredArenaServer->IsInChanges(test1, sizeof(TEST1) + extraSize, oGetData(transitBuffer)), "false positive on a too-large test1 buffer test");
	
	oTESTB(!MirroredArenaServer->IsInChanges(oByteAdd(BASE_ADDRESS, ~0u), 16, oGetData(transitBuffer)), "false positive on an address outside of arena before");
	oTESTB(!MirroredArenaServer->IsInChanges(oByteAdd(BASE_ADDRESS, ARENA_SIZE), 16, oGetData(transitBuffer)), "false positive on an address outside of arena after");

	// Set up a socket to communicate with other process
	oRef<threadsafe oSocket> ClientSocket;
	{
		oSocket::DESC desc;
		oFromString( &desc.Addr, "127.0.0.1:1234" );
		desc.ConnectionTimeoutMS = 1000;
		desc.Style = oSocket::BLOCKING;
		oTESTB0(oSocketCreate("MirroredArena Server's Client", desc, &ClientSocket));
	}

	oTRACE("test1: 0x%p", test1);
	oTRACE("test2Strings: 0x%p", test2Strings);

	int SizeToSend = sizeof(test1);
	oTESTB(ClientSocket->Send(&SizeToSend, sizeof(SizeToSend), &test1, SizeToSend), "Failed to send test1");

	SizeToSend = sizeof(test2Strings);
	oTESTB(ClientSocket->Send(&SizeToSend, sizeof(SizeToSend), &test2Strings, SizeToSend), "Failed to send test2Strings");

	SizeToSend = (oSocket::size_t)changeSize;
	oTESTB(ClientSocket->Send(&SizeToSend, sizeof(SizeToSend), oGetData(transitBuffer), SizeToSend), "Failed to send memory diffs");

	AllocatorServer->Reset(); // blow away the memory, buffer is in flight

	oTESTB(Client->Wait(10000), "Client did not close cleanly");

	int exitcode = 0;
	oTESTB(Client->GetExitCode(&exitcode), "Failed to get final exit code");

	if (exitcode)
	{
		char msg[4096];
		size_t bytes = Client->ReadFromStdout(msg, oCOUNTOF(msg));
		msg[bytes] = 0;
		oPrintf(_StrStatus, _SizeofStrStatus, "%s", msg);
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

		if (SUCCESS != RunTest(subStatus, oCOUNTOF(subStatus), oMirroredArena::READ_WRITE))
		{
			oPrintf(_StrStatus, _SizeofStrStatus, "READ_WRITE: %s", subStatus);
			return FAILURE;
		}

		oTRACE("--- Running oMirroredArena test with exception-based diffing - expect a lot of write access violations ---");
		if (SUCCESS != RunTest(subStatus, oCOUNTOF(subStatus), oMirroredArena::READ_WRITE_DIFF))
		{
			oPrintf(_StrStatus, _SizeofStrStatus, "READ_WRITE_DIFF: %s", subStatus);
			return FAILURE;
		}

		oTRACE("--- no more write access violations should occur ---");

		if (SUCCESS != RunTest(subStatus, oCOUNTOF(subStatus), oMirroredArena::READ_WRITE_DIFF_NO_EXCEPTIONS))
		{
			oPrintf(_StrStatus, _SizeofStrStatus, "READ_WRITE_DIFF_NO_EXCEPTIONS: %s", subStatus);
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
		oTRACE("%s: Run Start", GetName());

		oRef<oMirroredArena> MirroredArenaClient;

		oOnScopeExit OnScopeExit([&] { 
			oPageUnreserve(BASE_ADDRESS); 
			MirroredArenaClient = nullptr;
		});
		{
			oMirroredArena::DESC desc;
			desc.BaseAddress = oPageReserveAndCommit(BASE_ADDRESS, ARENA_SIZE, false);
			desc.Usage = oMirroredArena::READ;
			desc.Size = ARENA_SIZE;
			oTESTB(oMirroredArenaCreate(desc, &MirroredArenaClient), "Failed to create mirrored arena for client");
		}

		oTRACE("%s: MirroredArenaClient created", GetName());

		oEvent connectEvent;
		connectEvent.Reset();
		// Listen for a connection
		oRef<threadsafe oSocketServer2> server;
		oRef<threadsafe oSocket> client;
		{
			oSocketServer2::DESC desc;
			desc.ListenPort = 1234;
			desc.BlockingSettings.RecvTimeout = 2000;
			desc.BlockingSettings.SendTimeout = 2000;
			desc.NewConnectionCallback = [&](threadsafe oSocket* _Socket){
				if(!client)
				{
					client = _Socket;
					oTRACE("SERVER: %s received connection from %s (%s)", server->GetDebugName(), client->GetDebugName(), oErrorGetLast() ? oAsString(oErrorGetLast()) : "OK");
					connectEvent.Set();
				}
			};
			oTRACE("%s: MirroredArenaClient server about to be created", GetName());
			oTESTB(oSocketServer2Create("MirroredArena Client's connection Server", desc, &server), "Failed to create server");
		}
		
		oTRACE("%s: MirroredArenaClient server created", GetName());
		NotifyReady();

		oTESTB(connectEvent.Wait(2000), "Timed out waiting for connection");

		std::vector<char> transitBuffer(ARENA_SIZE + 1024);

		unsigned int* test1 = 0;
		const char** test2Strings = 0;
		void* diffs = 0;

		while (!test1 || !test2Strings || !diffs)
		{
			int SzToReceive = 0;
			oTESTB( sizeof(int) == client->Recv(&SzToReceive, sizeof(int)), "Failed to receive size info");
			int received = 0;
			while(received != SzToReceive)
			{
				received += client->Recv(oByteAdd( oGetData(transitBuffer), received), SzToReceive - received);
			}
			oTESTB(received, "CLIENT: Failed to receive data from server %s", oErrorGetLastString());

			// Strange look to accommodate Nagel's algorithm
			void* p = oGetData(transitBuffer);
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
			oTESTB(!oStrcmp(test2Strings[i], TEST2[i]), "memcmp of test2Strings[%u] failed", i);

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTMirroredArena);
oTEST_REGISTER(TESTMirroredArenaClient);

