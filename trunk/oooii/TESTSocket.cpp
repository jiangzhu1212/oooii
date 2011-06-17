// $(header)
#include <oooii/oErrno.h>
#include <oooii/oEvent.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSocket.h>
#include <oooii/oStdio.h>
#include <oooii/oSTL.h>
#include <oooii/oTest.h>
#include <oooii/oThread.h>
#include <oooii/oProcess.h>
#include <oooii/oooii.h>

unsigned short SERVER_PORT = 1234;
unsigned short SENDER_PORT = 1234;
unsigned short RECEIVER_PORT = 1235;

const char* SERVER_HOSTNAME = "localhost:1234";
const char* SENDER_DESTINATION_HOSTNAME = "localhost:1235";
const char* RECEIVER_SOURCE_HOSTNAME = "localhost:1234";

//const unsigned int TIMEOUT = 5000;
const unsigned int TIMEOUT = oINFINITE_WAIT;

const unsigned int INITIAL_CONNECTION_TIMEOUT = 10000;

struct TESTSocketReliableServer : public oSpecialTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{		
		oRef<threadsafe oSocketServer> Server;
		oRef<threadsafe oSocketBlocking> Client;

		unsigned int Timeout = INITIAL_CONNECTION_TIMEOUT;
		unsigned int Counter = 0;

		oSocketServer::DESC desc;
		desc.ListenPort = SERVER_PORT;
		desc.MaxNumConnections = 4;
		oTESTB( oSocketServer::Create("Server", desc, &Server), "Failed to create server! winsock error: %s", oGetLastErrorDesc() );

		if (Timeout)
			oTRACE("SERVER: %s waiting for connection...", Server->GetDebugName());

		oRef<threadsafe oSocketBlocking> NewConnection;
		if (Server->WaitForConnection(&NewConnection, Timeout))
		{
			Client = NewConnection;
			oTRACE("SERVER: %s received connection from %s (%s)", Server->GetDebugName(), Client->GetDebugName(), oGetLastError() ? oGetErrnoString(oGetLastError()) : "OK");

			const char* s = "Server acknowledges your connection, waiting to receive data.";

			oTESTB( Client->Send(s, (oSocket::size_t)strlen(s)+1), "SERVER: Client %s send failed", Client->GetDebugName() );

			// From now on poll
			Timeout = 0;
		}

		else
		{
			oTESTB( false, "SERVER: %s %s: %s.", Server->GetDebugName(), oGetErrnoString(oGetLastError()), oGetLastErrorDesc());
		}

		while( 1 )
		{
			char msg[1024];
			memset(msg, 0, sizeof(msg));

			if (!Counter)
			{
				oTRACE("SVRCXN: %s waiting to receive OnConnect data...", Client->GetDebugName());
				size_t bytesReceived = Client->Receive(msg, oCOUNTOF(msg));
				if (bytesReceived)
				{
					if (strstr(msg, "OnConnectMsg"))
					{
						oTRACE("SVRCXN: received connection request (%s), issuing upload ok's", msg);
						Counter = 1;
					}
					else
						oTRACE("SVRCXN: wanted OnConnectMsg data, got %s", msg);

					if (bytesReceived > (strlen(msg) + 1))
					{
						char* curr = msg + strlen(msg) + 1;
						char* end = msg + sizeof(msg);

						while (*curr && curr < end)
						{
							oTRACE("SVRCXN: Nagel-concatenated messages: %s", curr);
							curr += strlen(curr) + 1;
						}
					}
				}
			}

			else
			{
				sprintf_s(msg, "ok to start upload %u", Counter);
				Counter++;
				oTESTB( Client->Send(msg, (oSocket::size_t)strlen(msg)+1), "SERVER: %s send failed", Server->GetDebugName() );
				oTRACE("SVRCXN: %s waiting to receive...", Client->GetDebugName());
				size_t bytesReceived = Client->Receive(msg, oCOUNTOF(msg));
				
				if (bytesReceived)
				{
					oTRACE("SVRCXN: %s received message: %s", Client->GetDebugName(), msg);

					if (strstr(msg, "goodbye"))
					{
						oTRACE("SVRCXN: Closing connection with one final giant send...");

						char* buf = new char[2 * 1024 * 1024];
						memset(buf, 42, _msize(buf));

						oTESTB(Client->Send(buf, (oSocket::size_t)_msize(buf)), "Failed to send final buffer");

						delete [] buf;

						return SUCCESS;
					}
				}
			}
		}
	}
};

