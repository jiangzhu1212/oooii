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
// Various singletons at different module scopes and threading scopes
//
// NOTE: At this time, this singleton does not destroy itself, even at static
// deinitialization time, so no code will execute in a derived singleton's 
// destructor.
#pragma once
#ifndef oSingleton_h
#define oSingleton_h

#include <oooii/oInterface.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <typeinfo>

namespace detail {

struct oSingletonBase : oNoncopyable, oInterface
{
	oSingletonBase();
	virtual ~oSingletonBase();

	oDEFINE_NOOP_QUERYINTERFACE(); // @oooii-tony: TODO: Implement this more appropriately.

	// Singletons that depend on other singletons should reference the 
	// dependencies in their ctors and release dependencies in their dtors. For 
	// example on windows oAssert depends on oDebugger for tracing, which depends
	// on oDbgHelp for stack traces for memory leaks. oAssert references oDebugger
	// and oDebugger references oDbgHelp, ensuring ultimately that oDbgHelp is 
	// available to very late for any debug reporting.
	virtual int Reference() threadsafe;
	virtual void Release() threadsafe;

protected:
	// returns true if the specified object was created, false if found already.
	static void FindOrCreate(void** _ppInstance, oFUNCTION<void (void*)> _Ctor, const char* _TypeInfoName, size_t _TypeSize, bool _IsProcessUnique, bool _IsThreadUnique);
	static void CallCtor(void* _Memory, oFUNCTION<void (void*)> _Ctor, void** _ppInstance, const char* _TypeInfoName, bool _IsThreadUnique);

	void** ppInstance;
	void* hModule;
	const char* TypeInfoName;
	oRefCount RefCount;
};

template<typename T, bool IsProcessUnique> struct oProcessSingletonTBase : oSingletonBase
{
	static T* Singleton(bool _CreateOnFirstCall = true)
	{
		static oRef<T> sInstance;
		if (!sInstance && _CreateOnFirstCall)
			FindOrCreate((void**)sInstance.address(), Ctor, typeid(T).name(), sizeof(T), IsProcessUnique, false);
		return sInstance;
	}

protected:
	static void Ctor(void* _Memory) { new (_Memory) T(); }
};

template<typename T, bool IsProcessUnique> struct oThreadlocalSingletonTBase : oSingletonBase
{
	static T* Singleton(bool _CreateOnFirstCall = true)
	{
		static oTHREADLOCAL T* sInstance = 0;
		if (!sInstance && _CreateOnFirstCall)
			FindOrCreate((void**)&sInstance, Ctor, typeid(T).name(), sizeof(T), IsProcessUnique, true);
		return sInstance;
	}

protected:
	static void Ctor(void* _Memory) { new (_Memory) T(); }
};

} // namespace detail

template<typename T> struct oProcessSingleton : public detail::oProcessSingletonTBase<T, true> {};
template<typename T> struct oProcessThreadlocalSingleton : public detail::oThreadlocalSingletonTBase<T, true> {};

// Ensures a single instance across a module. Dlls in the same process will have 
// different copies
template<typename T> struct oModuleSingleton : oNoncopyable
{
	static T* Singleton()
	{
		static T sInstance;
		return &sInstance;
	}
};

void oReleaseAllProcessThreadlocalSingletons();
bool oIsValidSingletonPointer(void* _pSingleton);

#endif
