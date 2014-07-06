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
// Cross-platform API for the major vocabulary of 3D rendering while trying to 
// remain policy-agnostic (i.e. this is not a renderer, just a hardware abstraction).
#pragma once
#ifndef oGPU_h
#define oGPU_h

#include <oBase/allocate.h>
#include <oBase/dec3n.h>
#include <oBase/gpu_api.h>
#include <oBase/invalid.h>
#include <oBase/macros.h>
#include <oBase/types.h>
#include <oBase/vendor.h>
#include <oBase/version.h>
#include <oMesh/mesh.h>
#include <oSurface/buffer.h>
#include <array>

#include <oGPU/render_target.h>
#include <oGPU/shader.h>
#include <oGPU/state.h>
#include <oGPU/vertex_layouts.h>

#include <oGUI/oGUI.h> // only for ouro::draw_context_handle... can it be abstracted

namespace ouro {

class window;

	namespace gpu {

		namespace oGPUUtilLayout { enum value; }

static const uint max_num_input_slots = 3;
static const uint max_num_unordered_buffers = 8;
static const uint max_num_viewports = 16;
//static const uint max_num_mrts = 8;

// _____________________________________________________________________________
// Device concepts

namespace debug_level
{ oDECLARE_SMALL_ENUM(value, uchar) {

	none, // No driver debug reporting
	normal, // Trivial/auto-handled warnings by driver squelched
	unfiltered, // No suppression of driver warnings
	
	count,

};}

namespace resource_type
{ oDECLARE_SMALL_ENUM(value, uchar) {

	buffer,
	texture,

	count,

};}

struct device_init
{
	device_init(const char* _DebugName = "GPU device")
		: debug_name(_DebugName)
		, version(11,0)
		, min_driver_version(0,0)
		, driver_debug_level(debug_level::none)
		, adapter_index(0)
		, virtual_desktop_position(oDEFAULT, oDEFAULT)
		, use_software_emulation(false)
		, use_exact_driver_version(false)
		, multithreaded(true)
	{}

	// Name associated with this device in debug output
	sstring debug_name;

	// The version of the underlying API to use.
	struct version version;

	// The minimum version of the driver required to successfully create the 
	// device. If the driver version is 0.0.0.0 (the default) then a hard-coded
	// internal value is used based on QA verificiation.
	struct version min_driver_version;

	// Specify to what degree driver warnings/errors are reported. GPU-level 
	// errors and warnings are always reported.
	debug_level::value driver_debug_level;

	// If virtual_desktop_position is oDEFAULT, oDEFAULT then use the nth found
	// device as specified by this Index. If virtual_desktop_position is anything
	// valid then the device used to handle that desktop position will be used
	// and adapter_index is ignored.
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

// Main SW abstraction for a graphics processor. The class is defined below since it uses
// all other objects defined in this header.
class device;

// Main SW abstraction for a single thread of graphics command preparation. This object
// is defined below since it uses all other objecsts defined in this header.
class command_list;

class device_child
{
	// Anything allocated from the device is a device child
public:
	// fill the specified pointer with this resources's associated device. The 
	// device's ref count is incremented.
	virtual std::shared_ptr<device> get_device() const = 0;

	// Returns the identifier as specified at create time.
	virtual const char* name() const = 0;
};

class resource : public device_child
{
	// Anything that contains data intended primarily for read-only access by the 
	// GPU processor is a resource. This does not exclude write access but 
	// generally differentiates these objects from process and target classs
	// that are mostly write-only.

public:
	// Returns the type of this resource.
	virtual resource_type::value type() const = 0;

	// Returns an ID for this resource fit for use as a hash.
	virtual uint id() const = 0;

	// Returns the component sizes of a subresource. X is the RowSize or the 
	// number of valid bytes in one scanline of a texture or the size of one 
	// element of any other buffer. Y is the number of scanlines/rows in a 
	// texture or the number of elements in the buffer.
	virtual uint2 byte_dimensions(int _Subresource) const = 0;
};

// _____________________________________________________________________________
// Buffer concepts

namespace buffer_type
{ oDECLARE_SMALL_ENUM(value, uchar) {
	// Binding fit for rasterization HW. Using this requires a structure byte size
	// to be specified.
	constant,

	// A constant buffer fit for receiving a copy from GPU memory to CPU-
	// accessible memory.
	readback,

	// A raw buffer indexed by bytes (32-bit aligned access only though). This is
	// the type to use for dispatch parameters from the GPU (indirect drawing).
	unordered_raw,

	// Buffer that does not guarantee order of access. Such ordering must be done
	// by the explicit use of atomics in client shader code. Using this requires
	// a surface::format to be specified.
	unordered_unstructured,

	// Buffer that does not guarantee order of access. Such ordering must be done
	// by the explicit use of atomics in client shader code. Using this requires
	// a structure byte size to be specified.
	unordered_structured,

	// Like unordered_structured but also support extra bookkeeping to enable 
	// atomic append/consume. Using this requires a structure byte size to be 
	// specified.
	unordered_structured_append, 

	// Like unordered_structured but also support extra bookkeeping to enable 
	// atomic increment/decrement. Using this requires a structure byte size to be 
	// specified.
	unordered_structured_counter,

	count,

};}

inline bool has_counter(const buffer_type::value& _Type) { return _Type == buffer_type::unordered_structured_append || _Type == buffer_type::unordered_structured_counter; }

struct buffer_info
{
	// A constant buffer (view, draw, material). Client code can defined whatever 
	// value are to be passed to a shader that expects them. struct_byte_size must 
	// be 16-byte aligned. For unstructured buffers specify size = 0, and provide
	// a format.

