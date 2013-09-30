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
#include <oBase/image.h>
#include <oBase/finally.h>
#include <oBase/surface_convert.h>
#include <oBase/throw.h>
#include <lodepng/lodepng.h>

#define LODE_CALL(fn) do \
{ uint lodeerror__ = fn; \
	if (lodeerror__) throw std::exception(lodepng_error_text(lodeerror__)); \
} while(false)

namespace ouro {

class image_impl : public image
{
public:
	image_impl(const struct image::info& _Info);
	image_impl(std::shared_ptr<surface::buffer>& _Buffer);

	info get_info() const override;
	void copy_from(const void* _pSourceBuffer, size_t _SourceRowPitch, bool _FlipVertically = false) override;
	void copy_to(void* _pDestinationBuffer, size_t _DestinationRowPitch, bool _FlipVertically = false) const override;
	void convert(format _NewFormat) override;
	std::shared_ptr<surface::buffer> buffer() override { return b; }
	std::shared_ptr<const surface::buffer> buffer() const override { return b; }
	void put(const int2& _Coordinate, color _Color) override;
	color get(const int2& _Coordinate) const override;

private:
	std::shared_ptr<surface::buffer> b;

	surface::info surface_info(const struct image::info& _Info) const
	{
		surface::info si;
		si.format = surface::format(_Info.format);
		si.layout = surface::image;
		si.dimensions = int3(_Info.dimensions, 1);
		si.array_size = 1;
		return si;		
	}
};

image_impl::image_impl(const struct info& _Info)
	: b(surface::buffer::make(surface_info(_Info)))
{}

image_impl::image_impl(std::shared_ptr<surface::buffer>& _Buffer)
	: b(_Buffer)
{}

std::shared_ptr<image> image::make(const struct image::info& _Info)
{
	return std::make_shared<image_impl>(_Info);
}

image::info image_impl::get_info() const
{
	surface::info si = b->get_info();
	info i;
	i.format = format(si.format);
	i.row_pitch = surface::row_pitch(si);
	i.dimensions = si.dimensions.xy();
	return i;
}

void image_impl::copy_from(const void* _pSourceBuffer, size_t _SourceRowPitch, bool _FlipVertically)
{
	surface::const_mapped_subresource msr;
	msr.data = _pSourceBuffer;
	msr.row_pitch = static_cast<uint>(_SourceRowPitch);
	msr.depth_pitch = 0;
	b->update_subresource(0, msr, _FlipVertically);
}
	
void image_impl::copy_to(void* _pDestinationBuffer, size_t _DestinationRowPitch, bool _FlipVertically) const
{
	surface::mapped_subresource msr;
	msr.data = _pDestinationBuffer;
	msr.row_pitch = static_cast<uint>(_DestinationRowPitch);
	msr.depth_pitch = 0;
	b->copy_to(0, &msr, _FlipVertically);
}

static bool is_swizzle(image::format _SourceFormat, image::format _DestinationFormat)
{
	return ((_SourceFormat == image::rgb24 && _DestinationFormat == image::bgr24)
		|| (_SourceFormat == image::bgr24 && _DestinationFormat == image::rgb24)
		|| (_SourceFormat == image::rgba32 && _DestinationFormat == image::bgra32)
		|| (_SourceFormat == image::bgra32 && _DestinationFormat == image::rgba32));
}

void image_impl::convert(format _NewFormat)
{
	image::info ii = get_info();
	if (is_swizzle(ii.format, _NewFormat))
		b->swizzle(surface::format(_NewFormat));
	else
	{
		surface::info si = b->get_info();
		si.format = surface::format(_NewFormat);
		b = b->convert(si);
	}
}

void image_impl::put(const int2& _Coordinate, color _Color)
{
	const struct info i = info();
	const int elSize = surface::element_size(surface::format(i.format));
	surface::mapped_subresource msr;
	uchar* p = (uchar*)msr.data + (_Coordinate.y * i.row_pitch) + (_Coordinate.x * elSize);
	int rr,gg,bb,aa;
	_Color.decompose(&rr, &gg, &bb, &aa);
	uchar r = uchar(rr), g = uchar(gg), b = uchar(bb), a = uchar(aa);
	this->b->map(0, &msr);
	switch (i.format)
	{
		case rgba32: *p++ = r; *p++ = g; *p++ = b; *p++ = a; break;
		case rgb24: *p++ = r; *p++ = g; *p++ = b; break;
		case bgra32: *p++ = b; *p++ = g; *p++ = r; *p++ = a; break;
		case bgr24: *p++ = b; *p++ = g; *p++ = r; break;
		case r8: *p++ = r; break;
		default: break;
	}
	this->b->unmap(0);
}
	
color image_impl::get(const int2& _Coordinate) const
{
	const struct info i = info();
	const int elSize = surface::element_size(surface::format(i.format));
	surface::const_mapped_subresource msr;
	const uchar* p = (const uchar*)msr.data + (_Coordinate.y * i.row_pitch) + (_Coordinate.x * elSize);
	int r=0,g=0,b=0,a=0;
	this->b->map_const(0, &msr);
	switch (i.format)
	{
		case rgba32: r = *p++; g = *p++; b = *p++; a = *p++; break;
		case rgb24: r = *p++; g = *p++; b = *p++; break;
		case bgra32: b = *p++; g = *p++; r = *p++; a = *p++; break;
		case bgr24: b = *p++; g = *p++; r = *p++; break;
		case r8: r = *p++; break;
		default: break;
	}
	this->b->unmap_const(0);
	return color(r,g,b,a);
}

std::shared_ptr<image> decode(const void* _pBuffer, size_t _BufferSize, surface::alpha_option::value _Option)
{
	std::shared_ptr<surface::buffer> b = surface::decode(_pBuffer, _BufferSize, _Option);
	return std::move(std::make_shared<image_impl>(b));
}

} // namespace ouro
