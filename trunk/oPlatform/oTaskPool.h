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
// It is common in threadpool implementations that the queue needs to 
// retain the specified oFUNCTION, but the underlying system API still
// takes a raw function pointer. To support oFUNCTION in these 
// interfaces, a new instance must be allocated, so here's a cache of
// those to speed up the allocation process.
#pragma once
#ifndef oTaskPool_h
#define oTaskPool_h

#include <oBasis/oFunction.h>

class oTask* oTaskPoolAllocate(oTASK _Task);
void oTaskPoolDeallocate(class oTask* _pTask);

class oTask
{
	oTASK Task;
	oDECLARE_NEW_DELETE();
	oTask(oTASK _Task) : Task(_Task) {}
	~oTask() {}

	friend oTask* oTaskPoolAllocate(oTASK _Task);
	friend void oTaskPoolDeallocate(oTask* _pTask);

public:
	inline void operator()() { Task(); oTaskPoolDeallocate(this); }
};

#endif