	buffer_info()
		: type(buffer_type::constant)
		, format(surface::unknown)
		, struct_byte_size(0)
		, array_size(1)
	{}

	// Specifies the type of the constant buffer. Normally the final buffer size
	// is struct_byte_size * array_size. If the type is specified as 
	// unordered_unstructured then StructByteSize must be 0 and the size is 
	// calculated as (size of Format) * ArraySize.
	buffer_type::value type;

	// This must be valid for unordered_unstructured types, and surface::unknown 
	// for all other types.
	surface::format format;

	// This must be invalid_size for unordered_unstructured types, but valid for 
	// all other types.
	ushort struct_byte_size;

	// The number of format elements or structures in the buffer.
	uint array_size;
};

class buffer : public resource
{
	// Buffers are directly accessed as memory - not through a sampler.
public:
	virtual buffer_info get_info() const = 0;

	virtual void* get_buffer() const = 0;
};

// _____________________________________________________________________________
// Texture concepts

namespace cube_face
{ oDECLARE_SMALL_ENUM(value, uchar) {

	posx,
	negx,
	posy,
	negy,
	posz,
	negz,

	count,

};}

namespace texture_type
{ oDECLARE_SMALL_ENUM(value, uchar) {

	// 8-bits: array, mipped, usage, type
	// 00AMUsTy

	type_mask = 0x3,
	type_1d = 0,
	type_2d = 1,
	type_3d = 2,
	type_cube = 3,
	usage_mask = 0xc,
	usage_default = 0,
	usage_readback = 4,
	usage_render_target = 8,
	usage_unordered = 12,
	flag_mipped = 16,
	flag_array = 32,

	// 1D texture.
	default_1d = type_1d,
	mipped_1d = type_1d | flag_mipped,
	array_1d = type_1d | flag_array,
	mipped_array_1d = type_1d | flag_array | flag_mipped,
	render_target_1d = type_1d | usage_render_target,
	mipped_render_target_1d = type_1d | flag_mipped | usage_render_target,
	readback_1d = type_1d | usage_readback,
	mipped_readback_1d = type_1d | flag_mipped | usage_readback,
	readback_array_1d = type_1d | flag_array | usage_readback,
	mipped_readback_array_1d = type_1d | flag_array | flag_mipped | usage_readback,

	// "normal" 2D texture.
	default_2d = type_2d,
	mipped_2d = type_2d | flag_mipped,
	array_2d = type_2d | flag_array,
	mipped_array_2d = type_2d | flag_array | flag_mipped,
	render_target_2d = type_2d | usage_render_target,
	mipped_render_target_2d = type_2d | flag_mipped | usage_render_target,
	readback_2d = type_2d | usage_readback,
	mipped_readback_2d = type_2d | flag_mipped | usage_readback,
	readback_array_2d = type_2d | flag_array | usage_readback,
	mipped_readback_array_2d = type_2d | flag_array | flag_mipped | usage_readback,

	// a "normal" 2D texture, no mips, configured for unordered access. Currently
	// all GPGPU access to such buffers are one subresource at a time so there is 
	// no spec that describes unordered access to arbitrary mipped memory.
	unordered_2d = type_2d | usage_unordered,
	
	// 6- 2D slices that form the faces of a cube that is sampled from its center.
	default_cube = type_cube,
	mipped_cube = type_cube | flag_mipped,
	array_cube = type_cube | flag_array,
	mipped_array_cube = type_cube | flag_array | flag_mipped,
	render_target_cube = type_cube | usage_render_target,
	mipped_render_target_cube = type_cube | flag_mipped | usage_render_target,
	readback_cube = type_cube | usage_readback,
	mipped_readback_cube = type_cube | flag_mipped | usage_readback,

	// Series of 2D slices sampled as a volume
	default_3d = type_3d,
	mipped_3d = type_3d | flag_mipped,
	array_3d = type_3d | flag_array,
	mipped_array_3d = type_3d | flag_array | flag_mipped,
	render_target_3d = type_3d | usage_render_target,
	mipped_render_target_3d = type_3d | flag_mipped | usage_render_target,
	readback_3d = type_3d | usage_readback,
	mipped_readback_3d = type_3d | flag_mipped | usage_readback,

};}

inline bool is_1d(const texture_type::value& _Type) { return texture_type::type_1d == (_Type & texture_type::type_mask); }
inline bool is_2d(const texture_type::value& _Type) { return texture_type::type_2d == (_Type & texture_type::type_mask); }
inline bool is_3d(const texture_type::value& _Type) { return texture_type::type_3d == (_Type & texture_type::type_mask); }
inline bool is_cube(const texture_type::value& _Type) { return texture_type::type_cube == (_Type & texture_type::type_mask); }
inline bool is_default(const texture_type::value& _Type) { return texture_type::usage_default == (_Type & texture_type::usage_mask); }
inline bool is_readback(const texture_type::value& _Type) { return texture_type::usage_readback == (_Type & texture_type::usage_mask); }
inline bool is_render_target(const texture_type::value& _Type) { return texture_type::usage_render_target == (_Type & texture_type::usage_mask); }
inline bool is_unordered(const texture_type::value& _Type) { return texture_type::usage_unordered == (_Type & texture_type::usage_mask); }
inline bool is_mipped(const texture_type::value& _Type) { return 0 != (_Type & texture_type::flag_mipped); }
inline bool is_array(const texture_type::value& _Type) { return 0 != ((int)_Type & texture_type::flag_array); }

inline texture_type::value get_type(const texture_type::value& _Type) { return (texture_type::value)(_Type & texture_type::type_mask); }
inline texture_type::value get_usage(const texture_type::value& _Type) { return (texture_type::value)(_Type & texture_type::usage_mask); }

inline texture_type::value make_default(const texture_type::value& _Type) { return (texture_type::value)((_Type & ~texture_type::usage_mask) | texture_type::usage_default); }
inline texture_type::value make_readback(const texture_type::value& _Type) { return (texture_type::value)((_Type & ~texture_type::usage_mask) | texture_type::usage_readback); }
inline texture_type::value make_render_target(const texture_type::value& _Type) { return (texture_type::value)((_Type & ~texture_type::usage_mask) | texture_type::usage_render_target); }
inline texture_type::value make_unordered(const texture_type::value& _Type) { return (texture_type::value)((_Type & ~texture_type::usage_mask) | texture_type::usage_unordered); }

inline texture_type::value add_mipped(const texture_type::value& _Type) { return (texture_type::value)(_Type | texture_type::flag_mipped); }
inline texture_type::value add_array(const texture_type::value& _Type) { return (texture_type::value)(_Type | texture_type::flag_array); }

inline texture_type::value remove_mipped(const texture_type::value& _Type) { return (texture_type::value)(_Type & ~texture_type::flag_mipped); }
inline texture_type::value remove_render_target(const texture_type::value& _Type) { return (texture_type::value)(_Type & ~texture_type::usage_render_target); }

struct texture_info
{
	texture_info()
		: type(texture_type::default_2d)
		, format(surface::b8g8r8a8_unorm)
		, dimensions(0, 0, 0)
		, array_size(1)
	{}

