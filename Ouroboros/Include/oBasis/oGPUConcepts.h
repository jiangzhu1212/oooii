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
// This header contains definitions of concepts relating to graphics processing.
// This header has been separated into oBasis to enable generic code such as 
// file formats and parsing and geometry synthesis to use a common vocabulary, 
// but this is also intended to be the primary vocabulary for a cross-platform 
// graphics interface as well.

// Some notable definitions for concepts used here:
// Pipeline: total configuration of GPU hardware execution that produces as 
//           result when Draw()/Submit() is called. This includes programmable 
//           shader binding as well as fixed-function state and input layout 
//           definition.

#pragma once
#ifndef oGPUConcepts_h
#define oGPUConcepts_h

#include <oBase/dec3n.h>
#include <oBase/invalid.h>
#include <oBase/macros.h>
#include <oBase/types.h>
#include <oBase/vendor.h>
#include <oBase/version.h>
#include <oMesh/mesh.h>
#include <oSurface/surface.h>
#include <array>

namespace ouro {

	namespace gpu {

static const uint max_num_input_slots = 3;
static const uint max_num_samplers = 16;
static const uint max_num_mrts = 8;
static const uint max_num_unordered_buffers = 8;
static const uint max_num_viewports = 16;
static const uint max_num_thread_groups_per_dimension = 65535;
static const uint max_num_thread_groups_per_dimension_mask = 0xffff;
static const uint max_num_thread_groups_per_dimension_shift = 16;

namespace api
{ oDECLARE_SMALL_ENUM(value, unsigned char) {

	unknown,
	d3d11,
	ogl,
	ogles,
	webgl,
	custom,

	count,

};}

namespace debug_level
{ oDECLARE_SMALL_ENUM(value, unsigned char) {

	none, // No driver debug reporting
	normal, // Trivial/auto-handled warnings by driver squelched
	unfiltered, // No suppression of driver warnings
	
	count,

};}

namespace cube_face
{ oDECLARE_SMALL_ENUM(value, unsigned char) {

	posx,
	negx,
	posy,
	negy,
	posz,
	negz,

	count,

};}

namespace resource_type
{ oDECLARE_SMALL_ENUM(value, unsigned char) {

	buffer,
	texture,

	count,

};}

namespace buffer_type
{ oDECLARE_SMALL_ENUM(value, unsigned char) {
	// Binding fit for rasterization HW. Using this requires a structure byte size
	// to be specified.
	constant,

	// A constant buffer fit for receiving a copy from GPU memory to CPU-
	// accessible memory.
	readback,

	// A buffer fit for rasterization HW. This often acts to indirect into a 
	// vertex buffer to describe primitives. There is often a special place in the 
	// pipeline for index buffers and it is likely not able to be bound to any 
	// other part.
	index,

	// An index buffer fit for receiving a copy from GPU memory to CPU-accessible 
	// memory.
	index_readback,

	// A buffer fit for rasterization HW. This holds the data representation for 
	// geometry primitives such as those described by oGPU_PRIMITIVE_TYPE.
	vertex,

	// A vertex buffer fit for receiving a copy from GPU memory to CPU-accessible 
	// memory.
	vertex_readback,

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

namespace texture_trait
{ oDECLARE_SMALL_ENUM(value, ushort) {

	cube = 1 << 0,
	_1d = 1 << 1,
	_2d = 1 << 2,
	_3d = 1 << 3,
	mipped = 1 << 4,
	array = 1 << 5,
	readback = 1 << 6,
	unordered = 1 << 7,
	render_target = 1 << 8,

};}

namespace texture_type
{ oDECLARE_SMALL_ENUM(value, ushort) {

	// 1D texture.
	default_1d = texture_trait::_1d,
	mipped_1d = texture_trait::_1d | texture_trait::mipped,
	array_1d = texture_trait::_1d | texture_trait::array,
	mipped_array_1d = texture_trait::_1d | texture_trait::array | texture_trait::mipped,
	render_target_1d = texture_trait::_1d | texture_trait::render_target,
	mipped_render_target_1d = texture_trait::_1d | texture_trait::mipped | texture_trait::render_target,
	readback_1d = texture_trait::_1d | texture_trait::readback,
	mipped_readback_1d = texture_trait::_1d | texture_trait::mipped | texture_trait::readback,
	readback_array_1d = texture_trait::_1d | texture_trait::array | texture_trait::readback,
	mipped_readback_array_1d = texture_trait::_1d | texture_trait::array | texture_trait::mipped | texture_trait::readback,

	// "normal" 2D texture.
	default_2d = texture_trait::_2d,
	mipped_2d = texture_trait::_2d | texture_trait::mipped,
	array_2d = texture_trait::_2d | texture_trait::array,
	mipped_array_2d = texture_trait::_2d | texture_trait::array | texture_trait::mipped,
	render_target_2d = texture_trait::_2d | texture_trait::render_target,
	mipped_render_target_2d = texture_trait::_2d | texture_trait::mipped | texture_trait::render_target,
	readback_2d = texture_trait::_2d | texture_trait::readback,
	mipped_readback_2d = texture_trait::_2d | texture_trait::mipped | texture_trait::readback,
	readback_array_2d = texture_trait::_2d | texture_trait::array | texture_trait::readback,
	mipped_readback_array_2d = texture_trait::_2d | texture_trait::array | texture_trait::mipped | texture_trait::readback,

	// a "normal" 2D texture, no mips, configured for unordered access. Currently
	// all GPGPU access to such buffers are one subresource at a time so there is 
	// no spec that describes unordered access to arbitrary mipped memory.
	unordered_2d = texture_trait::_2d | texture_trait::unordered,
	
	// 6- 2D slices that form the faces of a cube that is sampled from its center.
	default_cube = texture_trait::cube,
	mipped_cube = texture_trait::cube | texture_trait::mipped,
	array_cube = texture_trait::cube | texture_trait::array,
	mipped_array_cube = texture_trait::cube | texture_trait::array | texture_trait::mipped,
	render_target_cube = texture_trait::cube | texture_trait::render_target,
	mipped_render_target_cube = texture_trait::cube | texture_trait::mipped | texture_trait::render_target,
	readback_cube = texture_trait::cube | texture_trait::readback,
	mipped_readback_cube = texture_trait::cube | texture_trait::mipped | texture_trait::readback,

	// Series of 2D slices sampled as a volume
	default_3d = texture_trait::_3d,
	mipped_3d = texture_trait::_3d | texture_trait::mipped,
	array_3d = texture_trait::_3d | texture_trait::array,
	mipped_array_3d = texture_trait::_3d | texture_trait::array | texture_trait::mipped,
	render_target_3d = texture_trait::_3d | texture_trait::render_target,
	mipped_render_target_3d = texture_trait::_3d | texture_trait::mipped | texture_trait::render_target,
	readback_3d = texture_trait::_3d | texture_trait::readback,
	mipped_readback_3d = texture_trait::_3d | texture_trait::mipped | texture_trait::readback,
};}

namespace query_type
{ oDECLARE_SMALL_ENUM(value, unsigned char) {

	timer,

	count,

};}

namespace surface_state
{ oDECLARE_SMALL_ENUM(value, unsigned char) {

	// Front-facing is clockwise winding order. Back-facing is counter-clockwise.

	front_face, // Draws all faces whose normal points towards the viewer
	back_face,  // Draws all faces whose normal points away from the viewer
	two_sided, // Draws all faces
	front_wireframe, // Draws the borders of all faces whose normal points towards the viewer
	back_wireframe,  // Draws the borders of all faces whose normal points away from the viewer
	two_sided_wireframe, // Draws the borders of all faces

	count,

};}

namespace depth_stencil_state
{ oDECLARE_SMALL_ENUM(value, unsigned char) {

	// No depth or stencil operation.
	none,

	// normal z-buffering mode where if occluded or same value (<= less_equal 
	// comparison), exit else render and write new Z value. No stencil operation.
	test_and_write,
	
	// test depth only using same method as test-and-write but do not write. No 
	// stencil operation.
	test,
	
	count,

};}

namespace blend_state
{ oDECLARE_SMALL_ENUM(value, unsigned char) {

	// Blend mode math from http://en.wikipedia.org/wiki/Blend_modes

	opaque, // Output.rgba = Source.rgba
	alpha_test, // Same as opaque, test is done in user code
	accumulate, // Output.rgba = Source.rgba + Destination.rgba
	additive, // Output.rgb = Source.rgb * Source.a + Destination.rgb
	multiply, // Output.rgb = Source.rgb * Destination.rgb
	screen, // Output.rgb = Source.rgb * (1 - Destination.rgb) + Destination.rgb (as reduced from webpage's 255 - [((255 - Src)*(255 - Dst))/255])
	translucent, // Output.rgb = Source.rgb * Source.a + Destination.rgb * (1 - Source.a)
	min_, // Output.rgba = min(Source.rgba, Destination.rgba)
	max_, // Output.rgba = max(Source.rgba, Destination.rgba)

	count,

};}

namespace sampler_type
{ oDECLARE_SMALL_ENUM(value, unsigned char) {

	// Use 100% of the texel nearest to the sample point. If the sample is outside 
	// the texture, use an edge texel.
	point_clamp, 
	
	// Use 100% of the texel nearest to the sample point. Use only the fractional 
	// part of the sample point.
	point_wrap,
	
	// Trilinear sample texels around sample point. If the sample is outside the 
	// texture, use an edge texel.
	linear_clamp,
	
	// Trilinear sample texels around sample point. Use only the fractional part 
	// of the sample point.
	linear_wrap,
	
	// Anisotropically sample texels around sample point. If the sample is outside 
	// the texture, use an edge texel.
	aniso_clamp,
	
	// Anisotropically sample texels around sample point. Use only the fractional 
	// part of the sample point.
	aniso_wrap,

	// Same as above, but with a mip bias that evaluates to one mip level higher
	// than that which is ideal for hardware. (sharper, more shimmer)
	point_clamp_bias_up1,
	point_wrap_bias_up1,
	linear_clamp_bias_up1,
	linear_wrap_bias_up1,
	aniso_clamp_bias_up1,
	aniso_wrap_bias_up1,

	// Same as above, but with a mip bias that evaluates to one mip level lower
	// than that which is ideal for hardware. (blurrier, less shimmer)
	point_clamp_bias_down1,
	point_wrap_bias_down1,
	linear_clamp_bias_down1,
	linear_wrap_bias_down1,
	aniso_clamp_bias_down1,
	aniso_wrap_bias_down1,

	// Same as above, but with a mip bias that evaluates to two mip levels higher
	// than that which is ideal for hardware. (shader, more shimmer)
	point_clamp_bias_up2,
	point_wrap_bias_up2,
	linear_clamp_bias_up2,
	linear_wrap_bias_up2,
	aniso_clamp_bias_up2,
	aniso_wrap_bias_up2,

	// Same as above, but with a mip bias that evaluates to two mip levels lower
	// than that which is ideal for hardware. (blurrier, less shimmer)
	point_clamp_bias_down2,
	point_wrap_bias_down2,
	linear_clamp_bias_down2,
	linear_wrap_bias_down2,
	aniso_clamp_bias_down2,
	aniso_wrap_bias_down2,

	count,

};}

namespace pipeline_stage
{ oDECLARE_SMALL_ENUM(value, unsigned char) {
	
	vertex,
	hull,
	domain,
	geometry,
	pixel,

	count,

};}

namespace clear_type
{ oDECLARE_SMALL_ENUM(value, unsigned char) {

	depth,
	stencil,
	depth_stencil,
	color,
	color_depth,
	color_stencil,
	color_depth_stencil,

	count,

};}

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

struct texture_info
{
	texture_info()
		: type(texture_type::default_2d)
		, format(ouro::surface::b8g8r8a8_unorm)
		, dimensions(0, 0, 0)
		, array_size(1)
	{}

	texture_type::value type;
	surface::format format;
	ushort3 dimensions;
	uint array_size;
};

struct basic_pipeline_info
{
	const char* debug_name;
	mesh::layout_array vertex_layouts;
	mesh::primitive_type::value primitive_type;
	const void* vs, *hs, *ds, *gs, *ps;
};

struct pipeline_info : basic_pipeline_info
{
	pipeline_info()
	{
		debug_name = "unnamed pipeline";
		vertex_layouts.fill(mesh::layout::none);
		primitive_type = mesh::primitive_type::unknown;
		vs = nullptr; hs = nullptr; ds = nullptr; gs = nullptr; ps = nullptr;
	}
};

struct basic_compute_kernel
{
	const char* debug_name;
	const void* cs;
};

struct compute_kernel : basic_compute_kernel
{
	compute_kernel()
	{
		debug_name = "unnamed compute kernel";
		cs = nullptr;
	}
};

struct clear_info
{
	clear_info()
		: depth_clear_value(1.0f)
		, stencil_clear_value(0)
	{ clear_color.fill(color(0)); }

	std::array<color, max_num_mrts> clear_color;
	float depth_clear_value;
	uchar stencil_clear_value;
};

struct render_target_info
{
	render_target_info()
		: type(texture_type::render_target_2d)
		, depth_stencil_format(surface::unknown)
		, mrt_count(1)
		, dimensions(0, 0, 0)
		, array_size(1)
	{ format.fill(surface::unknown); }

	texture_type::value type;
	surface::format depth_stencil_format;
	ushort mrt_count;
	ushort3 dimensions;
	uint array_size;
	std::array<surface::format, max_num_mrts> format;
	clear_info clear;
};

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
		, api(api::unknown)
		, vendor(ouro::vendor::unknown)
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
	unsigned long long native_memory;

	// Number of bytes reserved by the system to accommodate data transfer to the 
	// device
	unsigned long long dedicated_system_memory;

	// Number of bytes reserved in system memory used instead of a separate bank 
	// of NativeMemory 
	unsigned long long shared_system_memory;

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
	api::value api;

	// Describes the company that made the device.
	vendor::value vendor;

	// True if the device was created in software emulation mode.
	bool is_software_emulation;

	// True if the device was created with debug reporting enabled.
	bool debug_reporting_enabled;
};

inline bool is_mipped(const texture_type::value& _Type) { return 0 != ((int)_Type & texture_trait::mipped); }
inline bool is_readback(const texture_type::value& _Type) { return 0 != ((int)_Type & texture_trait::readback); }
inline bool is_render_target(const texture_type::value& _Type) { return 0 != ((int)_Type & texture_trait::render_target); }
inline bool is_array(const texture_type::value& _Type) { return 0 != ((int)_Type & texture_trait::array); }
inline bool is_1d(const texture_type::value& _Type) { return 0 != ((int)_Type & texture_trait::_1d); }
inline bool is_2d(const texture_type::value& _Type) { return 0 != ((int)_Type & texture_trait::_2d); }
inline bool is_3d(const texture_type::value& _Type) { return 0 != ((int)_Type & texture_trait::_3d); }
inline bool is_cube(const texture_type::value& _Type) { return 0 != ((int)_Type & texture_trait::cube); }
inline bool is_unordered(const texture_type::value& _Type) { return 0 != ((int)_Type & texture_trait::unordered); }

inline texture_type::value add_readback(const texture_type::value& _Type) { return (texture_type::value)((int)_Type | texture_trait::readback); }
inline texture_type::value add_mipped(const texture_type::value& _Type) { return (texture_type::value)((int)_Type | texture_trait::mipped); }
inline texture_type::value add_render_target(const texture_type::value& _Type) { return (texture_type::value)((int)_Type | texture_trait::render_target); }
inline texture_type::value add_array(const texture_type::value& _Type) { return (texture_type::value)((int)_Type | texture_trait::array); }
inline texture_type::value get_basic(const texture_type::value& _Type) { return (texture_type::value)((int)_Type & (texture_trait::_1d|texture_trait::_2d|texture_trait::_3d|texture_trait::cube)); }

inline texture_type::value remove_readback(const texture_type::value& _Type) { return (texture_type::value)((int)_Type & ~texture_trait::readback); }
inline texture_type::value remove_mipped(const texture_type::value& _Type) { return (texture_type::value)((int)_Type & ~texture_trait::mipped); }
inline texture_type::value remove_render_target(const texture_type::value& _Type) { return (texture_type::value)((int)_Type & ~texture_trait::render_target); }

// Returns a buffer info that will auto-size to 16-bit indices or 32-bit indices depending
// on the number of vertices.
buffer_info make_index_buffer_info(uint _NumIndices, uint _NumVertices);

// Returns a buffer info for the vertex buffer with the specified usage and layout. Only vertex traits matching
// the specified layout will be described by the buffer info,
buffer_info make_vertex_buffer_info(uint _NumVertices, const mesh::layout::value& _Layout);

	} // namespace gpu
} // namespace ouro

#endif
