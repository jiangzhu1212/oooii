// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_timer_query_h
#define oGPU_timer_query_h

namespace ouro { namespace gpu {

class device;
class command_list;

class timer_query
{
public:
	timer_query() { impl[0] = impl[1] = impl[2] = nullptr; }
	~timer_query() { deinitialize(); }
	
	void initialize(const char* name, device& dev);
	void deinitialize();

	// call this inside a command_lists begin/end block to scope a timing capture
	void begin(command_list& cl);
	void end(command_list& cl);

	// this must be called outside of a command_list's begin/end block
	// if the value is negative, then the timing is not yet ready. This throws on
	// error such as HW removed that would cause the timer to return but with 
	// unreliable results.
	double get_time(bool blocking = true);

private:
	void* impl[3];
};

}}

#endif
