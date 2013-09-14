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
#include <oBasis/oBuffer.h>
#include <oConcurrency/event.h>
#include <oStd/finally.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oSocket.h>

const unsigned int TESTSocketMessage[] = 
{ 
	0x474e5089,0x0a1a0a0d,0x0d000000,0x52444849,0x80000000,0x40000000,0x00000608,0x7fd6d200,0x0000007f,0x47527301,
	0xceae0042,0x0000e91c,0x4b620600,0xff004447,0xff00ff00,0x93a7bda0,0x09000000,0x73594870,0x130b0000,0x130b0000,
	0x9c9a0001,0x00000018,0x4d497407,0x05db0745,0x011b0215,0xf341bfc6,0x1d000000,0x74585469,0x6d6d6f43,0x00746e65,
	0x00000000,0x61657243,0x20646574,0x68746977,0x4d494720,0x652e6450,0x03000007,0x4144496c,0xedda7854,0xc3b6d95d,
	0x398c0820,0xe65ffff9,0xf1ad34be,0x6d02e2ca,0xdb5bcccc,0x80e29054,0x1b44d026,0x163c06d1,0x07004c3b,0xe38c1e00,
	0x182a2af2,0xc87de624,0x060f970f,0x10000840,0x90ec2e02,0xff446c5e,0x470aa7de,0xb3f7d771,0x58f1dcd3,0xd7cc9439,
	0x60cf46d2,0xa704c480,0x25b4fdf3,0x6727192c,0x5748716c,0xecc56806,0xdab511d5,0x11955128,0xc8ed5e56,0x067e8001,
	0x21aa8da0,0xf9d98a40,0x33cfd5f5,0x64573bda,0x5afe5e39,0xcfd0af1d,0xecd7356b,0xb357d350,0xd2fb4955,0x48d72ddc,
	0x4ce8b9f6,0x1cef9b1f,0x80f96a05,0xbae92066,0x8e57989e,0xcaf6bda7,0xb176ac59,0x97a117e8,0x3d2533a8,0xde2ab404,
	0x38cb4ab2,0x6d21616c,0x99ae5b55,0x64d4ab95,0x672b56d5,0xb1744a13,0x56eea8c1,0x0f32e4db,0x4a49034f,0x9d584a9d,
	0xd85c9a40,0xb1b371d6,0xdac74e64,0x9e4ef082,0xe6d33aab,0xf13d00ef,0x36db36ff,0xc961ae43,0x08e1be59,0xd9f7cda5,
	0x8d1aa05b,0xb6e08695,0x44837760,0x4039d1d9,0xfd99ce93,0x468bc977,0x678b55a8,0x4a8bfc65,0x5a56bc88,0xf95b932e,
	0x7f8276df,0x0b6a533a,0x71ad2701,0x4996c2c7,0xd6683196,0xc79db4c4,0xddd6394c,0xe5079e40,0x505d6bdf,0xeaaaf323,
	0x4107c14c,0xee9a766a,0xc5c89a73,0x3037697b,0x3a6ea2c2,0x5d3818bf,0xe85b091e,0xbe18b87e,0x2c403cf0,0x8bbf0003,
	0xea201d25,0x1c05605c,0xfb8a6a26,0xc6befae5,0x8c53236d,0x1fcf6ffc,0x5890100c,0xba848aae,0x0000e400,0x0380001c,
	0x5805d800,0xb8cdd0ee,0xec633371,0xe73c3191,0x8e989236,0x4bceb7ca,0x200419bd,0x00700004,0xc0000e00,0x00380001,
	0x147030b0,0xe000060c,0x001c0000,0x0f000380,0x24c2b1c4,0x78e8676a,0x01c7a420,0x982a7a56,0xf1efe784,0x0c5af054,
	0x03c2d620,0xb401cbcf,0x49005dea,0x80879020,0x56b0755a,0x9015c8e0,0xde53ed4a,0xb09f139f,0x9149b4f5,0x244d6df4,
	0xca95df2d,0xc45a5bc3,0x3d4b6458,0xec793408,0x87945438,0xa91ce7fb,0x94f78b39,0x5aec7b37,0xf28cea61,0xa8f359f0,
	0xd36afcea,0x240cf473,0x092b7d96,0xc9746d19,0xf8fc6742,0x331dced6,0xb3cdc90d,0xca3c0956,0xe625234e,0x63e9d955,
	0xa0cbddfb,0x7aa28d1e,0x4c110577,0x33fde970,0xeab93431,0x2b09767c,0x0a8db45b,0xf4a462d1,0x209de290,0xf28cf939,
	0x339d3770,0x8937bbfb,0xf1bf3f62,0x8cc95d49,0x879a525a,0xce55a883,0xe75be039,0xdeec1397,0xe4ab708a,0x0a1ac054,
	0xda8a7624,0xef32d7f7,0xd1990696,0x73ab29c9,0xae389b4b,0x7ae5a843,0x7a584b6e,0x38accce1,0x4667f0ea,0x53ad188b,
	0x96ddf4e8,0x6b8622d4,0x6093dad5,0xea883479,0x41e2c169,0x44bb56b9,0x3b2d00e7,0xb877fcb3,0xa976123c,0x64ccf20c,
	0x4c66f07f,0xf4f0b768,0x7c85d945,0x1c3f051c,0xad33b8c7,0xda218ff5,0xd86af651,0x7a1be25b,0x6fc538ee,0xfa99a6c7,
	0x4db45842,0x0723e41c,0x00435a00,0x32a03cf0,0xc0000c08,0xe2c78001,0xf93a170f,0x80553b95,0x00000090,0x4e454900,
	0x6042ae44,0x00000082,
};

