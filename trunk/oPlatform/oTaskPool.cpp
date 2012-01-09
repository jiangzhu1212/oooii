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
#include "oTaskPool.h"
#include <oBasis/oBackoff.h>
#include <oBasis/oBlockAllocatorGrowable.h>
#include <oPlatform/oSingleton.h>

struct oTaskPoolContext : oProcessSingleton<oTaskPoolContext>
{
	static const oGUID GUID;
	oBlockAllocatorGrowableT<oTask> Pool;
};

// {20241B5E-39D6-4388-BF40-296923402B55}
const oGUID oTaskPoolContext::GUID = { 0x20241b5e, 0x39d6, 0x4388, { 0xbf, 0x40, 0x29, 0x69, 0x23, 0x40, 0x2b, 0x55 } };

void* oTask::operator new(size_t _Size) { return oTaskPoolContext::Singleton()->Pool.Allocate(); }
void oTask::operator delete(void* _Pointer) { oTaskPoolContext::Singleton()->Pool.Deallocate((oTask*)_Pointer); }
void* oTask::operator new[](size_t _Size) { return oTaskPoolContext::Singleton()->Pool.Allocate(); }
void oTask::operator delete[](void* _Pointer) { oTaskPoolContext::Singleton()->Pool.Deallocate((oTask*)_Pointer); }

oTask* oTaskPoolAllocate(oTASK _Task)
{
	oBackoff bo;
	oTask* t = nullptr;
	while (true)
	{
		t = new oTask(_Task);
		if (t) break;
		bo.Pause();
	}

	return t;
}

void oTaskPoolDeallocate(oTask* _pTask)
{
	delete _pTask;
}
