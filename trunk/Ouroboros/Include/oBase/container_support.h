// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Declaration of common functors for standard containers.
#pragma once
#ifndef oBase_container_support_h
#define oBase_container_support_h

#include <functional>

namespace ouro {

template<typename T> struct less : public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x < y; } };
template<typename T> struct less_i: public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x < y; } };

template<typename T> struct same : public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x == y; } };
template<typename T> struct same_i : public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x == y; } };

}

#endif
