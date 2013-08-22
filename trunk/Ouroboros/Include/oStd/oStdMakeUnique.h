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
// This will be built into c++ 14. We use oRef's instead of shared_ptr's in 
// general, however unique_ptr's are still useful for light weight objects that 
// don't need the machinery of an oRef. Standards committee forgot to add 
// make_unique though, so they are annoying to build. Use this until c++ 14 adds 
// the missing function.
#pragma once
#ifndef oStdMakeUnique_h
#define oStdMakeUnique_h
#include <memory>

//Herb Sutter's implementation from http://herbsutter.com/gotw/_102/ should work 
// with second update to Visual Studio 2012
//template<typename T, typename ...Args>
//std::unique_ptr<T> make_unique(Args&& ...args)
//{
//	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
//}

namespace oStd
{
	template<typename T>
	std::unique_ptr<T> make_unique()
	{
		return std::unique_ptr<T>(new T());
	}

	//could use the defines from oCallable, hoping to get variadics soon. just hard code 5 for now

	template<typename T, typename Arg1>
	std::unique_ptr<T> make_unique(Arg1&& _Arg1)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(_Arg1)));
	}

	template<typename T, typename Arg1, typename Arg2>
	std::unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(_Arg1), std::forward<Arg2>(_Arg2)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3>
	std::unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(_Arg1), std::forward<Arg2>(_Arg2), std::forward<Arg3>(_Arg3)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	std::unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(_Arg1), std::forward<Arg2>(_Arg2), std::forward<Arg3>(_Arg3), std::forward<Arg4>(_Arg4)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
	std::unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(_Arg1), std::forward<Arg2>(_Arg2), std::forward<Arg3>(_Arg3), std::forward<Arg4>(_Arg4), std::forward<Arg5>(_Arg5)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
	std::unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5, Arg6&& _Arg6)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(_Arg1), std::forward<Arg2>(_Arg2), std::forward<Arg3>(_Arg3), std::forward<Arg4>(_Arg4), std::forward<Arg5>(_Arg5), std::forward<Arg6>(_Arg6)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
	std::unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5, Arg6&& _Arg6, Arg7&& _Arg7)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(_Arg1), std::forward<Arg2>(_Arg2), std::forward<Arg3>(_Arg3), std::forward<Arg4>(_Arg4), std::forward<Arg5>(_Arg5), std::forward<Arg6>(_Arg6), std::forward<Arg7>(_Arg7)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
	std::unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5, Arg6&& _Arg6, Arg7&& _Arg7, Arg8&& _Arg8)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(_Arg1), std::forward<Arg2>(_Arg2), std::forward<Arg3>(_Arg3), std::forward<Arg4>(_Arg4), std::forward<Arg5>(_Arg5), std::forward<Arg6>(_Arg6), std::forward<Arg7>(_Arg7), std::forward<Arg8>(_Arg8)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
	std::unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5, Arg6&& _Arg6, Arg7&& _Arg7, Arg8&& _Arg8, Arg9&& _Arg9)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(_Arg1), std::forward<Arg2>(_Arg2), std::forward<Arg3>(_Arg3), std::forward<Arg4>(_Arg4), std::forward<Arg5>(_Arg5), std::forward<Arg6>(_Arg6), std::forward<Arg7>(_Arg7), std::forward<Arg8>(_Arg8), std::forward<Arg9>(_Arg9)));
	}

	template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
	std::unique_ptr<T> make_unique(Arg1&& _Arg1, Arg2&& _Arg2, Arg3&& _Arg3, Arg4&& _Arg4, Arg5&& _Arg5, Arg6&& _Arg6, Arg7&& _Arg7, Arg8&& _Arg8, Arg9&& _Arg9, Arg10&& _Arg10)
	{
		return std::unique_ptr<T>(new T(std::forward<Arg1>(_Arg1), std::forward<Arg2>(_Arg2), std::forward<Arg3>(_Arg3), std::forward<Arg4>(_Arg4), std::forward<Arg5>(_Arg5), std::forward<Arg6>(_Arg6), std::forward<Arg7>(_Arg7), std::forward<Arg8>(_Arg8), std::forward<Arg9>(_Arg9), std::forward<Arg10>(_Arg10)));
	}
}

#endif
