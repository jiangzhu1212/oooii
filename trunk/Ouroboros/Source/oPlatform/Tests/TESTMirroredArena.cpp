/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oBase/tlsf_allocator.h>
#include <oBase/byte.h>
#include <oCore/page_allocator.h>
#include <oBasis/oError.h>
#include <oBase/event.h>
#include <oPlatform/oMirroredArena.h>
#include <oPlatform/oSocket.h>
#include <oPlatform/oTest.h>
#include <chrono>

using namespace ouro::page_allocator;

void* BASE_ADDRESS = (void*)(oMirroredArena::GetRequiredAlignment() * 10);
const static size_t ARENA_SIZE = 512 * 1024;

static const unsigned int TEST1[] = { 0, 1, 2, 3, 4, 5, 6, };
static const char* TEST2[] = { "This is a test", "This is only a test", "We now return you to " };

#define oTESTB_MIR(fn, msg, ...) do { if (!(expr)) { snprintf(_StrStatus, _SizeofStrStatus, format, ## __VA_ARGS__); oTRACE("FAILING: %s", _StrStatus); goto FailureLabel; } } while(false)

static oTest::RESULT RunTest(char* _StrStatus, size_t _SizeofStrStatus, oMirroredArena::USAGE _Usage)
{
#ifdef o32BIT
	snprintf(_StrStatus, _SizeofStrStatus, "disabled in 32-bit for now");
	return oTest::SKIPPED;
#else
	*_StrStatus = 0;

	std::shared_ptr<ouro::process> Client;
	{
		int exitcode = 0;
		char msg[512];
		*msg = 0;
		oTESTB(oSpecialTest::CreateProcess("PLATFORM_oMirroredArenaClient", &Client), "");
		oTESTB(oSpecialTest::Start(Client.get(), msg, oCOUNTOF(msg), &exitcode), "%s", msg);
		oTRACE("SERVER: starting mirrored arena construction...");
	}

	ouro::intrusive_ptr<oMirroredArena> MirroredArenaServer;

	ouro::finally OnScopeExit([&] { 
		unreserve(BASE_ADDRESS); 
		MirroredArenaServer = nullptr;
	});
	{
		oMirroredArena::DESC desc;
		oTRACE("SERVER: oPageReserveAndCommit...");
		desc.BaseAddress = reserve_and_commit(BASE_ADDRESS, ARENA_SIZE);

		if (!desc.BaseAddress)
		{
			oTRACE("SERVER: no base address (this should return skipped).");
			oErrorSetLast(std::errc::permission_denied, "Could not allocate arena at 0x%p from server (another process may be using that space).", BASE_ADDRESS);
			snprintf(_StrStatus, _SizeofStrStatus, oErrorGetLastString());
			return oTest::SKIPPED;
		}
		else
			oTRACE("SERVER: got base address (no skip).");

		desc.Usage = _Usage;
		desc.Size = ARENA_SIZE;
		oTRACE("SERVER: creating mirrored arena...");
		oTESTB0(oMirroredArenaCreate(desc, &MirroredArenaServer));
		oTRACE("SERVER: mirrored arena created.");
	}

	// Mark all memory as dirty and make it so we can debug a bit better by writing 
	// a known pattern to the whole arena.

	// NOTE: If you get a crash in here, it might be because you have break-on-
	// access-violation. In MSVC, go to Debug|Exceptions... and in Win32 Exceptions
	// uncheck the box beside Access Violation.
	*(int*)BASE_ADDRESS = 1;

	ouro::memset4(BASE_ADDRESS, 0xdeadbeef, ARENA_SIZE);

	oTRACE("SERVER: creating TLSF allocator...");
	ouro::tlsf_allocator AllocatorServer(BASE_ADDRESS, ARENA_SIZE);
	oTRACE("SERVER: TLSF allocator created.");

	// Copy some test data into the server heap

	unsigned int* test1 = static_cast<unsigned int*>(AllocatorServer.allocate(sizeof(TEST1)));

	// Ignore asserts about leaving dangling memory because if that's so, it's 
	// probably because the test failed somewhere else, so have THAT error come
	// through, not the fact that we aborted before fully tidying up.
	ouro::finally OSEReset([&] { AllocatorServer.reset(); } );

	oTRACE("SERVER: writing to shared memory...");

	oTESTB(test1, "test1 allocation failed");
	memcpy(test1, TEST1, sizeof(TEST1));

	// ensure some space so when we're testing for ranges below, there's some
	// gap.
	static const size_t kPad = page_size() * 2;

	char** test2Strings = static_cast<char**>(AllocatorServer.allocate(kPad + oCOUNTOF(TEST2) * sizeof(char*)));
	oTESTB(test2Strings, "test2Strings allocation failed");
	test2Strings += kPad;

	for (size_t i = 0; i < oCOUNTOF(TEST2); i++)
	{
		size_t bufferSize = 1 + strlen(TEST2[i]);
		test2Strings[i] = static_cast<char*>(AllocatorServer.allocate(sizeof(char) * bufferSize));
		oTESTB(test2Strings[i], "test2Strings[%u] allocation failed", i);
		strlcpy(test2Strings[i], TEST2[i], bufferSize);
	}

	size_t sizeRequired = 0;
	oTESTB(MirroredArenaServer->RetrieveChanges(0, 0, &sizeRequired), "Failed to get size required for changes: %s.", oErrorGetLastString());
	oTESTB(sizeRequired, "Nothing was written to sizeRequired");

	std::vector<char> transitBuffer(sizeRequired);

	oTRACE("SERVER: packing diffs...");

	size_t changeSize = 0;
	oTESTB(MirroredArenaServer->RetrieveChanges(ouro::data(transitBuffer), ouro::size(transitBuffer), &changeSize) && changeSize == sizeRequired, "RetreiveChanges failed");
	oTESTB(MirroredArenaServer->IsInChanges(test1, sizeof(TEST1), ouro::data(transitBuffer)), "test1 cannot be confirmed in the changes");
	
	// @tony: It'd be nice to test what happens if the pages are non-
	// contiguous but I think Allocate either writes a 0xdeadbeef type pattern to
	// memory or the allocator might dirty a portion of a page under these small
	// allocation conditions. Really more of this test should be expanded to ensure
	// this works, but I gotta get back to other things at the moment. oBug_1383
	//size_t extraSize = _Usage == oMirroredArena::READ_WRITE ? ARENA_SIZE : kPad-16;
	//oTESTB(!MirroredArenaServer->IsInChanges(test1, sizeof(TEST1) + extraSize, ouro::data(transitBuffer)), "false positive on a too-large test1 buffer test");
	
	oTESTB(!MirroredArenaServer->IsInChanges(ouro::byte_add(BASE_ADDRESS, ouro::invalid), 16, ouro::data(transitBuffer)), "false positive on an address outside of arena before");
	oTESTB(!MirroredArenaServer->IsInChanges(ouro::byte_add(BASE_ADDRESS, ARENA_SIZE), 16, ouro::data(transitBuffer)), "false positive on an address outside of arena after");

	oTRACE("SERVER: creating a socket to send diffs...");

	// Set up a socket to communicate with other process
	ouro::intrusive_ptr<threadsafe oSocket> ClientSocket;
	{
		oSocket::DESC desc;
		ouro::from_string( &desc.Addr, "127.0.0.1:1234" );
		desc.ConnectionTimeoutMS = 1000;
		desc.Style = oSocket::BLOCKING;
		oTESTB0(oSocketCreate("MirroredArena Server's Client", desc, &ClientSocket));
	}

	oTRACE("SERVER: socket created.");

	oTRACE("test1: 0x%p", test1);
	oTRACE("test2Strings: 0x%p", test2Strings);

	oTRACE("SERVER: send1...");
	int SizeToSend = sizeof(test1);
	oTESTB(ClientSocket->Send(&SizeToSend, sizeof(SizeToSend), &test1, SizeToSend), "Failed to send test1");

	oTRACE("SERVER: send2...");
	SizeToSend = sizeof(test2Strings);
	oTESTB(ClientSocket->Send(&SizeToSend, sizeof(SizeToSend), &test2Strings, SizeToSend), "Failed to send test2Strings");

	oTRACE("SERVER: send3...");
	SizeToSend = (oSocket::size_t)changeSize;
	oTESTB(ClientSocket->Send(&SizeToSend, sizeof(SizeToSend), ouro::data(transitBuffer), SizeToSend), "Failed to send memory diffs");

	oTRACE("SERVER: all sends complete.");

	AllocatorServer.reset(); // blow away the memory, buffer is in flight

	oTRACE("SERVER: waiting for client process to exit.");
	oTESTB(Client->wait_for(std::chrono::seconds(10)), "Client did not close cleanly");
	oTRACE("SERVER: client process exited.");

	int exitcode = 0;
	oTESTB(Client->exit_code(&exitcode), "Failed to get final exit code");

	if (exitcode)
	{
		char msg[4096];
		size_t bytes = Client->from_stdout(msg, oCOUNTOF(msg));
		msg[bytes] = 0;
		snprintf(_StrStatus, _SizeofStrStatus, "%s", msg);
		return oTest::FAILURE;
	}

	oTESTB(exitcode == 0, "Exitcode: %d", exitcode);

	return oTest::SUCCESS;
#endif
}

