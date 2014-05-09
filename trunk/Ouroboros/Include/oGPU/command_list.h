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
#ifndef oGPU_command_list_h
#define oGPU_command_list_h

// this needs to know about:
// resource buffer texture
// render_target
// clear_type
// surface.h
// mesh::primitive_type
// rasterizer_state
// blend_state
// depth_stencil_state
// sampler_state
// shaders
// aaboxf (viewport)

#include <oMesh/mesh.h>
#include <oSurface/surface.h>

namespace ouro {
	namespace gpu {

class resource;
class render_target;
class vertex_layout;
class vertex_shader;
class hull_shader;
class domain_shader;
class geometry_shader;
class pixel_shader;
class compute_shader;

class command_list
{
public:

	// return -1 for the immediate context, else this returns the order the command list
	// will execute on the device.
	int get_draw_order() const;

	// all rendering api calls for this command list should occur between begin() and end()
	void begin();
	void end();

	// immediately send the command queue to the GPU
	void flush();

	// resets default command list state
	void reset();

	// Allocates internal device memory that can be written to (not read) and 
	// committed to the device to update the specified resource.
	surface::mapped_subresource reserve(resource* r, int subresource);

	// Commits memory to the specified resource. If the memory in _Source. 
	// was reserved, then this will free the memory. If _Source.data is user 
	// memory, it will not be freed. If the specified rectangle is empty on any 
	// dimension, then the entire surface will be copied (default behavior). If 
	// the item is a buffer then units are in structs, i.e. Left=0, Right=ArraySize 
	// would be a full copy. Ensure that the other dimension are not empty/equal 
	// even in the buffer case.
	void commit(resource* r, int subresource, const surface::mapped_subresource& source, const surface::box& subregion = surface::box());
	inline void commit(resource* r, int subresource, const surface::const_mapped_subresource& source, const surface::box& subregion = surface::box()) { commit(_resource, subresource, (const surface::mapped_subresource&)source, subregion); }
	
	// Copies the contents from one resource to another. Both must have compatible 
	// (often identical) topologies. A common use of this API is to copy from a 
	// source resource to a readback copy of the same resource so it can be 
	// accessed from the CPU.
	void copy(resource* destination, resource* source);

	// Copies from one buffer to another with offsets in bytes.
	void copy(resource* destination, uint destination_offset_bytes, const resource* source, uint source_offset_bytes, uint size_bytes);

	// _____________________________________________________________________________
	// Resource usage (valid for compute and graphics apis)

	// Set constant buffers for all stages
	void set_buffers(uint start, uint num_buffers, const buffer* const* buffers);
	inline void set_buffer(uint start, const buffer* b) { set_buffers(start, 1, &b); }

	// sets non-constant read-only buffer for textures for all stages
	// these should not be bound as a render target or for unordered access
	void set_resources(uint start, uint num_resources, const resource* resources);
	inline void set_resource(uint start, const resource* r) { set_resources(start, 1, &_resource); }

	// _____________________________________________________________________________
	// Graphics

	// Sets the render target to which rendering will occur. By default a single
	// full-target viewport is created else it can be overridden. A viewport is 
	// a 3D box whose minimum is at the top, left, near corner of the viewable 
	// frustum and whose maximum is at the bottom, right, far corner of the 
	// viewable frustum. A full-target viewport would most often be: 
	// aaboxf(float3(0.0f), float3(RTWidth, RTHeight, 1.0f)). 
	inline void set_rt(render_target* rt, uint num_viewports = 0, const aaboxf* viewports = nullptr) { set_rtut(_render_target, num_viewports, viewports, 0, 0, (resource*)nullptr); }
	void clear(render_target* rt, const clear_type::value& clear);
	
	// Generates mips on the GPU from the top-level mip target
	void generate_mips(render_target* rt);

	// set fixed-function state
	void set_prim(const mesh::primitive_type& type);
	void set_rs(const rasterizer_state::value& state);
	void set_bs(const blend_state::value& state);
	void set_dss(const depth_stencil_state::value& state);

	// Set the texture sampler states for all shaders
	void set_samplers(uint start, uint num_states, const sampler_state::value* states);
	inline void set_sampler(uint start, const sampler_state::value& state) { return set_samplers(start, 1, &state); }
	
