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
#include <oCore/scheduler.h>

// include one implementation and define one NAMESPACE

#include "tbb_scheduler.h"
#define NAMESPACE tbb

//#include "ouro_scheduler.h"
//#define NAMESPACE ouro_scheduler

namespace ouro {
	namespace scheduler {

const char* name()
{
	return NAMESPACE::name();
}

void ensure_initialized()
{
	return NAMESPACE::ensure_initialized();
}

void dispatch(const std::function<void()>& _Task)
{
	NAMESPACE::dispatch(_Task);
}

void parallel_for(size_t _Begin, size_t _End, const std::function<void(size_t _Index)>& _Task)
{
	NAMESPACE::parallel_for(_Begin, _End, _Task);
}

	} // namespace scheduler

std::shared_ptr<task_group> task_group::make()
{
	return NAMESPACE::make_task_group();
}

} // namespace ouro
