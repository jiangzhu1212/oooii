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
#include <oBase/surface_buffer.h>
#include <oBase/surface_convert.h>
#include <oBase/memory.h>
#include <oStd/mutex.h>

namespace ouro {
	namespace surface {

class buffer_impl : public buffer
{
public:
	buffer_impl(const surface::info& _SurfaceInfo, void* _pData); // _pData will be freed in ~buffer_impl()
	buffer_impl(const surface::info& _SurfaceInfo);
	~buffer_impl();

	info get_info() const override;
	void update_subresource(int _Subresource, const const_mapped_subresource& _Source, bool _FlipVertically = false) override;
	void update_subresource(int _Subresource, const box& _Box, const const_mapped_subresource& _Source, bool _FlipVertically = false) override;
	void map(int _Subresource, mapped_subresource* _pMapped, int2* _pByteDimensions = nullptr) override;
	void unmap(int _Subresource) override;
	void map_const(int _Subresource, const_mapped_subresource* _pMapped, int2* _pByteDimensions = nullptr) const override;
	void unmap_const(int _Subresource) const override;
	void copy_to(int _Subresource, mapped_subresource* _pMapped, bool _FlipVertically = false) const override;
	std::shared_ptr<buffer> convert(const info& _ConvertedInfo) const override;
	void swizzle(format _NewFormat) override;
private:
	void* Data;
	info Info;
	oStd::shared_mutex Mutex; // todo: separate locking mechanism to be per-subresource
};

buffer_impl::buffer_impl(const surface::info& _SurfaceInfo, void* _pData)
	: Data(_pData)
	, Info(_SurfaceInfo)
{}

// use c-style malloc/free to be compatible with many file format c libraries
buffer_impl::buffer_impl(const surface::info& _SurfaceInfo)
	: Data(malloc(total_size(_SurfaceInfo)))
	, Info(_SurfaceInfo)
{}

buffer_impl::~buffer_impl()
{
	if (Data)
		free(Data);
}

std::shared_ptr<buffer> buffer::make(const info& _Info)
{
	return std::move(std::make_shared<buffer_impl>(_Info));
}

std::shared_ptr<buffer> buffer::make(const info& _Info, void* _pData)
{
	return std::move(std::make_shared<buffer_impl>(_Info, _pData));
}

info buffer_impl::get_info() const
{
	return Info;
}

void buffer_impl::update_subresource(int _Subresource, const const_mapped_subresource& _Source, bool _FlipVertically)
{
	int2 ByteDimensions;
	mapped_subresource Dest = get_mapped_subresource(Info, _Subresource, 0, Data, &ByteDimensions);
	oStd::lock_guard<oStd::shared_mutex> lock(Mutex);
	memcpy2d(Dest.data, Dest.row_pitch, _Source.data, _Source.row_pitch, ByteDimensions.x, ByteDimensions.y, _FlipVertically);
}

void buffer_impl::update_subresource(int _Subresource, const box& _Box, const const_mapped_subresource& _Source, bool _FlipVertically)
{
	if (is_block_compressed(Info.format) || Info.format == r1_unorm)
		throw std::invalid_argument("block compressed and bit formats not supported");

	int2 ByteDimensions;
	mapped_subresource Dest = get_mapped_subresource(Info, _Subresource, 0, Data, &ByteDimensions);

	const int NumRows = _Box.height();
	int PixelSize = element_size(Info.format);
	int RowSize = PixelSize * _Box.width();

	// Dest points at start of subresource, so offset to subrect of first slice
	Dest.data = byte_add(Dest.data, (_Box.top * Dest.row_pitch) + _Box.left * PixelSize);

	const void* pSource = _Source.data;

	oStd::lock_guard<oStd::shared_mutex> lock(Mutex);
	for (uint slice = _Box.front; slice < _Box.back; slice++)
	{
		memcpy2d(Dest.data, Dest.row_pitch, pSource, _Source.row_pitch, RowSize, NumRows, _FlipVertically);
		Dest.data = byte_add(Dest.data, Dest.depth_pitch);
		pSource = byte_add(pSource, _Source.depth_pitch);
	}
}

void buffer_impl::map(int _Subresource, mapped_subresource* _pMapped, int2* _pByteDimensions)
{
	Mutex.lock();
	*_pMapped = get_mapped_subresource(Info, _Subresource, 0, Data, _pByteDimensions);
}

void buffer_impl::unmap(int _Subresource)
{
	Mutex.unlock();
}

void buffer_impl::map_const(int _Subresource, const_mapped_subresource* _pMapped, int2* _pByteDimensions) const
{
	const_cast<oStd::shared_mutex&>(Mutex).lock_shared();
	*_pMapped = get_const_mapped_subresource(Info, _Subresource, 0, Data, _pByteDimensions);
}

void buffer_impl::unmap_const(int _Subresource) const
{
	const_cast<oStd::shared_mutex&>(Mutex).unlock_shared();
}

void buffer_impl::copy_to(int _Subresource, mapped_subresource* _pMapped, bool _FlipVertically) const
{
	int2 ByteDimensions;
	const_mapped_subresource Source = get_const_mapped_subresource(Info, _Subresource, 0, Data, &ByteDimensions);
	oStd::shared_lock<oStd::shared_mutex> lock(const_cast<oStd::shared_mutex&>(Mutex));
	memcpy2d(_pMapped->data, _pMapped->row_pitch, Source.data, Source.row_pitch, ByteDimensions.x, ByteDimensions.y, _FlipVertically);
}

std::shared_ptr<buffer> buffer_impl::convert(const info& _ConvertedInfo) const
{
	info SourceInfo = get_info();
	std::shared_ptr<buffer> converted = buffer::make(_ConvertedInfo);
	shared_lock slock(this);
	lock_guard dlock(converted.get());
	surface::convert(SourceInfo, slock.mapped, _ConvertedInfo, &dlock.mapped);
	return converted;
}

void buffer_impl::swizzle(format _NewFormat)
{
	lock_guard lock(this);
	convert_swizzle(Info, _NewFormat, &lock.mapped);
	Info.format = _NewFormat;
}

	} // namespace surface
} // namespace ouro
