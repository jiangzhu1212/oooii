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
#pragma once
#ifndef oFibering_h
#define oFibering_h

//////////////////////////////////////////////////////////////////////////
// Ugly implementation, see below for how this works
//////////////////////////////////////////////////////////////////////////
#define oFIBER_SLEEP_BASE( instance ) \
	pOFiber_CTX->FiberSetLine( instance ); \
	return; \
	case instance: 

// Instantiator needed to ensure we have the same instance for all variables 
// __COUNTER__ is called once
#define oFIBER_SLEEP_BASE_INSTANTIATOR( instance ) \
	oFIBER_SLEEP_BASE( instance )

#define oFIBER_SLEEP_IMPL() oFIBER_SLEEP_BASE_INSTANTIATOR( __COUNTER__ )

#define oFIBER_BLOCK_BEGIN_IMPL( context ) \
	oFiberCtx* pOFiber_CTX = context; \
	switch( pOFiber_CTX->FiberGetLine() ) \
	{ \
	case -1:

#define oFIBER_BLOCK_END_IMPL() \
	} 


//////////////////////////////////////////////////////////////////////////
// Fibering macros (lightheight threads, other keywords Coroutines/Duffs device):
// Every fibering function should be implemented with the following form:
// void foo()
// {
//		oFIBER_BLOCK_BEGIN( &FooContext );
//		foo_bar( &FooContext.Flag );
//		oFIBER_SLEEP();		// This will cause foo to return true
//		if( FooContext.Flag )
//		{
//			/* Interesting work */
//		}
//		oFIBER_BLOCK_END();
// }
// Every time a fiber block is executed it will continue at the point of the
// last sleep.  Typically the caller will terminate the fiber through some
// kind of return value stored on the fiber context
//////////////////////////////////////////////////////////////////////////

#define oFIBER_BLOCK_BEGIN( context ) oFIBER_BLOCK_BEGIN_IMPL( context )
#define oFIBER_BLOCK_END() oFIBER_BLOCK_END_IMPL()
#define oFIBER_SLEEP() oFIBER_SLEEP_IMPL();
	
struct oFiberCtx
{
	oFiberCtx()
		: line(-1)
	{}

	void FiberSetLine(int _line){ line = _line; }
	int FiberGetLine() const { return line; }
private:
	int line;
};

#endif //oFibering_h