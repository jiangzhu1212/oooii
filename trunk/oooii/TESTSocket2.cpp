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
#include <oooii/oTest.h>
#include <oooii/oProcess.h>
#include <oooii/oRef.h>
#include <oooii/oSocket.h>
#include <oooii/oooii.h>

const char* TEST_SOCKET_2_SERVER_HOSTNAME = "localhost:1234";
static const unsigned int TEST_PORT = 1234;

static const char TESTSocket2Question[] = "What is your favorite color?";
static const char TESTSocket2Answer0[] = "It's green...";
static const char TESTSocket2Answer1[] = "No, red...";
static const char TESTSocket2Answer2[] = "No.  Ahhhhh!!";

static const char TestSocket2Statement1[] = "That's no ordinary rabbit!";

struct TESTSocket2Server : public oSpecialTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oRef<threadsafe oSocketServer> Server;
		oSocketServer::DESC Desc;
		Desc.ListenPort = TEST_PORT;
		Desc.MaxNumConnections = 1;
		oTESTB( oSocketServer::Create( "TESTSocketServer", Desc, &Server ), "Failed to create server" );

		oRef<threadsafe oSocketAsyncReceiver> Receiver;
		oTESTB( oSocketAsyncReceiver::Create( "TESTSocketServerReceiver", &Receiver ), "Failed to create receiver!" );

		oRef<threadsafe oSocketAsync> Client;
		oTESTB( Server->WaitForConnection( &Client, 3000 ), "Failed to connect client");

		Receiver->AddSocket( Client, oBIND( &TESTSocket2Server::ReceiveMessage, this, oBIND1, oBIND2 ) );

		Sleep( 500 );

		static const size_t MsgLeng = oCOUNTOF( TESTSocket2Question ) + 1;
		char assembledMessage[MsgLeng];
		memset( assembledMessage, NULL, oCOUNTOF(TESTSocket2Question) );

		int cSrc = 0;
		int cDest = 0;
		for(; cSrc < MsgLeng; ++cSrc )
		{
			if( !msg[cSrc] )
				++cSrc;

			assembledMessage[cDest++] = msg[cSrc];
		}

		oTESTB(  strcmp( assembledMessage, TESTSocket2Question ) == 0, "Failed to receive expected question.");

		const char* msgs[3];
		msgs[0] = &TESTSocket2Answer0[0];
		msgs[1] = &TESTSocket2Answer1[0];
		msgs[2] = &TESTSocket2Answer2[0];

		for( int i = 0; i < 3; ++i )
		{
			void* pData;
			oSocket::size_t sendSz = static_cast<oSocket::size_t>(strlen( msgs[i] ));
			Client->MapSend( sendSz, &pData );
			memcpy(pData, (void*)msgs[i], sendSz );
			Client->UnmapSend( sendSz, pData );
		}

		Sleep( 500 );
		oTESTB( strcmp( msg, TestSocket2Statement1 ) == 0, "Failed to receive expected question.");

		return SUCCESS;
	}

	void ReceiveMessage(void* _pMessage, oSocket::size_t size )
	{
		memcpy( &msg[0], _pMessage, size );
	}

	char msg[_MAX_PATH];
};


TESTSocket2Server TESTSocket2ServerInstance;

struct TESTSocket2 : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oSocketAsync::DESC desc;
		desc.Peername = TEST_SOCKET_2_SERVER_HOSTNAME;
		desc.ConnectionTimeoutMS = 3000;
		desc.MaxSendSize = 16;
		desc.MaxReceiveSize = 16;

		oRef<threadsafe oSocketAsync> Client;
		oTESTB(!oSocketAsync::Create("Client", desc, &Client), "Incorrectly created client when server wasn't running", oGetLastErrorDesc());
		oTESTB(!Client, "Valid client when there shouldnt be one!");
		threadsafe oRef<oProcess> Server;
		{
			int exitcode = 0;
			char msg[512];
			oTESTB(oTestRunSpecialTest("TESTSocket2Server", msg, oCOUNTOF(msg), &exitcode, &Server), "%s", msg);
		}

		oTESTB(oSocketAsync::Create("Client", desc, &Client), "Failed to create client", oGetLastErrorDesc());

		oRef<threadsafe oSocketAsyncReceiver> Receiver;
		oTESTB( oSocketAsyncReceiver::Create( "TestSocket2Reciever", &Receiver ), "Failed to create receiver!" );
		// Bind the receiver to look for the answers later
		int ResponseCount = 0;
		{
			const char* Responses[3];
			Responses[0] = &TESTSocket2Answer0[0];
			Responses[1] = &TESTSocket2Answer1[0];
			Responses[2] = &TESTSocket2Answer2[0];
		

			Receiver->AddSocket( Client, oBIND( &TESTSocket2::ReceiveMessage, this, oBIND1, oBIND2, Responses, &ResponseCount ) );
		}
		
		void* pData;
		Client->MapSend( oCOUNTOF(TESTSocket2Question) * 3 , &pData );
		memcpy(pData, TESTSocket2Question, oCOUNTOF(TESTSocket2Question) );
		Client->UnmapSend(oCOUNTOF(TESTSocket2Question),  pData );

		oSleep( 1000 );

		bool bSuccess = false;
		for( int i =0; i < 100; ++i )
		{
			if( ResponseCount == 3 )
			{
				bSuccess = true;
				break;
			}
			Sleep( 10 );
		}

		oTESTB( bSuccess, "Failed to receive answer!");

		// Send the first statement
		Client->MapSend(oCOUNTOF(TestSocket2Statement1), &pData );
		memcpy( pData,TestSocket2Statement1, oCOUNTOF(TestSocket2Statement1) );
		Client->UnmapSend( oCOUNTOF(TestSocket2Statement1), pData );

		return SUCCESS;
	}

	void ReceiveMessage(void* _pMessage, oSocket::size_t size, 	const char** ResponsesToCheck, int* ResponseCount )
	{
		const char* resp = ResponsesToCheck[*ResponseCount];
		char incoming[64];
		memset( incoming, NULL, 64 );
		memcpy( &incoming[0], _pMessage, size );
		if( 0 == strcmp( incoming, resp ) )
			++(*ResponseCount);
	}
};

TESTSocket2 TESTSocket2Instance;