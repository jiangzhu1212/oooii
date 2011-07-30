// $(header)

#pragma once
#ifndef oNetworking_h
#define oNetworking_h

#include <oooii/oHashString.h>
#include <oooii/oBitStream.h>
#include <oooii/oInterface.h>

typedef unsigned short tPort;
typedef unsigned int tIP;

struct oNetAddress
{
	//tIP ip;
	//tPort port;
	char address[64];
};

struct oMessage
{
	static const int MaxDataSize = 1460 - sizeof(oHashString::tHash);
	unsigned short size;
	BYTE rawData[MaxDataSize];
};

interface oNetworking : oInterface
{
	typedef oFUNCTION<void(const oNetAddress&, const oHashString&, oMessage*)> receiver_callback_t;

	struct DESC
	{
		DESC()
			: Port(0)
			, ReceiveBufferSize(32)
			, SendBufferSize(32)
		{}

		tPort Port;
		size_t ReceiveBufferSize;
		size_t SendBufferSize;
	};

	virtual ~oNetworking() {}

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	virtual const char* GetDebugName() const threadsafe = 0;

	static bool Create(const char* _DebugName, const DESC& _pDesc, threadsafe oNetworking** _ppNetworking);

	virtual bool SendMessage(const oNetAddress& _recipient, const oHashString& _type, oMessage* _pMessage) threadsafe = 0;
	virtual bool RegisterReceiver(const oHashString& _MessageType, receiver_callback_t _Callback) threadsafe = 0;
};

struct oNetAddr;
interface oBuffer;

// oBroadcastStream is a reliable, ordered stream shared by all members of a
// LAN. The RecvCallback will be called by different threads, but is guaranteed
// to only be called by one thread at a time.
interface oBroadcastStreamSender : oInterface
{
	typedef oFUNCTION<void(void*, size_t, const oNetAddr&)> receiver_callback_t;

	struct DESC
	{
		tPort Port;

		// The Global broadcast address is 255.255.255.255, but the subnet
		// broadcast address should be used if known.
		const char* BroadcastAddress;
	};

	virtual void		GetDesc(DESC* _pDesc) const threadsafe = 0;
	virtual const char*	GetDebugName() const threadsafe = 0;

	static bool			Create(const char* _DebugName, const DESC& _Desc, oBroadcastStreamSender** _ppBroadcastStream);

	virtual bool		Send(threadsafe oBuffer* _pBuffer, size_t _Size) = 0;
};

interface oBroadcastStreamReceiver : oInterface
{
	typedef oFUNCTION<void(void*, size_t, const oNetAddr&)> receiver_callback_t;

	struct DESC
	{
		tPort Port;
		receiver_callback_t RecvCallback;

		// The Global broadcast address is 255.255.255.255, but the subnet
		// broadcast address should be used if known.
		const char* BroadcastAddress;
	};

	virtual void		GetDesc(DESC* _pDesc) const threadsafe = 0;
	virtual const char*	GetDebugName() const threadsafe = 0;

	static bool			Create(const char* _DebugName, const DESC& _Desc, oBroadcastStreamReceiver** _ppBroadcastStream);
};

#endif // oNetworking_h
