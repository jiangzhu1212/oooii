// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Convenience "all headers" header for precompiled header files. Do NOT use 
// this to be lazy when including headers in .cpp files. Be explicit.
#pragma once
#ifndef oCore_all_h
#define oCore_all_h
#include <oCore/adapter.h>
#include <oCore/camera.h>
#include <oCore/cpu.h>
#include <oCore/debugger.h>
#include <oCore/display.h>
#include <oCore/filesystem.h>
#include <oCore/filesystem_monitor.h>
#include <oCore/filesystem_util.h>
#include <oCore/module.h>
#include <oCore/mutex.h>
#include <oCore/page_allocator.h>
#include <oCore/process.h>
#include <oCore/process_heap.h>
#include <oCore/process_stats_monitor.h>
#include <oCore/reporting.h>
#include <oCore/system.h>
#include <oCore/thread_traits.h>
#endif
