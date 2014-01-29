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
// Missing make_unique is considered an oversight in C++11, so here it is.
#pragma once
#ifndef oBase_std_make_unique_h
#define oBase_std_make_unique_h
#include <memory>

// Herb Sutter's implementation from http://herbsutter.com/gotw/_102/ should  
// work with second update to Visual Studio 2012
// template<typename T, typename ...Args>
// unique_ptr<T> make_unique(Args&& ...args)
// {
//	 return unique_ptr<T>(new T(forward<Args>(args)...));
// }

namespace std
{
	template<typename T>
	unique_ptr<T> make_unique()
	{
		return unique_ptr<T>(new T());
	}

	template<typename T, typename Arg1>
	unique_ptr<T> make_unique(Arg1&& _Arg1)
	{
		return unique_ptr<T>(new T(forward<Arg1>(_Arg1)));
	}

	template<typename T, typename Arg1, typename Arg2>
	unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2)
	{
		return unique_ptr<T>(new T(forward<Arg1>(_Arg1), forward<Arg2>(_Arg2)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3>
	unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3)
	{
		return unique_ptr<T>(new T(forward<Arg1>(_Arg1), forward<Arg2>(_Arg2), forward<Arg3>(_Arg3)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4)
	{
		return unique_ptr<T>(new T(forward<Arg1>(_Arg1), forward<Arg2>(_Arg2), forward<Arg3>(_Arg3), forward<Arg4>(_Arg4)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
	unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5)
	{
		return unique_ptr<T>(new T(forward<Arg1>(_Arg1), forward<Arg2>(_Arg2), forward<Arg3>(_Arg3), forward<Arg4>(_Arg4), forward<Arg5>(_Arg5)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
	unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5, Arg6&& _Arg6)
	{
		return unique_ptr<T>(new T(forward<Arg1>(_Arg1), forward<Arg2>(_Arg2), forward<Arg3>(_Arg3), forward<Arg4>(_Arg4), forward<Arg5>(_Arg5), forward<Arg6>(_Arg6)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
	unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5, Arg6&& _Arg6, Arg7&& _Arg7)
	{
		return unique_ptr<T>(new T(forward<Arg1>(_Arg1), forward<Arg2>(_Arg2), forward<Arg3>(_Arg3), forward<Arg4>(_Arg4), forward<Arg5>(_Arg5), forward<Arg6>(_Arg6), forward<Arg7>(_Arg7)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
	unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5, Arg6&& _Arg6, Arg7&& _Arg7, Arg8&& _Arg8)
	{
		return unique_ptr<T>(new T(forward<Arg1>(_Arg1), forward<Arg2>(_Arg2), forward<Arg3>(_Arg3), forward<Arg4>(_Arg4), forward<Arg5>(_Arg5), forward<Arg6>(_Arg6), forward<Arg7>(_Arg7), forward<Arg8>(_Arg8)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
	unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5, Arg6&& _Arg6, Arg7&& _Arg7, Arg8&& _Arg8, Arg9&& _Arg9)
	{
		return unique_ptr<T>(new T(forward<Arg1>(_Arg1), forward<Arg2>(_Arg2), forward<Arg3>(_Arg3), forward<Arg4>(_Arg4), forward<Arg5>(_Arg5), forward<Arg6>(_Arg6), forward<Arg7>(_Arg7), forward<Arg8>(_Arg8), forward<Arg9>(_Arg9)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
	unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5, Arg6&& _Arg6, Arg7&& _Arg7, Arg8&& _Arg8, Arg9&& _Arg9, Arg10&& _Arg10)
	{
		return unique_ptr<T>(new T(forward<Arg1>(_Arg1), forward<Arg2>(_Arg2), forward<Arg3>(_Arg3), forward<Arg4>(_Arg4), forward<Arg5>(_Arg5), forward<Arg6>(_Arg6), forward<Arg7>(_Arg7), forward<Arg8>(_Arg8), forward<Arg9>(_Arg9), forward<Arg10>(_Arg10)));
	}
}

#endif
