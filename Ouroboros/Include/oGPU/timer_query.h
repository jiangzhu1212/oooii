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
	
	void initialize(const char* name, device* dev);
	void deinitialize();

	// call this inside a command_lists begin/end block to scope a timing capture
	void begin(command_list* cl);
	void end(command_list* cl);

	// this must be called outside of a command_list's begin/end block
	// if the value is negative, then the timing is not yet ready. This throws on
	// error such as HW removed that would cause the timer to return but with 
	// unreliable results.
	double get_time(device* dev, bool blocking = true);

private:
	void* impl[3];
};

}}

#endif
