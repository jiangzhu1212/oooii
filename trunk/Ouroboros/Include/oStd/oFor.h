/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Preview of the new for syntax as best as it can be implemented in VS 2010.

// !!! WARNING !!! Like BOOST_FOREACH, this doesn't support break and continue
// well because of the double-for-loop method of encapsulating the iteration
// variables.

#pragma once
#ifndef oFor_h
#define oFor_h

#include <oStd/config.h>
#include <type_traits>

#ifndef oHAS_AUTO
	#error unsupported platform
#endif

template<typename ContainerType> auto oFORBegin(ContainerType& _Container) -> 
	typename std::enable_if<!std::is_const<ContainerType>::value
	, decltype(std::begin(_Container))>::type
{
	return std::begin(_Container);
}

template<typename ContainerType> auto oFORBegin(ContainerType& _Container) -> 
	typename std::enable_if<std::is_const<ContainerType>::value
	, decltype(std::cbegin(_Container))>::type
{
	return std::cbegin(_Container);
}

template<typename ContainerType> auto oFOREnd(ContainerType& _Container) -> 
	typename std::enable_if<!std::is_const<ContainerType>::value
	, decltype(std::end(_Container))>::type
{
	return std::end(_Container);
}

template<typename ContainerType> auto oFOREnd(ContainerType& _Container) -> 
	typename std::enable_if<std::is_const<ContainerType>::value
	, decltype(std::cend(_Container))>::type
{
	return std::cend(_Container);
}

// Details of oFOR implementations
#define oFOR_INTERNAL__(_ElementDeclaration, _Container, _InstanceID) \
	bool bLoopOnce##_InstanceID = 0; \
	for (auto oFOREACH_iterator = oFORBegin(_Container); oFOREACH_iterator != oFOREnd(_Container); ++oFOREACH_iterator, bLoopOnce##_InstanceID = false) \
	for (_ElementDeclaration = *oFOREACH_iterator; !bLoopOnce##_InstanceID; bLoopOnce##_InstanceID = true)
#define oFOR_INTERNAL2__(_ElementDeclaration, _Container, _InstanceID) oFOR_INTERNAL__(_ElementDeclaration, _Container, _InstanceID)

// Use this similar to BOOST_FOREACH or C++11's new for syntax. Example:
// oFOR(int x&, writableArrayOfInts)
//	x = 10;
// oFOR(const myobj& x, readableVectorOfObjs)
//	foo += x->GetValue();
#define oFOR(_ElementDeclaration, _Container) oFOR_INTERNAL2__(_ElementDeclaration, _Container, __COUNTER__)

#endif
