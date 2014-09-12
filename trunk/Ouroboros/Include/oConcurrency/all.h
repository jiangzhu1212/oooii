// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oConcurrency_all_h
#define oConcurrency_all_h
// Convenience "all headers" header for precompiled header files. Do NOT use 
// this to be lazy when including headers in .cpp files. Be explicit.
#include <oConcurrency/backoff.h>
#include <oConcurrency/concurrency.h>
#include <oConcurrency/concurrent_stack.h>
#include <oConcurrency/coroutine.h>
#include <oConcurrency/countdown_latch.h>
#include <oConcurrency/event.h>
#include <oConcurrency/lock_free_queue.h>
#include <oConcurrency/tagged_pointer.h>
#include <oConcurrency/threadpool.h>
#endif
