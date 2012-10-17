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
// Policy addition/clarification to C++. The volatile keyword has no semantic on 
// class methods, so co-opt it into a limiter of when the method can be called. 
// Like non-const methods cannot be called by const pointers/references to a 
// class, non-volatile methods cannot be called by volatile pointers/references. 
// So marking a class pointer/reference as volatile marks the class as only 
// being allowed to call volatile methods without special handling. Rename 
// volatile 'threadsafe' and you have a well-documented mechanism to mark 
// classes that threadsafety is a consideration and make it clear which methods
// are safe to call and which are not.
#pragma once
#ifndef oThreadsafe_h
#define oThreadsafe_h

#ifndef threadsafe
	#define threadsafe volatile
#endif

// Differentiate thread_cast from const_cast so it's easier to grep the code 
// looking for situations where the threadsafe qualifier was cast away.
template<typename T, typename U> inline T thread_cast(const U& threadsafeObject) { return const_cast<T>(threadsafeObject); }

#endif