static const char TESTSocketTCP0[] = "It was the best of times, it was the worst of times, it was the age of wisdom, it was the age of foolishness, it was the epoch of belief, it was the epoch of incredulity, it was the season of Light, it was the season of Darkness, it was the spring of hope, it was the winter of despair, we had everything before us, we had nothing before us, we were all going direct to heaven, we were all going direct the other way - in short, the period was so far like the present period, that some of its noisiest authorities insisted on its being received, for good or for evil, in the superlative degree of comparison only.";
static const char TESTSocketTCP1[] = "I hate Dickens.";

static const char* TESTSocketTCPServer = "localhost:4545";
static const unsigned int TESTSocketTCPTimeout = 2000;
static const int TESTNumUDPAttempts = 20;

struct TCPServerCallback : public oSocketAsyncCallback
{
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_NOOP_QUERYINTERFACE();

	TCPServerCallback()
		: CurrentOffset(0)
	{
	}

	void ProcessSocketReceive(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe override
	{
		thread_cast<TCPServerCallback*>(this)->ProcessTCPReceive(_pData, _SizeData,_pSocket);
	}

	void ProcessTCPReceive(void*_pData, oSocket::size_t _SizeData, threadsafe oSocket* _pSocket)
	{
		CurrentOffset += _SizeData;
		if(CurrentOffset == ExpectedMessageLength)
		{
			CurrentOffset = 0;
			MessageArrived.set();
		}
		else
			InitiateReceive(_pSocket);
	}
	void ProcessSocketSend(void*_pHeader, void* _pBody, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe override
	{ 
		MessageArrived.set();
	}

	void InitiateReceive(threadsafe oSocket* _pSocket, bool _IssueCompleteReceive = false)
	{
		if(_IssueCompleteReceive)
			_pSocket->Recv(Results, ExpectedMessageLength);
		else
			_pSocket->Recv(Results + CurrentOffset, __min(oCOUNTOF(Results) - CurrentOffset, 32));
	}

	oRefCount Refcount;

	unsigned int CurrentOffset;
	unsigned int ExpectedMessageLength;
	oConcurrency::event MessageArrived;
	char Results[1024*1024];
};

struct PLATFORM_oSocketAsync : public oTest
{
	struct GenericCallback : public oSocketAsyncCallback
	{
		oDEFINE_REFCOUNT_INTERFACE(Refcount);
		oDEFINE_NOOP_QUERYINTERFACE();

		GenericCallback()
			: SendCount(0)
		{
			void* pData = new char[sizeof(TESTSocketMessage)];
			memset(pData, NULL, sizeof(TESTSocketMessage));
			oBufferCreate("Generic Callback buffer", pData, sizeof(TESTSocketMessage), oBuffer::Delete, &ReceiveBuffer);
		}

		void ProcessSocketReceive(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe override
		{
			InitiateReceive(_pSocket);
			SendCount.fetch_add(1);
		}
		void ProcessSocketSend(void*_pHeader, void* _pBody, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe override
		{ 
			SendCount.fetch_add(1);
		}

		void InitiateReceive(threadsafe oSocket* _pSocket) threadsafe
		{
			oLockedPointer<oBuffer> LockedBuffer(ReceiveBuffer);
			_pSocket->Recv(LockedBuffer->GetData(), oUInt(LockedBuffer->GetSize()));
		}

		oStd::atomic_uint SendCount;
		oStd::atomic_uint ReceiveCount;
		oRefCount Refcount;

		oStd::intrusive_ptr<oBuffer> ReceiveBuffer;
	};

	RESULT TestUDPServerASYNC(char* _StrStatus, size_t _SizeofStrStatus, unsigned short Port)
	{
		oSocket::DESC SocketDesc;
		SocketDesc.Protocol = oSocket::UDP;
		oSocketPortSet(Port, &SocketDesc.Addr);
		

		oStd::intrusive_ptr<GenericCallback> Sender( new GenericCallback, false );
		
		SocketDesc.Style = oSocket::ASYNC;
		SocketDesc.AsyncSettings.Callback = Sender;

		oStd::intrusive_ptr<threadsafe oSocket> Socket;
		oTESTB(oSocketCreate("Test UDP", SocketDesc, &Socket), oErrorGetLastString());

		char AddrStr[64];
		oPrintf(AddrStr, "localhost:%u", Port);

		oNetAddr Addr;
		oStd::from_string(&Addr, AddrStr);

		for(unsigned int i = 0; i < SocketDesc.AsyncSettings.MaxSimultaneousMessages; ++i)
		{
			Socket->SendTo(TESTSocketMessage, sizeof(TESTSocketMessage), Addr);
		}
		oSleep(1000);
		oTESTB(Sender->SendCount.fetch_add(0) == SocketDesc.AsyncSettings.MaxSimultaneousMessages, "UDPSender: Failed to account for all sends");

		return SUCCESS;
	}
	RESULT TestTCPServerASYNC(char* _StrStatus, size_t _SizeofStrStatus)
	{
		oStd::intrusive_ptr<TCPServerCallback> Receiver(new TCPServerCallback, false);

		oStd::intrusive_ptr<threadsafe oSocketServer2> Server;
		oStd::intrusive_ptr<threadsafe oSocket> Socket;
		oConcurrency::event connectEvent;

		oSocketServer2::DESC ServerDesc;
		ServerDesc.ListenPort = 4545;
		ServerDesc.BlockingSettings.RecvTimeout = 10000;
		ServerDesc.BlockingSettings.SendTimeout = 10000;
		ServerDesc.NewConnectionCallback = [&](threadsafe oSocket* _Socket){
			if(!Socket)
			{
				Socket = _Socket;

				oTRACE("received connection from %s (%s)", Socket->GetDebugName(), oErrorGetLast() ? oErrorAsString(oErrorGetLast()) : "OK");
				connectEvent.set();
			}
		};

		oTESTB(oSocketServer2Create("TestTCPServerASYNC", ServerDesc, &Server), oErrorGetLastString());

		oTESTB(connectEvent.wait_for(oStd::chrono::milliseconds(2000)), "Timed out waiting for connection");

		oSocket::ASYNC_SETTINGS Settings;
		Settings.Callback = Receiver;
		Socket->GoAsynchronous(Settings);

		Receiver->ExpectedMessageLength = oUInt(oStrlen(TESTSocketTCP0)) + 1;
		
		oSocket::DESC Desc;
		Socket->GetDesc(&Desc);
		Receiver->InitiateReceive(Socket);

		oTESTB(Receiver->MessageArrived.wait_for(oStd::chrono::milliseconds(TESTSocketTCPTimeout)), "Failed to recieve TCP message from client");
		oTESTB(0 == oStricmp(Receiver->Results, TESTSocketTCP0), "Failed to retrieve TCP message");
		Receiver->MessageArrived.reset();
		Socket->Send(TESTSocketTCP1, oCOUNTOF(TESTSocketTCP1));
		oTESTB(Receiver->MessageArrived.wait_for(oStd::chrono::milliseconds(TESTSocketTCPTimeout)), "Failed to send TCP message to client");

		Server = nullptr; //make sure this goes away first, he references some locals in callback lambdas.

		return SUCCESS;
	}

	void RunServer(oFUNCTION<RESULT(char* _StrStatus, size_t _SizeofStrStatus)> _ServerRoutine)
	{
		ServerResult = _ServerRoutine(ServerResultString.c_str(), ServerResultString.capacity());
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		// Test support code
		{
			oNetAddr TestAddress;
			oStd::from_string(&TestAddress, "192.168.10.2:1234");

			unsigned short Port = 0;
			oSocketPortGet(TestAddress, &Port);
			oTESTB(1234 == Port, "Port is %s expected 1234.", Port);

			// Reset
			TestAddress = oNetAddr();
			oSocketPortSet(4242, &TestAddress);
			oSocketPortGet(TestAddress, &Port);
			oTESTB(4242 == Port, "Port is %s expected 4242.", Port);
		}

		// Create our server thread
		
		std::shared_ptr<oConcurrency::task_group> g = oConcurrency::make_task_group();
		oStd::finally onexit([&] { g->wait(); });

		static const unsigned short TestPort = 30777;

		// Test UDP Async
		{
			// UDP is unreliable, seems especially so on localhost, so if we fail to receive our response back, try again.
			// Most of the time this test will pass on the first try.  Sometimes it may take more tries.
			// We fail after TESTNumUDPAttempts times.  The test is successful if we receive our message back.
			bool bSuccess = false;
			int totalCount;
			for (totalCount = 0; totalCount < TESTNumUDPAttempts; ++totalCount)
			{
				// Issue server command
				ServerResult = FAILURE;
				oFUNCTION<RESULT(char* _StrStatus, size_t _SizeofStrStatus)> Func = oBIND(&PLATFORM_oSocketAsync::TestUDPServerASYNC, this, oBIND1, oBIND2, TestPort);
				g->run(oBIND(&PLATFORM_oSocketAsync::RunServer, this, Func));

				oSocket::DESC SocketDesc;
				SocketDesc.Protocol = oSocket::UDP;
				oSocketPortSet(TestPort, &SocketDesc.Addr);

				oStd::intrusive_ptr<GenericCallback> Callback(new GenericCallback, false );

				SocketDesc.Style = oSocket::ASYNC;
				SocketDesc.AsyncSettings.Callback = Callback;

				oStd::intrusive_ptr<threadsafe oSocket> Socket;
				oTESTB(oSocketCreate("Test UDP", SocketDesc, &Socket), oErrorGetLastString());

				// Initiate the first receive
				Callback->InitiateReceive(Socket);

				g->wait();
				oTESTB(SUCCESS == ServerResult, ServerResultString.c_str());

				oLockedPointer<oBuffer> Locked(Callback->ReceiveBuffer);
				void *pTest = Locked->GetData();
				if (0 == memcmp(Locked->GetData(), TESTSocketMessage, sizeof(TESTSocketMessage)))
				{
					bSuccess = true;
					break;
				}
			}

			if (bSuccess)
				oPrintf(_StrStatus, _SizeofStrStatus, "UDP Tests Successful: (%i tries)", totalCount + 1);
			else
			{
				oTESTB(bSuccess, "Receive failed to get message (%i tries)", totalCount + 1);
			}
		}
		
		// Test TCP Async
		oSocket::PROTOCOL Protocols[] = {oSocket::TCP};
		for(int i = 0; i < oCOUNTOF(Protocols); ++i)
		{
			// Issue server command	
			ServerResult = FAILURE;
			oFUNCTION<RESULT(char* _StrStatus, size_t _SizeofStrStatus)> Func = oBIND(&PLATFORM_oSocketAsync::TestTCPServerASYNC, this, oBIND1, oBIND2);
			g->run(oBIND(&PLATFORM_oSocketAsync::RunServer, this, Func));

			oSocket::DESC SocketDesc;
			SocketDesc.Protocol = Protocols[i];
			SocketDesc.ConnectionTimeoutMS = TESTSocketTCPTimeout;
			oStd::from_string(&SocketDesc.Addr, TESTSocketTCPServer);

			struct TCPSender : public oSocketAsyncCallback
			{
				oDEFINE_REFCOUNT_INTERFACE(Refcount);
				oDEFINE_NOOP_QUERYINTERFACE();

				void ProcessSocketReceive(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe override
				{

				}
				void ProcessSocketSend(void*_pHeader, void* _pBody, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe override
				{
				}

				oRefCount Refcount;
			};

			oStd::intrusive_ptr<TCPSender> Sender(new TCPSender, false );

			SocketDesc.Style = oSocket::ASYNC;
			SocketDesc.AsyncSettings.Callback = Sender;
			oStd::intrusive_ptr<threadsafe oSocket> Socket;
			oTESTB0(oSocketCreate("Test TCP", SocketDesc, &Socket));
			oTESTB0(Socket->IsConnected());

			// Split the send into two parts
			oSocket::size_t Split = 128;
			Socket->Send(TESTSocketTCP0, Split, oStd::byte_add(TESTSocketTCP0, Split), oCOUNTOF(TESTSocketTCP0) - Split);

			g->wait();
			oTESTB(SUCCESS == ServerResult, ServerResultString.c_str());
		}
				
		return SUCCESS;
	}

	RESULT ServerResult;
	oStd::fixed_string<char, 256> ServerResultString;
};

oTEST_REGISTER(PLATFORM_oSocketAsync);