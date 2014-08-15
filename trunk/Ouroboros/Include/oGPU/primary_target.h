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
#ifndef oGPU_primary_target_h
#define oGPU_primary_target_h

#include <oGPU/resource.h>
#include <oGPU/color_target.h>
#include <vector>

namespace ouro { 
	
class color;
class window;

	namespace gpu {

class command_list;
class device;
class depth_target;

class primary_target : public basic_color_target
{
public:
	primary_target();
	~primary_target() { deinitialize(); }

	void initialize(window* win, device& dev, bool enable_os_render);
	void deinitialize();

	operator bool() const { return !!ro; }

	inline uint num_presents() const { return npresents; }

	// all api below must be called from the same thread as 
	// processes windows events
	inline void resize(const uint2& dimensions) { internal_resize(dimensions, nullptr); }

	void* begin_os_frame();
	void end_os_frame();

	bool is_fullscreen_exclusive() const;
	void set_fullscreen_exclusive(bool fullscreen);
	
	void present(uint interval = 1);

private:
	void* swapchain;
	mutable shared_mutex mutex;
	uint npresents;

	void internal_resize(const uint2& dimensions, device* dev);
};
	
}}

#endif
