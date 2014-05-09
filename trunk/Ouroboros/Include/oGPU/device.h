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
#ifndef oGPU_device_h
#define oGPU_device_h

#include <oGPU/render_target.h>
#include <oGPU/buffer.h>
#include <oGPU/texture.h>
#include <oGPU/vertex_layout.h>
#include <oGPU/shader.h>

#include <oBase/gpu_api.h>

namespace ouro {

class window;

	namespace gpu {
	
struct device_init
{
	device_init(const char* name = "gpu device")
		: debug_name(name)
		, version(11,0)
		, min_driver_version(0,0)
		, adapter_index(0)
		, virtual_desktop_position(oDEFAULT, oDEFAULT)
		, use_software_emulation(false)
		, use_exact_driver_version(false)
		, multithreaded(true)
		, enable_driver_reporting(false)
	{}

	// Name associated with this device in debug output
	sstring debug_name;

	// The version of the underlying API to use.
	struct version version;

	// The minimum version of the driver required to successfully create the 
	// device. If the driver version is 0.0.0.0 (the default) then a hard-coded
	// internal value is used based on QA verificiation.
	struct version min_driver_version;

	// If virtual_desktop_position is oDEFAULT, then use the nth found device as 
	// specified by this index. If virtual_desktop_position is anything valid then 
	// use the device associated with that desktop position and ignore this index.
	int adapter_index;

	// Position on the desktop and thus on a monitor to be used to determine which 
	// GPU is used for that monitor and create a device for that GPU.
	int2 virtual_desktop_position;

	// Allow SW emulation for the specified version. If false, a create will fail
	// if HW acceleration is not available.
	bool use_software_emulation;

	// If true, == is used to match min_driver_version to the specified GPU's 
	// driver. If false cur_version >= min_driver_version is used.
	bool use_exact_driver_version;

	// If true, the device is thread-safe.
	bool multithreaded;

	// Control if driver warnings/errors are reported.
	bool enable_driver_reporting;
};

struct device_info
{
	device_info()
		: native_memory(0)
		, dedicated_system_memory(0)
		, shared_system_memory(0)
		, adapter_index(0)
		, api(api::unknown)
		, vendor(vendor::unknown)
		, is_software_emulation(false)
		, debug_reporting_enabled(false)
	{}

	// Name associated with this device in debug output
	sstring debug_name;

	// Description as provided by the device vendor
	mstring device_description;

	// Description as provided by the driver vendor
	mstring driver_description;

	// Number of bytes present on the device (AKA VRAM)
	ullong native_memory;

	// Number of bytes reserved by the system to accommodate data transfer to the 
	// device
	ullong dedicated_system_memory;

	// Number of bytes reserved in system memory used instead of a separate bank 
	// of NativeMemory 
	ullong shared_system_memory;

	// The version for the software that supports the native API. This depends on 
	// the API type being used.
	version driver_version;

	// The feature level the device supports. This depends on the API type being 
	// used.
	version feature_version; 

	// The zero-based index of the adapter. This may be different than what is 
	// specified in device_init in certain debug/development modes.
	int adapter_index;

	// Describes the API used to implement the oGPU API
	gpu_api::value api;

	// Describes the company that made the device.
	vendor::value vendor;

	// True if the device was created in software emulation mode.
	bool is_software_emulation;

	// True if the device was created with debug reporting enabled.
	bool debug_reporting_enabled;
};

class device
{
public:
	device_info get_info() const;
	
	// this value is incremented with each call to end-frame
	uint frame_id() const;

	// the immediate command list is always created with the device
	command_list* immediate();

	// copies the contents of a readback resource into the user-provided destination
	bool read_resource(void* destination, uint destination_size, const resource* readback_resource, bool blocking = false);

	// copies the contents of a query into the user-provided destination
	bool read_query(void* destination, uint destination_size, const* source, bool blocking = false);

	// all command_list api should be called between these apis
	bool begin_frame();
	void end_frame();

	// After out of a begin_frame/end_frame bracket, the device can provide a 
	// handle to OS CPU-based rendering. All OS calls should occur in between 
	// begin_os_frame and end_os_frame and this should be called after end_frame, 
	// but before present to prevent tearing.
	void* begin_os_frame();
	void end_os_frame();

	// This should only be called on same thread as the window passed to 
	// make_primary_render_target. If a primary render target does not exist this
	// will noop otherwise the entire client area of the associated window will
	// receive the contents of the back buffer. Call this after end_frame() to 
	// ensure all commands have been flushed.

	bool is_fullscreen_exclusive() const;
	void set_fullscreen_exclusive(bool fullscreen);

	void present(uint present_interval);

	// factory api
#if 0
	// rendering must be associated with the GUI for presentation
	render_target* make_primary_render_target(window* 
		, const surface::format& depth_stencil_format, bool enable_OS_rendering);
	
	// factory apis to create gpu resources all resources are associated with the
	// creating device and should not be used with other devices.
	command_list* make_command_list(const command_list_info& info, const char* debug_name = "");
	render_target* make_render_target(const render_target_info& info, const char* debug_name = "");
	query* make_query(const query_info& info, const char* debug_name = "");
	vertex_layout* make_vertex_layout(const vertex_layout_info& info, const char* debug_name = "");
	buffer* make_buffer(const buffer_info& info, const char* debug_name = "");
	texture* make_texture(const texture_info& info, const char* debug_name = "");
	vertex_shader* make_vertex_shader(const void* bytecode, const char* debug_name = "");
	hull_shader* make_hull_shader(const void* bytecode, const char* debug_name = "");
	domain_shader* make_domain_shader(const void* bytecode, const char* debug_name = "");
	geometry_shader* make_geometry_shader(const void* bytecode, const char* debug_name = "");
	pixel_shader* make_pixel_shader(const void* bytecode, const char* debug_name = "");
	compute_shader* make_compute_shader(const void* bytecode, const char* debug_name = "");
	
	// use these to free made device objects
	void unmake_command_list(command_list* cl);
	void unmake_render_target(render_target* rt);
	void unmake_query(query* q);
	void unmake_vertex_layout(vertex_layout* vl);
	void unmake_buffer(buffer* b);
	void unmake_texture(texture* t);
	void unmake_vertex_shader(vertex_shader* v);
	void unmake_hull_shader(hull_shader* h);
	void unmake_domain_shader(domain_shader* d);
	void unmake_geometry_shader(geometry_shader* g);
	void unmake_pixel_shader(pixel_shader* p);
	void unmake_compute_shader(compute_shader* c);
#endif
};

	} // namespace gpu
} // namespace ouro

#endif
