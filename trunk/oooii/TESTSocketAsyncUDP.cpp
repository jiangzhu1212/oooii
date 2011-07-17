// $(header)
#include <oooii/oErrno.h>
#include <oooii/oEvent.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSocket.h>
#include <oooii/oStdio.h>
#include <oooii/oTest.h>
#include <oooii/oThread.h>
#include <oooii/oProcess.h>
#include <oooii/oooii.h>

const int NUM_PACKETS = 10;
const int MSG_SIZE = 100;

struct TESTSocketAsyncUDP : public oTest
{
	void ReceiveCB(void* _pData, oSocket::size_t _Size, const oNetAddr& _Source)
	{
		char sourceHost[512];
		oToString(sourceHost, _Source);
		oTRACE("Received data from %s", sourceHost);
		oINC(&PacketsRecvd);

		// Queue up a new receive with the same buffer.
		Socket->Recv(_pData, MSG_SIZE);
	}

	void SendCB(void* _pData, oSocket::size_t _Size, const oNetAddr& _Source)
	{

	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		PacketsRecvd = 0;

		BYTE RecvBuf[NUM_PACKETS][MSG_SIZE];

		oRef<threadsafe oSocketAsyncUDP> _Socket;
		{
			oSocketAsyncUDP::DESC desc;
			desc.Port = 11000;
			desc.RecvCallback = oBIND(&TESTSocketAsyncUDP::ReceiveCB, this, oBIND1, oBIND2, oBIND3);
			desc.SendCallback = oBIND(&TESTSocketAsyncUDP::SendCB, this, oBIND1, oBIND2, oBIND3);
			oTESTB(oSocketAsyncUDP::Create("Socket", &desc, &_Socket), "Failed to create client socket: %s", oGetLastErrorDesc());

			Socket = _Socket.c_ptr();

			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			unsigned long NumThreads = sysInfo.dwNumberOfProcessors;

			for(unsigned long i = 0; i < NumThreads; i++)
			{
				Socket->Recv(RecvBuf[i], MSG_SIZE);
			}
		}

		oNetAddr DestinationAddr;
		oFromString(&DestinationAddr, "127.0.0.1:11000");

		BYTE buf[NUM_PACKETS][MSG_SIZE];

		for(int i = 0; i < NUM_PACKETS; i++)
		{
			void* pData = buf[i];

			memset(pData, i, MSG_SIZE);
			Socket->Send(pData, MSG_SIZE, DestinationAddr);
			//oSleep(50);
		}

		double time = oTimer();
		oSPIN_UNTIL(PacketsRecvd >= NUM_PACKETS || oTimer() > time + 2.0);

		oTESTB(PacketsRecvd == NUM_PACKETS, "Received %i of %i packets.", PacketsRecvd, NUM_PACKETS);

		return SUCCESS;
	}

	threadsafe oSocketAsyncUDP* Socket;
	int PacketsRecvd;
};

oTEST_REGISTER(TESTSocketAsyncUDP);