	texture_type::value type;
	surface::format format;
	ushort3 dimensions;
	uint array_size;
};

namespace clear_type
{ oDECLARE_SMALL_ENUM(value, uchar) {

	depth,
	stencil,
	depth_stencil,
	color,
	color_depth,
	color_stencil,
	color_depth_stencil,

	count,

};}

class texture : public resource
{
	// A large buffer usually filled with image data that is often accessed
	// through a texture sampler. A texture is often read-only or the read
	// interface for a render_target.
public:
	virtual texture_info get_info() const = 0;
};

class render_target : public device_child
{
	// A write-only buffer onto which hardware-accelerating rasterization occurs.
public:
	virtual render_target_info get_info() const = 0;

	// Modifies the values for clearing without modifying other topology
	virtual void set_clear_depth_stencil(float _Depth, uchar _Stencil) = 0;
	virtual void set_clear_color(uint _Index, color _Color) = 0;
	inline void set_clear_color(color _Color) { set_clear_color(0, _Color); }

	// Resizes all buffers without changing formats or other topology
	virtual void resize(const int3& _NewDimensions) = 0;
	inline void resize(const int2& _NewDimensions) { resize(int3(_NewDimensions, 1)); }

	// Accesses a readable texture for the specified render target in an MRT.
	virtual std::shared_ptr<texture> get_texture(int _MRTIndex) = 0;

	// Accesses a readable texture for the depth-stencil buffer. This will throw if 
	// there is no depth-stencil buffer or if the buffer is a non-readable format.
	virtual std::shared_ptr<texture> get_depth_texture() = 0;

	// Creates a buffer of the contents of the render target. This should be 
	// called at times when it is known the render target has been fully resolved,
	// mostly outside of begin_frame/end_frame. If this is called on the primary
	// render target the back-buffer is captured.
	virtual std::shared_ptr<surface::buffer> make_snapshot(int _MRTIndex) = 0;
};

// _____________________________________________________________________________
// Pipeline concepts

namespace query_type
{ oDECLARE_SMALL_ENUM(value, uchar) {

	timer,

	count,

};}

struct basic_pipeline1_info
{
	const char* debug_name;
	mesh::element_array vertex_layout;
	mesh::primitive_type::value primitive_type;
	const void* vs, *hs, *ds, *gs, *ps;
};

struct pipeline1_info : public basic_pipeline1_info
{
	pipeline1_info()
	{
		debug_name = "unnamed pipeline";
		vertex_layout.fill(mesh::element());
		primitive_type = mesh::primitive_type::unknown;
		vs = nullptr; hs = nullptr; ds = nullptr; gs = nullptr; ps = nullptr;
	}
};

struct basic_compute_kernel_info
{
	const char* debug_name;
	const void* cs;
};

struct compute_kernel_info : public basic_compute_kernel_info
{
	compute_kernel_info()
	{
		debug_name = "unnamed compute kernel";
		cs = nullptr;
	}
};

class pipeline1 : public device_child 
{
	// A pipeline is the result of setting all stages of the programmable pipeline 
	// (all shaders) and the vertex input format to that pipeline.
public:
	virtual pipeline1_info get_info() const = 0;
};

class compute_kernel : public device_child 
{
	// That other pipeline mode GPUs support: This is the CUDA/Compute/OpenCL 
	// path that ignores fixed-function rasterization stages and exposes more 
	// general-purpose treatment of the GPU.
public:
	virtual compute_kernel_info get_info() const = 0;
};

// _____________________________________________________________________________
// Execution concepts

struct query_info
{
	query_info()
		: type(query_type::timer)
	{}