#define RUNTEST(_DiffType) do \
	{	oTest::RESULT r = RunTest(subStatus, oCOUNTOF(subStatus), oMirroredArena::_DiffType); \
		if (r != SUCCESS) \
		{	snprintf(_StrStatus, _SizeofStrStatus, #_DiffType ": %s", subStatus); \
			return r; \
		} \
	} while (false)

struct PLATFORM_oMirroredArena : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		char subStatus[1024];
		RUNTEST(READ_WRITE);
		oTRACE("--- Running oMirroredArena test with exception-based diffing - expect a lot of write access violations ---");
		RUNTEST(READ_WRITE_DIFF);
		oTRACE("--- no more write access violations should occur ---");
		RUNTEST(READ_WRITE_DIFF_NO_EXCEPTIONS);
		return SUCCESS;
	}
};

struct PLATFORM_oMirroredArenaClient : public oSpecialTest
{
	static const unsigned int WAIT_FOR_CONNECTION_TIMEOUT = 5000;

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oTRACE("%s: Run Start", GetName());

		ouro::intrusive_ptr<oMirroredArena> MirroredArenaClient;

		ouro::finally OnScopeExit([&] { 
			unreserve(BASE_ADDRESS); 
			MirroredArenaClient = nullptr;
		});
		{
			oMirroredArena::DESC desc;
			desc.BaseAddress = reserve_and_commit(BASE_ADDRESS, ARENA_SIZE, false);

			if (!desc.BaseAddress)
			{
				oErrorSetLast(std::errc::permission_denied, "Could not allocate arena at 0x%p from client (another process may be using that space).", BASE_ADDRESS);
				strlcpy(_StrStatus, oErrorGetLastString(), _SizeofStrStatus);
				return oTest::SKIPPED;
			}

			desc.Usage = oMirroredArena::READ;
			desc.Size = ARENA_SIZE;
			oTESTB(oMirroredArenaCreate(desc, &MirroredArenaClient), "Failed to create mirrored arena for client");
		}

