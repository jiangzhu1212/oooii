// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oTCPServer_h
#define oTCPServer_h
#include <oBasis/oInitOnce.h>
#include <oPlatform/oSocket.h>
#include <set>

//DescT must have at least 2 members named 
//unsigned short Port;
//unsigned int ConnectionTimeoutMS;
//
//HandlerT must have functions ShouldCloseSocket, ProcessSocketReceive, ProcessSocketSend, and InitiateReceive
template<typename HandlerT, typename DescT>
class oTCPServer
{
public:

	oTCPServer(const DescT& _Desc, bool *_pSuccess)
	{
		*_pSuccess = false;

		Desc.Initialize(_Desc);

		oSocketServer2::DESC ServerDesc;
		ServerDesc.ListenPort = Desc->Port;
		ServerDesc.BlockingSettings.RecvTimeout = Desc->ConnectionTimeoutMS;
		ServerDesc.BlockingSettings.SendTimeout = Desc->ConnectionTimeoutMS;
		ServerDesc.NewConnectionCallback = std::bind(&oTCPServer::AcceptConnection, this, std::placeholders::_1);

		if(!oSocketServer2Create("oHTTPServerSeocket", ServerDesc, &SocketServer))
			return;

		*_pSuccess = true;
	}
	~oTCPServer()
	{
		SocketServer = nullptr;
		ConnectedSockets.clear();
	}

	void GetDesc(DescT* _pDesc) threadsafe { *_pDesc = *Desc.c_ptr(); }

private:

	void AcceptConnection(threadsafe oSocket* _Socket)
	{
		oSocket::ASYNC_SETTINGS settings;
		settings.Callback = ouro::intrusive_ptr<HandlerT>(new HandlerT(*Desc), false);
		_Socket->GoAsynchronous(settings);

		((HandlerT*)settings.Callback.c_ptr())->InitiateReceive(_Socket);

		std::set<const ouro::intrusive_ptr<threadsafe oSocket>> socketsToRemove;

		{
			ouro::lock_guard<ouro::shared_mutex> lock(ConnectedSocketsMutex);
			ConnectedSockets.push_back(_Socket);
		}

		// goes through twice, first time with a read lock, and if necessary a 
		// second time with a write lock. Hopefully that second time doesn't happen 
		// often.
		{
			ouro::shared_lock<ouro::shared_mutex> lock(ConnectedSocketsMutex);
			for (const auto& _socket : ConnectedSockets)
			{
				oSocket::DESC desc;
				_socket->GetDesc(&desc);
				ouro::intrusive_ptr<HandlerT> handler;
				desc.AsyncSettings.Callback->QueryInterface(&handler);
				if (handler->ShouldCloseSocket(_socket))
					socketsToRemove.insert(_socket);
			}
		}

		// don't care if we get the job or not, surely someone will someday.
		if (!socketsToRemove.empty() && ConnectedSocketsMutex.try_lock())
		{
			auto it = std::remove_if(std::begin(ConnectedSockets), std::end(ConnectedSockets)
				, [&](const ouro::intrusive_ptr<threadsafe oSocket>& _socket) -> bool
					{
						if (socketsToRemove.find(_socket) != std::end(socketsToRemove))
							return true;
						return false;
					});
			ConnectedSockets.erase(it, std::end(ConnectedSockets));
			ConnectedSocketsMutex.unlock();
		}
	}

	oInitOnce<DescT> Desc;
	ouro::intrusive_ptr<threadsafe oSocketServer2> SocketServer;
	ouro::shared_mutex ConnectedSocketsMutex;
	std::vector<ouro::intrusive_ptr<threadsafe oSocket>> ConnectedSockets;
};

#endif
