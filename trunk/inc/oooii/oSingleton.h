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
// A singleton that ensures multi-threaded init safety (first calls to Singleton()
// from multiple threads) and ensures there is a single, unique instance per 
// process whether this containing lib is statically linked, dynamically linked,
// or both.
//
// NOTE: At this time, this singleton does not destroy itself, even at static
// deinitialization time, so no code will execute in a derived singleton's 
// destructor.
#pragma once
#ifndef oSingleton_h
#define oSingleton_h

#include <oooii/oStddef.h>
#include <oooii/oHeap.h>
#include <oooii/oNonCopyable.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <typeinfo>

namespace detail {

	struct oSingletonBase : oNoncopyable
	{
		oSingletonBase();
		virtual ~oSingletonBase();

		void Reference() threadsafe;
		void Release() threadsafe;

	protected:
		static void ResolveStaticInstance(void** _ppInstance, oFUNCTION<void*()> _ConstructStaticInstance);
		static const char* GetUniqueName(const type_info& _TypeInfo);

		static bool Allocate(const char* _Name, size_t _Size, void** _pPointer);
		static void Deallocate(const char* _Name);

		oRefCount RefCount;
		void* ModuleId;
		void** ppInstance;
	};

	template<typename T> struct oSingletonTBase : public oSingletonBase
	{
		static void* ConstructStaticInstance()
		{
			void* p = 0;
			if (oHeap::StaticAllocateShared(GetUniqueName(typeid(T)), sizeof(T), &p))
				new (p) T();
			else
				static_cast<oSingletonBase*>(p)->Reference();
			
			return p;
		}
	};

} // namespace detail

inline void intrusive_ptr_add_ref(threadsafe detail::oSingletonBase* _pSingleton) { _pSingleton->Reference(); }
inline void intrusive_ptr_release(threadsafe detail::oSingletonBase* _pSingleton) { _pSingleton->Release(); }

template<typename T> struct oSingleton : public detail::oSingletonTBase<T>
{
	static T* Singleton()
	{
		static oRef<T> sInstance = 0;
		ResolveStaticInstance((void**)sInstance.address(), &oSingleton<T>::ConstructStaticInstance);
		return sInstance;
	}
};

template<typename T> struct oSingletonThreadlocal : public detail::oSingletonTBase<T>
{
	static T* Singleton()
	{
		static oTHREADLOCAL T* sInstance = 0;
		ResolveStaticInstance((void**)&sInstance, &oSingleton<T>::ConstructStaticInstance);
		return sInstance;
	}
};

#endif
