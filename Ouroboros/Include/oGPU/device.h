// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_device_h
#define oGPU_device_h

#include <oBase/gpu_api.h>
#include <oBase/vendor.h>
#include <oBase/version.h>
#include <oGPU/command_list.h>
#include <oGPU/shader.h>

namespace ouro { namespace gpu {

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
		, api(gpu_api::unknown)
		, vendor(vendor::unknown)
		, is_software_emulation(false)
		, driver_reporting_enabled(false)
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
	gpu_api api;

	// Describes the company that made the device.
	vendor vendor;

	// True if the device was created in software emulation mode.
	bool is_software_emulation;

	// True if the device was created with debug reporting enabled.
	bool driver_reporting_enabled;
};

class device
{
public:
	device() : dev(nullptr), supports_deferred(false) {}
	device(const device_init& init) : dev(nullptr), supports_deferred(false) { initialize(init); }
	~device() { deinitialize(); }

	void initialize(const device_init& init);
	void deinitialize();

	device_info get_info() const;
	
	// the immediate command list is always created with the device
	command_list& immediate() { return imm; }

	// clears the command list's idea of state back to its default
	void reset();

	// sends the push buffer immediately rather than waiting for it to fill
	// this can be useful if the GPU stalls early in the frame while the 
	// push buffer is still being assembled.
	void flush();

private:
	friend compute_shader* get_noop_cs(device& dev);

	void* dev;
	command_list imm;
	compute_shader noop;
	bool supports_deferred;
	bool is_sw;
};

}}

#endif
