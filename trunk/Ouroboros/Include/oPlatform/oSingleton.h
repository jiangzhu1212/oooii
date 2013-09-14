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
// This provides several no-really-it's-a-singleton singleton types that can
// transcend nasty platform details such as soft-link DLLs and threadlocal 
// matters so that the useful singleton pattern can still be used in complex 
// projects.
#pragma once
#ifndef oSingleton_h
#define oSingleton_h

#include <oBasis/oInterface.h>
#include <oBasis/oRefCount.h>
#include <oStd/type_info.h>

typedef void* (*NewVFn)();

bool oConstructOnceV(void* volatile* _pPointer, NewVFn _New);
template<typename T> bool oConstructOnce(T* volatile* _pPointer, T* (*_New)() ) { return oConstructOnceV((void* volatile*)_pPointer, (NewVFn)_New); }

// This must be defined in the module where an oSingleton is defined and must be
// given the class name of the singleton. What this guarantees that if a dynamic 
// module triggers singleton instantiation that the code used is always from a 
// static module so if the dynamic module is unloaded, there's still code to 
// unload a singleton at the end of the program.
#define oSINGLETON_REGISTER(_Type) oSingletonRegister oCONCAT(oSingletonRegister,_Type)(#_Type, _Type::GUID, oStd::type_info<_Type>::default_construct)

struct oSingletonRegister
{
	// This registers the ctor pointer at static init time with the process heap
	// so that when a singleton is instantiated we can allocate not only memory
	// we know will be around until the end of the very last dynamically loaded
	// library, but also the code used will be there too. The first-to-register
	// will be the last-to-unregister as C++ static init specifies.
	oSingletonRegister::oSingletonRegister(const char* _SingletonName, const oGUID& _SingletonGUID, oStd::type_info_default_constructor _PlacementNew);
};

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
	static void* NewV(const char* _TypeInfoName, size_t _Size, oStd::type_info_default_constructor _Ctor, const oGUID& _GUID, bool _IsThreadLocal);
};

template<typename T, bool ThreadLocal = false>
class oSingletonBaseT : public oSingletonBase
{
public:
	oSingletonBaseT(int _InitialRefCount = 1) : oSingletonBase(_InitialRefCount) {}
protected:
	static T* New() { return static_cast<T*>(NewV(typeid(T).name(), sizeof(T), oStd::type_info<T>::default_construct, typename T::GUID, ThreadLocal)); } // GUID must be defined as a static member of the derived class
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
			// If the above static were an oStd::ref, it is possible to go through static
			// deinit, see that sInstance is null and noop on the dtor, only to have
			// later deinit code instantiate the singleton for the first time (for
			// very low-level debug/reporting/allocation singletons). But the dtor had
			// already executed and doesn't get executed again! So ignore anything 
			// that the first static might do and if we are truly the ones to create
			// the instance, ref it here because this will be first-access of this 
			// static and thus the dtor will be registered at this time.
			static oStd::intrusive_ptr<T> sAnotherInstance(sInstance, false);
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
		if (oConstructOnce(&sInstance, New))
		{
			// We need to add a module singleton reference to this instance
			// since we can get here more than once per thread if it is called
			// from a different module. 
			static oStd::intrusive_ptr<T> sAnotherInstance(sInstance, true);
		}
		return sInstance;
	}
};

template<typename T> class oModuleSingleton
{
public:
	static T* Singleton()
	{
		static T sInstance;
		return &sInstance;
	}
};

#endif
