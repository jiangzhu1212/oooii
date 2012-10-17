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
// C++11 array doesn't separate the notion of "active" size() verses max
// capacity, so wrap it to add this functionality for those times where we 
// basically want statically allocated memory for a std::vector without 
// dealing with some of the subtlety of a std::allocator that doesn't really
// allocate.
#pragma once
#ifndef oArray_h
#define oArray_h

#include <oBasis/oAssert.h>
#include <array>

template<typename T, size_t N> class oArray
{
public:
	typedef std::tr1::array<T, N> array_t;
	typedef typename array_t::const_iterator const_iterator;
	typedef typename array_t::const_pointer const_pointer;
	typedef typename array_t::const_reference const_reference;
	typedef typename array_t::const_iterator const_reverse_iterator;
	typedef typename array_t::difference_type difference_type;
	typedef typename array_t::iterator iterator;
	typedef typename array_t::pointer pointer;
	typedef typename array_t::reference reference;
	typedef typename array_t::iterator reverse_iterator;
	typedef typename array_t::size_type size_type;
	typedef typename array_t::value_type value_type;

	oArray()
		: Size(0)
	{
	}

	oArray(const oArray& _Other)
		: Array(_Other.Array)
		, Size(_Other.Size)
	{
	}

	// void assign(iterator _First, iterator _Last) {}

	void fill(const T& _Value) { std::fill(begin(), end(), _Value); }
	void assign(size_type _N, const T& _Value) { Size = _N; fill(_Value); }

	reference at(size_type _Index) { return Array.at(_Index); }
	const_reference at(size_type _Index) const { return Array.at(_Index); }

	reference operator[](size_t _Index) { return Array[_Index]; }
	const_reference operator[](size_t _Index) const { return Array[_Index]; }
	const array_t& operator=(const array_t& _Other) { Array = _Other.Array; Size = _Other.Size; }

	reference front() { return Array.front(); }
	reference back() { return Array.at(Size-1); }
	const_reference front() const { return Array.front(); }
	const_reference back() const { return Array.at(Size-1); }

	iterator begin() { return Array.begin(); }
	iterator end() { return Array.begin() + Size; }
	const_iterator begin() const { return Array.begin(); }
	const_iterator end() const { return Array.begin() + Size; }
	const_iterator cbegin() const { return Array.cbegin(); }
	const_iterator cend() const { return Array.begin() + Size; }

	reverse_iterator rbegin() { return Array.rend() + Size; }
	reverse_iterator rend() { return Array.rend(); }
	const_reverse_iterator crbegin() const { return Array.rend() + Size; }
	const_reverse_iterator crend() const { return Array.crend(); }

	T* data() { return Array.data(); }
	const T* data() const { return Array.data(); }

	void resize(size_t _NewSize, T _InitialValue = T())
	{
		oASSERT(_NewSize <= max_size(), "New size %u too large for oArray", _NewSize);

		if (_NewSize < Size)
			for (size_t i = _NewSize; i < Size; i++)
				Array[i].~T();

		for (size_t i = Size; i < _NewSize; i++)
			Array[i] = _InitialValue;

		Size = _NewSize;
	}

	void clear() { resize(0); }
	bool empty() const { return Size == 0; }
	size_t max_size() const { return Array.max_size(); }
	size_t size() const { return Size; }
	size_t capacity() const { return Array.max_size(); }

	void push_back(const T& _Value)
	{
		oASSERT(Size < max_size(), "Array is at capacity (%u)", max_size());
		Array[Size++] = _Value;
	}

	void pop_back()
	{
		if (Size)
			Array[--Size].~T();
	}

	iterator insert(iterator _Position, const T& _Value)
	{
		oASSERT(Size < max_size(), "Array is at capacity (%u)", max_size());
		size_t PositionIndex = std::distance(begin(), _Position);

		for (size_t i = Size; i > PositionIndex; i--)
			Array[i] = Array[i-1];
		Array[PositionIndex] = _Value;
		Size++;

		return _Position;
	}

	// void insert(iterator _Position, size_type _N, const T& _Value) {}
	// void insert(iterator _Position, iterator _First, iterator _Last) {}

	iterator erase(iterator _Position)
	{
		return erase(_Position,_Position+1);
	}

	iterator erase(iterator _First, iterator _Last)
	{
		for (iterator it = _First; it != _Last; ++it)
			(*it).~T();

		std::copy(_Last,end(),_First);
		Size -= std::distance(_First, _Last);

		return _First;
	}

protected:
	array_t Array;
	size_t Size;
};

// Support for C++11 begin() and end() functions
template<typename T, size_t N> typename oArray<T, N>::iterator begin(oArray<T, N>& _Array)
{
	return _Array.begin();
}

template<typename T, size_t N> typename oArray<T, N>::iterator end(oArray<T, N>& _Array)
{
	return _Array.end();
}

#endif
