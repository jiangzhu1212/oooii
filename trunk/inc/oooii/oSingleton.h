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

#include <oooii/oStddef.h>
#include <oooii/oHeap.h>
#include <oooii/oNonCopyable.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oThread.h>
#include <typeinfo>

namespace detail {

	struct oSingletonBase : oNoncopyable
	{
		oSingletonBase();
		virtual ~oSingletonBase(){}
		virtual void Reference() threadsafe;
		virtual void Release() threadsafe;

		// @oooii-mike temp
		void Set(int _refs) { RefCount.Set(_refs); }

	protected:
		static bool ResolveStaticInstance(void** _ppInstance, oFUNCTION<void*()> _ConstructStaticInstance);

		// Singletons have a distinct destroy function
		// that will eventually call the derived object's
		// destructor when the singleton has really gone out
		// of scope which is different for each type of singleton
		// hence a destroy that is separate from the destructor
		virtual void Destroy() threadsafe;
		oRefCount RefCount;
		void** ppInstance;
	};

	struct oProcessSingletonBase : oSingletonBase
	{
		oProcessSingletonBase(bool systemSingleton = false, bool singletonMgr = false);
		virtual ~oProcessSingletonBase();

		// This returns the module that created the singleton
		size_t GetConstructorModuleHandle() const threadsafe{ return ModuleId; }

		void Set(int _RefCount) threadsafe { RefCount.Set(_RefCount); }

	protected:
		static const char* GetUniqueName(const type_info& _TypeInfo, bool _ThreadUnique);

		static bool Allocate(const char* _Name, size_t _Size, void** _pPointer);
		static void Deallocate(const char* _Name);

		void Destroy() threadsafe override;
		
		size_t ModuleId;
		
	};

	template<typename T> struct oProcessSingletonTBase : public oProcessSingletonBase
	{
		oProcessSingletonTBase(bool systemSingleton = false, bool singletonMgr = false)
			: oProcessSingletonBase(systemSingleton, singletonMgr)
		{}

		static void* ConstructStaticInstance(bool _ThreadUnique)
		{
			void* p = 0;
			if (oHeap::StaticAllocateSharedMap(GetUniqueName(typeid(T), _ThreadUnique), _ThreadUnique, sizeof(T), &p))
				new (p) T();
			else
				static_cast<oProcessSingletonBase*>(p)->Reference();

			oHeap::StaticAllocateSharedUnmap();
			
			return p;
		}
	};

	struct oModuleSingletonBase : oSingletonBase
	{
	protected:
		virtual void Destroy() threadsafe;
	};

	template<typename T> struct oModuleSingletonTBase : public oModuleSingletonBase
	{
		static void* ConstructStaticInstance()
		{
			// oModuleSingletonTBase uses static memory to avoid spurious memory leaks
			// since static memory comes from a special per-process heap.
			return new( oHeap::StaticAllocate( sizeof(T) ) ) T();
		}
	};

	void AddThreadLocalSingleton(oSingletonBase** pSingleton);
	void ReleaseThreadLocalSingletons();
} // namespace detail

inline void intrusive_ptr_add_ref(threadsafe detail::oSingletonBase* _pSingleton) { _pSingleton->Reference(); }
inline void intrusive_ptr_release(threadsafe detail::oSingletonBase* _pSingleton) { _pSingleton->Release(); }

// Ensures a single instance across the entire process regardless of linkage 
template<typename T> struct oProcessSingleton : public detail::oProcessSingletonTBase<T>
{
	oProcessSingleton(bool systemSingleton = false) : detail::oProcessSingletonTBase<T>(systemSingleton) {}

	static T* Singleton()
	{
		static oRef<T> sInstance = 0;
		ResolveStaticInstance((void**)sInstance.address(), oBIND( &oProcessSingleton<T>::ConstructStaticInstance, false ));
		return sInstance;
	}
};

// Ensures a single instance per thread across the entire process regardless of linkage 
template<typename T> struct oProcessThreadlocalSingleton : public detail::oProcessSingletonTBase<T>
{
	static T* Singleton()
	{
		static oTHREADLOCAL T* sInstance = 0;
		if(ResolveStaticInstance((void**)&sInstance, oBIND( &oProcessSingleton<T>::ConstructStaticInstance, true )))
			AddThreadLocalSingleton((oSingletonBase**)&sInstance);
		return sInstance;
	}
};

// Ensures a single instance across an entire module (dlls in the same process will have different copies)
template<typename T> struct oModuleSingleton : public detail::oModuleSingletonTBase<T>
{
	static T* Singleton()
	{
		static oRef<T> sInstance = 0;
		ResolveStaticInstance((void**)sInstance.address(), &oModuleSingletonTBase<T>::ConstructStaticInstance );
		return sInstance;
	}
};

// Ensures

#endif
