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
// A fiber is a cooperative lightweight thread.
// http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html

// Example usage:
/*
// Inherit your context from an oFiber
class MyContext : public oFiber
{
	bool Flag;
};

void MyExecute(MyContext& _MyContext);
{
	oFIBER_BEGIN(&_MyContext);
	MyASyncProcessingThatWillWriteFlagEventually(&_MyContext.Flag);
	oFIBER_SLEEP(); // this causes MyExecute to return, but it has marked where it has left off
	if (_MyContext.Flag)
	{
		MyOtherProcessing();
	}
	oFIBER_END();
*/
#pragma once
#ifndef oFibering_h
#define oFibering_h

#define oFIBER_INVALID_SLEEPID (-1)
#define oFIBER_INTERNAL_SLEEP(continuation,_SleepID) oFiber__->SetSleepID(_SleepID); continuation; return; case _SleepID:
#define oFIBER_INTERNAL_SLEEP_INSTANTIATOR(continuation, _SleepID) oFIBER_INTERNAL_SLEEP(continuation, _SleepID)

// _____________________________________________________________________________
// Public API
// Client code should derive a fiber context (like a thread context) from oFiber
// and use that with macros defined below.
class oFiber
{
	int SleepID; // a unique number that indicates where a coroutine left off if it sleeps
public:
	oFiber() : SleepID(oFIBER_INVALID_SLEEPID) {}
	void SetSleepID(int _SleepID) { SleepID = _SleepID; }
	int GetSleepID() const { return SleepID; }
};

// Pass the address of an oFiber derivative to begin code execution in a fibrous way.
#define oFIBER_BEGIN(_pFiber) oFiber* oFiber__ = _pFiber; switch(oFiber__->GetSleepID()) { case oFIBER_INVALID_SLEEPID:

// End a fiber block
#define oFIBER_END() }

// Exit out of the fiber block, but record the location so that when reentering the
// fiber block the code starts from where it left off. Because of this restart it is
// important to maintain all state in a class derived from oFiber so all the data is
// still there when reentering the fiber block.
#define oFIBER_SLEEP() oFIBER_INTERNAL_SLEEP_INSTANTIATOR(;,__COUNTER__)

// Like oFIBER_SLEEP but calls a continuation function after recording the reentry point.
// This allows for safe stack-based asynchronous operations that may call back into
// the caller.
#define oFIBER_SLEEP_CONTINUATION(continuation) oFIBER_INTERNAL_SLEEP_INSTANTIATOR(continuation, __COUNTER__)
	
#endif
