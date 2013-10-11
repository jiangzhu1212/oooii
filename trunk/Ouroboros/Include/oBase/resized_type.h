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
// Until we have enum classes, here's a method for making enums and other types
// use a smaller storage class.
#ifndef oBase_resized_type_h
#define oBase_resized_type_h
#include <oBase/operators.h>

namespace ouro { 

template<typename T, typename StorageT> 
class resized_type : public oOperators<resized_type<T, StorageT>, T>
{
public:
	typedef resized_type<T, StorageT> type;
	typedef T type_type;
	typedef StorageT storage_type;

	inline resized_type() {}
	inline resized_type(const T& _X) { operator=(_X); }
	inline resized_type(const resized_type& _That) { operator=(_That); }
	inline const resized_type& operator=(const resized_type& _That) { X = _That.X; return *this; }
	inline const resized_type& operator=(const T& _That) { X = static_cast<StorageT>(_That); return *this; }
	inline operator T() const { return T(X); }

	inline bool operator==(const resized_type& _That) { return X == _That.X; }
	inline bool operator<(const resized_type& _That) { return X < _That.X; }

	inline bool operator==(const T& _That) { return (T)X == _That; }
	inline bool operator<(const T& _That) { return (T)X < _That; }

private:
	StorageT X;
};

} // namespace ouro

#endif
