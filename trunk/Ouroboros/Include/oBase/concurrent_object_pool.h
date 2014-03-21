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
#pragma once
#ifndef oBase_concurrent_object_pool
#define oBase_concurrent_object_pool

#include <oBase/concurrent_index_allocator.h>

namespace ouro {

template<typename T>
class concurrent_object_pool
{
public:
	concurrent_object_pool() : pObjects(nullptr) {}
	~concurrent_object_pool();
	concurrent_object_pool(size_t _NumObjects);
	concurrent_object_pool(concurrent_object_pool&& _That) { operator=(std::move(_That)); }
	concurrent_object_pool& operator=(concurrent_object_pool&& _That)
	{
		if (this != &_That)
		{
			Allocator = std::move(_That.Allocator);
			pObjects = _That.pObjects; _That.pObjects = nullptr;
		}
		return *this;
	}

	bool operator!() const { return !pObjects; }

	T* allocate();
	void deallocate(T* _pObject);
	bool valid(T* _pObject) const;

private:
	concurrent_index_allocator Allocator;
	T* pObjects;

	concurrent_object_pool(const concurrent_object_pool& _That); /* = delete; */
	const concurrent_object_pool& operator=(const concurrent_object_pool& _That); /* = delete; */
};

template<typename T>
concurrent_object_pool<T>::concurrent_object_pool(size_t _NumObjects)
	: pObjects(nullptr)
{
	pObjects = new T[_NumObjects];
	void* pArena = new unsigned int[_NumObjects];
	Allocator = std::move(concurrent_index_allocator(pArena, sizeof(unsigned int) * _NumObjects));
}

template<typename T>
concurrent_object_pool<T>::~concurrent_object_pool()
{
	unsigned int* pArena = (unsigned int*)Allocator.get_arena();
	Allocator = concurrent_index_allocator();

	delete [] pArena;
	delete [] pObjects;
}

template<typename T>
T* concurrent_object_pool<T>::allocate()
{
	T* o = nullptr;
	unsigned int index = Allocator.allocate();
	if (index != concurrent_index_allocator::invalid_index)
		o = pObjects + index;
	return o;
}

template<typename T>
void concurrent_object_pool<T>::deallocate(T* _pObject)
{
	if (!valid(_pObject))
		throw std::invalid_argument("invalid object");
	unsigned int index = (unsigned int)index_of(_pObject, pObjects);
	Allocator.deallocate(index);
}

template<typename T>
bool concurrent_object_pool<T>::valid(T* _pObject) const
{
	return _pObject >= pObjects && _pObject < (pObjects + Allocator.capacity());
}

} // namespace ouro

#endif
