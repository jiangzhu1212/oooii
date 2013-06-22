/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#pragma once
#ifndef oTCPServer_h
#define oTCPServer_h
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
		ServerDesc.NewConnectionCallback = oBIND(&oTCPServer::AcceptConnection, this, oBIND1);

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
		settings.Callback = oRef<HandlerT>(new HandlerT(*Desc), false);
		_Socket->GoAsynchronous(settings);

		((HandlerT*)settings.Callback.c_ptr())->InitiateReceive(_Socket);

		std::set<const oRef<threadsafe oSocket>> socketsToRemove;

		{
			oStd::lock_guard<oStd::shared_mutex> lock(ConnectedSocketsMutex);
			ConnectedSockets.push_back(_Socket);
		}

		// goes through twice, first time with a read lock, and if necessary a 
		// second time with a write lock. Hopefully that second time doesn't happen 
		// often.
		{
			oStd::shared_lock<oStd::shared_mutex> lock(ConnectedSocketsMutex);
			oFOR(const auto& _socket, ConnectedSockets)
			{
				oSocket::DESC desc;
				_socket->GetDesc(&desc);
				oRef<HandlerT> handler;
				desc.AsyncSettings.Callback->QueryInterface(&handler);
				if (handler->ShouldCloseSocket(_socket))
					socketsToRemove.insert(_socket);
			}
		}

		// don't care if we get the job or not, surely someone will someday.
		if (!socketsToRemove.empty() && ConnectedSocketsMutex.try_lock())
		{
			auto it = std::remove_if(std::begin(ConnectedSockets), std::end(ConnectedSockets)
				, [&](const oRef<threadsafe oSocket>& _socket) -> bool
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
	oRef<threadsafe oSocketServer2> SocketServer;
	oStd::shared_mutex ConnectedSocketsMutex;
	std::vector<oRef<threadsafe oSocket>> ConnectedSockets;
};

#endif
