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
// Allows client code to wait on a subset of tasks submitted to a threadpool.
// The assumption is that there is one singleton threadpool in the system,
// but that is not implemented in oBase, so neither the task_group 
// implementation nor its factory are implemented. Client code should 
// implement these interfaces or there will be a link error.
#pragma once
#ifndef oBase_task_group_h
#define oBase_task_group_h

#include <functional>
#include <memory>

namespace ouro {

class task_group
{
public:
	static std::shared_ptr<ouro::task_group> make();

	virtual ~task_group() {}

	// dispatches a task flagged as part of this group
	virtual void run(const std::function<void()>& _Task) = 0;

	// waits for only tasks dispatched with run() to finish
	virtual void wait() = 0;
};

} // namespace ouro

#endif
