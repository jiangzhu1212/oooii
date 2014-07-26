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
#include <oSurface/texel_buffer.h>
#include <oSurface/convert.h>
#include <oBase/memory.h>
#include <oBase/throw.h>
#include <mutex>

namespace ouro { namespace surface {

texel_buffer::texel_buffer(texel_buffer&& that) 
	: bits(that.bits)
	, inf(that.inf)
	, alloc(that.alloc)
{
	that.bits = nullptr;
	that.inf = info();
	that.alloc = allocator();
}

texel_buffer& texel_buffer::operator=(texel_buffer&& that)
{
	if (this != &that)
	{
		mtx.lock();
		that.mtx.lock();
		deinitialize();
		bits = that.bits; that.bits = nullptr; 
		inf = that.inf; that.inf = info();
		alloc = that.alloc; that.alloc = allocator();
		that.mtx.unlock();
		mtx.unlock();
	}
	return *this;
}

void texel_buffer::initialize(const info& i, const allocator& a)
{
	deinitialize();
	inf = i;
	alloc = a;
	bits = alloc.allocate(size(), 0);
}

void texel_buffer::initialize(const info& i, const void* data, const allocator& a)
{
	deinitialize();
	inf = i;
	alloc = a;
	bits = (void*)data;
}

void texel_buffer::initialize_array(const texel_buffer* const* sources, uint num_sources, bool mips)
{
	deinitialize();
	info si = sources[0]->get_info();
	oCHECK_ARG(si.layout == layout::image, "all images in the specified array must be simple types and the same 2D dimensions");
	oCHECK_ARG(si.dimensions.z == 1, "all images in the specified array must be simple types and the same 2D dimensions");
	si.layout = mips ? layout::tight : layout::image;
	si.array_size = static_cast<int>(num_sources);
	initialize(si);

	const uint nMips = num_mips(mips, si.dimensions);
	const uint nSlices = max(1, si.array_size);
	for (uint i = 0; i < nSlices; i++)
	{
		int dst = calc_subresource(0, i, 0, nMips, nSlices);
		int src = calc_subresource(0, 0, 0, nMips, 0);
		copy_from(dst, *sources[i], src);
	}

	if (mips)
		generate_mips();
}

void texel_buffer::initialize_3d(const texel_buffer* const* sources, uint num_sources, bool mips)
{
	deinitialize();
	info si = sources[0]->get_info();
	oCHECK_ARG(si.layout == layout::image, "all images in the specified array must be simple types and the same 2D dimensions");
	oCHECK_ARG(si.dimensions.z == 1, "all images in the specified array must be simple types and the same 2D dimensions");
	oCHECK_ARG(si.array_size == 0, "arrays of 3d surfaces not yet supported");
	si.layout = mips ? layout::tight : layout::image;
	si.dimensions.z = static_cast<int>(num_sources);
	si.array_size = 0;
	initialize(si);

	box region;
	region.right = si.dimensions.x;
	region.bottom = si.dimensions.y;
	for (int i = 0; i < si.dimensions.z; i++)
	{
		region.front = i;
		region.back = i + 1;
		shared_lock lock(sources[i]);
		update_subresource(0, region, lock.mapped);
	}

	if (mips)
		generate_mips();
}

void texel_buffer::deinitialize()
{
	if (bits && alloc.deallocate)
		alloc.deallocate(bits);
	bits = nullptr;
	alloc = allocator();
}

void texel_buffer::clear()
{
	lock_t lock(mtx);
	memset(bits, 0, size());
}

void texel_buffer::flatten()
{
	if (is_block_compressed(inf.format))
		oTHROW(not_supported, "block compressed formats not handled yet");

	int rp = row_pitch(inf);
	size_t sz = size();
	inf.layout = surface::image;
	inf.dimensions = int3(rp / element_size(inf.format), int(sz / rp), 1);
	inf.array_size = 0;
}

void texel_buffer::update_subresource(uint subresource, const const_mapped_subresource& src, const copy_option& option)
{
	int2 bd;
	mapped_subresource dst = get_mapped_subresource(inf, subresource, 0, bits, &bd);
	lock_t lock(mtx);
	memcpy2d(dst.data, dst.row_pitch, src.data, src.row_pitch, bd.x, bd.y, option == copy_option::flip_vertically);
}

void texel_buffer::update_subresource(uint subresource, const box& _box, const const_mapped_subresource& src, const copy_option& option)
{
	if (is_block_compressed(inf.format) || inf.format == r1_unorm)
		throw std::invalid_argument("block compressed and bit formats not supported");

	int2 bd;
	mapped_subresource Dest = get_mapped_subresource(inf, subresource, 0, bits, &bd);

	const int NumRows = _box.height();
	int PixelSize = element_size(inf.format);
	int RowSize = PixelSize * _box.width();

	// Dest points at start of subresource, so offset to subrect of first slice
	Dest.data = byte_add(Dest.data, _box.front * Dest.depth_pitch + _box.top * Dest.row_pitch + _box.left * PixelSize);

	const void* pSource = src.data;

	lock_t lock(mtx);
	for (uint slice = _box.front; slice < _box.back; slice++)
	{
		memcpy2d(Dest.data, Dest.row_pitch, pSource, src.row_pitch, RowSize, NumRows, option == copy_option::flip_vertically);
		Dest.data = byte_add(Dest.data, Dest.depth_pitch);
		pSource = byte_add(pSource, src.depth_pitch);
	}
}

void texel_buffer::map(uint subresource, mapped_subresource* _pMapped, int2* _pByteDimensions)
{
	mtx.lock();
	*_pMapped = get_mapped_subresource(inf, subresource, 0, bits, _pByteDimensions);
}

void texel_buffer::unmap(uint subresource)
{
	mtx.unlock();
}

void texel_buffer::map_const(uint subresource, const_mapped_subresource* _pMapped, int2* _pByteDimensions) const
{
	lock_shared();
	*_pMapped = get_const_mapped_subresource(inf, subresource, 0, bits, _pByteDimensions);
}

void texel_buffer::unmap_const(uint subresource) const
{
	unlock_shared();
}

void texel_buffer::copy_to(uint subresource, const mapped_subresource& dst, const copy_option& option) const
{
	int2 bd;
	const_mapped_subresource src = get_const_mapped_subresource(inf, subresource, 0, bits, &bd);
	lock_shared_t lock(mtx);
	memcpy2d(dst.data, dst.row_pitch, src.data, src.row_pitch, bd.x, bd.y, option == copy_option::flip_vertically);
}

texel_buffer texel_buffer::convert(const info& dst_info) const
{
	return convert(dst_info, alloc);
}

texel_buffer texel_buffer::convert(const info& dst_info, const allocator& a) const
{
	info src_info = get_info();
	texel_buffer converted(dst_info);
	shared_lock slock(this);
	lock_guard dlock(converted);
	surface::convert(src_info, slock.mapped, dst_info, dlock.mapped);
	return converted;
}

void texel_buffer::convert_to(uint subresource, const mapped_subresource& dst, const format& dst_format, const copy_option& option) const
{
	if (inf.format == dst_format)
		copy_to(subresource, dst, option);
	else
	{
		shared_lock slock(this, subresource);
		info src_info = get_info();
		info dst_info = src_info;
		dst_info.format = dst_format;
		surface::convert(src_info, slock.mapped, dst_info, (mapped_subresource&)dst);
	}
}

void texel_buffer::convert_from(uint subresource, const const_mapped_subresource& src, const format& src_format, const copy_option& option)
{
	if (inf.format == src_format)
		copy_from(subresource, src, option);
	else
	{
		info src_info = inf;
		src_info.format = src_format;
		subresource_info sri = surface::subresource(src_info, subresource);
		lock_guard lock(this);
		convert_subresource(sri, src, inf.format, lock.mapped, option);
	}
}

void texel_buffer::convert_in_place(const format& fmt)
{
	lock_guard lock(this);
	convert_swizzle(inf, fmt, lock.mapped);
	inf.format = fmt;
}

void texel_buffer::generate_mips(const filter& f)
{
	lock_t lock(mtx);

	int nMips = num_mips(inf);
	int nSlices = max(1, inf.array_size);

	for (int slice = 0; slice < nSlices; slice++)
	{
		int mip0subresource = calc_subresource(0, slice, 0, nMips, inf.array_size);
		const_mapped_subresource mip0 = get_const_mapped_subresource(inf, mip0subresource, 0, bits);

		for (int mip = 1; mip < nMips; mip++)
		{
			uint subresource = calc_subresource(mip, slice, 0, nMips, inf.array_size);
			subresource_info subinfo = surface::subresource(inf, subresource);

			for (int DepthIndex = 0; DepthIndex < subinfo.dimensions.z; DepthIndex++)
			{
				mapped_subresource dst = get_mapped_subresource(inf, subresource, DepthIndex, bits);
				info di = inf;
				di.dimensions = subinfo.dimensions;
				resize(inf, mip0, di, dst, f);
			}
		}
	}
}

float calc_rms(const texel_buffer& b1, const texel_buffer& b2)
{
	return calc_rms(b1, b2, nullptr);
}

float calc_rms(const texel_buffer& b1, const texel_buffer& b2, texel_buffer* out_diffs, int diff_scale, const allocator& a)
{
	info si1 = b1.get_info();
	info si2 = b2.get_info();

	if (any(si1.dimensions != si2.dimensions)) throw std::invalid_argument("mismatched dimensions");
	if (si1.format != si2.format) throw std::invalid_argument("mismatched format");
	if (si1.array_size != si2.array_size) throw std::invalid_argument("mismatched array_size");
	int n1 = num_subresources(si1);
	int n2 = num_subresources(si2);
	if (n1 != n2) throw std::invalid_argument("incompatible layouts");

	info dsi;
	if (out_diffs)
	{
		dsi = si1;
		dsi.format = r8_unorm;
		out_diffs->initialize(dsi);
	}

	float rms = 0.0f;
	for (int i = 0; i < n1; i++)
	{
		mapped_subresource msr;
		if (out_diffs)
			out_diffs->map(i, &msr);

		shared_lock lock1(b1, i);
		shared_lock lock2(b2, i);
	
		if (out_diffs)
			rms += calc_rms(si1, lock1.mapped, lock2.mapped, dsi, msr);
		else
			rms += calc_rms(si1, lock1.mapped, lock2.mapped);

		if (out_diffs)
			out_diffs->unmap(i);
	}

	return rms / static_cast<float>(n1);
}

	} // namespace surface
} // namespace ouro