	query_type::value type;
};

struct command_list_info
{
	static const short immediate = -1;

	short draw_order;
};

class query : public device_child 
{
public:
	virtual query_info get_info() const = 0;
};

class command_list : public device_child 
{
	// A container for a list of commands issued by the user to the graphics 
	// device. All operations herein are single-threaded. For parallelism separate 
	// command lists can be built in different threads.
public:
	virtual command_list_info get_info() const = 0;

	// Begins/ends recording of GPU command submissions. All rendering for this context 
	// should occur between Begin() and End().
	virtual void begin() = 0;
	virtual void end() = 0;

	// The specified query will record events in between these begin/ends.
	virtual void begin_query(query* _pQuery) = 0;
	virtual void end_query(query* _pQuery) = 0;

	inline void begin_query(std::shared_ptr<query>& _pQuery) { begin_query(_pQuery.get()); }
	inline void end_query(std::shared_ptr<query>& _pQuery) { end_query(_pQuery.get()); }

	// This should never be required to be called, and has bad performance 
	// implications if called but is sometimes required, especially with the 
	// immediate context during debugging.
	virtual void flush() = 0;

	// Clears the command list's idea of state back to its default. In general 
	// client code should set state absolute state, but if a command list is 
	// reassigned to a different responsibility, it may make sense to start with
	// a clean slate.
	virtual void reset() = 0;

	// Allocates internal device memory that can be written to (not read) and 
	// committed to the device to update the specified resource.
	virtual surface::mapped_subresource reserve(resource* _pResource, int _Subresource) = 0;
	template<typename T> surface::mapped_subresource reserve(std::shared_ptr<T>& _pResource, int _Subresource) { return reserve(_pResource.get(), _Subresource); }

	// Commits memory to the specified resource. If the memory in _Source. 
	// was reserved, then this will free the memory. If _Source.data is user 
	// memory, it will not be freed. If the specified rectangle is empty on any 
	// dimension, then the entire surface will be copied (default behavior). If 
	// the item is a buffer then units are in structs, i.e. Left=0, Right=ArraySize 
	// would be a full copy. Ensure that the other dimension are not empty/equal 
	// even in the buffer case.
	virtual void commit(resource* _pResource, int _Subresource, const surface::mapped_subresource& _Source, const surface::box& _Subregion = surface::box()) = 0;
	inline void commit(resource* _pResource, int _Subresource, const surface::const_mapped_subresource& _Source, const surface::box& _Subregion = surface::box()) { commit(_pResource, _Subresource, (const surface::mapped_subresource&)_Source, _Subregion); }
	
	template<typename T> void commit(std::shared_ptr<T>& _pResource, int _Subresource, const surface::mapped_subresource& _Source, const surface::box& _Subregion = surface::box()) { commit(_pResource.get(), _Subresource, (const surface::mapped_subresource&)_Source, _Subregion); }
	template<typename T> void commit(std::shared_ptr<T>& _pResource, int _Subresource, const surface::const_mapped_subresource& _Source, const surface::box& _Subregion = surface::box()) { commit(_pResource.get(), _Subresource, _Source, _Subregion); }

	// Copies the contents from one resource to another. Both must have compatible 
	// (often identical) topologies. A common use of this API is to copy from a 
	// source resource to a readback copy of the same resource so it can be 
	// accessed from the CPU.
	virtual void copy(resource* _pDestination, resource* _pSource) = 0;
	template<typename T, typename U> void copy(std::shared_ptr<T>& _pDestination, std::shared_ptr<U>& _pSource) { copy(_pDestination.get(), _pSource.get()); }

	// Copies from one buffer to another with offsets in bytes.
	virtual void copy(buffer* _pDestination, uint _DestinationOffsetBytes, buffer* _pSource, uint _SourceOffsetBytes, uint _SizeBytes) = 0;
	template<typename T, typename U> void copy(std::shared_ptr<T>& _pDestination, uint _DestinationOffsetBytes, std::shared_ptr<U>& _pSource, uint _SourceOffsetBytes, uint _SizeBytes) { copy(_pDestination.get(), _DestinationOffsetBytes, _pSource.get(), _SourceOffsetBytes, _SizeBytes); }

	// Copy the counter value stored in a source buffer of type 
	// buffer_type::unordered_structured_append or buffer_type::unordered_structured_counter 
	// to a destination buffer. An offset into that buffer can be specified - it must be 
	// aligned to sizeof(uint). To read back this value to the CPU, _pDestination should be 
	// a readback buffer of at least sizeof(uint) size. Then use map_read() on the device to
	// access the uint counter value.
	virtual void copy_counter(buffer* _pDestination, uint _DestinationAlignedOffset, buffer* _pUnorderedSource) = 0;
	template<typename T, typename U> void copy_counter(std::shared_ptr<T>& _pDestination, uint _DestinationAlignedOffset, std::shared_ptr<U>& _pUnorderedSource) { copy_counter(_pDestination.get(), _DestinationAlignedOffset, _pUnorderedSource.get()); }