struct TESTSocketReliable : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		{
			int exitcode = 0;
			char msg[512];
			oTESTB(oTestRunSpecialTest("TESTSocketReliableServer", msg, oCOUNTOF(msg), &exitcode), "%s", msg);
			oSleep( 2000 );
		}

		oRef<threadsafe oSocketBlocking> Client;
		{
			oSocketBlocking::DESC desc;
			desc.Peername = SERVER_HOSTNAME;
			desc.ConnectionTimeoutMS = 30000;
			desc.MaxSendSize = 100 * 1024;
			desc.MaxReceiveSize = 100 * 1024;
			oTESTB(oSocketBlocking::Create("Client", desc, &Client), "Failed to create client socket: %s", oGetLastErrorDesc());
		}

		std::vector<char> msg(oMB(2));

		size_t bytesReceived = Client->Receive(oGetData(msg), (oSocket::size_t)oGetDataSize(msg));
		if (bytesReceived)
		{
			oTRACE("CLIENT: %s received message: %s", Client->GetDebugName(), msg);
		}

		else
			oTESTB(false, "CLIENT: First receive failed: %s", oGetErrnoString(oGetLastError()));

		
		oTRACE("CLIENT: Sending...");
		const char* onConnect = "OnConnectMsg";
		oTESTB(Client->Send(onConnect, (oSocket::size_t)strlen(onConnect)+1), "Client Send failed: %s", oGetErrnoString(oGetLastError()));

		oTRACE("CLIENT: waiting to receive...");

		const char* sMessages[] = 
		{
			"Here's message 1",
			"And message 2",
			"And finally a goodbye",
			"This message should not be sent because the server will close the connection. It is sent in response to receiving a large buffer.",
		};

		for (size_t i = 0; i < oCOUNTOF(sMessages); i++)
		{
			*oGetData(msg) = 0;
			oTRACE("CLIENT: Waiting to receive data from server (%u)... ", i);
			size_t bytesReceived = Client->Receive(oGetData(msg), (oSocket::size_t)oGetDataSize(msg));
			oTESTB(bytesReceived, "%u == Receive() failed on message %u %s: %s: %s", bytesReceived, i, i == 3 ? "(a large buffer)" : "", oGetErrnoString(oGetLastError()), oGetLastErrorDesc());
			if (bytesReceived)
			{
				oTRACE("CLIENT: received: %s", bytesReceived < 1024 ? oGetData(msg) : "A large buffer");

				if (bytesReceived > 1024)
				{
					// check the contents of the message received to make sure we got it
					// all correctly.

					for (size_t j = 0; j < bytesReceived; j++)
						oTESTB((unsigned char)msg[j] == 42, "Large buffer compare failed at byte %u", i);

					oTESTB(i >= (oCOUNTOF(sMessages)-1), "Received end msg from server, but we haven't requested a close yet");

					//Send should fail at this point, so don't test it.
					break;
				}

				if (!Client->Send(sMessages[i], (oSocket::size_t)strlen(sMessages[i])+1))
				{
					errno_t err = oGetLastError();
					if (err)
					{
						if (err == ECONNABORTED && i != oCOUNTOF(sMessages) - 1)
							oTESTB(false, "Connection aborted too early.");
						else
							oTESTB(false, "Send failed %s: %s", oGetErrnoString(oGetLastError()), oGetLastErrorDesc());
					}
				}
			}
		}

		// Try to receive data beyond that which is sent to test failure condition
		bytesReceived = Client->Receive(oGetData(msg), (oSocket::size_t)oGetDataSize(msg), 1000);

		oTESTB(!bytesReceived, "client should not have received more data");

		return SUCCESS;
	}
};

