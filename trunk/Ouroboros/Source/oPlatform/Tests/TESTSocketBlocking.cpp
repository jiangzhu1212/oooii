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
#include <oBasis/oError.h>
#include <oBasis/oRefCount.h>
#include <oConcurrency/event.h>
#include <oPlatform/oInterprocessEvent.h>
#include <oPlatform/oSocket.h>
#include <oPlatform/oTest.h>

unsigned short SERVER_PORT = 1234;
unsigned short SENDER_PORT = 1234;
unsigned short RECEIVER_PORT = 1235;

const char* SERVER_HOSTNAME = "localhost:1234";
const char* SENDER_DESTINATION_HOSTNAME = "localhost:1235";
const char* RECEIVER_SOURCE_HOSTNAME = "localhost:1234";

const unsigned int TIMEOUT = 3000;

const unsigned int INITIAL_CONNECTION_TIMEOUT = 10000;

struct PLATFORM_oSocketBlockingServer : public oSpecialTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{		
		oStd::intrusive_ptr<threadsafe oSocketServer2> Server;
		oStd::intrusive_ptr<threadsafe oSocket> Client;
		oConcurrency::event connectEvent;
		connectEvent.reset();
		
		unsigned int Timeout = INITIAL_CONNECTION_TIMEOUT;
		unsigned int Counter = 0;

		oSocketServer2::DESC desc;
		desc.ListenPort = SERVER_PORT;
		desc.BlockingSettings.RecvTimeout = TIMEOUT;
		desc.BlockingSettings.SendTimeout = TIMEOUT;
		desc.NewConnectionCallback = [&](threadsafe oSocket* _Socket){
			if(!Client)
			{
				Client = _Socket;
				oTRACE("SERVER: %s received connection from %s (%s)", Server->GetDebugName(), Client->GetDebugName(), oErrorGetLast() ? oErrorAsString(oErrorGetLast()) : "OK");
				connectEvent.set();
			}
		};

		oTESTB( oSocketServer2Create("Server", desc, &Server), "Failed to create server! winsock error: %s", oErrorGetLastString() );

		if (Timeout)
			oTRACE("SERVER: %s waiting for connection...", Server->GetDebugName());

		NotifyReady();
		
		oTESTB(connectEvent.wait_for(oStd::chrono::milliseconds(2000)), "Timed out waiting for connection");

		const char* s = "Server acknowledges your connection, waiting to receive data.";

		oTESTB( Client->Send(s, (oSocket::size_t)oStrlen(s)+1), "SERVER: Client %s send failed", Client->GetDebugName() );

		while( 1 )
		{
			char msg[1024];
			memset(msg, 0, sizeof(msg));

			if (!Counter)
			{
				oTRACE("SVRCXN: %s waiting to receive OnConnect data...", Client->GetDebugName());
				size_t bytesReceived = Client->Recv(msg, oCOUNTOF(msg));
				if (bytesReceived)
				{
					if (strstr(msg, "OnConnectMsg"))
					{
						oTRACE("SVRCXN: received connection request (%s), issuing upload ok's", msg);
						Counter = 1;
					}
					else
						oTRACE("SVRCXN: wanted OnConnectMsg data, got %s", msg);

					if (bytesReceived > (oStrlen(msg) + 1))
					{
						char* curr = msg + oStrlen(msg) + 1;
						char* end = msg + sizeof(msg);

						while (*curr && curr < end)
						{
							oTRACE("SVRCXN: Nagel-concatenated messages: %s", curr);
							curr += oStrlen(curr) + 1;
						}
					}
				}
			}

			else
			{
				oPrintf(msg, "ok to start upload %u", Counter);
				Counter++;
				oTESTB( Client->Send(msg, (oSocket::size_t)oStrlen(msg)+1), "SERVER: %s send failed", Server->GetDebugName() );
				oTRACE("SVRCXN: %s waiting to receive...", Client->GetDebugName());
				size_t bytesReceived = Client->Recv(msg, oCOUNTOF(msg));

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

		Client = nullptr;
		Server = nullptr;

		return SUCCESS;
	}
};

struct PLATFORM_oSocketBlocking : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		{
			int exitcode = 0;
			char msg[512];
			std::shared_ptr<oCore::process> Server;
			oTESTB(oSpecialTest::CreateProcess("PLATFORM_oSocketBlockingServer", &Server), "");
			oTESTB(oSpecialTest::Start(Server.get(), msg, oCOUNTOF(msg), &exitcode), "%s", msg);
		}

		oStd::intrusive_ptr<threadsafe oSocket> Client;
		{
			oSocket::DESC desc;
			oStd::from_string( &desc.Addr, SERVER_HOSTNAME );
			desc.ConnectionTimeoutMS = INITIAL_CONNECTION_TIMEOUT;
			desc.BlockingSettings.RecvTimeout = TIMEOUT;
			desc.BlockingSettings.SendTimeout = TIMEOUT;
			desc.Style = oSocket::BLOCKING;
			oTESTB(oSocketCreate("Client", desc, &Client), "Failed to create client socket: %s", oErrorGetLastString());
		}

		std::vector<char> msg(oMB(2));

		size_t bytesReceived = Client->Recv(oStd::data(msg), (oSocket::size_t)oStd::size(msg));
		if (bytesReceived)
		{
			oTRACE("CLIENT: %s received message: %s", Client->GetDebugName(), msg);
		}

		else
			oTESTB(false, "CLIENT: First receive failed: %s", oErrorAsString(oErrorGetLast()));


		oTRACE("CLIENT: Sending...");
		const char* onConnect = "OnConnectMsg";
		oTESTB(Client->Send(onConnect, (oSocket::size_t)oStrlen(onConnect)+1), "Client Send failed: %s", oErrorAsString(oErrorGetLast()));

		oTRACE("CLIENT: waiting to receive...");

		const char* sMessages[] = 
		{
			"Here's message 1",
			"And message 2",
			"And oStd::finally a goodbye",
			"This message should not be sent because the server will close the connection. It is sent in response to receiving a large buffer.",
		};

		for (size_t i = 0; i < oCOUNTOF(sMessages); i++)
		{
			*oStd::data(msg) = 0;
			oTRACE("CLIENT: Waiting to receive data from server (%u)... ", i);
			size_t bytesReceived = Client->Recv(oStd::data(msg), (oSocket::size_t)oStd::size(msg));
			oTESTB(bytesReceived, "%u == Receive() failed on message %u %s: %s: %s", bytesReceived, i, i == 3 ? "(a large buffer)" : "", oErrorAsString(oErrorGetLast()), oErrorGetLastString());
			if (bytesReceived)
			{
				oTRACE("CLIENT: received: %s", bytesReceived < 1024 ? oStd::data(msg) : "A large buffer");

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

				if (!Client->Send(sMessages[i], (oSocket::size_t)oStrlen(sMessages[i])+1))
				{
					errno_t err = oErrorGetLast();
					if (err)
					{
						if (err == ECONNABORTED && i != oCOUNTOF(sMessages) - 1)
							oTESTB(false, "Connection aborted too early.");
						else
							oTESTB(false, "Send failed %s: %s", oErrorAsString(oErrorGetLast()), oErrorGetLastString());
					}
				}
			}
		}

		// Try to receive data beyond that which is sent to test failure condition
		bytesReceived = Client->Recv(oStd::data(msg), (oSocket::size_t)oStd::size(msg));

		oTESTB(!bytesReceived, "client should not have received more data");

		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_oSocketBlockingServer);
oTEST_REGISTER(PLATFORM_oSocketBlocking);
