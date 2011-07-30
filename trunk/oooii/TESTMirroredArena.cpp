// $(header)
#include <oooii/oAllocatorTLSF.h>
#include <oooii/oByte.h>
#include <oooii/oErrno.h>
#include <oooii/oMemory.h>
#include <oooii/oMirroredArena.h>
#include <oooii/oPageAllocator.h>
#include <oooii/oPath.h>
#include <oooii/oProcess.h>
#include <oooii/oRef.h>
#include <oooii/oSocket.h>
#include <oooii/oStdio.h>
#include <oooii/oSTL.h>
#include <oooii/oTest.h>

void* BASE_ADDRESS = (void*)(oMirroredArena::GetRequiredAlignment() * 10);
const static size_t ARENA_SIZE = 512 * 1024;

static const unsigned int TEST1[] = { 0, 1, 2, 3, 4, 5, 6, };
static const char* TEST2[] = { "This is a test", "This is only a test", "We now return you to " };

#define oTESTB_MIR(fn, msg, ...) do { if (!(expr)) { sprintf_s(_StrStatus, _SizeofStrStatus, format, ## __VA_ARGS__); oTRACE("FAILING: %s", _StrStatus); goto FailureLabel; } } while(false)

static oTest::RESULT RunTest(char* _StrStatus, size_t _SizeofStrStatus, oMirroredArena::USAGE _Usage)
{
	oRef<threadsafe oProcess> Client;
	{
		int exitcode = 0;
		char msg[512];
		oTESTB(oSpecialTest::CreateProcess("TESTMirroredArenaClient", &Client), "");
		oTESTB(oSpecialTest::Start(Client, msg, oCOUNTOF(msg), &exitcode), "%s", msg);
	}

	oRef<oMirroredArena> MirroredArenaServer;
	{
		oMirroredArena::DESC desc;
		desc.BaseAddress = BASE_ADDRESS;
		desc.Usage = _Usage;
		desc.Size = ARENA_SIZE;
		oTESTB(oMirroredArena::Create(&desc, &MirroredArenaServer), "Failed to create mirrored arena for server");
	}

	// Mark all memory as dirty and make it so we can debug a bit better by writing 
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

	// ensure some space so when we're testing for ranges below, there's some
	// gap.
	static const size_t kPad = oPageAllocator::GetPageSize() * 2;

	char** test2Strings = static_cast<char**>(AllocatorServer->Allocate(kPad + oCOUNTOF(TEST2) * sizeof(char*)));
	oTESTB(test2Strings, "test2Strings allocation failed");
	test2Strings += kPad;

	for (size_t i = 0; i < oCOUNTOF(TEST2); i++)
	{
		size_t bufferSize = 1 + strlen(TEST2[i]);
		test2Strings[i] = static_cast<char*>(AllocatorServer->Allocate(sizeof(char) * bufferSize));
		oTESTB(test2Strings[i], "test2Strings[%u] allocation failed", i);
		strcpy_s(test2Strings[i], bufferSize, TEST2[i]);
	}

	size_t sizeRequired = 0;
	oTESTB(MirroredArenaServer->RetrieveChanges(0, 0, &sizeRequired), "Failed to get size required for changes: %s.", oGetLastErrorDesc());
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
	oRef<threadsafe oSocketBlocking> ClientSocket;
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
	oTESTB(ClientSocket->Send(oGetData(transitBuffer), (oSocket::size_t)changeSize), "Failed to send memory diffs");

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

		if (SUCCESS != RunTest(subStatus, oCOUNTOF(subStatus), oMirroredArena::READ_WRITE))
		{
			sprintf_s(_StrStatus, _SizeofStrStatus, "READ_WRITE: %s", subStatus);
			return FAILURE;
		}

		oTRACE("--- Running oMirroredArena test with exception-based diffing - expect a lot of write access violations ---");
		if (SUCCESS != RunTest(subStatus, oCOUNTOF(subStatus), oMirroredArena::READ_WRITE_DIFF))
		{
			sprintf_s(_StrStatus, _SizeofStrStatus, "READ_WRITE_DIFF: %s", subStatus);
			return FAILURE;
		}

		oTRACE("--- no more write access violations should occur ---");

		if (SUCCESS != RunTest(subStatus, oCOUNTOF(subStatus), oMirroredArena::READ_WRITE_DIFF_NO_EXCEPTIONS))
		{
			sprintf_s(_StrStatus, _SizeofStrStatus, "READ_WRITE_DIFF_NO_EXCEPTIONS: %s", subStatus);
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
		{
			oMirroredArena::DESC desc;
			desc.BaseAddress = BASE_ADDRESS;
			desc.Usage = oMirroredArena::READ;
			desc.Size = ARENA_SIZE;
			oTESTB(oMirroredArena::Create(&desc, &MirroredArenaClient), "Failed to create mirrored arena for client");
		}

		oTRACE("%s: MirroredArenaClient created", GetName());

		// Listen for a connection
		oRef<threadsafe oSocketServer> server;
		{
			oSocketServer::DESC desc;
			desc.ListenPort = 1234;
			desc.MaxNumConnections = 1;
			oTRACE("%s: MirroredArenaClient server about to be created", GetName());
			oTESTB(oSocketServer::Create("MirroredArena Client's connection Server", desc, &server), "Failed to create server");
		}

		oTRACE("%s: MirroredArenaClient server created", GetName());
		NotifyReady();

		oRef<threadsafe oSocketBlocking> client;
		oTESTB(server->WaitForConnection(&client, WAIT_FOR_CONNECTION_TIMEOUT), "WaitForConnection failed");

		std::vector<char> transitBuffer(ARENA_SIZE + 1024);

		unsigned int* test1 = 0;
		const char** test2Strings = 0;
		void* diffs = 0;

		while (!test1 || !test2Strings || !diffs)
		{
			size_t received = client->Receive(oGetData(transitBuffer), (oSocket::size_t)oGetDataSize(transitBuffer));
			oTESTB(received, "CLIENT: Failed to receive data from server %s", oGetLastErrorDesc());

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
			oTESTB(!strcmp(test2Strings[i], TEST2[i]), "memcmp of test2Strings[%u] failed", i);

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTMirroredArena);
oTEST_REGISTER(TESTMirroredArenaClient);