	// Sets the counter in the specified buffer to the specified value. This 
	// incurs a dispatch and should not be used in the main loop of production 
	// code. This is exposed primarily for test cases and initialization. For main 
	// loop code use the _pInitialCounts parameter of set_unordered_resources or 
	// set_render_target_and_unordered_resources. REMEMBER: This will occur when the 
	// command list is submitted to the device during end_frame(). If the desire is
	// immediate ensure this command list is the one retrieved from 
	// device::get_immediate_command_list().
	virtual void set_counters(uint _NumUnorderedResources, resource** _ppUnorderedResources, uint* _pValues) = 0;
	inline void set_counters(uint _NumUnorderedBuffers, buffer** _ppUnorderedBuffers, uint* _pValues) { set_counters(_NumUnorderedBuffers, (resource**)_ppUnorderedBuffers, _pValues); }
	inline void set_counters(resource* _pResource, uint _Value) { set_counters(1, &_pResource, &_Value); }

	// Set the texture sampler states in this context
	virtual void set_samplers(uint _StartSlot, uint _NumStates, const sampler_state::value* _pSamplerState) = 0;
	inline void set_sampler(uint _StartSlot, const sampler_state::value& _SamplerState) { set_samplers(_StartSlot, 1, &_SamplerState); }

	// Set any shader resources (textures or buffers not accessed as constants)
	virtual void set_shader_resources(uint _StartSlot, uint _NumResources, const resource* const* _ppResources) = 0;
	inline void set_shader_resources(uint _StartSlot, uint _NumResources, const buffer* const* _ppResources) { set_shader_resources(_StartSlot, _NumResources, (const resource* const*)_ppResources); }
	inline void set_shader_resources(uint _StartSlot, uint _NumResources, const texture* const* _ppResources) { set_shader_resources(_StartSlot, _NumResources, (const resource* const*)_ppResources); }
	
	template<uint size> void set_shader_resources(uint _StartSlot, const resource* const (&_ppResources)[size]) { set_shader_resources(_StartSlot, size, (const resource* const*)_ppResources); }
	template<uint size> void set_shader_resources(uint _StartSlot, const buffer* const (&_ppResources)[size]) { set_shader_resources(_StartSlot, size, (const buffer* const*)_ppResources); }
	template<uint size> void set_shader_resources(uint _StartSlot, const texture* const (&_ppResources)[size]) { set_shader_resources(_StartSlot, size, (const texture* const*)_ppResources); }

	template<uint size> void set_shader_resources(uint _StartSlot, const std::shared_ptr<resource> (&_ppResources)[size]) { set_shader_resources(_StartSlot, size, (const resource* const*)_ppResources); }
	template<uint size> void set_shader_resources(uint _StartSlot, const std::shared_ptr<buffer> (&_ppResources)[size]) { set_shader_resources(_StartSlot, size, (const buffer* const*)_ppResources); }
	template<uint size> void set_shader_resources(uint _StartSlot, const std::shared_ptr<texture> (&_ppResources)[size]) { set_shader_resources(_StartSlot, size, (const texture* const*)_ppResources); }
	
	inline void set_shader_resource(uint _StartSlot, const resource* _pResource) { set_shader_resources(_StartSlot, 1, &_pResource); }
	inline void set_shader_resource(uint _StartSlot, const texture* _pResource) { set_shader_resources(_StartSlot, 1, &_pResource); }

	inline void set_shader_resource(uint _StartSlot, const std::shared_ptr<resource>& _pResource) { set_shader_resource(_StartSlot, _pResource.get()); }
	inline void set_shader_resource(uint _StartSlot, const std::shared_ptr<texture>& _pResource) { set_shader_resource(_StartSlot, _pResource.get()); }

