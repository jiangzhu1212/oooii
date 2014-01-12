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
// A small class that spawns a thread to monitor CPU usage by the calling 
// process.
#pragma once
#ifndef oCore_process_stats_monitor_h
#define oCore_process_stats_monitor_h

#include <oCore/debugger.h>
#include <oCore/process.h>
#include <oBase/backoff.h>
#include <oBase/moving_average.h>
#include <oStd/shared_mutex.h>
#include <oStd/thread.h>

namespace ouro {

class process_stats_monitor
{
public:
	struct info
	{
		info()
			: average_usage(0.0f)
			, low_usage(0.0f)
			, high_usage(0.0f)
		{}

		float average_usage;
		float low_usage;
		float high_usage;
	};

	inline process_stats_monitor(process::id _ProcessID = process::id()
		, oStd::chrono::milliseconds _PollRate = oStd::chrono::milliseconds(1000))
			: PID(_ProcessID == process::id() ? this_process::get_id() : _ProcessID)
			, Done(true)
			, PollRate(_PollRate)
	{
		reset();
		Thread = std::move(std::thread(&process_stats_monitor::thread_proc, this));
		backoff bo;
		while (Done)
			bo.pause();
	}

	inline process_stats_monitor(process_stats_monitor&& _That)
	{
		operator=(std::move(_That));
	}

	inline ~process_stats_monitor()
	{
		join();
	}

	inline process_stats_monitor& operator=(process_stats_monitor&& _That)
	{
		if (this != &_That)
		{
			Stats = std::move(_That.Stats);
			PreviousSystemTime = std::move(_That.PreviousSystemTime);
			PreviousProcessTime = std::move(_That.PreviousProcessTime);
			MA = std::move(_That.MA);
			PollRate = std::move(_That.PollRate);
			PID = std::move(_That.PID);
			Done = std::move(_That.Done);
			Thread = std::move(_That.Thread);
		}
		return *this;
	}

	inline info get_info() const
	{
		shared_lock<shared_mutex> lock(const_cast<shared_mutex&>(StatsMutex));
		return Stats;
	}

	inline void reset()
	{
		Stats.average_usage = 0.0f;
		Stats.low_usage = 100.0f;
		Stats.high_usage = 0.0f;
		MA = moving_average<double>();
		PreviousSystemTime = 0;
		PreviousProcessTime = 0;
	}

private:
	inline void thread_proc()
	{
		debugger::thread_name("process_stats_monitor");
		Done = false;

		process::cpu_usage(PID, &PreviousSystemTime, &PreviousProcessTime);
		do
		{
			oStd::this_thread::sleep_for(PollRate);
			double usage = process::cpu_usage(PID, &PreviousSystemTime, &PreviousProcessTime);
			float usagef = static_cast<float>(usage);
			float avg = static_cast<float>(MA.calculate(usage));
			oStd::lock_guard<shared_mutex> lock(StatsMutex);
			Stats.average_usage = avg;
			Stats.low_usage = __min(Stats.low_usage, usagef);
			Stats.high_usage = __max(Stats.high_usage, usagef);

		} while (!Done);
	}

	inline bool joinable() const
	{
		return Thread.joinable();
	}

	inline void join()
	{
		if (joinable())
		{
			Done = true;
			Thread.join();
		}
	}

	shared_mutex StatsMutex;
	info Stats;
	unsigned long long PreviousSystemTime;
	unsigned long long PreviousProcessTime;
	moving_average<double> MA;
	oStd::chrono::milliseconds PollRate;
	process::id PID;
	bool Done;
	std::thread Thread;
};

} // namespace ouro

#endif