	// sets the vertex layout that must match the input into the set vertex shader
	void set_vertex_layout(vertex_layout* layout);

	// set shader programs to appropriate slots
	void set_vs(vertex_shader* vs);
	void set_hs(hull_shader* hs);
	void set_ds(domain_shader* ds);
	void set_gs(geometry_shader* gs);
	void set_ps(pixel_shader* ps);

	// submits the specified geometry for drawing under the current state. Start
	// vertex is an offset into the bound buffer where indexing begins (i.e. where
	// vertex 0 is). Then start_prim/num_prims will use indices to form prims as
	// set in the current state. If num_instances is not zero, then instanced 
	// drawing will be used. Use SV_InstanceID, not a vertex stream to utilize
	// instancing.
	void draw(uint vertex_byte_offset, uint num_vertex_buffers, const buffer* vertices
		, uint start_prim, uint num_prim, const buffer* indices, uint num_instances = 0);
	
	// use the specified buffer for drawing parameters
	void draw(const buffer* draw_args, uint args_offset_uint_aligned);

	
	// _____________________________________________________________________________
	// Compute
	
	// sets writable unordered targets for computation
	void set_uts(uint start, uint num_unordered_targets, resource* unordered_targets);

	// render targets and unordered targets share the same register namespace, so
	// this API atomically sets both types of objects. Unordered resources start
	// at the index after the last MRT element of the render target is bound.
	void set_rtut(render_target* rt, uint num_viewports, const aaboxf* viewports, uint num_unordered_resources, resource* unordered_resources, uint* initial_counts = nullptr);
	inline void clear_rtut() { set_rtut(nullptr, 0, nullptr, 0, max_num_unordered_buffers, (resource**)nullptr); }
	
	// clears an unordered resource
	void cleari(resource* unordered_resource, const uint4& values);
	void clearf(resource* unordered_resource, const float4& values);

	// If all 3 values in _ThreadGroupCount are invalid, then the counts are 
	// automatically determined by the GPU. Unordered resources can be 
	// buffer or oGPUTexture, but each must have been created readied
	// for unordered access. If not null, _pInitialCounts should be 
	// _NumUnorderedResources in length and is used to set the initial value of a 
	// counter or append/consume count for buffers of type 
	// buffer_type::unordered_structured_append or 
	// buffer_type::unordered_structured_counter. Specify invalid to skip 
	// initialization of an entry, thus retaining any prior value of the counter.
	void dispatch(compute_shader* shader, const int3& dispatch_thread_count);

	// Same as dispatch above, but it gets its thread group count from buffer.
	void dispatch(compute_shader* shader, buffer dispatch_thread_count, uint offset_uint_aligned);

	// Copy the counter value stored in a source buffer of type 
	// buffer_type::unordered_structured_append or buffer_type::unordered_structured_counter 
	// to a destination buffer. An offset into that buffer can be specified - it 
	// must be aligned to sizeof(uint). To read back this value to the CPU 
	// destination should be a readback buffer of at least sizeof(uint) size. Then 
	// use map_read() on the device to access the uint counter value.
	void copy_counter(resource* destination, uint destination_offset_uint_aligned, buffer* unordered_source);

	// Sets the counter in the specified buffer to the specified value. This 
	// incurs a dispatch and should not be used in the main loop of production 
	// code. This is exposed primarily for test cases and initialization. For main 
	// loop code use the initial_counts parameter of set_rtut or 
	// set_rtut. REMEMBER: This will occur when the 
	// command list is submitted to the device during end_frame(). If the desire is
	// immediate ensure this command list is the one retrieved from 
	// device::get_immediate_command_list().
	void set_counters(uint num_unordered_resources, resource* unordered_resources, uint* initial_counts);
	inline void set_counters(uint num_unordered_resources, buffer** unordered_resources, uint* initial_counts) { set_counters(num_unordered_resources, (resource**)unordered_resources, _pValues); }
	inline void set_counters(resource* r, uint _Value) { set_counters(1, &r, &_Value); }
};

	} // namespace gpu
} // namespace ouro

#endif