	// Sets the render target to which rendering will occur. By default a single
	// full-target viewport is created else it can be overridden. A viewport is 
	// a 3D box whose minimum is at the top, left, near corner of the viewable 
	// frustum, and whose maximum is at the bottom, right, far corner of the 
	// viewable frustum. A full-target viewport would most often be: 
	// boundf(float3(0.0f), float3(RTWidth, RTHeight, 1.0f)). In addition this
	// also sets up unordered resources resetting any counters according to values 
	// in the _pInitialCounts array, which should be the same count as the number 
	// of unordered resources being bound. This API can also bound unordered 
	// resources for access during Dispatch() calls if _SetForDispatch is true. 
	// If _SetForDispatch is true, then _pRenderTarget must be null, though 
	// viewport settings will be respected. If _UnorderedResourceStartSlot is 
	// invalid, the value _pRenderTarget::DESC.MRTCount will be used. If 
	// _pRenderTarget is nullptr then a valid _UnorderedResourceStartSlot value 
	// must be specified. If _NumUnorderedResources is invalid, all slots after 
	// the render target's MRTs are cleared. See SetRnederTarget() as an example 
	// of setting the RT while clearing unordered targets).
	virtual void set_render_target_and_unordered_resources(render_target* _pRenderTarget, uint _NumViewports, const aaboxf* _pViewports, bool _SetForDispatch, uint _UnorderedResourcesStartSlot, uint _NumUnorderedResources, resource** _ppUnorderedResources, uint* _pInitialCounts = nullptr) = 0;
	inline void set_render_target_and_unordered_resources(render_target* _pRenderTarget, uint _NumViewports, const aaboxf* _pViewports, bool _SetForDispatch, uint _UnorderedResourcesStartSlot, uint _NumUnorderedResources, buffer** _ppUnorderedResources, uint* _pInitialCounts = nullptr) { set_render_target_and_unordered_resources(_pRenderTarget, _NumViewports, _pViewports, _SetForDispatch, _UnorderedResourcesStartSlot, _NumUnorderedResources, (resource**)_ppUnorderedResources, _pInitialCounts); }
	template<uint size> inline void set_render_target_and_unordered_resources(render_target* _pRenderTarget, uint _NumViewports, const aaboxf* _pViewports, bool _SetForDispatch, uint _UnorderedResourcesStartSlot, resource* (&_ppUnorderedResources)[size], uint (&_pInitialCounts)[size]) { set_render_target_and_unordered_resources(_pRenderTarget, _NumViewports, _pViewports, _SetForDispatch, _UnorderedResourcesStartSlot, size, _ppUnorderedResources, _pInitialCounts); }
	inline void set_unordered_resources(uint _UnorderedResourcesStartSlot, uint _NumUnorderedResources, resource** _ppUnorderedResources, uint* _pInitialCounts = nullptr) { set_render_target_and_unordered_resources(nullptr, 0, nullptr, true, _UnorderedResourcesStartSlot, _NumUnorderedResources, _ppUnorderedResources, _pInitialCounts); }
	template<uint size> inline void set_unordered_resources(uint _UnorderedResourcesStartSlot, resource* (&_ppUnorderedResources)[size]) { set_render_target_and_unordered_resources(nullptr, 0, nullptr, true, _UnorderedResourcesStartSlot, size, _ppUnorderedResources); }
	template<uint size> inline void set_unordered_resources(uint _UnorderedResourcesStartSlot, resource* (&_ppUnorderedResources)[size], uint (&_pInitialCounts)[size]) { set_render_target_and_unordered_resources(nullptr, 0, nullptr, true, _UnorderedResourcesStartSlot, size, _ppUnorderedResources, _pInitialCounts); }
	inline void set_unordered_resources(uint _UnorderedResourcesStartSlot, uint _NumUnorderedResources, buffer** _ppUnorderedResources, uint* _pInitialCounts = nullptr) { set_unordered_resources(_UnorderedResourcesStartSlot, _NumUnorderedResources, (resource**)_ppUnorderedResources, _pInitialCounts); }
	inline void set_unordered_resource(uint _UnorderedResourcesStartSlot, buffer* _pUnorderedResources, uint _InitialCount = invalid) { set_unordered_resources(_UnorderedResourcesStartSlot, 1, &_pUnorderedResources, _InitialCount == invalid ? nullptr : &_InitialCount); }
	inline void set_unordered_resource(uint _UnorderedResourcesStartSlot, std::shared_ptr<buffer>& _pUnorderedResources, uint _InitialCount = invalid) { set_unordered_resource(_UnorderedResourcesStartSlot, _pUnorderedResources.get(), _InitialCount); }

	// Simpler version that clears UAVs since likely during rasterization UAVs 
	// will be used as shader resources, which conflicts with being a target. If
	// this is not the desired behavior, use the above more explicit/complicated 
	// version.
	inline void set_render_target(render_target* _pRenderTarget, int _NumViewports = 0, const aaboxf* _pViewports = nullptr) { set_render_target_and_unordered_resources(_pRenderTarget, _NumViewports, _pViewports, false, invalid, invalid, (resource**)nullptr); }
	inline void set_render_target(std::shared_ptr<render_target>& _pRenderTarget, int _NumViewports = 0, const aaboxf* _pViewports = nullptr) { set_render_target(_pRenderTarget.get(), _NumViewports, _pViewports); }

	inline void clear_render_target_and_unordered_resources() { set_render_target_and_unordered_resources(nullptr, 0, nullptr, false, 0, max_num_unordered_buffers, (resource**)nullptr); }

	inline void clear_unordered_resources() { set_render_target_and_unordered_resources(nullptr, 0, nullptr, true, 0, max_num_unordered_buffers, (resource**)nullptr); } 

	// _____________________________________________________________________________
	// Rasterization-specific

	virtual void set_pipeline(const pipeline1* _pPipeline) = 0;
	inline void set_pipeline(const std::shared_ptr<pipeline1>& _pPipeline) { set_pipeline(_pPipeline.get()); }

	// Set the rasterization state in this context
	virtual void set_rasterizer_state(const rasterizer_state::value& _State) = 0;

	// Set the output merger (blend) state in this context
	virtual void set_blend_state(const blend_state::value& _State) = 0;

	// Set the depth-stencil state in this context
	virtual void set_depth_stencil_state(const depth_stencil_state::value& _State) = 0;

