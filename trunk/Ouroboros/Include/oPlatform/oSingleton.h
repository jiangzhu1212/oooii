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
#pragma once
#ifndef oSingleton_h
#define oSingleton_h

#include <oBasis/oGUID.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>

typedef void* (*NewVFn)();

bool oConstructOnceV(void* volatile* _pPointer, NewVFn _New);
template<typename T> bool oConstructOnce(T* volatile* _pPointer, T* (*_New)() ) { return oConstructOnceV((void* volatile*)_pPointer, (NewVFn)_New); }

class oSingletonBase : public oInterface
{
public:
	oSingletonBase(int _InitialRefCount = 1);
	virtual ~oSingletonBase();
	virtual int Reference() threadsafe;
	virtual void Release() threadsafe;
	oDEFINE_NOOP_QUERYINTERFACE();

protected:
	void* hModule;
	const char* Name;
	oRefCount RefCount;
	static void* NewV(const char* _TypeInfoName, size_t _Size, void (*_Ctor)(void*), const oGUID& _GUID, bool _IsThreadLocal);
};

template<typename T, bool ThreadLocal = false>
class oSingletonBaseT : public oSingletonBase
{
public:
	oSingletonBaseT(int _InitialRefCount = 1) : oSingletonBase(_InitialRefCount) {}
protected:
	static void Ctor(void* _Pointer) { new (_Pointer) T(); }
	static T* New() { return static_cast<T*>(NewV(typeid(T).name(), sizeof(T), Ctor, typename T::GUID, ThreadLocal)); } // GUID must be defined as a static member of the derived class
};

template<typename T>
class oProcessSingleton : public oSingletonBaseT<T, false>
{
public:
	oProcessSingleton(int _InitialRefCount = 1)
		: oSingletonBaseT<T, false>(_InitialRefCount)
	{}

	static T* Singleton()
	{
		static T* sInstance;
		if (oConstructOnce(&sInstance, New))
		{
			// If the above static were an oRef, it is possible to go through static
			// deinit, see that sInstance is null and noop on the dtor, only to have
			// later deinit code instantiate the singleton for the first time (for
			// very low-level debug/reporting/allocation singletons). But the dtor had
			// already executed and doesn't get executed again! So ignore anything 
			// that the first static might do and if we are truly the ones to create
			// the instance, ref it here because this will be first-access of this 
			// static and thus the dtor will be registered at this time.
			static oRef<T> sAnotherInstance(sInstance, false);
		}

		return sInstance;
	}
};

template<typename T>
class oThreadlocalSingleton : public oSingletonBaseT<T, true>
{
public:
	static T* Singleton()
	{
		thread_local static T* sInstance;
		oConstructOnce(&sInstance, New);
		return sInstance;
	}
};

template<typename T> class oModuleSingleton : public oNoncopyable
{
public:
	static T* Singleton()
	{
		static T sInstance;
		return &sInstance;
	}
};

#endif
