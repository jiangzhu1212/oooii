// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
	inline resized_type(const T& _That) { operator=(_That); }
	inline resized_type(const resized_type& _That) { operator=(_That); }
	template<typename U> resized_type(const U& _That) { T t = (T)_That; operator=(t); }
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

}

#endif
