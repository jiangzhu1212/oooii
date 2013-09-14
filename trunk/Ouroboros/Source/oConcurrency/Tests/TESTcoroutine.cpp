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
#include <oConcurrency/coroutine.h>
#include <oStd/future.h>

namespace oConcurrency {
	namespace tests {

class MyContext : public oConcurrency::coroutine_context
{
public:
	MyContext() 
		: Flag(false)
		, Done(false)
		, Counter(0)
	{}

	oStd::future<void> Future;
	bool Flag;
	bool Done;
	int Counter;
};

void MyExecute(MyContext& _MyContext)
{
	_MyContext.Counter++;
	oCOROUTINE_BEGIN(&_MyContext);
	_MyContext.Future = oStd::async([&](){ _MyContext.Flag = true; });
	oCOROUTINE_SLEEP(); // this causes MyExecute to return, but it has marked where it has left off
	if (!_MyContext.Future.is_ready())
		return;
	_MyContext.Done = true;
	oCOROUTINE_END();
}

void TESTcoroutine()
{
	MyContext myContext;

	while (!myContext.Done)
	{
		MyExecute(myContext);
		oStd::this_thread::sleep_for(oStd::chrono::milliseconds(1));
	}
}

	} // namespace tests
} // namespace oConcurrency
