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
// An implementation of a stack based co-routine using a Duffs device
// http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html

// Example usage:
/*
// Inherit your context from a coroutine_context
class MyContext : public coroutine_context
{
	bool Flag;
};

void MyExecute(MyContext& _MyContext)
{
	oCOROUTINE_BEGIN(&_MyContext);
	MyASyncProcessingThatWillWriteFlagEventually(&_MyContext.Flag);
	// this causes MyExecute to return, but it has marked where it has left off
	oCOROUTINE_SLEEP();
	if (_MyContext.Flag)
	{
		MyOtherProcessing();
	}
	oCOROUTINE_END();
}
*/
#pragma once
#ifndef oConcurrency_coroutine_h
#define oConcurrency_coroutine_h

#define oCOROUTINE_INVALID_SLEEPID (-1)
#define oCOROUTINE_INTERNAL_SLEEP(continuation,_SleepID) coroutine_context__->SetSleepID(_SleepID); continuation; return; case _SleepID:
#define oCOROUTINE_INTERNAL_SLEEP_INSTANTIATOR(continuation, _SleepID) oCOROUTINE_INTERNAL_SLEEP(continuation, _SleepID)

// Pass the address of an coroutine_context derivative to begin code execution 
// in a coroutine way.
#define oCOROUTINE_BEGIN(_pCoroutineContext) oConcurrency::coroutine_context* coroutine_context__ = _pCoroutineContext; switch(coroutine_context__->GetSleepID()) { case oCOROUTINE_INVALID_SLEEPID:

// End a coroutine block
#define oCOROUTINE_END() }

// Exit the coroutine block but record the location so that when reentering the 
// coroutine block the code starts from where it left off. Because of this 
// restart it is important to maintain all state in a class derived from 
// coroutine_context so all the data is still there when reentering the 
// coroutine block.
#define oCOROUTINE_SLEEP() oCOROUTINE_INTERNAL_SLEEP_INSTANTIATOR(;,__COUNTER__)

// Like oCOROUTINE_SLEEP but calls a continuation function after recording the reentry point.
// This allows for safe stack-based asynchronous operations that may call back into
// the caller.
#define oCOROUTINE_SLEEP_CONTINUATION(continuation) oCOROUTINE_INTERNAL_SLEEP_INSTANTIATOR(continuation, __COUNTER__)

namespace oConcurrency {

// Client code should derive a coroutine context (like a thread context) from 
// coroutine_context and use that with macros defined below.
class coroutine_context
{
	// a unique number that indicates where a coroutine left off if it sleeps
	int SleepID;
public:
	coroutine_context() : SleepID(oCOROUTINE_INVALID_SLEEPID) {}
	void SetSleepID(int _SleepID) { SleepID = _SleepID; }
	int GetSleepID() const { return SleepID; }
};

} // namespace oConcurrency

#endif
