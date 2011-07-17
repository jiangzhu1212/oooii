// $(header)
#pragma once
#ifndef oIOCP_h
#define oIOCP_h

#include <oooii/oInterface.h>

typedef HANDLE oHandle;

struct oIOCP : public oInterface
{
	typedef oFUNCTION<void(oHandle& _Handle, void* _pOverlapped)> callback_t;

	virtual bool RegisterHandle(oHandle& _Handle, callback_t _Callback) = 0;

	// @oooii-mike: There doesn't seem to be a way to disassociate a HANDLE from
	// an IOCP. This shouldn't ever be a problem, however, as UnregisterHandle
	// should only be called when a HANDLE is being destroyed.
	virtual bool UnregisterHandle(oHandle& _Handle) = 0;

	static bool Create(const char* _DebugName, oIOCP** _ppIOCP);
};

#endif // oIOCP_h
