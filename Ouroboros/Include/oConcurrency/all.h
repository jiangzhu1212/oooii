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
// Convenience "all headers" header for precompiled header files. Do NOT use 
// this to be lazy when including headers in .cpp files. Be explicit.
#pragma once
#ifndef oConcurrency_all_h
#define oConcurrency_all_h
#include <oConcurrency/basic_threadpool.h>
#include <oConcurrency/block_allocator.h>
#include <oConcurrency/concurrent_queue.h>
#include <oConcurrency/concurrent_queue_opt.h>
#include <oConcurrency/concurrent_worklist.h>
#include <oConcurrency/coroutine.h>
#include <oConcurrency/fixed_block_allocator.h>
#include <oConcurrency/joinable_threadpool_base.h>
#include <oConcurrency/lock_free_queue.h>
#include <oConcurrency/oConcurrency.h>
#include <oConcurrency/tagged_pointer.h>
#include <oConcurrency/task_group_threadpool.h>
#include <oConcurrency/thread_safe.h>
#include <oConcurrency/threadpool.h>
#endif