		oTRACE("%s: MirroredArenaClient created", GetName());

		ouro::event connectEvent;
		connectEvent.reset();
		// Listen for a connection
		ouro::intrusive_ptr<threadsafe oSocketServer2> server;
		ouro::intrusive_ptr<threadsafe oSocket> client;
		{
			oSocketServer2::DESC desc;
			desc.ListenPort = 1234;
			desc.BlockingSettings.RecvTimeout = 2000;
			desc.BlockingSettings.SendTimeout = 2000;
			desc.NewConnectionCallback = [&](threadsafe oSocket* _Socket){
				if(!client)
				{
					client = _Socket;
					oTRACE("SERVER: %s received connection from %s (%s)", server->GetDebugName(), client->GetDebugName(), oErrorGetLast() ? oErrorAsString(oErrorGetLast()) : "OK");
					connectEvent.set();
				}
			};
			oTRACE("%s: MirroredArenaClient server about to be created", GetName());
			oTESTB(oSocketServer2Create("MirroredArena Client's connection Server", desc, &server), "Failed to create server");
		}
		
		oTRACE("%s: MirroredArenaClient server created", GetName());
		NotifyReady();

		oTESTB(connectEvent.wait_for(std::chrono::milliseconds(2000)), "Timed out waiting for connection");

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
				received += client->Recv(ouro::byte_add( ouro::data(transitBuffer), received), SzToReceive - received);
			}
			oTESTB(received, "CLIENT: Failed to receive data from server %s", oErrorGetLastString());

			// Strange look to accommodate Nagel's algorithm
			void* p = ouro::data(transitBuffer);
			while (received)
			{
				if (!test1)
				{
					test1 = *(unsigned int**)p;
					received -= sizeof(unsigned int*);
					p = ouro::byte_add(p, sizeof(unsigned int*));
				}

				if (received && !test2Strings)
				{
					test2Strings = *(const char***)p;
					received -= sizeof(const char**);
					p = ouro::byte_add(p, sizeof(const char**));
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

oTEST_REGISTER(PLATFORM_oMirroredArena);
oTEST_REGISTER(PLATFORM_oMirroredArenaClient);

