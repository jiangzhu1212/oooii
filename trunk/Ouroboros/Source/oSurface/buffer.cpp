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
#include <oSurface/buffer.h>
#include <oSurface/convert.h>
#include <oBase/memory.h>
#include <oBase/throw.h>
#include <mutex>

namespace ouro {
	namespace surface {

class buffer_impl : public buffer
{
public:
	buffer_impl(const surface::info& _SurfaceInfo, void* _pData); // _pData will be freed in ~buffer_impl()
	buffer_impl(const surface::info& _SurfaceInfo);
	~buffer_impl();

	info get_info() const override;
	void clear() override;
	void flatten() override;
	void update_subresource(int _Subresource, const const_mapped_subresource& _Source, bool _FlipVertically = false) override;
	void update_subresource(int _Subresource, const box& _Box, const const_mapped_subresource& _Source, bool _FlipVertically = false) override;
	void map(int _Subresource, mapped_subresource* _pMapped, int2* _pByteDimensions = nullptr) override;
	void unmap(int _Subresource) override;
	void map_const(int _Subresource, const_mapped_subresource* _pMapped, int2* _pByteDimensions = nullptr) const override;
	void unmap_const(int _Subresource) const override;
	void copy_to(int _Subresource, mapped_subresource* _pMapped, bool _FlipVertically = false) const override;
	std::shared_ptr<buffer> convert(const info& _ConvertedInfo) const override;
	void swizzle(format _NewFormat) override;
	void generate_mips(filter::value _Filter = filter::lanczos2) override;
private:
	void* Data;
	info Info;
	shared_mutex Mutex; // todo: separate locking mechanism to be per-subresource
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
	return std::make_shared<buffer_impl>(_Info);
}

std::shared_ptr<buffer> buffer::make(const info& _Info, void* _pData)
{
	return std::make_shared<buffer_impl>(_Info, _pData);
}

info buffer_impl::get_info() const
{
	return Info;
}

void buffer_impl::clear()
{
	ouro::lock_guard<shared_mutex> lock(Mutex);
	memset(Data, 0, total_size(Info));
}

void buffer_impl::flatten()
{
	if (is_block_compressed(Info.format))
		oTHROW(not_supported, "block compressed formats not handled yet");

	int rp = row_pitch(Info);
	size_t sz = size();
	Info.layout = image;
	Info.dimensions = int3(rp / element_size(Info.format), int(sz / rp), 1);
	Info.array_size = 0;
}

void buffer_impl::update_subresource(int _Subresource, const const_mapped_subresource& _Source, bool _FlipVertically)
{
	int2 ByteDimensions;
	mapped_subresource Dest = get_mapped_subresource(Info, _Subresource, 0, Data, &ByteDimensions);
	std::lock_guard<ouro::shared_mutex> lock(Mutex);
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

	std::lock_guard<ouro::shared_mutex> lock(Mutex);
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
	const_cast<shared_mutex&>(Mutex).lock_shared();
	*_pMapped = get_const_mapped_subresource(Info, _Subresource, 0, Data, _pByteDimensions);
}

void buffer_impl::unmap_const(int _Subresource) const
{
	const_cast<shared_mutex&>(Mutex).unlock_shared();
}

void buffer_impl::copy_to(int _Subresource, mapped_subresource* _pMapped, bool _FlipVertically) const
{
	int2 ByteDimensions;
	const_mapped_subresource Source = get_const_mapped_subresource(Info, _Subresource, 0, Data, &ByteDimensions);
	ouro::shared_lock<shared_mutex> lock(const_cast<shared_mutex&>(Mutex));
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

void buffer_impl::generate_mips(filter::value _Filter)
{
	std::lock_guard<ouro::shared_mutex> lock(Mutex);

	int nMips = num_mips(Info);

	for (int slice = 0; slice < Info.array_size; slice++)
	{
		int mip0subresource = calc_subresource(0, slice, 0, nMips, Info.array_size);
		const_mapped_subresource mip0 = get_const_mapped_subresource(Info, mip0subresource, 0, Data);

		for (int mip = 1; mip < nMips; mip++)
		{
			int subresource = calc_subresource(mip, slice, 0, nMips, Info.array_size);
			mapped_subresource dst = get_mapped_subresource(Info, subresource, 0, Data);
			subresource_info subinfo = surface::subresource(Info, subresource);

			info di = Info;
			di.dimensions = subinfo.dimensions;
			resize(Info, mip0, di, &dst, _Filter);
		}
	}
}

float calc_rms(const buffer* _pBuffer1, const buffer* _pBuffer2, buffer* _pDifferences, int _DifferenceScale)
{
	info si1 = _pBuffer1->get_info();
	info si2 = _pBuffer2->get_info();
	info dsi;
	if (_pDifferences)
		dsi = _pDifferences->get_info();

	if (any(si1.dimensions != si2.dimensions) || (_pDifferences && any(si1.dimensions != dsi.dimensions))) throw std::invalid_argument("mismatched dimensions");
	if (si1.format != si2.format) throw std::invalid_argument("mismatched format");
	if (si1.array_size != si2.array_size) throw std::invalid_argument("mismatched array_size");
	int n1 = num_subresources(si1);
	int n2 = num_subresources(si2);
	if (n1 != n2) throw std::invalid_argument("incompatible layouts");

	float rms = 0.0f;
	for (int i = 0; i < n1; i++)
	{
		mapped_subresource msr;
		if (_pDifferences)
		_pDifferences->map(i, &msr);

		shared_lock lock1(_pBuffer1, i);
		shared_lock lock2(_pBuffer2, i);
	
		if (_pDifferences)
			rms += calc_rms(si1, lock1.mapped, lock2.mapped, dsi, msr);
		else
			rms += calc_rms(si1, lock1.mapped, lock2.mapped);

		if (_pDifferences)
			_pDifferences->unmap(i);
	}

	return rms / static_cast<float>(n1);
}

	} // namespace surface
} // namespace ouro