	// Uses a render target's CLEAR_DESC to clear all associated buffers
	// according to the type of clear specified here. If _pRenderTarget is nullptr
	// the the currently set render target is cleared.
	virtual void clear(render_target* _pRenderTarget, const clear_type::value& _Clear) = 0;
	inline void clear(std::shared_ptr<render_target>& _pRenderTarget, const clear_type::value& _Clear) { clear(_pRenderTarget.get(), _Clear); }

	// Generates mips from the top-level mip in the specified render target
	// This happens wholly on the GPU.
	virtual void generate_mips(render_target* _pRenderTarget) = 0;
	inline void generate_mips(std::shared_ptr<render_target>& _pRenderTarget) { generate_mips(_pRenderTarget.get()); }

	// _____________________________________________________________________________
	// Compute-specific

	// Clears the specified resource created for unordered access.
	virtual void cleari(resource* _pUnorderedResource, const uint4& _Values) = 0;

	// Clears the specified resource created for unordered access.
	virtual void clearf(resource* _pUnorderedResource, const float4& _Values) = 0;

	// If all 3 values in _ThreadGroupCount are invalid, then the counts are 
	// automatically determined by the GPU. Unordered resources can be 
	// buffer or oGPUTexture, but each must have been created readied
	// for unordered access. If not null, _pInitialCounts should be 
	// _NumUnorderedResources in length and is used to set the initial value of a 
	// counter or append/consume count for buffers of type 
	// buffer_type::unordered_structured_append or 
	// buffer_type::unordered_structured_counter. Specify invalid to skip 
	// initialization of an entry, thus retaining any prior value of the counter.
	virtual void dispatch(compute_kernel* _pComputeKernel, const int3& _ThreadGroupCount) = 0;
	inline void dispatch(std::shared_ptr<compute_kernel>& _pComputeKernel, const int3& _ThreadGroupCount) { dispatch(_pComputeKernel.get(), _ThreadGroupCount); }
};

class device
{
	// Main SW abstraction for a graphics processor
public:
	static std::shared_ptr<device> make(const device_init& _Init);

	virtual device_info get_info() const = 0;
	
	virtual const char* name() const = 0;
	virtual int frame_id() const = 0;

	// Returns a command list with an invalid DrawOrder in its DESC that does not
	// lock or wait for begin_frame/end_frame. Be very careful with this because it
	// sets state that deferred command lists may not be aware of and does so
	// immediately without care for ordering. This should only be called from a 
	// single thread. Calls to this CommandList need not be inside a begin_frame/
	// end_frame. Begin()/End() for the immediate command list will noop, so either
	// they don't need to be called or an immediate command list should be able to 
	// substitute for a regular (deferred) command list without code change 
	// (though timing and order differences are likely).
	virtual std::shared_ptr<command_list> get_immediate_command_list() = 0;

	virtual command_list* immediate() = 0;

	virtual std::shared_ptr<render_target> make_primary_render_target(window* _pWindow, surface::format _DepthStencilFormat, bool _EnableOSRendering) = 0;
	inline std::shared_ptr<render_target> make_primary_render_target(std::shared_ptr<window>& _pWindow, surface::format _DepthStencilFormat, bool _EnableOSRendering) { return make_primary_render_target(_pWindow.get(), _DepthStencilFormat, _EnableOSRendering); }

	virtual std::shared_ptr<command_list> make_command_list(const char* _Name, const command_list_info& _Info) = 0;
	virtual std::shared_ptr<pipeline1> make_pipeline1(const char* _Name, const pipeline1_info& _Info) = 0;
	virtual std::shared_ptr<compute_kernel> make_compute_kernel(const char* _Name, const compute_kernel_info& _Info) = 0;
	virtual std::shared_ptr<query> make_query(const char* _Name, const query_info& _Info) = 0;
	virtual std::shared_ptr<render_target> make_render_target(const char* _Name, const render_target_info& _Info) = 0;
	virtual std::shared_ptr<texture> make_texture(const char* _Name, const texture_info& _Info) = 0;

	// convenience versions of the above
	inline std::shared_ptr<command_list> make_command_list(const char* _Name, short _DrawOrder = 0) { command_list_info i; i.draw_order = _DrawOrder; return make_command_list(_Name, i); } 
	inline std::shared_ptr<pipeline1> make_pipeline1(const pipeline1_info& _Info) { return make_pipeline1(_Info.debug_name, _Info); }
	inline std::shared_ptr<compute_kernel> make_compute_kernel(const compute_kernel_info& _Info) { return make_compute_kernel(_Info.debug_name, _Info); }

	// map_read is a non-blocking call to read from the specified resource by the
	// mapped data populated in the specified _pMappedSubresource. If the function
	// would block, this returns false. If it succeeds, call ReadEnd to unlock.
	// the buffer. This will also return false for resources not of type READBACK.
	virtual bool map_read(resource* _pReadbackResource, int _Subresource, surface::mapped_subresource* _pMappedSubresource, bool _Blocking = false) = 0;
	virtual void unmap_read(resource* _pReadbackResource, int _Subresource) = 0;
	template<typename T> bool map_read(std::shared_ptr<T>& _pReadbackResource, int _Subresource, surface::mapped_subresource* _pMappedSubresource, bool _Blocking = false) { return map_read(_pReadbackResource.get(), _Subresource, _pMappedSubresource, _Blocking); }
	template<typename T> void unmap_read(std::shared_ptr<T>& _pReadbackResource, int _Subresource) { unmap_read(_pReadbackResource.get(), _Subresource); }

