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
// Uses surface utility functions to manage a buffer filled with all that a 
// surface can support. This is basically a CPU-side version of similar GPU 
// buffers in D3D and OGL.
#pragma once
#ifndef oSurface_image_h
#define oSurface_image_h

#include <oBase/allocate.h>
#include <oCore/mutex.h>
#include <oSurface/surface.h>
#include <oSurface/resize.h>
#include <memory>

namespace ouro { namespace surface {
	
class image
{
public:
	image() : bits(nullptr) {}
	image(const info& i, const allocator& a = default_allocator) { initialize(i, a); }
	image(const info& i, const void* data, const allocator& a = noop_allocator) { initialize(i, data, a); }

	~image() { deinitialize(); }

	image(image&& that);
	image& operator=(image&& that);

	// create a buffer with uninitialized bits
	void initialize(const info& i, const allocator& a = default_allocator);
	
	// create a buffer using the specified pointer. the specified allocator will be used
	// to manage its lifetime.
	void initialize(const info& i, const void* data, const allocator& a = noop_allocator);

	// create an array buffer out of several subbuffers of the same format
	void initialize_array(const image* const* sources, uint num_sources, bool mips = false);
	template<size_t N> void initialize_array(const image* const (&sources)[N], bool mips = false) { initialize_array(sources, N, mips); }

	// creates a 3d surface out of several subbuffers of the same format
	void initialize_3d(const image* const* texel_sources, uint num_sources, bool mips = false);
	template<size_t N> void initialize_3d(const image* const (&sources)[N], bool mips = false) { initialize_3d(sources, N, mips); }

	void deinitialize();

	operator bool() const { return !!bits; }
	inline bool immutable() const { return !!bits && !alloc; }

	inline info get_info() const { return inf; }
	inline void set_semantic(const semantic& s) { inf.semantic = s; }

	// returns the size of the bit data: all subresources and padding, not including the info
	inline size_t size() const { return total_size(get_info()); }

	// Sets all subresources to 0
	void clear();

	// Without modifying the data this updates the info to be an image layout with 
	// array_size of 0. This is useful for saving the buffer to a files as the entire
	// surface is laid out.
	void flatten();

	// copies the specified src of the same format and dimensions into a subresource in the current instance
	void update_subresource(uint subresource, const const_mapped_subresource& src, const copy_option& option = copy_option::none);
	void update_subresource(uint subresource, const box& b, const const_mapped_subresource& src, const copy_option& option = copy_option::none);

	// locks internal memory for read/write and returns parameters for working with it
	void map(uint subresource, mapped_subresource* out_mapped, uint2* out_byte_dimensions = nullptr);
	void unmap(uint subresource);

	// locks internal memory for read-only and returns parameters for working with it
	void map_const(uint subresource, const_mapped_subresource* out_mapped, uint2* out_byte_dimensions = nullptr) const;
	void unmap_const(uint subresource) const;

	// copies from a subresource in this instance to a mapped destination of the same format and dimensions 
	void copy_to(uint subresource, const mapped_subresource& dst, const copy_option& option = copy_option::none) const;
	inline void copy_from(uint subresource, const const_mapped_subresource& src, const copy_option& option = copy_option::none) { update_subresource(subresource, src, option); }
	inline void copy_from(uint subresource, const image& src, uint src_subresource, const copy_option& option = copy_option::none);

	// initializes a resized and reformatted copy of this buffer allocated from the same or a user-specified allocator
	image convert(const info& dst_info) const;
	image convert(const info& dst_info, const allocator& a) const;

	// initializes a reformatted copy of this buffer allocated from the same or a user-specified allocator
	inline image convert(const format& dst_format) const { info si = get_info(); si.format = dst_format; return convert(si); }
	inline image convert(const format& dst_format, const allocator& a) const { info si = get_info(); si.format = dst_format; return convert(si, a); }

	// copies to a mapped subresource of the same dimension but the specified format
	void convert_to(uint subresource, const mapped_subresource& dst, const format& dst_format, const copy_option& option = copy_option::none) const;

	// copies into this instance from a source of the same dimension but a different format
	void convert_from(uint subresource, const const_mapped_subresource& src, const format& src_format, const copy_option& option = copy_option::none);

	// For compatible types such as RGB <-> BGR do conversion in-place
	void convert_in_place(const format& fmt);

	// Uses the top-level mip as a source and replaces all other mips with a filtered version
	void generate_mips(const filter& f = filter::lanczos2);

private:
	void* bits;
	allocator alloc;
	info inf;
	
	typedef ouro::shared_mutex mutex_t;
	typedef ouro::lock_guard<mutex_t> lock_t;
	typedef ouro::shared_lock<mutex_t> lock_shared_t;
	
	mutable mutex_t mtx;
	inline void lock_shared() const { mtx.lock_shared(); }
	inline void unlock_shared() const { mtx.unlock_shared(); }

	image(const image&);
	const image& operator=(const image&);
};

class lock_guard
{
public:
	lock_guard(image& b, uint subresource = 0)
		: buf(&b)
		, subresource(subresource)
	{ buf->map(subresource, &mapped, &byte_dimensions); }

	lock_guard(image* b, uint subresource = 0)
		: buf(b)
		, subresource(subresource)
	{ buf->map(subresource, &mapped, &byte_dimensions); }

	~lock_guard() { buf->unmap(subresource); }

	mapped_subresource mapped;
	uint2 byte_dimensions;

private:
	image* buf;
	uint subresource;

	lock_guard(const lock_guard&);
	const lock_guard& operator=(const lock_guard&);
};

class shared_lock
{
public:
	shared_lock(image& b, uint subresource = 0)
		: buf(&b)
		, subresource(subresource)
	{ buf->map_const(subresource, &mapped, &byte_dimensions); }

	shared_lock(image* b, uint subresource = 0)
		: buf(b)
		, subresource(subresource)
	{ buf->map_const(subresource, &mapped, &byte_dimensions); }

	shared_lock(const image* b, uint subresource = 0)
		: buf(b)
		, subresource(subresource)
	{ buf->map_const(subresource, &mapped, &byte_dimensions); }

	shared_lock(const image& b, uint subresource = 0)
		: buf(&b)
		, subresource(subresource)
	{ buf->map_const(subresource, &mapped, &byte_dimensions); }

	~shared_lock() { buf->unmap_const(subresource); }

	const_mapped_subresource mapped;
	uint2 byte_dimensions;

private:
	uint subresource;
	const image* buf;

	shared_lock(const shared_lock&);
	const shared_lock& operator=(const shared_lock&);
};

inline void image::copy_from(uint subresource, const image& src, uint src_subresource, const copy_option& option)
{
	shared_lock locked(src, src_subresource);
	copy_from(subresource, locked.mapped, option);
}

// returns the root mean square of the difference between the two surfaces. If
// the formats or sizes are different, this throws an exception. If out_diffs
// is passed in, it will be initialized using the specified allocator. The rms
// grayscale color will be multiplied by diff_scale.
float calc_rms(const image& b1, const image& b2);
float calc_rms(const image& b1, const image& b2, image* out_diffs, int diff_scale = 1, const allocator& a = default_allocator);

}}

#endif