struct TestUnreliableSenderThreadProc : public oThread::Proc
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	TestUnreliableSenderThreadProc()
	{
	}

	~TestUnreliableSenderThreadProc()
	{
	}

	void RunIteration()
	{
		if (Sender->Send(&Counter, sizeof(Counter)))
			oTRACE("SENDER: sent %u", Counter);
		else
		{
			oTRACE("SENDER: failed to send (%s)", oGetErrnoString(oGetLastError()));
			oThread::Current()->Exit();
		}

		Counter++;
	}

	bool OnBegin()
	{
		Timeout = INITIAL_CONNECTION_TIMEOUT;
		Counter = 0;

		oSocketSender::DESC desc;
		desc.SendPort = SENDER_PORT;
		desc.ReceiverHostname = SENDER_DESTINATION_HOSTNAME;
		desc.SendBufferSize = 100 * 1024;
		bool success = oSocketSender::Create("Sender", &desc, &Sender);
		if (success)
			threadInitialized.Set();
		else
			oASSERT(false, "winsock error: %s", oGetLastErrorDesc());
		return success;
	}
	
	void OnEnd()
	{
		Sender = 0;
	}

	oEvent threadInitialized;
	oRef<threadsafe oSocketSender> Sender;
	oRefCount RefCount;
	unsigned int Timeout;
	unsigned int Counter;
};

struct TestUnreliableSenderThread : oInterface
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<TestUnreliableSenderThread>());

	TestUnreliableSenderThread()
	{
		Proc /= new TestUnreliableSenderThreadProc();

		if (oThread::Create("Sender Thread", 64*1024, false, Proc, &Thread))
			Proc->threadInitialized.Wait();
	}

	~TestUnreliableSenderThread()
	{
		if (Thread)
		{
			Thread->Exit();
			Thread->Wait(2000);
		}
	}

protected:

	oRef<threadsafe oThread> Thread;
	oRef<TestUnreliableSenderThreadProc> Proc;

	oRefCount RefCount;
};

struct TESTSocketUnreliable : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oRef<TestUnreliableSenderThread> Server;
		Server /= new TestUnreliableSenderThread();

		oRef<threadsafe oSocketReceiver> Receiver;
		{
			oSocketReceiver::DESC desc;
			desc.ReceivePort = RECEIVER_PORT;
			desc.SenderHostname = RECEIVER_SOURCE_HOSTNAME;
			desc.ReceiveBufferSize = 100 * 1024;
			oTESTB(oSocketReceiver::Create("Receiver", &desc, &Receiver), "Failed to create receiver socket: %s", oGetLastErrorDesc());
		}

		double start = oTimer();
		int receivedCounter = 0;
		while (receivedCounter < 10 && (oTimer()-start) < 2.0)
		{
			if (!Receiver->Receive(&receivedCounter, sizeof(receivedCounter)))
				oTESTB(false, "Failed to receive message (%s)", oGetErrnoString(oGetLastError()));

			oTRACE("RECEIVER: got %d (%s)", receivedCounter, oGetErrnoString(oGetLastError()));
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTSocketReliableServer);
oTEST_REGISTER(TESTSocketReliable);
oTEST_REGISTER(TESTSocketUnreliable);

const oGUID& oGetGUID( threadsafe const TestUnreliableSenderThread* threadsafe const * )
{
	// {6BCF135D-BA8B-49db-9D56-A6656DEB790A}
	static const oGUID oIIDTestUnreliableSenderThread = { 0x6bcf135d, 0xba8b, 0x49db, { 0x9d, 0x56, 0xa6, 0x65, 0x6d, 0xeb, 0x79, 0xa } };
	return oIIDTestUnreliableSenderThread;
}
