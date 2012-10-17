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
#include <oPlatform/oHTTPServer.h>
#include <set>
#include "oHTTPInternal.h"
#include "oHTTPProtocol.h"

//when we commit to socket server 2 be sure to remove the dispatch queue that is passed into the create function, it wont be needed anymore
#define USE_SOCKET_SERVER2

class oHTTPServer_Impl : public oHTTPServer
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oHTTPServer_Impl(const DESC& _Desc, threadsafe oDispatchQueue* _pDispatchQueue, bool *pSuccess);
	~oHTTPServer_Impl();
	void GetDesc(DESC* _pDesc) override {* _pDesc = Desc; }

	struct CallbackHandler : public oSocketAsyncCallback
	{
	public:
		oDEFINE_REFCOUNT_INTERFACE(Refcount);
		oDEFINE_TRIVIAL_QUERYINTERFACE(CallbackHandler);

		CallbackHandler(const oHTTPServer::DESC& _Desc)
			: Desc(_Desc)
			, LastTimeStamp(0)
			, bCloseSocket(false)
			, Protocol(oHTTPProtocol::oHTTP_PROTOCOL_SERVER_MODE)
		{
			Protocol.Desc.StartResponse = nullptr;
			Protocol.Desc.FinishResponse = Desc.FinishResponse;
			Protocol.Desc.SupportedMethods = Desc.SupportedMethods;
		}

		~CallbackHandler() {}
		void InitiateReceive(threadsafe oSocket* _pSocket)
		{
			LastTimeStamp = oTimerMS();
			_pSocket->Recv(&ReceiveBuffer[0], oInt(oCOUNTOF(ReceiveBuffer)));
		}

		bool ShouldCloseSocket() { return bCloseSocket || oTimerMS() - LastTimeStamp > HTTPRequestTimeoutMS; }

	private:
		virtual void ProcessSocketReceive(void* _pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe;
		virtual void ProcessSocketSend(void* _pHeader, void* _pBody, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe;
		
#ifdef SUSPECTED_BUGS_IN_HTTP
		oGuardBand<64> TopGuard;
#endif
		oRefCount Refcount;
		oHTTPServer::DESC Desc;

		char ReceiveBuffer[HTTPRequestSize];
#ifdef SUSPECTED_BUGS_IN_HTTP
		oGuardBand<64> ReceiveGuard;
#endif
		unsigned int LastTimeStamp;
		bool bCloseSocket;

		oHTTPProtocol Protocol;
#ifdef SUSPECTED_BUGS_IN_HTTP
		oGuardBand<64> ProtocolGuard;
#endif
	};

private:

#ifdef USE_SOCKET_SERVER2
	void AcceptConnection(threadsafe oSocket* _Socket);
#else
	void ServerRoutine(oRef<threadsafe oDispatchQueue> _Queue);
#endif

	DESC Desc;
	oRefCount RefCount;
#ifdef USE_SOCKET_SERVER2
	oRef<threadsafe oSocketServer2> SocketServer;
	oLockedVector<oRef<threadsafe oSocket>> ConnectedSockets;
#else
	oRef<threadsafe oSocketServer> SocketServer;
	oRef<threadsafe oDispatchQueuePrivate> SelfConstructedQueue;
	std::vector<oRef<threadsafe oSocket>> ConnectedSockets;
#endif


};

const oGUID& oGetGUID(threadsafe const oHTTPServer_Impl::CallbackHandler* threadsafe const *)
{
	// {1C6B1DFE-C1AC-4EFE-B533-3865C0549187}
	static const oGUID oIIDoCallbackHandler =  { 0x1c6b1dfe, 0xc1ac, 0x4efe, { 0xb5, 0x33, 0x38, 0x65, 0xc0, 0x54, 0x91, 0x87 } };
	return oIIDoCallbackHandler;
}

oHTTPServer_Impl::oHTTPServer_Impl(const DESC& _Desc, threadsafe oDispatchQueue* _pDispatchQueue, bool* _pSuccess)
{
#ifdef USE_SOCKET_SERVER2
	oTRACEA("*********** Reminder, remove _pDispatchQueue once socket server 2 is switched to permanently.");
#endif

	Desc = _Desc;

#ifdef USE_SOCKET_SERVER2
	oSocketServer2::DESC ServerDesc;
	ServerDesc.ListenPort = Desc.Port;
	ServerDesc.BlockingSettings.RecvTimeout = Desc.ConnectionTimeoutMS;
	ServerDesc.BlockingSettings.SendTimeout = Desc.ConnectionTimeoutMS;
	ServerDesc.NewConnectionCallback = oBIND(&oHTTPServer_Impl::AcceptConnection, this, oBIND1);

	if(!oSocketServer2Create("oHTTPServerSeocket", ServerDesc, &SocketServer))
		return;

#else
	// Start up the server
	oSocketServer::DESC socketServerDesc;
	socketServerDesc.ListenPort = Desc.Port;
	if (!oSocketServerCreate("oHTTPServerSeocket", socketServerDesc, &SocketServer))
		return;

	if(!_pDispatchQueue)
	{
		if(!oDispatchQueueCreatePrivate("HTTPServer server thread", 2, &SelfConstructedQueue))
			return;

		_pDispatchQueue = SelfConstructedQueue;
	}

	_pDispatchQueue->Dispatch(&oHTTPServer_Impl::ServerRoutine, this, _pDispatchQueue);
#endif

	*_pSuccess = true;
}

oHTTPServer_Impl::~oHTTPServer_Impl()
{
#ifndef USE_SOCKET_SERVER2
	if (SelfConstructedQueue)
		SelfConstructedQueue->Join();
#endif
	SocketServer = nullptr;

	ConnectedSockets.clear();
}

#ifdef USE_SOCKET_SERVER2
void oHTTPServer_Impl::AcceptConnection(threadsafe oSocket* _Socket)
{
	oSocket::ASYNC_SETTINGS settings;
	settings.Callback = oRef<CallbackHandler>(new CallbackHandler(Desc), false);
	_Socket->GoAsynchronous(settings);

	((CallbackHandler *)settings.Callback.c_ptr())->InitiateReceive(_Socket);

	ConnectedSockets.push_back(_Socket);

	std::set<const oRef<threadsafe oSocket>> socketsToRemove;

	//goes through twice, first time with a read lock, and if necessary a second time with a write lock. 
	//	hopefully that second time doesn't happen often
	ConnectedSockets.const_foreach([&] (const oRef<threadsafe oSocket>& _socket) -> bool {
		oSocket::DESC desc;
		_socket->GetDesc(&desc);
		oRef<CallbackHandler> handler;
		desc.AsyncSettings.Callback->QueryInterface(&handler);
		if (handler->ShouldCloseSocket())
			socketsToRemove.insert(_socket);
		return true;
	});

	if(!socketsToRemove.empty())
	{
		//don't care if we get the job or not, surely someone will someday.
		ConnectedSockets.try_erase_if([&](const oRef<threadsafe oSocket>& _socket) -> bool {
			if(socketsToRemove.find(_socket) != end(socketsToRemove))
				return true;
			return false;
		});
	}
}
#else
void oHTTPServer_Impl::ServerRoutine(oRef<threadsafe oDispatchQueue> _Queue)
{
	oSocket::ASYNC_SETTINGS settings;
	settings.Callback = oRef<CallbackHandler>(new CallbackHandler(Desc), false);
	oRef<threadsafe oSocket> socket;

	if (SocketServer->WaitForConnection(settings, &socket, Desc.ConnectionTimeoutMS))
	{
		ConnectedSockets.push_back(socket);
		((CallbackHandler *)settings.Callback.c_ptr())->InitiateReceive(socket);
	}

	for (auto it = ConnectedSockets.begin(); it != ConnectedSockets.end();)
	{
		oSocket::DESC desc;
		(*it)->GetDesc(&desc);
		oRef<CallbackHandler> handler;
		desc.AsyncSettings.Callback->QueryInterface(&handler);
		if (handler->ShouldCloseSocket())
			it = ConnectedSockets.erase(it);
		else
			++it;
	}

	_Queue->Dispatch(&oHTTPServer_Impl::ServerRoutine, this, _Queue);
}
#endif




void oHTTPServer_Impl::CallbackHandler::ProcessSocketReceive(void* _pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe
{
#ifdef SUSPECTED_BUGS_IN_HTTP
	TopGuard.Check();
	ReceiveGuard.Check();
	ProtocolGuard.Check();
#endif
	auto pLockedThis = thread_cast<oHTTPServer_Impl::CallbackHandler*>(this); // Safe because we only get called again when we want it to (when doing another async request)

	if (!pLockedThis->Protocol.Desc.StartResponse)
		pLockedThis->Protocol.Desc.StartResponse = oBIND(pLockedThis->Desc.StartResponse, oBIND1, _Addr.Host, oBIND2);

	if (pLockedThis->Protocol.ProcessSocketReceive(_pData, _SizeData, _pSocket))
	{
		pLockedThis->InitiateReceive(_pSocket);
	}
	else
	{
		pLockedThis->bCloseSocket=true;
	}
#ifdef SUSPECTED_BUGS_IN_HTTP
	TopGuard.Check();
	ReceiveGuard.Check();
	ProtocolGuard.Check();
#endif
}

void oHTTPServer_Impl::CallbackHandler::ProcessSocketSend(void* _pHeader, void* _pBody, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe
{
#ifdef SUSPECTED_BUGS_IN_HTTP
	TopGuard.Check();
	ReceiveGuard.Check();
	ProtocolGuard.Check();
#endif
	auto pLockedThis = thread_cast<oHTTPServer_Impl::CallbackHandler*>(this); // Safe because we only get called again when we want it to (when doing another async request)

	if (!pLockedThis->Protocol.ProcessSocketSend(_pHeader, _pBody, _SizeData, _pSocket))
	{
		pLockedThis->bCloseSocket=true;
	}
#ifdef SUSPECTED_BUGS_IN_HTTP
	TopGuard.Check();
	ReceiveGuard.Check();
	ProtocolGuard.Check();
#endif
}

oAPI bool oHTTPServerCreate(const oHTTPServer::DESC& _Desc, oHTTPServer** _ppServer, threadsafe oDispatchQueue* _pDispatchQueue)
{
	bool success = false;
	oCONSTRUCT(_ppServer, oHTTPServer_Impl(_Desc, _pDispatchQueue, &success));
	return success;
}
