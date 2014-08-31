// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