	virtual bool read_query(query* _pQuery, void* _pData, uint _SizeofData) = 0;
	template<typename T> bool read_query(query* _pQuery, T* _pData) { return read_query(_pQuery, _pData, sizeof(T)); }
	template<typename T> bool read_query(std::shared_ptr<query>& _pQuery, T* _pData) { return read_query(_pQuery.get(), _pData, sizeof(T)); }

	virtual bool begin_frame() = 0;
	virtual void end_frame() = 0;

	// After out of end_frame, the device can provide a handle to OS CPU-based 
	// rendering. All OS calls should occur in between begin_os_frame and 
	// end_os_frame and this should be called after end_frame, but before present 
	// to ensure no tearing.
	virtual draw_context_handle begin_os_frame() = 0;
	virtual void end_os_frame() = 0;

	// This should only be called on same thread as the window passed to 
	// CreatePrimaryRenderTarget. If a primary render target does not exist, this
	// will noop, otherwise the entire client area of the associated window will
	// receive the contents of the back buffer. Call this after end_frame() to 
	// ensure all commands have been flushed.

	virtual bool is_fullscreen_exclusive() const = 0;
	virtual void set_fullscreen_exclusive(bool _Fullscreen) = 0;

	virtual void present(uint _PresentInterval) = 0;
};

class scoped_device_frame
{
	device* dev;
public:
	scoped_device_frame(device* dev) : dev(dev) { dev->begin_frame(); }
	~scoped_device_frame() { if (dev) dev->end_frame(); }
};

class scoped_command_line_frame
{
	command_list* cl;
public:
	scoped_command_line_frame(command_list* cl) : cl(cl) { cl->begin(); }
	~scoped_command_line_frame() { cl->end(); }
};


	} // namespace gpu
} // namespace ouro

#if 0

#include <oBasis/oBuffer.h>
#include <oSurface/surface.h>
#include <oGUI/window.h>

// @tony: I'm not sure if these APIs belong here at all, in oGPU, or 
// oGPUUtil. I do know they don't belong where they were before, which was 
// nowhere.
// Basically rendering at oooii grew up on D3D and D3DX, which have very robust
// libraries for image format conversion and handling. We didn't spend time 
// reproducing this on our own, so we're relying on this stuff as placeholder.
// I suspect if we add support for a different library, such robust support for
// image formats won't be there and we'll have to implement a lot of this stuff,
// so defer that dev time until then and then bring it back to something that
// can be promoted out of oGPU. For now, at least hide the D3D part...

// Convert the format of a surface into another format in another surface. This
// uses GPU acceleration for BC6H/7 and is currently a pass-through to D3DX11's
// conversion functions at the moment. Check debug logs if this function seems
// to hang because if for whatever reason the CPU/SW version of the BC6H/7
// codec is used, it can take a VERY long time.
bool oGPUSurfaceConvert(
	void* oRESTRICT _pDestination
	, uint _DestinationRowPitch
	, surface::format _DestinationFormat
	, const void* oRESTRICT _pSource
	, uint _SourceRowPitch
	, surface::format _SourceFormat
	, const int2& _MipDimensions);

// Extract the parameters for the above call directly from textures
bool oGPUSurfaceConvert(oGPUTexture* _pSourceTexture, surface::format _NewFormat, oGPUTexture** _ppDestinationTexture);

// Loads a texture from disk. The _Desc specifies certain conversions/resizes
// that can occur on load. Use 0 or surface::unknown to use values as 
// they are found in the specified image resource/buffer.
// @tony: At this time the implementation does NOT use oImage loading 
// code plus a simple call to call oGPUCreateTexture(). Because this API 
// supports conversion for any surface::format, at this time we defer to 
// DirectX's .dds support for advanced formats like BC6 and BC7 as well as their
// internal conversion library. When it's time to go cross-platform, we'll 
// revisit this and hopefully call more generic code.
bool oGPUTextureLoad(oGPUDevice* _pDevice, const texture_info& _Info, const char* _URIReference, const char* _DebugName, oGPUTexture** _ppTexture);
bool oGPUTextureLoad(oGPUDevice* _pDevice, const texture_info& _Info, const char* _DebugName, const void* _pBuffer, uint _SizeofBuffer, oGPUTexture** _ppTexture);

enum oGPU_FILE_FORMAT
{
	oGPU_FILE_FORMAT_DDS,
	oGPU_FILE_FORMAT_JPG,
	oGPU_FILE_FORMAT_PNG,
};

// Saves a texture to disk. The format will be determined from the contents of 
// the texture. If the specified format does not support the contents of the 
// texture the function will return false - check oErrorGetLast() for extended
// information.
bool oGPUTextureSave(oGPUTexture* _pTexture, oGPU_FILE_FORMAT _Format, void* _pBuffer, uint _SizeofBuffer);
bool oGPUTextureSave(oGPUTexture* _pTexture, oGPU_FILE_FORMAT _Format, const char* _Path);

#endif
#endif